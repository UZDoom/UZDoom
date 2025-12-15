/*
** The Video Options menu
**
**---------------------------------------------------------------------------
** Copyright 2001-2010 Randy Heit
** Copyright 2010-2017 Christoph Oelckers
** All rights reserved.
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
**---------------------------------------------------------------------------
**
*/

//=============================================================================
//
//
//
//=============================================================================

class VideoOptions : OptionMenu
{
	const MARGIN = 20;

	//=============================================================================
	//
	//
	//
	//=============================================================================

	override void Drawer()
	{
		DontDim = true;
		DontBlur = true;

		// Dim the background
		int x = 0;
		int y = 0;
		int h = mDesc.mPosition;
		int w = 0;
		if (h <= 0)
		{
			let font = menuDelegate.PickFont(mDesc.mFont);
			if (font && mDesc.mTitle.Length() > 0)
			{
				w = font.StringWidth(mDesc.mTitle) * CleanXfac_1;
				h = (-h + font.GetHeight()) * CleanYfac_1;
			}
			else
			{
				w = h = 0;
			}
		}

		int fw = screen.GetWidth();
		int fh = screen.GetHeight();

		ScreenArea box;
		GetTooltipArea(box);
		if (!DrawTooltips)
			box.y = fh;
		int lastrow = box.y - OptionHeight() * CleanYfac_1;
		int fontheight = OptionMenuSettings.mLinespacing * CleanYfac_1;

		int count = 0;
		int slider = -1;
		int sliderBound = 0;
		for (int i = 0; i < mDesc.mItems.Size() && h <= lastrow; i++)
		{
			if (i == mDesc.mScrollTop)
			{
				i += mDesc.mScrollPos;
				if (i >= mDesc.mItems.Size()) break;
				if (i < 0) i = 0;
			}

			if (!mDesc.mItems[i].Visible())
			{
				continue;
			}

			int t = Menu.OptionWidth(mDesc.mItems[i].mLabel) + mDesc.mItems[i].getIndent();

			if (mDesc.mItems[i] is "OptionMenuSliderBase")
			{
				int digits = OptionMenuSliderBase(mDesc.mItems[i]).mShowValue;

				slider = max(slider, digits);

				if (digits > 0)
				{
					float upper = OptionMenuSliderBase(mDesc.mItems[i]).mMax;
					float lower = OptionMenuSliderBase(mDesc.mItems[i]).mMin;

					String fmt = String.format("%%.%df", digits);
					String fmtLower = String.format(fmt, lower);
					String fmtUpper = String.format(fmt, upper);

					int widthLower = Menu.OptionWidth(fmtLower);
					int widthUpper = Menu.OptionWidth(fmtUpper);
					int width = max(widthLower, widthUpper);

					sliderBound = max(sliderBound, width);
				}
			}

			w = max(w, t*CleanXfac_1);
			h += fontheight;
		}

		// account for sliders popping out the side
		if (slider >= 0)
		{
			int extra = sliderBound + 2; // text + gap

			slider = (13*16) - w/(CleanXfac_1*2) + extra;
			slider *= CleanXfac_1;

			if (slider < 0) slider = 0;
		}
		if (slider < 0) slider = 0;

		w += fontheight + slider*2;
		h += fontheight;
		x = max(0, fw - w) / 2;
		y = 0;
		Screen.Dim(0u, 1, x, y, w, h);

		// positioning
		w = min(fw, max(fw*3/4, w));
		y = fh - h;
		h = fh - box.y;
		if (y-h > w) h = y - w;
		x = (fw - w)/2;
		w = x + w;

		PPShader.SetUniform1i("GammaTestPattern", "uXmin", x);
		PPShader.SetUniform1i("GammaTestPattern", "uXmax", w);
		PPShader.SetUniform1i("GammaTestPattern", "uYmin", h);
		PPShader.SetUniform1i("GammaTestPattern", "uYmax", y);
		PPShader.SetUniform1f("GammaTestPattern", "uInvGamma", 1.0/vid_gamma);
		PPShader.SetUniform1f("GammaTestPattern", "uWhitePoint", vid_whitepoint);
		PPShader.SetUniform1f("GammaTestPattern", "uBlackPoint", vid_blackpoint);
		PPShader.SetEnabled("GammaTestPattern", true);

		Super.Drawer();
	}

	//=============================================================================
	//
	//
	//
	//=============================================================================

	override bool MenuEvent(int mkey, bool fromcontroller)
	{
		if (mkey == MKEY_Back)
		{
			PPShader.SetEnabled("GammaTestPattern", false);
		}
		return Super.MenuEvent(mkey, fromcontroller);
	}
}
