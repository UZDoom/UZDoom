/*
** PexCache.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025 nikitalita
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
** Code written prior to 2026 is also licensed under:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <map>

#include <dap/protocol.h>
#include <dap/session.h>
#include <mutex>
#include <string>
#include "Utilities.h"
#include <range_map/range_map.h>
#include <name.h>
#include <shared_mutex>
#include <vmintern.h>

class PFunction;
class PClassType;
class PStruct;
class VMFunction;
class VMScriptFunction;

// TODO: we may not want to do it like this, but this is the easiest way to get the opinfo.
extern const VMOpInfo OpInfo[NUM_OPS];

namespace DebugServer
{
class PexCache;
struct Binary
{
	using NameStateFunctionMap = std::map<std::string, VMFunction *>;
	using NameFunctionMap = std::map<FName, PFunction *>;
	using NameClassMap = std::map<FName, PClassType *>;
	using NameStructMap = std::map<FName, PStruct *>;
	using FunctionLineMap = beneficii::range_map<uint32_t, VMScriptFunction *, std::less<uint32_t>, std::allocator<beneficii::range_map_item<uint32_t, VMScriptFunction *>>, true>;
	using FunctionCodeMap = beneficii::range_map<void *, VMScriptFunction *>;

private:
	friend class PexCache;
	std::string archiveName;
	std::string archivePath;
	std::string scriptName;
	std::string unqualifiedScriptPath;
	std::string compiledPath;
	std::string cachedSourceCode;
	int lump;
	int scriptReference;
	NameFunctionMap functions;
	FunctionLineMap functionLineMap;
	FunctionCodeMap functionCodeMap;
	NameStateFunctionMap stateFunctions;
	void PopulateFunctionMaps();

public:
	std::pair<int, int> GetFunctionLineRange(const VMScriptFunction *functionName) const;
	std::string GetQualifiedPath() const;
	dap::Source GetDapSource() const;
	std::string GetArchiveName() const;
	std::string GetArchivePath() const;
	size_t GetFunctionCount() const;
	std::stack<FunctionLineMap::const_iterator> FindFunctionRangesByLine(int line) const;
	std::stack<FunctionCodeMap::const_iterator> FindFunctionRangesByCode(void *address) const;
	int GetScriptRef() const { return scriptReference; }
	bool HasFunctions() const;
	bool HasFunctionLines() const;
	void ProcessScriptFunction(const std::string &qualPath, VMFunction *vmfunc);
};
struct DisassemblyLine
{
	void *address;
	int line = -1;
	int endLine = -1;
	int ref = -1;
	uint8_t bytesize = 4;
	bool is_valid_bp = false;
	std::string bytes;
	std::string instruction;
	std::string comment;
	std::string pointed_symbol;
	std::string function;
	VMFunction *funcPtr;
};


class PexCache
{
public:
	using BinaryPtr = std::shared_ptr<Binary>;
	using BinaryMap = std::map<int, BinaryPtr>;
	using DisassemblyLinePtr = std::shared_ptr<DisassemblyLine>;
	using DisassemblyMap = beneficii::range_map<void *, std::map<void *, DisassemblyLinePtr>>;
	PexCache() = default;
	bool HasScript(int scriptReference);
	bool HasScript(const std::string &scriptName);

	std::shared_ptr<Binary> GetCachedScript(const int ref);
	void PrintOutAllLoadedScripts();
	std::shared_ptr<Binary> GetScript(const dap::Source &source);

	std::shared_ptr<Binary> GetScript(std::string fqsn);
	bool GetDecompiledSource(const dap::Source &source, std::string &decompiledSource);

	bool GetDecompiledSource(const std::string &fqpn, std::string &decompiledSource);
	bool GetSourceData(const std::string &scriptName, dap::Source &data);
	void Clear();
	void ScanAllScripts();
	dap::ResponseOrError<dap::LoadedSourcesResponse> GetLoadedSources(const dap::LoadedSourcesRequest &request);

	static std::shared_ptr<DisassemblyLine>
	MakeInstruction(VMScriptFunction *func, int ref, const std::string &instruction_text, const std::string &opcode, const std::string &comment, unsigned long long ipnum, const std::string &pointed_symbol);

	bool GetDisassemblyLines(const VMOP *address, int64_t p_instructionOffset, int64_t p_count, std::vector<std::shared_ptr<DisassemblyLine>> &lines_vec);

	std::shared_ptr<Binary> AddScript(const std::string &scriptPath);
	std::vector<VMFunction *> GetFunctionsAtAddress(void *address);

	std::vector<dap::Module> GetModules();
	private:
	using scripts_lock = std::scoped_lock<std::recursive_mutex>;

	int FindFunctionDeclaration(const std::shared_ptr<Binary> &source, const VMScriptFunction *func, int start_line_from_1);
	bool GetOrCacheSource(BinaryPtr binary, std::string &decompiledSource);
	uint64_t AddDisassemblyLines(VMScriptFunction *func, DisassemblyMap &instructions);
	static bool GetSourceContent(const std::string &scriptPath, std::string &decompiledSource);

	static void PopulateCodeMap(BinaryPtr binary, Binary::FunctionCodeMap &functionCodeMap);
	static void PopulateFromPaths(const std::map<std::string, int> &scripts, BinaryMap &p_scripts, bool clobber = false);
	static void ScanScriptsInContainer(int baselump, BinaryMap &m_scripts, const std::string &filter = "");
	static BinaryPtr makeEmptyBinary(const std::string &scriptPath, int lump);
	
	DisassemblyMap m_disassemblyMap;
	Binary::FunctionCodeMap m_globalCodeMap;
	std::recursive_mutex m_scriptsMutex;
	BinaryMap m_scripts;

};
}
