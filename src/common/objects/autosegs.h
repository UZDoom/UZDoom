/*
** autosegs.h
**
** Arrays built at link-time
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2008-2016 Christoph Oelckers
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

#ifndef AUTOSEGS_H
#define AUTOSEGS_H

#include <type_traits>
#include <cstdint>

#if defined(__clang__)
#if defined(__has_feature) && __has_feature(address_sanitizer)
#define NO_SANITIZE __attribute__((no_sanitize("address")))
#else
#define NO_SANITIZE
#endif
#else
#define NO_SANITIZE
#endif

#if defined _MSC_VER
#define NO_SANITIZE_M __declspec(no_sanitize_address)
#else
#define NO_SANITIZE_M
#endif

#include <tarray.h>

template<typename T>
class FAutoSeg
{ // register things automatically without segment hackery

	template <typename T2>
	struct ArgumentType;

	template <typename Ret, typename Func, typename Arg>
	struct ArgumentType<Ret(Func:: *)(Arg) const>
	{
		using Type = Arg;
	};

	template <typename Func>
	using ArgumentTypeT = typename ArgumentType<Func>::Type;

	template <typename Func>
	struct ReturnType
	{
		using Type = std::invoke_result_t<Func, ArgumentTypeT<decltype(&Func::operator())>>;
	};

	template <typename Func>
	using ReturnTypeT = typename ReturnType<Func>::Type;

	template <typename Func, typename Ret>
	struct HasReturnType
	{
		static constexpr bool Value = std::is_same_v<ReturnTypeT<Func>, Ret>;
	};

	template <typename Func, typename Ret>
	static constexpr bool HasReturnTypeV = HasReturnType<Func, Ret>::Value;

public:
	TArray<T*> fields {TArray<T*>::NoInit}; // skip constructor for fields, globals are zero-initialized, so this is fine

	template <typename Func>
	void ForEach(Func func, std::enable_if_t<HasReturnTypeV<Func, void>> * = nullptr)
	{
		for (T * elem : fields)
		{
			func(elem);
		}
	}

	template <typename Func>
	void ForEach(Func func, std::enable_if_t<HasReturnTypeV<Func, bool>> * = nullptr)
	{
		for (T * elem : fields)
		{
			if (!func(elem))
			{
				return;
			}
		}
	}
};

template<typename T>
struct FAutoSegEntry
{
	FAutoSegEntry(FAutoSeg<T> &seg, T* value)
	{
		seg.fields.push_back(value);
	}
};

struct AFuncDesc;
struct FieldDesc;
struct ClassReg;
struct FPropertyInfo;
struct FMapOptInfo;
struct FCVarDecl;

namespace AutoSegs
{
	extern FAutoSeg<AFuncDesc> ActionFunctons;
	extern FAutoSeg<FieldDesc> ClassFields;
	extern FAutoSeg<ClassReg> TypeInfos;
	extern FAutoSeg<FPropertyInfo> Properties;
	extern FAutoSeg<FMapOptInfo> MapInfoOptions;
	extern FAutoSeg<FCVarDecl> CVarDecl;
}


#endif
