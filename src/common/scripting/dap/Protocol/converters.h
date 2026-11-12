/*
** converters.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2026 nikitalita
** Copyright 2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: MIT
**
**---------------------------------------------------------------------------
**
*/

#ifndef dap_converters_h
#define dap_converters_h
#include "dap/protocol.h"
#include "dap/session.h"
#include "dap/traits.h"
#include <string>

#include <functional>

namespace dap
{

struct ServerCaps
{
	bool columnsStartAt1 = true;
	bool linesStartAt1   = true;
	bool pathsAreURIs    = false;

	bool convertColumns                                      = true;
	bool convertLines                                        = true;
	bool convertStackFrameLineAndColumnWhenSourceDoesntExist = false;
	bool convertSourcePaths                                  = true;
	bool convertSourceOrigins                                = true;
	// Runs before any other path conversions
	std::function<void(dap::Source &)> SourceToClientCallback = nullptr;
	// Runs AFTER all other path conversions
	std::function<void(dap::Source &)> SourceToServerCallback = nullptr;
};
namespace detail
{
inline string PathToUri(const string &p)
{
	if (p.empty())
	{
		return p;
	}
	string path = p;
	std::transform(path.begin(), path.end(), path.begin(), [](char c) {
		if (c == '\\')
		{
			return '/';
		}
		return c;
	});
	return "file://" + path;
}

inline string UriToPath(const string &uri)
{
	if (uri.empty())
	{
		return uri;
	}
	std::string path;
	for (char c : uri)
	{
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '+' || c == '/' ||
		    c == ':' || c == '.' || c == '-' || c == '_' || c == '~' || c > '\xFF')
		{
			path += c;
		}
		else if (c == '\\')
		{
			path += '/';
		}
		else
		{
			path += "%" + std::to_string(static_cast<int>(c));
		}
	}
	return path;
}

static void ConvertPathToClient(string &path, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	bool clientPathsAreURIs = clientCaps.pathFormat.value("path") == "uri";
	if (serverCaps.pathsAreURIs == clientPathsAreURIs)
	{
		return;
	}
	if (clientPathsAreURIs)
	{
		path = PathToUri(path);
		return;
	}
	path = UriToPath(path);
}

static void ConvertPathToServer(string &path, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	bool clientPathsAreURIs = clientCaps.pathFormat.value("path") == "uri";
	if (serverCaps.pathsAreURIs == clientPathsAreURIs)
	{
		return;
	}
	if (clientPathsAreURIs)
	{
		path = UriToPath(path);
		return;
	}
	path = PathToUri(path);
}

[[nodiscard]] inline integer _ConvertLineToServer(integer line, const InitializeRequest &clientCaps,
                                                  const ServerCaps &serverCaps)
{
	if (!serverCaps.convertLines || serverCaps.linesStartAt1 == clientCaps.linesStartAt1.value(true))
	{
		return line;
	}
	if (serverCaps.linesStartAt1)
	{
		return line + 1;
	}
	return line - 1;
}

inline void ConvertLineToServer(optional<integer> &line, const InitializeRequest &clientCaps,
                                const ServerCaps &serverCaps)
{
	if (!line.has_value())
	{
		return;
	}
	line = _ConvertLineToServer(line.value(), clientCaps, serverCaps);
}

inline void ConvertLineToServer(integer &line, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	line = _ConvertLineToServer(line, clientCaps, serverCaps);
}

[[nodiscard]] inline integer _ConvertColumnToServer(integer column, const InitializeRequest &clientCaps,
                                                    const ServerCaps &serverCaps)
{
	if (!serverCaps.convertColumns || serverCaps.columnsStartAt1 == clientCaps.columnsStartAt1.value(true))
	{
		return column;
	}
	if (serverCaps.columnsStartAt1)
	{
		return column + 1;
	}
	return column - 1;
}

inline void ConvertColumnToServer(optional<integer> &column, const InitializeRequest &clientCaps,
                                  const ServerCaps &serverCaps)
{
	if (!column.has_value())
	{
		return;
	}
	column = _ConvertColumnToServer(column.value(), clientCaps, serverCaps);
}

inline void ConvertColumnToServer(integer &column, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	column = _ConvertColumnToServer(column, clientCaps, serverCaps);
}

[[nodiscard]] inline integer _ConvertLineToClient(integer line, const InitializeRequest &clientCaps,
                                                  const ServerCaps &serverCaps)
{
	if (!serverCaps.convertLines || serverCaps.linesStartAt1 == clientCaps.linesStartAt1.value(true))
	{
		return line;
	}
	if (serverCaps.linesStartAt1)
	{
		return line - 1;
	}
	return line + 1;
}

inline void ConvertLineToClient(optional<integer> &line, const InitializeRequest &clientCaps,
                                const ServerCaps &serverCaps)
{
	if (!line.has_value())
	{
		return;
	}
	line = _ConvertLineToClient(line.value(), clientCaps, serverCaps);
}

inline void ConvertLineToClient(integer &line, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	line = _ConvertLineToClient(line, clientCaps, serverCaps);
}

[[nodiscard]] inline integer _ConvertColumnToClient(integer column, const InitializeRequest &clientCaps,
                                                    const ServerCaps &serverCaps)
{
	if (!serverCaps.convertColumns || serverCaps.columnsStartAt1 == clientCaps.columnsStartAt1.value(true))
	{
		return column;
	}
	if (serverCaps.columnsStartAt1)
	{
		return column - 1;
	}
	return column + 1;
}

inline void ConvertColumnToClient(integer &column, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	column = _ConvertColumnToClient(column, clientCaps, serverCaps);
}

inline void ConvertColumnToClient(optional<integer> &column, const InitializeRequest &clientCaps,
                                  const ServerCaps &serverCaps)
{
	if (!column.has_value())
	{
		return;
	}
	column = _ConvertColumnToClient(column.value(), clientCaps, serverCaps);
}

template <typename T, typename Enable = void> struct request_converter
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &caps)
	{
		// do nothing
	}
};

template <typename T, typename Enable = void> struct response_converter
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &caps)
	{
		// do nothing
	}
};

template <typename T, typename Enable = void> struct event_converter
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &caps)
	{
		// do nothing
	}
};

// special functions for converting Source objects
static void ConvertSourceToClient(Source &source, const InitializeRequest &clientCaps, const ServerCaps &caps)
{
	if (caps.SourceToClientCallback)
	{
		caps.SourceToClientCallback(source);
	}
	if (!caps.convertSourcePaths && !caps.convertSourceOrigins)
	{
		return;
	}
	if (caps.convertSourcePaths && source.path.has_value())
	{
		ConvertPathToClient(source.path.value(), clientCaps, caps);
	}
	if (caps.convertSourceOrigins && source.origin.has_value())
	{
		ConvertPathToClient(source.origin.value(), clientCaps, caps);
	}
	if (source.sources.has_value())
	{
		for (auto &source : source.sources.value())
		{
			ConvertSourceToClient(source, clientCaps, caps);
		}
	}
}

static void ConvertSourceToClient(optional<Source> &source, const InitializeRequest &clientCaps, const ServerCaps &caps)
{
	if (!source.has_value())
	{
		return;
	}
	ConvertSourceToClient(source.value(), clientCaps, caps);
}

static void ConvertSourceToServer(Source &source, const InitializeRequest &clientCaps, const ServerCaps &caps)
{
	if (!caps.convertSourcePaths && !caps.convertSourceOrigins)
	{
		if (caps.SourceToServerCallback)
		{
			caps.SourceToServerCallback(source);
		}
		return;
	}
	if (caps.convertSourcePaths && source.path.has_value())
	{
		ConvertPathToServer(source.path.value(), clientCaps, caps);
	}
	if (caps.convertSourceOrigins && source.origin.has_value())
	{
		ConvertPathToServer(source.origin.value(), clientCaps, caps);
	}
	if (source.sources.has_value())
	{
		for (auto &source : source.sources.value())
		{
			ConvertSourceToServer(source, clientCaps, caps);
		}
	}
	if (caps.SourceToServerCallback)
	{
		caps.SourceToServerCallback(source);
	}
}

static void ConvertSourceToServer(optional<Source> &source, const InitializeRequest &clientCaps, const ServerCaps &caps)
{
	if (!source.has_value())
	{
		return;
	}
	ConvertSourceToServer(source.value(), clientCaps, caps);
}

// end of common converters

// special function for converting StackFrame objects
inline void ConvertStackFrameToClient(StackFrame &stackFrame, const InitializeRequest &clientCaps,
                                            const ServerCaps &caps)
{
	ConvertSourceToClient(stackFrame.source, clientCaps, caps);
	/* special handling for non-optional line and column; due to the ambiguity of
	the spec regarding this:
	        // The line within the source of the frame. If the source attribute is missing
	        // or doesn't exist, 'line' is 0 and should be ignored by the client.
	We only convert it if the source exists */
	if (caps.convertStackFrameLineAndColumnWhenSourceDoesntExist || (stackFrame.source.has_value()))
	{
		ConvertLineToServer(stackFrame.line, clientCaps, caps);
		ConvertColumnToServer(stackFrame.column, clientCaps, caps);
	}
	ConvertColumnToServer(stackFrame.endColumn, clientCaps, caps);
	ConvertLineToServer(stackFrame.endLine, clientCaps, caps);
}
// no need to convert StackFrame objects to server since the client doesn't send
// them

inline void ConvertBreakpointLocationToClient(BreakpointLocation      &obj,
                                                            const InitializeRequest &clientCaps,
                                                            const ServerCaps        &serverCaps)
{
	ConvertColumnToClient(obj.column, clientCaps, serverCaps);
	ConvertColumnToServer(obj.endColumn, clientCaps, serverCaps);
	ConvertLineToClient(obj.endLine, clientCaps, serverCaps);
	ConvertLineToClient(obj.line, clientCaps, serverCaps);
}

inline void ConvertDisassembledInstructionToClient(DisassembledInstruction &obj,
                                                                      const InitializeRequest &clientCaps,
                                                                      const ServerCaps        &serverCaps)
{
	ConvertColumnToClient(obj.column, clientCaps, serverCaps);
	ConvertColumnToClient(obj.endColumn, clientCaps, serverCaps);
	ConvertLineToClient(obj.endLine, clientCaps, serverCaps);
	ConvertLineToClient(obj.line, clientCaps, serverCaps);
	ConvertSourceToServer(obj.location, clientCaps, serverCaps);
}

inline void ConvertGotoTargetToClient(GotoTarget &obj, const InitializeRequest &clientCaps,
                                            const ServerCaps &serverCaps)
{
	ConvertColumnToClient(obj.column, clientCaps, serverCaps);
	ConvertColumnToClient(obj.endColumn, clientCaps, serverCaps);
	ConvertLineToClient(obj.endLine, clientCaps, serverCaps);
	ConvertLineToClient(obj.line, clientCaps, serverCaps);

}
inline void ConvertBreakpointToClient(Breakpoint &obj, const InitializeRequest &clientCaps,
                                            const ServerCaps &serverCaps)
{
	ConvertColumnToClient(obj.column, clientCaps, serverCaps);
	ConvertColumnToClient(obj.endColumn, clientCaps, serverCaps);
	ConvertLineToClient(obj.endLine, clientCaps, serverCaps);
	ConvertLineToClient(obj.line, clientCaps, serverCaps);
	ConvertSourceToClient(obj.source, clientCaps, serverCaps);
}

inline void ConvertScopeToClient(Scope &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	ConvertColumnToClient(obj.column, clientCaps, serverCaps);
	ConvertColumnToClient(obj.endColumn, clientCaps, serverCaps);
	ConvertLineToClient(obj.endLine, clientCaps, serverCaps);
	ConvertLineToClient(obj.line, clientCaps, serverCaps);
	ConvertSourceToClient(obj.source, clientCaps, serverCaps);
}

inline void ConvertSourceBreakpointToServer(SourceBreakpoint &obj, const InitializeRequest &clientCaps,
                                                        const ServerCaps &serverCaps)
{
	ConvertColumnToServer(obj.column, clientCaps, serverCaps);
	ConvertLineToServer(obj.line, clientCaps, serverCaps);
}

inline void ConvertStepInTargetToClient(StepInTarget &obj, const InitializeRequest &clientCaps,
                                                const ServerCaps &serverCaps)
{
	ConvertColumnToClient(obj.column, clientCaps, serverCaps);
	ConvertColumnToClient(obj.endColumn, clientCaps, serverCaps);
	ConvertLineToClient(obj.endLine, clientCaps, serverCaps);
	ConvertLineToClient(obj.line, clientCaps, serverCaps);
}

template <typename T> struct event_converter<T, traits::EnableIfIsType<BreakpointEvent, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		ConvertBreakpointToClient(obj.breakpoint, clientCaps, serverCaps);
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<BreakpointLocationsResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.breakpoints)
		{
			ConvertBreakpointLocationToClient(item, clientCaps, serverCaps);
		}
	}
};

template <typename T> struct request_converter<T, traits::EnableIfIsType<BreakpointLocationsRequest, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		ConvertColumnToServer(obj.column, clientCaps, serverCaps);
		ConvertColumnToServer(obj.endColumn, clientCaps, serverCaps);
		ConvertLineToServer(obj.endLine, clientCaps, serverCaps);
		ConvertLineToServer(obj.line, clientCaps, serverCaps);

		ConvertSourceToServer(obj.source, clientCaps, serverCaps);
	}
};

template <typename T> struct request_converter<T, traits::EnableIfIsType<CompletionsRequest, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		obj.column = ConvertColumnToServer(obj.column, clientCaps, serverCaps);
		ConvertLineToServer(obj.line, clientCaps, serverCaps);
	}
};
template <typename T> struct response_converter<T, traits::EnableIfIsType<DisassembleResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.instructions)
		{
			ConvertDisassembledInstructionToClient(item, clientCaps, serverCaps);
		}
	}
};

template <typename T> struct request_converter<T, traits::EnableIfIsType<EvaluateRequest, T>>
{
	void convert(EvaluateRequest &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		ConvertColumnToServer(obj.column, clientCaps, serverCaps);
		ConvertLineToServer(obj.line, clientCaps, serverCaps);
		ConvertSourceToServer(obj.source, clientCaps, serverCaps);
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<GotoTargetsResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.targets)
		{
			ConvertGotoTargetToClient(item, clientCaps, serverCaps);
		}
	}
};

template <typename T> struct request_converter<T, traits::EnableIfIsType<GotoTargetsRequest, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		ConvertColumnToServer(obj.column, clientCaps, serverCaps);
		ConvertLineToServer(obj.line, clientCaps, serverCaps);

		ConvertSourceToServer(obj.source, clientCaps, serverCaps);
	}
};

template <typename T> struct event_converter<T, traits::EnableIfIsType<LoadedSourceEvent, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		ConvertSourceToClient(obj.source, clientCaps, serverCaps);
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<LoadedSourcesResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.sources)
		{
			ConvertSourceToClient(item, clientCaps, serverCaps);
		}
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<LocationsResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		ConvertColumnToClient(obj.endColumn, clientCaps, serverCaps);
		ConvertColumnToClient(obj.column, clientCaps, serverCaps);
		ConvertLineToClient(obj.endLine, clientCaps, serverCaps);
		ConvertLineToClient(obj.line, clientCaps, serverCaps);

		ConvertSourceToClient(obj.source, clientCaps, serverCaps);
	}
};

template <typename T> struct event_converter<T, traits::EnableIfIsType<OutputEvent, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		ConvertColumnToClient(obj.column, clientCaps, serverCaps);
		ConvertLineToClient(obj.line, clientCaps, serverCaps);
		ConvertSourceToClient(obj.source, clientCaps, serverCaps);
	}
};
template <typename T> struct response_converter<T, traits::EnableIfIsType<ScopesResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.scopes)
		{
			ConvertScopeToClient(item, clientCaps, serverCaps);
		}
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<SetBreakpointsResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.breakpoints)
		{
			ConvertBreakpointToClient(item, clientCaps, serverCaps);
		}
	}
};
template <typename T> struct request_converter<T, traits::EnableIfIsType<SetBreakpointsRequest, T>>
{
	void convert(SetBreakpointsRequest &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		if (obj.breakpoints.has_value())
		{
			for (auto &item : obj.breakpoints.value())
			{
				ConvertSourceBreakpointToServer(item, clientCaps, serverCaps);
			}
		}
		if (obj.lines.has_value())
		{
			for (auto &item : obj.lines.value())
			{
				ConvertLineToServer(item, clientCaps, serverCaps);
			}
		}
		ConvertSourceToServer(obj.source, clientCaps, serverCaps);
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<SetDataBreakpointsResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.breakpoints)
		{
			ConvertBreakpointToClient(item, clientCaps, serverCaps);
		}
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<SetExceptionBreakpointsResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		if (obj.breakpoints.has_value())
		{
			for (auto &item : obj.breakpoints.value())
			{
				ConvertBreakpointToClient(item, clientCaps, serverCaps);
			}
		}
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<SetFunctionBreakpointsResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.breakpoints)
		{
			ConvertBreakpointToClient(item, clientCaps, serverCaps);
		}
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<SetInstructionBreakpointsResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.breakpoints)
		{
			ConvertBreakpointToClient(item, clientCaps, serverCaps);
		}
	}
};

template <typename T> struct request_converter<T, traits::EnableIfIsType<SourceRequest, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		ConvertSourceToServer(obj.source, clientCaps, serverCaps);
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<StackTraceResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.stackFrames)
		{
			ConvertStackFrameToClient(item, clientCaps, serverCaps);
		}
	}
};

template <typename T> struct response_converter<T, traits::EnableIfIsType<StepInTargetsResponse, T>>
{
	void convert(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
	{
		for (auto &item : obj.targets)
		{
			ConvertStepInTargetToClient(item, clientCaps, serverCaps);
		}
	}
};
} // namespace detail
inline bool IsConversionNecessary(const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	if (serverCaps.SourceToClientCallback || serverCaps.SourceToServerCallback)
	{
		return true;
	}
	if (!serverCaps.convertStackFrameLineAndColumnWhenSourceDoesntExist && !serverCaps.convertSourcePaths &&
	    !serverCaps.convertSourceOrigins && !serverCaps.convertLines && !serverCaps.convertColumns)
	{
		return false;
	}
	if ((serverCaps.pathsAreURIs == (clientCaps.pathFormat.value("path") == "uri")) &&
	    (serverCaps.linesStartAt1 == clientCaps.linesStartAt1.value(true)) &&
	    (serverCaps.columnsStartAt1 == clientCaps.columnsStartAt1.value(true)))
	{
		return false;
	}
	return true;
}

template <typename T>
[[nodiscard]] T ConvertRequestToServer(const T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	if (!IsConversionNecessary(clientCaps, serverCaps))
	{
		return obj;
	}
	detail::request_converter<T> converter;
	T                            result = T(obj);
	converter.convert(result, clientCaps, serverCaps);
	return result;
}

template <typename T>
T &ConvertRequestToServer(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	if (!IsConversionNecessary(clientCaps, serverCaps))
	{
		return obj;
	}
	detail::request_converter<T> converter;
	converter.convert(obj, clientCaps, serverCaps);
	return obj;
}

template <typename T>
[[nodiscard]] T &&ConvertRequestToServer(T &&obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	if (!IsConversionNecessary(clientCaps, serverCaps))
	{
		return std::move(obj);
	}
	detail::request_converter<T> converter;
	converter.convert(obj, clientCaps, serverCaps);
	return std::move(obj);
}

template <typename T> T &ConvertEventToClient(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	if (!IsConversionNecessary(clientCaps, serverCaps))
	{
		return obj;
	}
	detail::event_converter<T> converter;
	converter.convert(obj, clientCaps, serverCaps);
	return obj;
}

template <typename T>
[[nodiscard]] T &&ConvertEventToClient(T &&obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	if (!IsConversionNecessary(clientCaps, serverCaps))
	{
		return std::move(obj);
	}
	detail::event_converter<T> converter;
	converter.convert(obj, clientCaps, serverCaps);
	return std::move(obj);
}

template <typename T>
[[nodiscard]] T ConvertEventToClient(const T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	if (!IsConversionNecessary(clientCaps, serverCaps))
	{
		return obj;
	}
	T                          result = T(obj);
	detail::event_converter<T> converter;
	converter.convert(result, clientCaps, serverCaps);
	return result;
}

template <typename T>
T &ConvertResponseToClient(T &obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	if (!IsConversionNecessary(clientCaps, serverCaps))
	{
		return obj;
	}
	detail::response_converter<T> converter;
	converter.convert(obj, clientCaps, serverCaps);
	return obj;
}

template <typename T>
ResponseOrError<T> &ConvertResponseToClient(ResponseOrError<T> &obj, const InitializeRequest &clientCaps,
                                            const ServerCaps &serverCaps)
{
	if (!obj.error.message.empty())
	{
		return obj;
	}
	ConvertResponseToClient(obj.response, clientCaps, serverCaps);
	return obj;
}

template <typename T, typename = std::enable_if_t<std::is_base_of_v<Response, T>>>
[[nodiscard]] T &&ConvertResponseToClient(T &&obj, const InitializeRequest &clientCaps, const ServerCaps &serverCaps)
{
	if (!IsConversionNecessary(clientCaps, serverCaps))
	{
		return std::move(obj);
	}
	detail::response_converter<T> converter;
	converter.convert(obj, clientCaps, serverCaps);
	return std::move(obj);
}

template <typename U>
[[nodiscard]] ResponseOrError<U> &&ConvertResponseOrErrorToClient(ResponseOrError<U>     &&obj,
                                                                  const InitializeRequest &clientCaps,
                                                                  const ServerCaps        &serverCaps)
{
	if (!obj.error.message.empty())
	{
		return std::move(obj);
	}
	ConvertResponseToClient<U>(obj.response, clientCaps, serverCaps);
	return std::move(obj);
}

} // namespace dap
#endif // dap_converters_h
