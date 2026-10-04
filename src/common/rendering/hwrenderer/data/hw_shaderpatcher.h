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
#include <type_traits>
#include "shaderuniforms.h"

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
	FString GenerateInputsOutputs(bool isVulkan, bool isFrag, int flags, const TArray<VaryingFieldDesc> *varyings);

	inline FString GenerateInputsOutputs(bool isVulkan, bool isFrag, AllShaderIndex type, bool isGBuffer, bool hasClipDistance, const TArray<VaryingFieldDesc> *varyings)
	{
		return GenerateInputsOutputs(isVulkan, isFrag,
			ShaderProperties[static_cast<int>(type)] | (isGBuffer ? GBufferPass : 0) | (hasClipDistance ? HasClipDistance : 0), varyings);
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

	template<typename T>
	consteval size_t cpp_type_to_glsl_alignment()
	{ //TODO expand?
	  if constexpr(std::is_same_v<T, VSMatrix>)
	  {
		  return 16;
	  }
	  else if constexpr(std::is_same_v<T, DVector4>)
	  {
		  return 32;
	  }
	  else if constexpr(std::is_same_v<T, FVector4>)
	  {
		  return 16;
	  }
	  else if constexpr(std::is_same_v<T, DVector3>)
	  {
		  return 32;
	  }
	  else if constexpr(std::is_same_v<T, FVector3>)
	  {
		  return 16;
	  }
	  else if constexpr(std::is_same_v<T, DVector2>)
	  {
		  return 16;
	  }
	  else if constexpr(std::is_same_v<T, FVector2>)
	  {
		  return 8;
	  }
	  else if constexpr(std::is_same_v<T, double>)
	  {
		  return 8;
	  }
	  else if constexpr(std::is_same_v<T, float>)
	  {
		  return 4;
	  }
	  else if constexpr(std::is_same_v<T, int>)
	  {
		  return 4;
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
	consteval std::array<size_t, sizeof...(I)> get_field_alignments_impl(std::index_sequence<I...>)
	{
		return std::array<size_t, sizeof...(I)>{cpp_type_to_glsl_alignment<typename boost::pfr::tuple_element_t<I, T>>()...};
	}

	template<typename T, size_t N = boost::pfr::tuple_size_v<T>>
	consteval std::array<size_t, N> get_field_alignments()
	{
		return get_field_alignments_impl<T>(std::make_index_sequence<N>{});
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

	template<typename T, bool std430>
	consteval bool VerifyStructAlignment()
	{ //TODO support arrays and std430 when we switch to C++26
		auto field_sizes = get_field_sizes<T>();
		auto field_alignments = get_field_alignments<T>();
		auto field_offsets = get_field_offsets<T>();

		size_t expected_offset = 0;

		int n = boost::pfr::tuple_size_v<T>;
		for(int i = 0; i < n; i++)
		{
			size_t sz = field_sizes[i];
			size_t align = field_alignments[i];

			if((expected_offset % align) != 0)
			{
				expected_offset += (align - (expected_offset % align)); // do pad
			}

			if(field_offsets[i] != expected_offset)
			{
				return false;
			}

			expected_offset += sz;
		}

		return true;
	}

	template<typename T, bool std430 = false>
	FString GenerateStruct()
	{
		auto field_names = boost::pfr::names_as_array<T>();
		auto field_types = get_field_types<T>();

		static_assert(VerifyStructAlignment<T, std430>() == true, "struct does not conform with std140/std430 alignment");

		FString out = "{\n";

		int n = boost::pfr::tuple_size_v<T>;
		for(int i = 0; i < n; i++)
		{
			out << "    " << field_types[i] << " ";

			if(field_names[i][0] == 'm')
			{
				out << "u" << field_names[i].substr(1);
			}
			else
			{
				out << field_names[i];
			}

			out << ";\n";
		}
		out << "}";
		return out;
	}
}
