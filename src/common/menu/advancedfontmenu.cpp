#include "menu.h"
#include "namedef.h"
#include "v_font.h"
#include <vector>
#include "vm.h"
#include "dobject.h"
#include "c_cvars.h"
#include "gstrings.h"
#include "c_dispatch.h"

FString fontBeingRemapped;

CCMD(editfontmapping)
{
	if (argv.argc() > 0)
	{
		fontBeingRemapped = FString(argv[1]);
		M_StartControlPanel(true);
		M_SetMenu("AdvancedFontOptionsRemapChoiceMenu", -1);
	}
}

DEFINE_ACTION_FUNCTION(DAdvancedFontMenu, FillAdvancedFontMenu)
{
	PARAM_PROLOGUE;
	PARAM_OBJECT(desc, DOptionMenuDescriptor);
	desc->mItems.Clear();
	if (!desc)
	{
		return 0;
	}

	if (desc->IsKindOf(RUNTIME_CLASS(DOptionMenuDescriptor)))
	{
		FFont::MakeFontChoiceCVARs();
		auto &fonts = FFont::GetRemappableFonts();
		for (auto f : fonts)
		{
			const FString              fontNameString = f->GetName().GetChars();
			const FString              cmd            = "editfontmapping " + fontNameString;
			DMenuItemBase *const opt          = CreateOptionMenuItemCommand(f->GetName().GetChars(), cmd);

			desc->mItems.Push(opt);
		}
	}
	return 0;
}

DEFINE_ACTION_FUNCTION(DAdvancedFontMenuRemapChoiceMenu, FillAdvancedFontMenuRemapChoices)
{
	PARAM_PROLOGUE;
	PARAM_OBJECT(desc, DOptionMenuDescriptor);
	desc->mItems.Clear();
	if (!desc)
	{
		return 0;
	}

	if (desc->IsKindOf(RUNTIME_CLASS(DOptionMenuDescriptor)))
	{
		FFont::MakeFontChoiceCVARs();
		const FString fontNameBeingRemapped = fontBeingRemapped;
		const FString langTriplet           = GStrings.GetActiveLangID().name.GetChars();

		std::vector<const FFont*> dynamicFonts;
		for (const FFont *currentFont = FFont::GetFontListHead(); currentFont; currentFont = currentFont->GetNextFont())
		{
			if (currentFont->IsValidDynamicFont())
			{
				dynamicFonts.push_back(currentFont);
			}
		}

		//add a default option
		{
			FString              srcFontName = fontNameBeingRemapped;
			DMenuItemBase *const opt         = CreateOptionMenuItemCommand(
                "DEFAULT", FStringf("fontchoice_%s_%s FO-DEFAULT", srcFontName.GetChars(), langTriplet.GetChars()));
			desc->mItems.Push(opt);
		}
		
		for (auto &f : dynamicFonts)
		{
			const FString srcFontName = fontNameBeingRemapped;
			const FString              tgtFontName = f->GetName().GetChars();
			
			const FString cmd =
				FStringf("fontchoice_%s_%s %s", srcFontName.GetChars(), langTriplet.GetChars(), tgtFontName.GetChars());
			DMenuItemBase *const opt          = CreateOptionMenuItemCommand(f->GetName().GetChars(), cmd);
			desc->mItems.Push(opt);
		}
	}
	return 0;
}
