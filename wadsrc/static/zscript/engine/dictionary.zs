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
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
