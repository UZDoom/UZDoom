/*
** weightedlist.h
**
** A weighted list template class
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#include <stdlib.h>
#include <stdint.h>

class FRandom;

template<class T>
class TWeightedList
{
	template<class U>
	struct Choice
	{
		Choice(uint16_t w, U v) : Next(NULL), Weight(w), RandomVal(0), Value(v) {}

		Choice<U> *Next;
		uint16_t Weight;
		uint8_t RandomVal;	// 0 (never) - 255 (always)
		T Value;
	};

	public:
		TWeightedList (FRandom &pr) : Choices (NULL), RandomClass (pr) {}
		~TWeightedList ()
		{
			Choice<T> *choice = Choices;
			while (choice != NULL)
			{
				Choice<T> *next = choice->Next;
				delete choice;
				choice = next;
			}
		}

		void AddEntry (T value, uint16_t weight);
		T PickEntry () const;
		void ReplaceValues (T oldval, T newval);

	private:
		Choice<T> *Choices;
		FRandom &RandomClass;

		void RecalcRandomVals ();

		TWeightedList &operator= (const TWeightedList &) { return *this; }
};

template<class T> 
void TWeightedList<T>::AddEntry (T value, uint16_t weight)
{
	if (weight == 0)
	{ // If the weight is 0, don't bother adding it,
	  // since it will never be chosen.
		return;
	}

	Choice<T> **insAfter = &Choices, *insBefore = Choices;
	Choice<T> *theNewOne;

	while (insBefore != NULL && insBefore->Weight < weight)
	{
		insAfter = &insBefore->Next;
		insBefore = insBefore->Next;
	}
	theNewOne = new Choice<T> (weight, value);
	*insAfter = theNewOne;
	theNewOne->Next = insBefore;
	RecalcRandomVals ();
}

template<class T>
T TWeightedList<T>::PickEntry () const
{
	uint8_t randomnum = RandomClass();
	Choice<T> *choice = Choices;

	while (choice != NULL && randomnum > choice->RandomVal)
	{
		choice = choice->Next;
	}
	return choice != NULL ? choice->Value : NULL;
}

template<class T>
void TWeightedList<T>::RecalcRandomVals ()
{
	// Redistribute the RandomVals so that they form the correct
	// distribution (as determined by the range of weights).

	int numChoices, weightSums;
	Choice<T> *choice;
	double randVal, weightDenom;

	if (Choices == NULL)
	{ // No choices, so nothing to do.
		return;
	}

	numChoices = 1;
	weightSums = 0;

	for (choice = Choices; choice->Next != NULL; choice = choice->Next)
	{
		++numChoices;
		weightSums += choice->Weight;
	}

	weightSums += choice->Weight;
	choice->RandomVal = 255;	// The last choice is always randomval 255

	randVal = 0.0;
	weightDenom = 1.0 / (double)weightSums;

	for (choice = Choices; choice->Next != NULL; choice = choice->Next)
	{
		randVal += (double)choice->Weight * weightDenom;
		choice->RandomVal = (uint8_t)(randVal * 255.0);
	}
}

// Replace all values that match oldval with newval
template<class T>
void TWeightedList<T>::ReplaceValues(T oldval, T newval)
{
	Choice<T> *choice;

	for (choice = Choices; choice != NULL; choice = choice->Next)
	{
		if (choice->Value == oldval)
		{
			choice->Value = newval;
		}
	}
}
