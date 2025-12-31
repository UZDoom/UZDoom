/*
** nodebuild_events.cpp
**
** A red-black tree for keeping track of segs that get touched by a splitter.
**
**---------------------------------------------------------------------------
**
** Copyright 2002-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: LicenseRef-ZDoom-Conditional
**
**---------------------------------------------------------------------------
**
*/

#include <string.h>
#include "doomtype.h"
#include "nodebuild.h"

FEventTree::FEventTree ()
: Root (&Nil), Spare (NULL)
{
	memset (&Nil, 0, sizeof(Nil));
}

FEventTree::~FEventTree ()
{
	FEvent *probe;

	DeleteAll ();
	probe = Spare;
	while (probe != NULL)
	{
		FEvent *next = probe->Left;
		delete probe;
		probe = next;
	}
}

void FEventTree::DeleteAll ()
{
	DeletionTraverser (Root);
	Root = &Nil;
}

void FEventTree::DeletionTraverser (FEvent *node)
{
	if (node != &Nil && node != NULL)
	{
		DeletionTraverser (node->Left);
		DeletionTraverser (node->Right);
		node->Left = Spare;
		Spare = node;
	}
}

FEvent *FEventTree::GetNewNode ()
{
	FEvent *node;

	if (Spare != NULL)
	{
		node = Spare;
		Spare = node->Left;
	}
	else
	{
		node = new FEvent;
	}
	return node;
}

void FEventTree::Insert (FEvent *z)
{
	FEvent *y = &Nil;
	FEvent *x = Root;

	while (x != &Nil)
	{
		y = x;
		if (z->Distance < x->Distance)
		{
			x = x->Left;
		}
		else
		{
			x = x->Right;
		}
	}
	z->Parent = y;
	if (y == &Nil)
	{
		Root = z;
	}
	else if (z->Distance < y->Distance)
	{
		y->Left = z;
	}
	else
	{
		y->Right = z;
	}
	z->Left = &Nil;
	z->Right = &Nil;
}

FEvent *FEventTree::Successor (FEvent *event) const
{
	if (event->Right != &Nil)
	{
		event = event->Right;
		while (event->Left != &Nil)
		{
			event = event->Left;
		}
		return event;
	}
	else
	{
		FEvent *y = event->Parent;
		while (y != &Nil && event == y->Right)
		{
			event = y;
			y = y->Parent;
		}
		return y;
	}
}

FEvent *FEventTree::Predecessor (FEvent *event) const
{
	if (event->Left != &Nil)
	{
		event = event->Left;
		while (event->Right != &Nil)
		{
			event = event->Right;
		}
		return event;
	}
	else
	{
		FEvent *y = event->Parent;
		while (y != &Nil && event == y->Left)
		{
			event = y;
			y = y->Parent;
		}
		return y;
	}
}

FEvent *FEventTree::FindEvent (double key) const
{
	FEvent *node = Root;

	while (node != &Nil)
	{
		if (node->Distance == key)
		{
			return node;
		}
		else if (node->Distance > key)
		{
			node = node->Left;
		}
		else
		{
			node = node->Right;
		}
	}
	return NULL;
}

FEvent *FEventTree::GetMinimum ()
{
	FEvent *node = Root;

	if (node == &Nil)
	{
		return NULL;
	}
	while (node->Left != &Nil)
	{
		node = node->Left;
	}
	return node;
}

void FEventTree::PrintTree (const FEvent *event) const
{
	// Use the CRT's sprintf so that it shares the same formatting as ZDBSP's output.
	char buff[100];
	if (event != &Nil)
	{
		PrintTree(event->Left);
		snprintf(buff, sizeof(buff), " Distance %g, vertex %d, seg %u\n",
			g_sqrt(event->Distance/4294967296.0), event->Info.Vertex, (unsigned)event->Info.FrontSeg);
		Printf(PRINT_LOG, "%s", buff);
		PrintTree(event->Right);
	}
}
