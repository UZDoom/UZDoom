/*
** tflags.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2015 Teemu Piippo
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

/*
 * TFlags
 *
 * A Qt-inspired type-safe flagset type.
 *
 * T is the enum type of individual flags,
 * TT is the underlying integer type used (defaults to uint32_t)
 */
template<typename T, typename TT = uint32_t>
class TFlags
{
	struct ZeroDummy {};

public:
	typedef TFlags<T, TT> Self;
	typedef T EnumType;
	typedef TT IntType;

	TFlags() = default;
	TFlags(const Self& other) = default;
	constexpr TFlags (T value) : Value (static_cast<TT> (value)) {}

	// This allows initializing the flagset with 0, as 0 implicitly converts into a null pointer.
	constexpr TFlags (ZeroDummy*) : Value (0) {}

	// Relation operators
	constexpr Self operator| (const Self& other) const { return Self::FromInt (Value | other.GetValue()); }
	constexpr Self operator& (const Self& other) const { return Self::FromInt (Value & other.GetValue()); }
	constexpr Self operator^ (const Self& other) const { return Self::FromInt (Value ^ other.GetValue()); }
	constexpr Self operator| (T value) const { return Self::FromInt (Value | value); }
	constexpr Self operator& (T value) const { return Self::FromInt (Value & value); }
	constexpr Self operator^ (T value) const { return Self::FromInt (Value ^ value); }
	constexpr Self operator~() const { return Self::FromInt (~Value); }

	// Assignment operators
	constexpr Self& operator= (const Self& other) = default;
	constexpr Self& operator|= (const Self& other) { Value |= other.GetValue(); return *this; }
	constexpr Self& operator&= (const Self& other) { Value &= other.GetValue(); return *this; }
	constexpr Self& operator^= (const Self& other) { Value ^= other.GetValue(); return *this; }
	constexpr Self& operator= (T value) { Value = value; return *this; }
	constexpr Self& operator|= (T value) { Value |= value; return *this; }
	constexpr Self& operator&= (T value) { Value &= value; return *this; }
	constexpr Self& operator^= (T value) { Value ^= value; return *this; }

	// Access the value of the flagset
	constexpr TT GetValue() const { return Value; }
	constexpr operator TT() const { return Value; }

	// Set the value of the flagset manually with an integer.
	// Please think twice before using this.
	static constexpr Self FromInt (TT value) { return Self (static_cast<T> (value)); }

private:
	template<typename X> constexpr Self operator| (X value) const { return Self::FromInt (Value | value); }
	template<typename X> constexpr Self operator& (X value) const { return Self::FromInt (Value & value); }
	template<typename X> constexpr Self operator^ (X value) const { return Self::FromInt (Value ^ value); }

public:	// to be removed.
	TT Value;
};

/*
 * Additional operators for TFlags types.
 */
#define DEFINE_TFLAGS_OPERATORS(T) \
constexpr inline T operator| (T::EnumType a, T::EnumType b) { return T::FromInt (T::IntType (a) | T::IntType (b)); } \
constexpr inline T operator& (T::EnumType a, T::EnumType b) { return T::FromInt (T::IntType (a) & T::IntType (b)); } \
constexpr inline T operator^ (T::EnumType a, T::EnumType b) { return T::FromInt (T::IntType (a) ^ T::IntType (b)); } \
constexpr inline T operator| (T::EnumType a, T b) { return T::FromInt (T::IntType (a) | T::IntType (b)); } \
constexpr inline T operator& (T::EnumType a, T b) { return T::FromInt (T::IntType (a) & T::IntType (b)); } \
constexpr inline T operator^ (T::EnumType a, T b) { return T::FromInt (T::IntType (a) ^ T::IntType (b)); } \
constexpr inline T operator~ (T::EnumType a) { return T::FromInt (~T::IntType (a)); }

