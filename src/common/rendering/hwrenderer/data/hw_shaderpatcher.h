/*
** hw_shaderpatcher.h
**
** Modifies shader source to account for different syntax versions or engine changes.
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2018 Christoph Oelckers
** Copyright 2016-2018 Magnus Norddahl
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "tarray.h"
#include "zstring.h"
#include "matrix.h"
#include <utility>
#include "shaderuniforms.h"
#include "engineerrors.h"

#include <boost/pfr.hpp>
#include <qlibs/reflect>

FString RemoveLegacyUserUniforms(FString code);
FString RemoveSamplerBindings(FString code, TArray<std::pair<FString, int>> &samplerstobind);	// For GL 3.3 compatibility which cannot declare sampler bindings in the sampler source.
FString RemoveLayoutLocationDecl(FString code, const char *inoutkeyword);

struct FDefaultShader
{
	const char * ShaderName;
	const char * gettexelfunc;
	const char * lightfunc;
	const char * Defines;
};

struct FEffectShader
{
	const char *ShaderName;
	const char *vp;
	const char *fp1;
	const char *fp2;
	const char *fp3;
	const char *defines;
};

extern const FDefaultShader defaultshaders[];
extern const FEffectShader effectshaders[];

namespace ShaderInputsOutputs
{
	enum ShaderProperty
	{
		GBufferPass = 1,
		EffectShader = 2,
		HasClipDistance = 4,
		Simple = 8,
	};

	enum class ShaderPosition
	{
		VInput, // vertex input
		VOutput, // vertex output/frag input
		FOutput, // frag output
	};

	struct ShaderIOEntry
	{
		ShaderPosition position;
		VaryingFieldDesc field;
		int requiredProperties;
		int forbiddenProperties;
	};

	extern int ShaderProperties[ALLSHADER_COUNT];
	extern TArray<ShaderIOEntry> ShaderFields;

	//varying list must match between the frag and vertex shader of the same program
	FString GenerateInputsOutputs(bool isVulkan, bool isFrag, int flags);

	inline FString GenerateInputsOutputs(bool isVulkan, bool isFrag, AllShaderIndex type, bool isGBuffer, bool hasClipDistance)
	{
		return GenerateInputsOutputs(isVulkan, isFrag,
			ShaderProperties[static_cast<int>(type)] | (isGBuffer ? GBufferPass : 0) | (hasClipDistance ? HasClipDistance : 0));
	}

	template<typename T>
	consteval const char * cpp_type_to_glsl_type()
	{ //TODO expand?
		if constexpr(std::is_same_v<T, VSMatrix>)
		{
			return "mat4";
		}
		else if constexpr(std::is_same_v<T, DVector4>)
		{
			return "dvec4";
		}
		else if constexpr(std::is_same_v<T, FVector4>)
		{
			return "vec4";
		}
		else if constexpr(std::is_same_v<T, DVector3>)
		{
			return "dvec3";
		}
		else if constexpr(std::is_same_v<T, FVector3>)
		{
			return "vec3";
		}
		else if constexpr(std::is_same_v<T, DVector2>)
		{
			return "dvec2";
		}
		else if constexpr(std::is_same_v<T, FVector2>)
		{
			return "vec2";
		}
		else if constexpr(std::is_same_v<T, double>)
		{
			return "double";
		}
		else if constexpr(std::is_same_v<T, float>)
		{
			return "float";
		}
		else if constexpr(std::is_same_v<T, int>)
		{
			return "int";
		}
		else
		{
			static_assert(std::is_same_v<T, void> && std::is_same_v<T, int>, "unknown type");
		}
	}

	template<typename T, size_t... I>
	consteval std::array<const char *, sizeof...(I)> get_field_types_impl(std::index_sequence<I...>)
	{
		return std::array<const char *, sizeof...(I)>{cpp_type_to_glsl_type<typename boost::pfr::tuple_element_t<I, T>>()...};
	}

	template<typename T, size_t N = boost::pfr::tuple_size_v<T>>
	consteval std::array<const char *, N> get_field_types()
	{
		return get_field_types_impl<T>(std::make_index_sequence<N>{});
	}

	template<typename T, size_t... I>
	consteval std::array<size_t, sizeof...(I)> get_field_sizes_impl(std::index_sequence<I...>)
	{
		return std::array<size_t, sizeof...(I)>{sizeof(typename boost::pfr::tuple_element_t<I, T>)...};
	}

	template<typename T, size_t N = boost::pfr::tuple_size_v<T>>
	consteval std::array<size_t, N> get_field_sizes()
	{
		return get_field_sizes_impl<T>(std::make_index_sequence<N>{});
	}

	template<typename T, size_t... I>
	consteval std::array<size_t, sizeof...(I)> get_field_offsets_impl(std::index_sequence<I...>)
	{
		return std::array<size_t, sizeof...(I)>{(reflect::offset_of<I, T>())...};
	}

	template<typename T, size_t N = boost::pfr::tuple_size_v<T>>
	consteval std::array<size_t, N> get_field_offsets()
	{
		return get_field_offsets_impl<T>(std::make_index_sequence<N>{});
	}

	template<typename T>
	FString GenerateStruct()
	{
		auto field_names = boost::pfr::names_as_array<T>();
		auto field_types = get_field_types<T>();
		auto field_sizes = get_field_sizes<T>();
		auto field_offsets = get_field_offsets<T>();

		FString out = "{\n";

		size_t expected_offset = 0;

		int n = boost::pfr::tuple_size_v<T>;
		for(int i = 0; i < n; i++)
		{
			size_t sz = field_sizes[i];
			size_t align = sz;

			if(sz == 12) // vec3, ivec3
			{
				align = 16; // can only align in multiples of 1, 2 and 4 elements
			}

			if(sz == 24) //dvec3
			{
				align = 32; // can only align in multiples of 1, 2 and 4 elements
			}

			if((expected_offset % align) != 0)
			{
				expected_offset += (align - (expected_offset % align)); // do pad
			}

			if(field_offsets[i] != expected_offset)
			{
				I_Error("bad struct");
			}

			expected_offset += sz;

			if(field_names[i][0] == 'm')
			{
				out << "    " << field_types[i] << " u" << field_names[i].substr(1) << ";\n";
			}
			else
			{
				out << "    " << field_types[i] << " " << field_names[i] << ";\n";
			}
		}
		out << "}";
		return out;
	}
}
