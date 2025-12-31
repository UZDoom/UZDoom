/*
** scoreboard.zs
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
*/

extend class BaseStatusBar
{
	const MAX_SCOREBOARD_ROWS = 10;
	const MAX_TEAM_SCORE_ROWS = 2;
	const SCOREBOARD_PADDING = 5;

	native Font ScoreboardFont;
	native Font BigScoreboardFont;

	native static bool IsScoreboardOpen();

    static clearscope int ComparePlayerPoints(int playerA, int playerB)
    {
	    // Compare first by frags, then by name.
	    PlayerInfo p1 = Players[playerA];
	    PlayerInfo p2 = Players[playerB];

		int diff;
		if (deathmatch)
		{
			// Compare first by teams if teamplay.
			if (teamplay)
				diff = p2.GetTeam() - p1.GetTeam();
			if (!diff)
				diff = p1.FragCount - p2.FragCount;
		}
		else
		{
			diff = p1.KillCount - p2.KillCount;
		}

		if (!diff)
			diff = p2.GetUserName().CompareNoCase(p1.GetUserName());
		if (!diff)
			diff = playerB - playerA;

	    return diff;
    }

    void SortScoreboardPlayers(out Array<int> sorted, Function<clearscope int(int, int)> compareFunc)
    {
		sorted.Clear();

		int pNum = -1;
		while ((pNum = PlayerInfo.GetNextPlayerNumber(pNum)) != -1)
		{
			int i;
			for (; i < sorted.Size(); ++i)
			{
				if(compareFunc.Call(pNum, sorted[i]) > 0)
				{
					sorted.Insert(i, pNum);
					break;
				}
			}

			if (i >= sorted.Size())
				sorted.Push(pNum);
		}
    }

	void DrawScoreboardText(Font fnt, int col, double x, double y, String text, double xOfs = 0.0, double yOfs = 0.0)
	{
		if (xOfs)
			x += int(fnt.StringWidth(text) * CleanXFac_1 * xOfs);
		if (yOfs)
			y += int(fnt.GetHeight() * CleanYFac_1 * yOfs);
		Screen.DrawText(fnt, col, x, y, text, DTA_CleanNoMove_1, true);
	}

	void DrawScoreboardImage(TextureID tex, double x, double y)
	{
		Screen.DrawTexture(tex, true, x, y, DTA_CenterOffset, true, DTA_CleanNoMove_1, true);
	}

	int GetScoreboardTextColor(PlayerInfo player)
	{
		if (deathmatch)
			return player.Mo.PlayerNumber() == ConsolePlayer ? sb_deathmatch_yourplayercolor : sb_deathmatch_otherplayercolor;

		return player.Mo.PlayerNumber() == ConsolePlayer ? sb_cooperative_yourplayercolor : sb_cooperative_otherplayercolor;
	}

	Color GetScoreboardPlayerColor(PlayerInfo player)
	{
		if (deathmatch && teamplay && Team.IsValid(player.GetTeam()))
			return Teams[player.GetTeam()].GetPlayerColor();

		return player.GetDisplayColor();
	}

	int, int GetScoreboardIconDimensions()
	{
		int maxWidth, maxHeight;
		PlayerInfo player;
		while ((player = PlayerInfo.GetNextPlayer(player)))
		{
			TextureID icon = player.Mo.ScoreIcon;
			if (!icon.IsValid())
				continue;
			
			let [width, height] = TexMan.GetSize(icon);
			if (width > maxWidth)
				maxWidth = width;
			if (height > maxHeight)
				maxHeight = height;
		}

		return maxWidth, maxHeight;
	}

	version("4.15.1") virtual void InitScoreboard()
	{
		ScoreboardFont = NewSmallFont;
		BigScoreboardFont = BigFont;
	}

    version("4.15.1") virtual bool DrawScoreboard(double ticFrac)
    {
		if (!ScoreboardFont || GameState != GS_LEVEL || !CPlayer
			|| (CPlayer.PlayerState != PST_DEAD && !IsScoreboardOpen()))
		{
			return false;
		}

	    if (deathmatch)
	    {
		    if (teamplay)
		    {
			    if(!sb_teamdeathmatch_enable)
				    return false;
		    }
		    else if (!sb_deathmatch_enable)
		    {
			    return false;
		    }
	    }
	    else if (!multiplayer || !sb_cooperative_enable)
	    {
		    return false;
	    }

		DrawRemainingTime(ticFrac);
        
        Array<int> sortedPlayers;
		SortScoreboardPlayers(sortedPlayers, ComparePlayerPoints);
		DrawPlayerScores(sortedPlayers, ticFrac);
		return true;
    }

	version("4.15.1") virtual void DrawRemainingTime(double ticFrac)
	{
		if (!deathmatch || timelimit <= 0.0 || GameState != GS_LEVEL)
			return;

		int timeLeft = Max(int(timelimit * 60 * GameTicRate) - Level.MapTime, 0);
		int hours = timeLeft / (GameTicRate * 3600);
		timeLeft -= hours * GameTicRate * 3600;
		int minutes = timeLeft / (GameTicRate * 60);
		timeleft -= minutes * GameTicRate * 60;
		int seconds = timeLeft / GameTicRate;
		
		String timer;
		if (timelimit >= 60.0)
			timer = String.Format("%2d:%02d:%02d", hours, minutes, seconds);
		else
			timer = String.Format("%2d:%02d", minutes, seconds);

		DrawScoreboardText(ScoreboardFont, Font.CR_WHITE, Screen.GetWidth() / 2, GetTopOfStatusBar() - 5 * CleanYFac_1, timer, -0.5, -1.0);
	}
	
	version("4.15.1") virtual void DrawPlayerScores(Array<int> sortedPlayers, double ticFrac)
	{
		int col = sb_cooperative_headingcolor;
		if (deathmatch)
		{
			if (teamplay)
				col = sb_teamdeathmatch_headingcolor;
			else
				col = sb_deathmatch_headingcolor;
		}

		int xPadding = SCOREBOARD_PADDING * CleanXFac_1;
		int yPadding = SCOREBOARD_PADDING * CleanYFac_1;
		let [iconWidth, iconHeight] = GetScoreboardIconDimensions();
		iconWidth *= CleanXFac_1;
		iconHeight *= CleanYFac_1;
		// Lock the scoreboard to 4:3 to make it more readable on widescreens.
		int scoreboardWidth = int(Screen.GetHeight() * (4.0 / 3.0)) - 150 * CleanXFac_1;
		int rowHeight = Max(iconHeight, ScoreboardFont.GetHeight() * CleanYFac_1) + yPadding * 2;
		int rowCenter = rowHeight / 2;

		String nameHeader = StringTable.Localize("$SCORE_NAME");

		String scoreHeader = StringTable.Localize(deathmatch ? "$SCORE_FRAGS" : "$SCORE_KILLS");
		int scoreOfs = int(scoreboardWidth * 0.6);

		String latencyHeader = StringTable.Localize("$SCORE_DELAY");
		int latencyOfs = int(scoreboardWidth * 0.8);

		// Start drawing.
		int x = (Screen.GetWidth() - scoreboardWidth) / 2;
		int y = (Screen.GetHeight() - rowHeight * MAX_SCOREBOARD_ROWS) / 2;

		Color borderCol = Color(144, 144, 144);
		DrawScoreboardText(ScoreboardFont, col, x + scoreOfs / 2, y, nameHeader, -0.5, -1.0);
		DrawScoreboardText(ScoreboardFont, col, x + scoreOfs + (latencyOfs - scoreOfs) / 2, y, scoreHeader, -0.5, -1.0);
		DrawScoreboardText(ScoreboardFont, col, x + latencyOfs + (scoreboardWidth - latencyOfs) / 2, y, latencyHeader, -0.5, -1.0);
		Screen.DrawThickLine(x, y, x + scoreboardWidth, y, CleanXFac_1, borderCol);

		int top = y - ScoreboardFont.GetHeight() * CleanYFac_1;
		int bottom = y + rowHeight * Min(sortedPlayers.Size(), MAX_SCOREBOARD_ROWS);
		Screen.DrawThickLine(x + scoreOfs, top, x + scoreOfs, bottom, CleanYFac_1, borderCol);
		Screen.DrawThickLine(x + latencyOfs, top, x + latencyOfs, bottom, CleanYFac_1, borderCol);

		int curRow = 1;
		int colBoxSize = 6 * CleanXFac_1;
		y += rowCenter;
		bool darkBackdrop;
		bool isTeamplay = deathmatch && teamplay;
		Map<int, int> teamScores;
		// Only check this if the player actually exists in the list.
		bool drewSelf = sortedPlayers.Find(ConsolePlayer) >= sortedPlayers.Size();
		foreach (pNum : sortedPlayers)
		{
			PlayerInfo player = Players[pNum];
			if (pNum == ConsolePlayer)
				drewSelf = true;

			int pTeam = player.GetTeam();
			if (isTeamplay && Team.IsValid(pTeam))
				teamScores.Insert(pTeam, teamScores.Get(pTeam) + player.FragCount);

			if (!drewSelf && curRow >= MAX_SCOREBOARD_ROWS)
				continue;
			if (curRow > MAX_SCOREBOARD_ROWS)
				continue;

			Screen.Dim(darkBackdrop ? Color(64, 64, 64) : Color(128, 128, 128), 0.2, x, y - rowCenter, scoreboardWidth, rowHeight);
			darkBackdrop = !darkBackdrop;

			if (pNum == ConsolePlayer)
				Screen.DrawLineFrame(~0u, x, y - rowCenter, scoreboardWidth, rowHeight, CleanXFac_1);

			Screen.Dim(GetScoreboardPlayerColor(player), 1.0, x + xPadding, y - colBoxSize / 2, colBoxSize, colBoxSize);

			// Split the name and player number drawing so the number can remain untranslated.
			String text = String.Format("%2d ", pNum);
			DrawScoreboardText(ScoreboardFont, Font.CR_WHITE, x + xPadding * 2 + colBoxSize, y, text, yOfs: -0.5);
			int numWidth = ScoreboardFont.StringWidth(text) * CleanXFac_1;
			text = player.GetUserName(32u);
			DrawScoreboardText(ScoreboardFont, GetScoreboardTextColor(player), x + xPadding * 2 + colBoxSize + numWidth, y, text, yOfs: -0.5);
			if (player.Mo.ScoreIcon.IsValid())
				DrawScoreboardImage(player.Mo.ScoreIcon, x - xPadding - iconWidth / 2, y);

			text = String.Format("%d", deathmatch ? player.FragCount : player.KillCount);
			DrawScoreboardText(ScoreboardFont, Font.CR_WHITE, x + latencyOfs - xPadding, y, text, -1.0, -0.5);

			text = String.Format("%dms", player.GetAverageLatency());
			DrawScoreboardText(ScoreboardFont, Font.CR_WHITE, x + scoreboardWidth - xPadding, y, text, -1.0, -0.5);

			y += rowHeight;
			++curRow;
		}

		if (isTeamplay && BigScoreboardFont)
		{
			int scalar = 2;
			int columnWidth = (BigScoreboardFont.StringWidth("0000") * CleanXFac_1 + xPadding * 2) * scalar;
			int largeRowHeight = (BigScoreboardFont.GetHeight() * CleanYFac_1 + yPadding * 2) * scalar;
			uint maxColumns = Max(scoreboardWidth / columnWidth, 1);
			xPadding *= scalar;
			yPadding *= scalar;

			int baseOfs = (scoreboardWidth - columnWidth * maxColumns) / 2;
			int xOfs;
			if (teamScores.CountUsed() < maxColumns)
				xOfs = columnWidth * (maxColumns - teamScores.CountUsed()) / 2;

			y = top - largeRowHeight * Min(int(Ceil(double(teamScores.CountUsed()) / maxColumns)), MAX_TEAM_SCORE_ROWS);
			curRow = 1;
			uint column;
			int countedTeams;
			drewSelf = !teamScores.CheckKey(Players[ConsolePlayer].GetTeam());
			foreach (t, score : teamScores)
			{
				++countedTeams;
				if (t == Players[ConsolePlayer].GetTeam())
					drewSelf = true;

				if (!drewSelf && curRow >= MAX_TEAM_SCORE_ROWS && column + 1 >= maxColumns)
					continue;

				int xPos = x + baseOfs + xOfs + columnWidth * column;
				
				TextureID icon = Teams[t].GetLogo();
				if (icon.IsValid())
				{
					// Try and fill the score vertically, otherwise filling it horizontally
					// if it bleeds out the edges. The icon is allowed to go slightly into
					// the margins to make it always somewhat visible behind the numbers.
					let [w, h] = TexMan.GetSize(icon);
					double iconScalar = double(largeRowHeight - yPadding) / h;
					if (w * iconScalar > columnWidth - xPadding)
						iconScalar = double(columnWidth - xPadding) / w;
					Screen.DrawTexture(icon, true,
										xPos + columnWidth / 2, y + largeRowHeight / 2,
										DTA_CenterOffset, true, DTA_Alpha, 0.5,
										DTA_ScaleX, iconScalar, DTA_ScaleY, iconScalar);
				}

				String text = String.Format("%4d", score);
				Screen.DrawText(BigScoreboardFont, Teams[t].GetTextColor(),
								xPos + xPadding, y + yPadding, text,
								DTA_ScaleX, CleanXFac_1 * scalar, DTA_ScaleY, CleanYFac_1 * scalar);

				if (++column >= maxColumns)
				{
					y += largeRowHeight;
					column = 0u;
					if (++curRow > MAX_TEAM_SCORE_ROWS)
						break;

					uint remaining = teamScores.CountUsed() - countedTeams;
					if (remaining < maxColumns)
						xOfs = columnWidth * (maxColumns - remaining) / 2;
				}
			}
		}
	}
}
