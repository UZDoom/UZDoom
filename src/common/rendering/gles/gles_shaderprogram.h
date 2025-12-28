/*
** gles_shaderprogram.h
**
** Postprocessing framework
**
**---------------------------------------------------------------------------
**
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
** Copyright 2016-2020 Magnus Norddahl
**
** This software is provided 'as-is', without any express or implied
** warranty.  In no event will the authors be held liable for any damages
** arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose,
** including commercial applications, and to alter it and redistribute it
** freely, subject to the following restrictions:
**
** 1. The origin of this software must not be misrepresented; you must not
**    claim that you wrote the original software. If you use this software
**    in a product, an acknowledgment in the product documentation would be
**    appreciated but is not required.
** 2. Altered source versions must be plainly marked as such, and must not be
**    misrepresented as being the original software.
** 3. This notice may not be removed or altered from any source distribution.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "gles_system.h"
#include "gles_shader.h"
#include "hwrenderer/postprocessing/hw_postprocess.h"

namespace OpenGLESRenderer
{

class FShaderProgram : public PPShaderBackend
{
public:
	FShaderProgram();
	~FShaderProgram();

	enum ShaderType
	{
		Vertex,
		Fragment,
		NumShaderTypes
	};

	void Compile(ShaderType type, const char *lumpName, const char *defines, int maxGlslVersion);
	void Compile(ShaderType type, const char *name, const FString &code, const char *defines, int maxGlslVersion);
	void Link(const char *name);
	void SetUniformBufferLocation(int index, const char *name);

	void Bind();

	GLuint Handle() { return mProgram; }
	//explicit operator bool() const { return mProgram != 0; }

	std::unique_ptr<IDataBuffer> Uniforms;
	GLuint mProgram = 0;
private:
	FShaderProgram(const FShaderProgram &) = delete;
	FShaderProgram &operator=(const FShaderProgram &) = delete;

	void CompileShader(ShaderType type);
	FString PatchShader(ShaderType type, const FString &code, const char *defines, int maxGlslVersion);

	void CreateShader(ShaderType type);
	FString GetShaderInfoLog(GLuint handle);
	FString GetProgramInfoLog(GLuint handle);


	GLuint mShaders[NumShaderTypes];
	FString mShaderSources[NumShaderTypes];
	FString mShaderNames[NumShaderTypes];
	TArray<std::pair<FString, int>> samplerstobind;
};

template<typename T>
class ShaderUniformsGles
{
public:
	ShaderUniformsGles()
	{
		memset(&Values, 0, sizeof(Values));
	}

	~ShaderUniformsGles()
	{
		if (mBuffer != nullptr)
			delete mBuffer;
	}

	FString CreateDeclaration(const char* name, const std::vector<UniformFieldDesc>& fields)
	{
		mFields = fields;
		FString decl;
		decl += "\n";
		for (size_t i = 0; i < fields.size(); i++)
		{
			decl.AppendFormat("\tuniform %s %s;\n", GetTypeStr(fields[i].Type), fields[i].Name);
		}
		decl += "\n";
		return decl;
	}

	void Init()
	{
		if (mBuffer == nullptr)
			mBuffer = screen->CreateDataBuffer(-1, false, false);
	}

	void SetData()
	{
		if (mBuffer != nullptr)
			mBuffer->SetData(sizeof(T), &Values, BufferUsageType::Static);
	}

	IDataBuffer* GetBuffer() const
	{
		// OpenGL needs to mess around with this in ways that should not be part of the interface.
		return mBuffer;
	}

	T* operator->() { return &Values; }
	const T* operator->() const { return &Values; }

	T Values;

	std::vector<UniformFieldDesc> mFields;
	std::vector<int> UniformLocation;

private:
	ShaderUniformsGles(const ShaderUniformsGles&) = delete;
	ShaderUniformsGles& operator=(const ShaderUniformsGles&) = delete;

	IDataBuffer* mBuffer = nullptr;

private:
	static const char* GetTypeStr(UniformType type)
	{
		switch (type)
		{
		default:
		case UniformType::Int: return "int";
		case UniformType::UInt: return "uint";
		case UniformType::Float: return "float";
		case UniformType::Vec2: return "vec2";
		case UniformType::Vec3: return "vec3";
		case UniformType::Vec4: return "vec4";
		case UniformType::IVec2: return "ivec2";
		case UniformType::IVec3: return "ivec3";
		case UniformType::IVec4: return "ivec4";
		case UniformType::UVec2: return "uvec2";
		case UniformType::UVec3: return "uvec3";
		case UniformType::UVec4: return "uvec4";
		case UniformType::Mat4: return "mat4";
		}
	}
};


class FPresentShaderBase
{
public:
	virtual ~FPresentShaderBase() {}
	virtual void Bind() = 0;

	ShaderUniformsGles<PresentUniforms> Uniforms;

protected:
	virtual void Init(const char * vtx_shader_name, const char * program_name);
	std::unique_ptr<FShaderProgram> mShader;
};

class FPresentShader : public FPresentShaderBase
{
public:
	void Bind() override;

};


}
