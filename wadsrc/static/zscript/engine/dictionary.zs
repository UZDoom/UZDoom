/*
** dictionary.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
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

/**
 * Dictionary provides key-value storage.
 *
 * Both keys and values are strings.
 *
 * @note keys are case-sensitive.
 */
class Dictionary deprecated("4.15.1", "Use Map<String, String> instead")
{
	native static Dictionary Create();

	native void Insert(String key, String value);
	native void Remove(String key);

	/**
	 * Returns the value for the specified key.
	 */
	native String At(String key) const;

	/**
	 * Deserializes a dictionary from a string.
	 *
	 * @param s serialized string, must be either empty or returned from ToString().
	 */
	native static Dictionary FromString(String s);

	/**
	 * Serializes a dictionary to a string.
	 */
	native String ToString() const;
}

/**
 * Provides iterating over a Dictionary.
 *
 * Order is not specified.
 *
 * DictionaryIterator is not serializable. To make DictionaryIterator a class
 * member, use `transient` keyword.
 */
class DictionaryIterator deprecated("4.15.1", "Use Map<String, String> instead")
{
	native static DictionaryIterator Create(Dictionary dict);

	/**
	 * Returns false if there are no more entries in the dictionary.
	 * Otherwise, returns true.
	 *
	 * While it returns true, get key and value for the current entry
	 * with Key() and Value() functions.
	 */
	native bool Next();

	/**
	 * Returns the key for the current dictionary entry.
	 * Do not call this function before calling Next().
	 */
	native String Key() const;

	/**
	 * Returns the value for the current dictionary entry.
	 * Do not call this function before calling Next().
	 */
	native String Value() const;
}
