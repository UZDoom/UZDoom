/*
** hw_shaderpatcher.cpp
**
** Modifies shader source to account for different syntax versions or engine changes.
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2018 Christoph Oelckers
** Copyright 2016-2018 Magnus Norddahl
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


#include "cmdlib.h"
#include "hw_shaderpatcher.h"
#include "textures.h"
#include "hw_renderstate.h"
#include "v_video.h"
#include "printf.h"


static bool IsGlslWhitespace(char c)
{
	switch (c)
	{
	case ' ':
	case '\r':
	case '\n':
	case '\t':
	case '\f':
		return true;
	default:
		return false;
	}
}

static FString NextGlslToken(const char *chars, ptrdiff_t len, ptrdiff_t &pos)
{
	// Eat whitespace
	ptrdiff_t tokenStart = pos;
	while (tokenStart != len && IsGlslWhitespace(chars[tokenStart]))
		tokenStart++;

	// Find token end
	ptrdiff_t tokenEnd = tokenStart;
	while (tokenEnd != len && !IsGlslWhitespace(chars[tokenEnd]) && chars[tokenEnd] != ';')
		tokenEnd++;

	pos = tokenEnd;
	return FString(chars + tokenStart, tokenEnd - tokenStart);
}

static bool isShaderType(const char *name)
{
	return !strcmp(name, "sampler1D") || !strcmp(name, "sampler2D") || !strcmp(name, "sampler3D") || !strcmp(name, "samplerCube") || !strcmp(name, "sampler2DMS");
}

FString RemoveLegacyUserUniforms(FString code)
{
	// User shaders must declare their uniforms via the GLDEFS file.

	bool found = code.Substitute("uniform sampler2D tex;", "                      ");
	found = code.Substitute("uniform float timer;", "                    ") || found;

	// The following code searches for legacy uniform declarations in the shader itself and replaces them with whitespace.

	ptrdiff_t len = code.Len();
	char *chars = code.LockBuffer();

	ptrdiff_t startIndex = 0;
	while (true)
	{
		ptrdiff_t matchIndex = code.IndexOf("uniform", startIndex);
		if (matchIndex == -1)
			break;

		bool isLegacyUniformName = false;

		bool isKeywordStart = matchIndex == 0 || IsGlslWhitespace(chars[matchIndex - 1]);
		bool isKeywordEnd = matchIndex + 7 == len || IsGlslWhitespace(chars[matchIndex + 7]);
		if (isKeywordStart && isKeywordEnd)
		{
			ptrdiff_t pos = matchIndex + 7;
			FString type = NextGlslToken(chars, len, pos);
			FString identifier = NextGlslToken(chars, len, pos);

			isLegacyUniformName = type.Compare("float") == 0 && identifier.Compare("timer") == 0;
		}

		if (isLegacyUniformName)
		{
			ptrdiff_t statementEndIndex = code.IndexOf(';', matchIndex + 7);
			if (statementEndIndex == -1)
				statementEndIndex = len;
			for (ptrdiff_t i = matchIndex; i <= statementEndIndex; i++)
			{
				if (!IsGlslWhitespace(chars[i]))
					chars[i] = ' ';
			}
			startIndex = statementEndIndex;
			found = true;
		}
		else
		{
			startIndex = matchIndex + 7;
		}
	}

	if(found)
	{
		DPrintf(DMSG_WARNING, TEXTCOLOR_ORANGE "timer and tex uniforms should not be explicitly declared.\n");
	}

	bool foundtexture2d = false;

	// Also remove all occurences of the token 'texture2d'. Some shaders may still use this deprecated function to access a sampler.
	// Modern GLSL only allows use of 'texture'.
	while (true)
	{
		ptrdiff_t matchIndex = code.IndexOf("texture2d", startIndex);
		if (matchIndex == -1)
			break;

		foundtexture2d = true;

		// Check if this is a real token.
		bool isKeywordStart = matchIndex == 0 || !isalnum(chars[matchIndex - 1] & 255);
		bool isKeywordEnd = matchIndex + 9 == len || !isalnum(chars[matchIndex + 9] & 255);
		if (isKeywordStart && isKeywordEnd)
		{
			chars[matchIndex + 7] = chars[matchIndex + 8] = ' ';
		}
		startIndex = matchIndex + 9;
	}

	if(foundtexture2d)
	{
		DPrintf(DMSG_WARNING, TEXTCOLOR_ORANGE "texture2d is deprecated, use texture instead.\n");
	}

	code.UnlockBuffer();

	return code;
}

FString RemoveSamplerBindings(FString code, TArray<std::pair<FString, int>> &samplerstobind)
{
	ptrdiff_t len = code.Len();
	char *chars = code.LockBuffer();

	ptrdiff_t startIndex = 0;
	ptrdiff_t startpos, endpos = 0;
	while (true)
	{
		ptrdiff_t matchIndex = code.IndexOf("layout(binding", startIndex);
		if (matchIndex == -1)
			break;

		bool isSamplerUniformName = false;

		bool isKeywordStart = matchIndex == 0 || IsGlslWhitespace(chars[matchIndex - 1]);
		bool isKeywordEnd = matchIndex + 14 == len || IsGlslWhitespace(chars[matchIndex + 14]) || chars[matchIndex + 14] == '=';
		if (isKeywordStart && isKeywordEnd)
		{
			ptrdiff_t pos = matchIndex + 14;
			startpos = matchIndex;
			while (IsGlslWhitespace(chars[pos])) pos++;
			if (chars[pos] == '=')
			{
				char *p;
				pos++;
				auto val = strtol(&chars[pos], &p, 0);
				if (p != &chars[pos])
				{
					pos = (p - chars);
					while (IsGlslWhitespace(chars[pos])) pos++;
					if (chars[pos] == ')')
					{
						endpos = ++pos;
						FString uniform = NextGlslToken(chars, len, pos);
						FString type = NextGlslToken(chars, len, pos);
						FString identifier = NextGlslToken(chars, len, pos);

						isSamplerUniformName = uniform.Compare("uniform") == 0 && isShaderType(type.GetChars());
						if (isSamplerUniformName)
						{
							samplerstobind.Push(std::make_pair(identifier, val));
							for (auto posi = startpos; posi < endpos; posi++)
							{
								if (!IsGlslWhitespace(chars[posi]))
									chars[posi] = ' ';
							}
						}
					}
				}
			}
		}

		if (isSamplerUniformName)
		{
			startIndex = endpos;
		}
		else
		{
			startIndex = matchIndex + 7;
		}
	}

	code.UnlockBuffer();

	return code;
}

FString RemoveLayoutLocationDecl(FString code, const char *inoutkeyword)
{
	char *chars = code.LockBuffer();

	ptrdiff_t startIndex = 0;
	while (true)
	{
		ptrdiff_t matchIndex = code.IndexOf("layout(location", startIndex);
		if (matchIndex == -1)
			break;

		ptrdiff_t endIndex = matchIndex;

		// Find end of layout declaration
		while (chars[endIndex] != ')' && chars[endIndex] != 0)
			endIndex++;

		if (chars[endIndex] == ')')
			endIndex++;
		else if (chars[endIndex] == 0)
			break;

		// Skip whitespace
		while (IsGlslWhitespace(chars[endIndex]))
			endIndex++;

		// keyword following the declaration?
		bool keywordFound = true;
		ptrdiff_t i;
		for (i = 0; inoutkeyword[i] != 0; i++)
		{
			if (chars[endIndex + i] != inoutkeyword[i])
			{
				keywordFound = false;
				break;
			}
		}
		if (keywordFound && IsGlslWhitespace(chars[endIndex + i]))
		{
			// yes - replace declaration with spaces
			for (auto ii = matchIndex; ii < endIndex; ii++)
				chars[ii] = ' ';
		}

		startIndex = endIndex;
	}

	code.UnlockBuffer();

	return code;
}

/////////////////////////////////////////////////////////////////////////////

// Note: the MaterialShaderIndex enum in gl_shader.h needs to be updated whenever this array is modified.
const FDefaultShader defaultshaders[] =
{
	{"Default",	"shaders/glsl/func_normal.fp", "shaders/glsl/material_normal.fp", ""},
	{"Warp 1",	"shaders/glsl/func_warp.fp", "shaders/glsl/material_normal.fp", "#define SHADERTYPE_WARP1\n#define USE_GETTEXCOORD\n"},
	{"Warp 2",	"shaders/glsl/func_warp.fp", "shaders/glsl/material_normal.fp", "#define SHADERTYPE_WARP2\n#define USE_GETTEXCOORD\n"},
	{"Specular", "shaders/glsl/func_spec.fp", "shaders/glsl/material_specular.fp", "#define SPECULAR\n#define NORMALMAP\n"},
	{"PBR","shaders/glsl/func_pbr.fp", "shaders/glsl/material_pbr.fp", "#define PBR\n#define NORMALMAP\n"},
	{"Paletted",	"shaders/glsl/func_paletted.fp", "shaders/glsl/material_nolight.fp", "#define PALETTE_EMULATION\n"},
	{"No Texture", "shaders/glsl/func_notexture.fp", "shaders/glsl/material_normal.fp", "#define NO_LAYERS\n"},
	{"Basic Fuzz", "shaders/glsl/fuzz_standard.fp", "shaders/glsl/material_normal.fp", ""},
	{"Smooth Fuzz", "shaders/glsl/fuzz_smooth.fp", "shaders/glsl/material_normal.fp", ""},
	{"Swirly Fuzz", "shaders/glsl/fuzz_swirly.fp", "shaders/glsl/material_normal.fp", ""},
	{"Translucent Fuzz", "shaders/glsl/fuzz_smoothtranslucent.fp", "shaders/glsl/material_normal.fp", ""},
	{"Jagged Fuzz", "shaders/glsl/fuzz_jagged.fp", "shaders/glsl/material_normal.fp", ""},
	{"Noise Fuzz", "shaders/glsl/fuzz_noise.fp", "shaders/glsl/material_normal.fp", ""},
	{"Smooth Noise Fuzz", "shaders/glsl/fuzz_smoothnoise.fp", "shaders/glsl/material_normal.fp", ""},
	{"Software Fuzz", "shaders/glsl/fuzz_software.fp", "shaders/glsl/material_normal.fp", ""},
	{nullptr,nullptr,nullptr,nullptr}
};

const FEffectShader effectshaders[] =
{
	{ "fogboundary", "shaders/glsl/main.vp", "shaders/glsl/fogboundary.fp", nullptr, nullptr, "#define NO_ALPHATEST\n" },
	{ "spheremap", "shaders/glsl/main.vp", "shaders/glsl/main.fp", "shaders/glsl/func_normal.fp", "shaders/glsl/material_normal.fp", "#define SPHEREMAP\n#define NO_ALPHATEST\n" },
	{ "burn", "shaders/glsl/main.vp", "shaders/glsl/burn.fp", nullptr, nullptr, "#define SIMPLE\n#define NO_ALPHATEST\n" },
	{ "stencil", "shaders/glsl/main.vp", "shaders/glsl/stencil.fp", nullptr, nullptr, "#define SIMPLE\n#define NO_ALPHATEST\n" },
	{ "dithertrans", "shaders/glsl/main.vp", "shaders/glsl/main.fp", "shaders/glsl/func_normal.fp", "shaders/glsl/material_normal.fp", "#define NO_ALPHATEST\n#define DITHERTRANS\n" },
};

namespace ShaderInputsOutputs
{
	int ShaderProperties[ALLSHADER_COUNT]
	{
		0, //MATSHADER_Default
		0, //MATSHADER_Warp1
		0, //MATSHADER_Warp2
		0, //MATSHADER_Specular
		0, //MATSHADER_PBR
		0, //MATSHADER_Paletted
		0, //MATSHADER_NoTexture
		0, //MATSHADER_BasicFuzz
		0, //MATSHADER_SmoothFuzz
		0, //MATSHADER_SwirlyFuzz
		0, //MATSHADER_TranslucentFuzz
		0, //MATSHADER_JaggedFuzz
		0, //MATSHADER_NoiseFuzz
		0, //MATSHADER_SmoothNoiseFuzz
		0, //MATSHADER_SoftwareFuzz
		0, //EFFSHADER_FogBoundary
		0, //EFFSHADER_SphereMap
		ShaderProperty::Simple, //EFFSHADER_Burn
		ShaderProperty::Simple, //EFFSHADER_Stencil
		0, //EFFSHADER_Dithertrans
	};

	TArray<ShaderIOEntry> ShaderFields
	{
		//vertex shader inputs
		{ShaderPosition::VInput, {UniformType::Vec4, "", "aPosition"}, 0, 0},
		{ShaderPosition::VInput, {UniformType::Vec2, "", "aTexCoord"}, 0, 0},
		{ShaderPosition::VInput, {UniformType::Vec4, "", "aColor"}, 0, 0},
		{ShaderPosition::VInput, {UniformType::Vec4, "", "aVertex2"}, 0, Simple},
		{ShaderPosition::VInput, {UniformType::Vec4, "", "aNormal"}, 0, Simple},
		{ShaderPosition::VInput, {UniformType::Vec4, "", "aNormal2"}, 0, Simple},
		{ShaderPosition::VInput, {UniformType::Vec3, "", "aLightmap"}, 0, Simple},
		{ShaderPosition::VInput, {UniformType::Vec4, "", "aBoneWeight"}, 0, Simple},
		{ShaderPosition::VInput, {UniformType::UVec4, "", "aBoneSelector"}, 0, Simple},
		//vertex shader outputs/frag shader inputs
		{ShaderPosition::VOutput, {UniformType::Vec4, "", "vTexCoord"}, 0, 0},
		{ShaderPosition::VOutput, {UniformType::Vec4, "", "vColor"}, 0, 0},
		{ShaderPosition::VOutput, {UniformType::Vec4, "", "pixelpos"}, 0, Simple},
		{ShaderPosition::VOutput, {UniformType::Vec3, "", "glowdist"}, 0, Simple},
		{ShaderPosition::VOutput, {UniformType::Vec3, "", "gradientdist"}, 0, Simple},
		{ShaderPosition::VOutput, {UniformType::Vec4, "", "vWorldNormal"}, 0, Simple},
		{ShaderPosition::VOutput, {UniformType::Vec4, "", "vEyeNormal"}, 0, Simple},
		{ShaderPosition::VOutput, {UniformType::Vec4, "", "ClipDistanceA"}, 0, HasClipDistance},
		{ShaderPosition::VOutput, {UniformType::Vec4, "", "ClipDistanceB"}, 0, HasClipDistance},
		{ShaderPosition::VOutput, {UniformType::Vec3, "", "vLightmap"}, 0, Simple},
		//frag shader outputs
		{ShaderPosition::FOutput, {UniformType::Vec4, "", "FragColor"}, 0, 0},
		{ShaderPosition::FOutput, {UniformType::Vec4, "", "FragFog"}, GBufferPass, 0},
		{ShaderPosition::FOutput, {UniformType::Vec4, "", "FragNormal"}, GBufferPass, 0},
	};

	FString GenerateInputsOutputs(bool isVulkan, bool isFrag, int flags)
	{
		using namespace ShaderInputsOutputs;

		FString generated;

		int inputIndex = 0;
		int outputIndex = 0;
		for(int i = 0; i < ShaderFields.SSize(); i++)
		{
			const auto &entry = ShaderFields[i];
			bool isOut;

			if((entry.requiredProperties & flags) != entry.requiredProperties) continue;
			if((entry.forbiddenProperties & flags)) continue;

			if(isFrag)
			{
				if(entry.position == ShaderPosition::VInput) continue;
				isOut = (entry.position == ShaderPosition::FOutput);
			}
			else
			{
				if(entry.position == ShaderPosition::FOutput) continue;
				isOut = (entry.position == ShaderPosition::VOutput);
			}
			if(isVulkan || entry.position != ShaderPosition::VOutput) // varyings don't use location for opengl
			{
				int index = (isOut ? outputIndex : inputIndex)++;
				generated.AppendFormat("layout(location=%d) ",index);
			}
			generated<<(isOut ? "out" : "in")<<" "<<entry.field.Property<<" "<<GetTypeStr(entry.field.Type)<<" "<<entry.field.Name<<";\n";
		}

		return generated;
	}
}

int DFrameBuffer::GetShaderCount()
{
	int i;
	for (i = 0; defaultshaders[i].ShaderName != nullptr; i++);

	return MAX_PASS_TYPES * (countof(defaultshaders) - 1 + usershaders.Size() + MAX_EFFECTS + SHADER_NoTexture);
}
