/*
** i_soundfont.h
**
** The sound font manager for the MIDI synths
**
**---------------------------------------------------------------------------
**
** Copyright 2018 Christoph Oelckers
** Copyright 2018-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
*
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
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "zstring.h"
#include "tarray.h"
#include "filesystem.h"
#include "files.h"
#include "filereadermusicinterface.h"

struct FSoundFontInfo
{
    FString mName;        // This is what the sounfont is identified with. It's the extension-less base file name
	FString mNameExt;     // Same with extension. Used for comparing with input names so they can be done with or without extension.
	FString mFilename;    // Full path to the backing file - this is needed by FluidSynth to load the sound font.
    int type;
};

//==========================================================================
//
//
//
//==========================================================================

class FSoundFontReader
{
protected:
    // This is only doable for loose config files that get set as sound fonts. All other cases read from a contained environment where this does not apply.
    bool mAllowAbsolutePaths = false;
    // This has only meaning if being run on a platform with a case sensitive file system and loose files.
    // When reading from an archive it will always be case insensitive, just like the lump manager.
    bool mCaseSensitivePaths = false;
    TArray<FString> mPaths;


    int pathcmp(const char *p1, const char *p2);


public:

    virtual ~FSoundFontReader() {}
    virtual FileReader OpenMainConfigFile() = 0;    // this is special because it needs to be synthesized for .sf files and set some restrictions for patch sets
	virtual FString MainConfigFileName()
	{
		return basePath() + "timidity.cfg";
	}

    virtual FileReader OpenFile(const char *name) = 0;
    std::pair<FileReader , FString> LookupFile(const char *name);
    void AddPath(const char *str);
	virtual FString basePath() const
	{
		return "";	// archived patch sets do not use paths
	}

	virtual FileReader Open(const char* name, std::string &filename);
    virtual void close()
    {
        delete this;
    }

	ZMusicCustomReader* open_interface(const char* name);

};

//==========================================================================
//
//
//
//==========================================================================

class FSF2Reader : public FSoundFontReader
{
	FString mMainConfigForSF2;
	FString mFilename;
public:
    FSF2Reader(const char *filename);
	virtual FileReader OpenMainConfigFile() override;
    virtual FileReader OpenFile(const char *name) override;
};

//==========================================================================
//
//
//
//==========================================================================

class FZipPatReader : public FSoundFontReader
{
    FResourceFile *resf;
public:
    FZipPatReader(const char *filename);
    ~FZipPatReader();
	virtual FileReader OpenMainConfigFile() override;
	virtual FileReader OpenFile(const char *name) override;
	bool isOk() { return resf != nullptr; }
};

//==========================================================================
//
//
//
//==========================================================================

class FLumpPatchSetReader : public FSoundFontReader
{
	int mLumpIndex;;
	FString mBasePath;

public:
    FLumpPatchSetReader(const char *filename);
    virtual FileReader OpenMainConfigFile() override;
    virtual FileReader OpenFile(const char *name) override;
	virtual FString basePath() const override
	{
		return mBasePath;
	}

};

//==========================================================================
//
//
//
//==========================================================================

class FPatchSetReader : public FSoundFontReader
{
	FString mBasePath;
	FString mFullPathToConfig;

public:
	FPatchSetReader(FileReader &reader);
	FPatchSetReader(const char *filename);
	virtual FileReader OpenMainConfigFile() override;
	virtual FileReader OpenFile(const char *name) override;
	virtual FString basePath() const override
	{
		return mBasePath;
	}
};

//==========================================================================
//
//
//
//==========================================================================

class FSoundFontManager
{
    TArray<FSoundFontInfo> soundfonts;

    void ProcessOneFile(const char* fn);

public:
    void CollectSoundfonts();
    const FSoundFontInfo *FindSoundFont(const char *name, int allowedtypes) const;
    FSoundFontReader *OpenSoundFont(const char *name, int allowedtypes);
    const auto &GetList() const { return soundfonts; } // This is for the menu

};


extern FSoundFontManager sfmanager;
