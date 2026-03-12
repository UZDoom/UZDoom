cmake_minimum_required(VERSION 3.11)
project(zwidget)

option(ENABLE_METAL "Enable Metal support for application rendering" ON)
option(ENABLE_OPENGL "Enable OpenGL support for application rendering" ON)

if(WIN32)
	option(ENABLE_SDL3 "Enable SDL3 backend. Optional on all platforms" OFF)
	option(ENABLE_SDL2 "Enable SDL2 backend. Optional on all platforms" OFF)
else()
	option(ENABLE_SDL3 "Enable SDL3 backend. Optional on all platforms" ON)
	option(ENABLE_SDL2 "Enable SDL2 backend. Optional on all platforms" ON)
endif()

if (UNIX AND NOT APPLE)
	include(FindPkgConfig)
	pkg_check_modules(DBUS dbus-1)
	if (!DBUS_FOUND)
		pkg_check_modules(DBUS REQUIRED dbus) # Fedora Linux looks for dbus instead
	endif()
	pkg_check_modules(WAYLANDPP
		wayland-client
		wayland-client++
		wayland-client-extra++
		wayland-client-unstable++
		wayland-cursor++
		xkbcommon>=0.5.0
	)
	pkg_check_modules(GLIB glib-2.0)
	pkg_check_modules(FONTCONFIG fontconfig)
endif()

# SDL3 finding stuff
# Optional on all platforms
# If both SDL3 and SDL2 are available, only SDL3 support will be enabled.
if(ENABLE_SDL3)
	find_package(SDL3 QUIET)
	if(NOT ${SDL3_FOUND})
		include(FindPkgConfig)
		pkg_search_module(SDL3 sdl3)
	endif()
endif()

# SDL2 finding stuff
# Optional on all platforms
if(ENABLE_SDL2)
	find_package(SDL2 QUIET)
	if(NOT ${SDL2_FOUND})
		include(FindPkgConfig)
		pkg_search_module(SDL2 sdl2)
	endif()
endif()

set(ZWIDGET_SOURCES
	src/core/canvas.cpp
	src/core/font.cpp
	src/core/font_impl.h
	src/core/image.cpp
	src/core/layout.cpp
	src/core/span_layout.cpp
	src/core/timer.cpp
	src/core/widget.cpp
	src/core/theme.cpp
	src/core/theme_style_tokenizer.cpp
	src/core/theme_style_tokenizer.h
	src/core/utf8reader.cpp
	src/core/pathfill.cpp
	src/core/truetypefont.cpp
	src/core/truetypefont.h
	src/core/picopng/picopng.cpp
	src/core/picopng/picopng.h
	src/core/nanosvg/nanosvg.cpp
	src/core/nanosvg/nanosvg.h
	src/core/nanosvg/nanosvgrast.h
	src/widgets/dialog/dialog.cpp
	src/widgets/dialog/messagebox.cpp
	src/widgets/dialog/textinputdialog.cpp
	src/widgets/lineedit/lineedit.cpp
	src/widgets/mainwindow/mainwindow.cpp
	src/widgets/menubar/menubar.cpp
	src/widgets/scrollbar/scrollbar.cpp
	src/widgets/statusbar/statusbar.cpp
	src/widgets/textedit/textedit.cpp
	src/widgets/toolbar/toolbar.cpp
	src/widgets/toolbar/toolbarbutton.cpp
	src/widgets/imagebox/imagebox.cpp
	src/widgets/textlabel/textlabel.cpp
	src/widgets/pushbutton/pushbutton.cpp
	src/widgets/checkboxlabel/checkboxlabel.cpp
	src/widgets/dropdown/dropdown.cpp
	src/widgets/listview/listview.cpp
	src/widgets/tabwidget/tabwidget.cpp
	src/widgets/layout/hboxlayout.cpp
	src/widgets/layout/vboxlayout.cpp
	src/window/window.cpp
	src/window/stub/stub_open_folder_dialog.cpp
	src/window/stub/stub_open_folder_dialog.h
	src/window/stub/stub_open_file_dialog.cpp
	src/window/stub/stub_open_file_dialog.h
	src/window/stub/stub_save_file_dialog.cpp
	src/window/stub/stub_save_file_dialog.h
	src/window/ztimer/ztimer.h
	src/window/ztimer/ztimer.cpp
	src/systemdialogs/open_folder_dialog.cpp
	src/systemdialogs/open_file_dialog.cpp
	src/systemdialogs/save_file_dialog.cpp
)

set(ZWIDGET_INCLUDES
	include/zwidget/core/canvas.h
	include/zwidget/core/colorf.h
	include/zwidget/core/font.h
	include/zwidget/core/image.h
	include/zwidget/core/rect.h
	include/zwidget/core/pathfill.h
	include/zwidget/core/span_layout.h
	include/zwidget/core/timer.h
	include/zwidget/core/widget.h
	include/zwidget/core/theme.h
	include/zwidget/core/utf8reader.h
	include/zwidget/core/resourcedata.h
	include/zwidget/widgets/dialog/dialog.h
	include/zwidget/widgets/dialog/messagebox.h
	include/zwidget/widgets/dialog/textinputdialog.h
	include/zwidget/widgets/lineedit/lineedit.h
	include/zwidget/widgets/mainwindow/mainwindow.h
	include/zwidget/widgets/menubar/menubar.h
	include/zwidget/widgets/scrollbar/scrollbar.h
	include/zwidget/widgets/statusbar/statusbar.h
	include/zwidget/widgets/textedit/textedit.h
	include/zwidget/widgets/toolbar/toolbar.h
	include/zwidget/widgets/toolbar/toolbarbutton.h
	include/zwidget/widgets/imagebox/imagebox.h
	include/zwidget/widgets/textlabel/textlabel.h
	include/zwidget/widgets/pushbutton/pushbutton.h
	include/zwidget/widgets/checkboxlabel/checkboxlabel.h
	include/zwidget/widgets/listview/listview.h
	include/zwidget/widgets/tabwidget/tabwidget.h
	include/zwidget/widgets/layout/hboxlayout.h
	include/zwidget/widgets/layout/vboxlayout.h
	include/zwidget/window/window.h
	include/zwidget/window/x11nativehandle.h
	include/zwidget/window/waylandnativehandle.h
	include/zwidget/window/win32nativehandle.h
	include/zwidget/window/sdlnativehandle.h
	include/zwidget/systemdialogs/open_folder_dialog.h
	include/zwidget/systemdialogs/open_file_dialog.h
	include/zwidget/systemdialogs/save_file_dialog.h
)

set(ZWIDGET_WIN32_SOURCES
	src/window/win32/win32_display_backend.cpp
	src/window/win32/win32_display_backend.h
	src/window/win32/win32_display_window.cpp
	src/window/win32/win32_display_window.h
	src/window/win32/win32_open_folder_dialog.cpp
	src/window/win32/win32_open_folder_dialog.h
	src/window/win32/win32_open_file_dialog.cpp
	src/window/win32/win32_open_file_dialog.h
	src/window/win32/win32_save_file_dialog.cpp
	src/window/win32/win32_save_file_dialog.h
	src/window/win32/win32_util.h
	src/core/resourcedata_win.cpp
)

set(ZWIDGET_DBUS_SOURCES
	src/window/dbus/dbus_open_folder_dialog.cpp
	src/window/dbus/dbus_open_folder_dialog.h
	src/window/dbus/dbus_open_file_dialog.cpp
	src/window/dbus/dbus_open_file_dialog.h
	src/window/dbus/dbus_save_file_dialog.cpp
	src/window/dbus/dbus_save_file_dialog.h
)

set(ZWIDGET_UNIX_SOURCES
	src/core/resourcedata_unix.cpp
)

set(ZWIDGET_COCOA_SOURCES
	src/core/resourcedata_mac.mm
)

set(ZWIDGET_COCOA_ARC_SOURCES
	src/window/cocoa/cocoa_open_file_dialog.mm
	src/window/cocoa/cocoa_save_file_dialog.mm
	src/window/cocoa/cocoa_open_folder_dialog.mm
	src/window/cocoa/cocoa_display_backend.mm
	src/window/cocoa/cocoa_display_window.mm
)

set(ZWIDGET_SDL2_SOURCES
	src/window/sdl2/sdl2_display_backend.cpp
	src/window/sdl2/sdl2_display_backend.h
	src/window/sdl2/sdl2_display_window.cpp
	src/window/sdl2/sdl2_display_window.h
)

set(ZWIDGET_SDL3_SOURCES
		src/window/sdl3/sdl3_display_backend.cpp
		src/window/sdl3/sdl3_display_backend.h
		src/window/sdl3/sdl3_display_window.cpp
		src/window/sdl3/sdl3_display_window.h
)

set(ZWIDGET_X11_SOURCES
	src/window/x11/x11_display_backend.cpp
	src/window/x11/x11_display_backend.h
	src/window/x11/x11_display_window.cpp
	src/window/x11/x11_display_window.h
	src/window/x11/x11_connection.cpp
	src/window/x11/x11_connection.h
)

set(ZWIDGET_WAYLAND_SOURCES
	src/window/wayland/wayland_display_backend.cpp
	src/window/wayland/wayland_display_backend.h
	src/window/wayland/wayland_display_window.cpp
	src/window/wayland/wayland_display_window.h
	src/window/wayland/wl_fractional_scaling_protocol.cpp
	src/window/wayland/wl_fractional_scaling_protocol.hpp
	src/window/wayland/wl_cursor_shape.cpp
	src/window/wayland/wl_cursor_shape.hpp
	src/window/wayland/wl_xdg_dialog_v1.cpp
	src/window/wayland/wl_xdg_dialog_v1.hpp
	src/window/wayland/wl_xdg_toplevel_icon.cpp
	src/window/wayland/wl_xdg_toplevel_icon.hpp
)

source_group("src" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/.+")
source_group("src\\core" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/core/.+")
source_group("src\\core\\picopng" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/core/picopng/.+")
source_group("src\\core\\nanosvg" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/core/nanosvg/.+")
source_group("src\\widgets" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/.+")
source_group("src\\widgets\\dialog" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/dialog/.+")
source_group("src\\widgets\\lineedit" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/lineedit/.+")
source_group("src\\widgets\\mainwindow" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/mainwindow/.+")
source_group("src\\widgets\\menubar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/menubar/.+")
source_group("src\\widgets\\scrollbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/scrollbar/.+")
source_group("src\\widgets\\statusbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/statusbar/.+")
source_group("src\\widgets\\textedit" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/textedit/.+")
source_group("src\\widgets\\toolbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/toolbar/.+")
source_group("src\\widgets\\imagebox" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/imagebox/.+")
source_group("src\\widgets\\textlabel" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/textlabel/.+")
source_group("src\\widgets\\pushbutton" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/pushbutton/.+")
source_group("src\\widgets\\checkboxlabel" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/checkboxlabel/.+")
source_group("src\\widgets\\dropdown" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/dropdown/.+")
source_group("src\\widgets\\listview" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/listview/.+")
source_group("src\\widgets\\tabwidget" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/tabwidget/.+")
source_group("src\\window" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/.+")
source_group("src\\window\\stub" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/stub/.+")
source_group("src\\window\\win32" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/win32/.+")
source_group("src\\window\\sdl2" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/sdl2/.+")
source_group("src\\window\\sdl3" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/sdl3/.+")
source_group("src\\window\\x11" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/x11/.+")
source_group("src\\window\\wayland" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/wayland/.+")
source_group("src\\window\\dbus" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/dbus/.+")
source_group("src\\systemdialogs" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/systemdialogs/.+")
source_group("include" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/.+")
source_group("include\\core" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/core/.+")
source_group("include\\widgets" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/.+")
source_group("include\\widgets\\lineedit" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/lineedit/.+")
source_group("include\\widgets\\mainwindow" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/mainwindow/.+")
source_group("include\\widgets\\menubar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/menubar/.+")
source_group("include\\widgets\\scrollbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/scrollbar/.+")
source_group("include\\widgets\\statusbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/statusbar/.+")
source_group("include\\widgets\\textedit" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/textedit/.+")
source_group("include\\widgets\\toolbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/toolbar/.+")
source_group("include\\widgets\\imagebox" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/imagebox/.+")
source_group("include\\widgets\\textlabel" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/textlabel/.+")
source_group("include\\widgets\\pushbutton" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/pushbutton/.+")
source_group("include\\widgets\\checkboxlabel" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/checkboxlabel/.+")
source_group("include\\widgets\\listview" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/listview/.+")
source_group("include\\widgets\\tabwidget" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/widgets/tabwidget/.+")
source_group("include\\window" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/window/.+")
source_group("include\\systemdialogs" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/zwidget/systemdialogs/.+")

# Include directory to external projects using zwidget
include_directories(include)

# Internal include dirs for building zwidget
set(ZWIDGET_INCLUDE_DIRS include/zwidget src)

set(ZWIDGET_COMPILE_OPTIONS)

if(WIN32)
	list(APPEND ZWIDGET_SOURCES ${ZWIDGET_WIN32_SOURCES})
	set(ZWIDGET_DEFINES -DUNICODE -D_UNICODE)
	set(ZWIDGET_LINK_OPTIONS)
	if(MSVC)
		# Use all cores for compilation
		set(ZWIDGET_COMPILE_OPTIONS ${ZWIDGET_COMPILE_OPTIONS} /MP)

		# Ignore specific warnings
		#set(ZWIDGET_COMPILE_OPTIONS ${ZWIDGET_COMPILE_OPTIONS} /wd4244 /wd4267 /wd4005 /wd4018)

		# Don't slow down std containers in debug builds
		set(ZWIDGET_DEFINES ${ZWIDGET_DEFINES})

		# Ignore warning about legacy CRT functions
		#set(ZWIDGET_DEFINES ${ZWIDGET_DEFINES} -D_CRT_SECURE_NO_WARNINGS)
	endif()
elseif(APPLE)
	list(APPEND ZWIDGET_SOURCES ${ZWIDGET_COCOA_SOURCES} ${ZWIDGET_COCOA_ARC_SOURCES})
	set_source_files_properties(${ZWIDGET_COCOA_ARC_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
	set(ZWIDGET_LIBS ${CMAKE_DL_LIBS} -ldl)
	set(ZWIDGET_DEFINES -DUNIX -D_UNIX)
	set(ZWIDGET_LINK_OPTIONS -pthread)

	find_library(COCOA_FRAMEWORK Cocoa)
	if (NOT COCOA_FRAMEWORK)
		message(FATAL_ERROR "Cocoa framework not found")
	endif()

	find_library(COREVIDEO_FRAMEWORK CoreVideo)
	if (NOT COREVIDEO_FRAMEWORK)
		message(FATAL_ERROR "CoreVideo framework not found")
	endif()

	if(ENABLE_METAL)
		find_library(METAL_FRAMEWORK Metal)
		find_library(QUARTZCORE_FRAMEWORK QuartzCore)
		if (NOT QUARTZCORE_FRAMEWORK)
			message(FATAL_ERROR "QuartzCore framework not found")
		endif()
	endif()

	if(ENABLE_OPENGL)
		find_library(OPENGL_FRAMEWORK OpenGL)
		if (NOT OPENGL_FRAMEWORK)
			message(FATAL_ERROR "OpenGL framework not found")
		endif()
	endif()

	find_library(UNIFORMTYPEIDENTIFIERS_FRAMEWORK UniformTypeIdentifiers)
	if (NOT UNIFORMTYPEIDENTIFIERS_FRAMEWORK)
		message(FATAL_ERROR "UniformTypeIdentifiers framework not found")
	endif()
elseif(HAIKU)
	set(ZWIDGET_SOURCES ${ZWIDGET_SOURCES} ${ZWIDGET_UNIX_SOURCES})
	set(ZWIDGET_LIBS ${CMAKE_DL_LIBS})
	set(ZWIDGET_DEFINES -DUNIX -D_UNIX)
	set(ZWIDGET_LINK_OPTIONS -pthread)
else()
	set(ZWIDGET_SOURCES ${ZWIDGET_SOURCES} ${ZWIDGET_UNIX_SOURCES})
	set(ZWIDGET_SOURCES ${ZWIDGET_SOURCES} ${ZWIDGET_X11_SOURCES})
	set(ZWIDGET_LIBS ${CMAKE_DL_LIBS} -lX11 -lXi)
	set(ZWIDGET_DEFINES -DUNIX -D_UNIX -DUSE_X11)
	set(ZWIDGET_LINK_OPTIONS -pthread)
	if (DBUS_FOUND)
		include_directories("/usr/lib64/dbus-1.0/include") # Bazzite (and probably Fedora 42) keeps the platform-specific dbus headers in this folder
		set(ZWIDGET_SOURCES ${ZWIDGET_SOURCES} ${ZWIDGET_DBUS_SOURCES})
		set(ZWIDGET_INCLUDE_DIRS ${ZWIDGET_INCLUDE_DIRS} ${DBUS_INCLUDE_DIRS})
		set(ZWIDGET_LIBS ${ZWIDGET_LIBS} ${DBUS_LDFLAGS})
		set(ZWIDGET_DEFINES ${ZWIDGET_DEFINES} -DUSE_DBUS)
	endif()
	if (WAYLANDPP_FOUND)
		set(ZWIDGET_SOURCES ${ZWIDGET_SOURCES} ${ZWIDGET_WAYLAND_SOURCES})
		set(ZWIDGET_INCLUDE_DIRS ${ZWIDGET_INCLUDE_DIRS} ${WAYLANDPP_INCLUDE_DIRS})
		set(ZWIDGET_LIBS ${ZWIDGET_LIBS} ${WAYLANDPP_LDFLAGS})
		set(ZWIDGET_DEFINES ${ZWIDGET_DEFINES} -DUSE_WAYLAND)
	endif()

	set(ZWIDGET_INCLUDE_DIRS ${ZWIDGET_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
	set(ZWIDGET_LIBS ${ZWIDGET_LIBS} ${GLIB_LDFLAGS} -lgio-2.0)

	set(ZWIDGET_INCLUDE_DIRS ${ZWIDGET_INCLUDE_DIRS} ${FONTCONFIG_INCLUDE_DIRS})
	set(ZWIDGET_LIBS ${ZWIDGET_LIBS} ${FONTCONFIG_LDFLAGS})
endif()

if(SDL3_FOUND)
	set(ZWIDGET_SOURCES ${ZWIDGET_SOURCES} ${ZWIDGET_SDL3_SOURCES})
	set(ZWIDGET_DEFINES ${ZWIDGET_DEFINES} -DUSE_SDL3)
endif()

if(SDL2_FOUND AND NOT SDL3_FOUND)
	set(ZWIDGET_SOURCES ${ZWIDGET_SOURCES} ${ZWIDGET_SDL2_SOURCES})
	set(ZWIDGET_DEFINES ${ZWIDGET_DEFINES} -DUSE_SDL2)
endif()

if(MSVC)
	set(CXX_WARNING_FLAGS /W3)
else()
	set(CXX_WARNING_FLAGS -Wall -Wpedantic)
endif()

add_library(zwidget STATIC ${ZWIDGET_SOURCES} ${ZWIDGET_INCLUDES})
target_compile_options(zwidget PRIVATE ${ZWIDGET_COMPILE_OPTIONS})
target_compile_definitions(zwidget PRIVATE ${ZWIDGET_DEFINES})
target_include_directories(zwidget PRIVATE ${ZWIDGET_INCLUDE_DIRS})
target_link_options(zwidget PRIVATE ${ZWIDGET_LINK_OPTIONS})
if(APPLE)
	target_link_libraries(zwidget PRIVATE ${ZWIDGET_LIBS} ${COCOA_FRAMEWORK} ${COREVIDEO_FRAMEWORK} ${UNIFORMTYPEIDENTIFIERS_FRAMEWORK} ${QUARTZCORE_FRAMEWORK})
	if(ENABLE_OPENGL AND OPENGL_FRAMEWORK)
		target_link_libraries(zwidget PRIVATE ${OPENGL_FRAMEWORK})
		target_compile_definitions(zwidget PRIVATE HAVE_OPENGL)
	endif()
	if(ENABLE_METAL AND METAL_FRAMEWORK)
		target_link_libraries(zwidget PRIVATE ${METAL_FRAMEWORK} ${QUARTZCORE_FRAMEWORK})
		target_compile_definitions(zwidget PRIVATE HAVE_METAL)
	endif()
else()
	target_link_libraries(zwidget PRIVATE ${ZWIDGET_LIBS})
endif()
if(SDL3_FOUND)
	target_include_directories(zwidget PRIVATE ${SDL3_INCLUDE_DIRS})
	if(TARGET SDL3::SDL3)
		target_link_libraries(zwidget PRIVATE SDL3::SDL3)
	else()
		target_link_libraries(zwidget PRIVATE ${SDL3_LIBRARY})
	endif()
endif()
if(SDL2_FOUND AND NOT SDL3_FOUND)
	target_include_directories(zwidget PRIVATE ${SDL2_INCLUDE_DIRS})
	if(TARGET SDL2::SDL2)
		target_link_libraries(zwidget PRIVATE SDL2::SDL2)
	else()
		target_link_libraries(zwidget PRIVATE ${SDL2_LIBRARY})
	endif()
endif()
set_target_properties(zwidget PROPERTIES CXX_STANDARD 20)
target_compile_options(zwidget PRIVATE ${CXX_WARNING_FLAGS})

if(MSVC)
	set_property(TARGET zwidget PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

option(ZWIDGET_BUILD_EXAMPLE "Build the zwidget example application" ON)

if(ZWIDGET_BUILD_EXAMPLE)
	source_group("example" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/example/.+")
	add_executable(zwidget_example WIN32
		example/example.cpp
		example/stylesheet.h
	)
	target_compile_options(zwidget_example PRIVATE ${CXX_WARNING_FLAGS})

	add_custom_command(
		TARGET zwidget_example POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			"${CMAKE_CURRENT_SOURCE_DIR}/example/banner.png"
			"${CMAKE_CURRENT_BINARY_DIR}/banner.png"
		COMMAND ${CMAKE_COMMAND} -E copy
			"${CMAKE_CURRENT_SOURCE_DIR}/example/OpenSans.ttf"
			"${CMAKE_CURRENT_BINARY_DIR}/OpenSans.ttf"
	)

	target_include_directories(zwidget_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/example)
	target_link_libraries(zwidget_example PRIVATE zwidget)

	if(WIN32)
		target_compile_definitions(zwidget_example PRIVATE UNICODE _UNICODE)
		target_link_libraries(zwidget_example PRIVATE gdi32 user32 shell32 comdlg32)
	elseif(APPLE)
		target_link_libraries(zwidget_example PRIVATE "-framework Cocoa" "-framework CoreVideo")
		if(ENABLE_OPENGL AND OPENGL_FRAMEWORK)
			target_link_libraries(zwidget_example PRIVATE "-framework OpenGL")
		endif()
		if(ENABLE_METAL AND METAL_FRAMEWORK)
			target_link_libraries(zwidget_example PRIVATE "-framework Metal" "-framework QuartzCore")
		endif()
	else()
		target_link_libraries(zwidget_example PRIVATE ${ZWIDGET_LIBS})
	endif()

	if(SDL3_FOUND)
		target_link_libraries(zwidget_example PRIVATE SDL3::SDL3)
	endif()

	if(SDL2_FOUND AND NOT SDL3_FOUND)
		target_link_libraries(zwidget_example PRIVATE SDL2::SDL2)
	endif()

	set_target_properties(zwidget_example PROPERTIES CXX_STANDARD 20)

	if(MSVC)
		set_property(TARGET zwidget_example PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()
endif()
