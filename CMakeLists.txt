cmake_minimum_required(VERSION 3.16)
project(GooeyGUI VERSION 1.0.2 LANGUAGES C)

# Set project version
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 2)

include(FetchContent)
include(CMakePackageConfigHelpers)
include(CheckIncludeFile)
include(CheckFunctionExists)

# ----------------------------
# Platform detection
# ----------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

# ----------------------------
# Output directories
# ----------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ----------------------------
# Build type and compiler flags
# ----------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Platform-specific options
option(ENABLE_WAYLAND "Enable Wayland support on Linux" ON)
option(ENABLE_X11 "Enable X11 support on Linux" ON)
option(ENABLE_ASAN "Enable AddressSanitizer for Debug builds" OFF)
set(BUILD_TESTING OFF)

# Position Independent Code (required for shared libraries on most platforms)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Common compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DDEBUG")
    endif()
endif()

# Windows-specific flags
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WIN32_WINNT=0x0601)
    if(MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi")
    endif()
endif()

# Linux-specific flags
if(LINUX)
    add_definitions(-D_POSIX_C_SOURCE=200112L)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEFAULT_SOURCE")
endif()

# Add AddressSanitizer flags for Debug builds if enabled
if(ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
        message(STATUS "AddressSanitizer enabled for Debug build")
    else()
        message(WARNING "AddressSanitizer is not supported by the current compiler (${CMAKE_C_COMPILER_ID})")
    endif()
endif()

# ----------------------------
# Dependencies configuration - DYNAMIC LINKING
# ----------------------------
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)

# Allow dependencies to build as shared libraries
set(CJSON_BUILD_SHARED_LIBS ON CACHE BOOL "Build cJSON shared library" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)

# Freetype configuration
set(FT_DISABLE_ZLIB ON CACHE BOOL "Disable ZLIB for freetype" FORCE)
set(FT_REQUIRE_ZLIB OFF CACHE BOOL "Require ZLIB for freetype" FORCE)

# ----------------------------
# System dependencies detection
# ----------------------------
if(WIN32)
    find_package(OpenGL REQUIRED)
else()
    find_package(OpenGL REQUIRED)
    find_package(PkgConfig REQUIRED)

    # X11 detection
    if(ENABLE_X11)
        find_package(X11 REQUIRED)
        if(X11_FOUND)
            message(STATUS "X11 support enabled")
            add_definitions(-DGOOEY_HAS_X11)
        else()
            message(WARNING "X11 not found but ENABLE_X11 is ON")
        endif()
    endif()

    # Wayland detection
    if(ENABLE_WAYLAND)
        pkg_check_modules(WAYLAND_CLIENT QUIET wayland-client)
        pkg_check_modules(WAYLAND_EGL QUIET wayland-egl)
        pkg_check_modules(WAYLAND_CURSOR QUIET wayland-cursor)

        if(WAYLAND_CLIENT_FOUND AND WAYLAND_EGL_FOUND AND WAYLAND_CURSOR_FOUND)
            message(STATUS "Wayland support enabled")
            add_definitions(-DGOOEY_HAS_WAYLAND)
            set(WAYLAND_FOUND TRUE)
        else()
            message(WARNING "Wayland not found but ENABLE_WAYLAND is ON")
            set(WAYLAND_FOUND FALSE)
        endif()
    endif()

    # Additional Linux libraries
    find_library(LIBDL dl)
    find_library(LIBRT rt)
    find_library(LIBPTHREAD pthread)
    find_library(LIBM m)
endif()

# ----------------------------
# Dependencies: cJSON, GLPS, FreeType - now build as SHARED
# ----------------------------
add_subdirectory(third_party/cjson)
add_subdirectory(third_party/glps)
add_subdirectory(third_party/freetype)
add_subdirectory(third_party/native_fdialog)

# Verify that dependencies are built as shared
get_target_property(cjson_type cjson TYPE)
get_target_property(glps_type GLPS TYPE)
get_target_property(freetype_type freetype TYPE)
get_target_property(nfd_type nfd TYPE)

message(STATUS "cjson build type: ${cjson_type}")
message(STATUS "GLPS build type: ${glps_type}")
message(STATUS "freetype build type: ${freetype_type}")
message(STATUS "nfd build type: ${nfd_type}")

# Apply ASan flags to dependencies ONLY if ASan is enabled
if(ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cjson PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(cjson PRIVATE -fsanitize=address)
    target_compile_options(GLPS PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(GLPS PRIVATE -fsanitize=address)
    target_compile_options(freetype PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(freetype PRIVATE -fsanitize=address)
    target_compile_options(nfd PRIVATE -fsanitize=address -fno-omit-frame-pointer -DNFD_PORTAL=1)
    target_link_options(nfd PRIVATE -fsanitize=address)
endif()

# ----------------------------
# Sources
# ----------------------------
set(SOURCES
        src/core/gooey.c
        src/core/gooey_window.c
        src/core/gooey_widget.c
        src/core/gooey_timers.c
        src/theme/gooey_theme.c
        src/widgets/gooey_drop_surface.c
        src/widgets/gooey_switch.c
        src/widgets/gooey_button.c
        src/widgets/gooey_canvas.c
        src/widgets/gooey_checkbox.c
        src/widgets/gooey_dropdown.c
        #src/widgets/gooey_messagebox.c
        src/widgets/gooey_label.c
        src/widgets/gooey_layout.c
        src/widgets/gooey_list.c
        src/widgets/gooey_menu.c
        src/widgets/gooey_radiobutton.c
        src/widgets/gooey_slider.c
        src/widgets/gooey_textbox.c
        src/widgets/gooey_plot.c
        src/widgets/gooey_progressbar.c
        src/widgets/gooey_image.c
        src/widgets/gooey_tabs.c
        src/widgets/gooey_container.c
        src/widgets/gooey_meter.c
        src/signals/gooey_signals.c
        src/animations/gooey_animations.c
        src/animations/gooey_animations_internal.c
        src/widgets/gooey_appbar.c
        src/core/gooey_timers_internal.c
        src/widgets/gooey_appbar_internal.c
        src/core/gooey_widget_internal.c
        src/widgets/gooey_layout_internal.c
        src/widgets/gooey_drop_surface_internal.c
        src/widgets/gooey_button_internal.c
        src/widgets/gooey_canvas_internal.c
        src/widgets/gooey_checkbox_internal.c
        src/widgets/gooey_dropdown_internal.c
        src/widgets/gooey_label_internal.c
        src/widgets/gooey_switch_internal.c
        src/widgets/gooey_list_internal.c
        src/widgets/gooey_menu_internal.c
        src/widgets/gooey_radiobutton_internal.c
        src/widgets/gooey_slider_internal.c
        src/widgets/gooey_textbox_internal.c
        src/widgets/gooey_plot_internal.c
        src/widgets/gooey_image_internal.c
        src/widgets/gooey_tabs_internal.c
        src/widgets/gooey_container_internal.c
        src/widgets/gooey_progressbar_internal.c
        src/widgets/gooey_debug_overlay_internal.c
        src/virtual/gooey_keyboard_internal.c
        internal/backends/utils/glad/glad.c
        src/widgets/gooey_window_internal.c
        internal/backends/utils/backend_utils_internal.c
        src/backends/glps_backend_internal.c
        src/logger/pico_logger_internal.c
        src/widgets/gooey_meter_internal.c
        src/widgets/gooey_webview.c
        src/widgets/gooey_webview_internal.c
        src/widgets/gooey_fdialog.c
        src/widgets/gooey_ctxmenu.c
        src/widgets/gooey_ctxmenu_internal.c
)

# ----------------------------
# Main library - SHARED LIBRARY with dynamic dependencies
# ----------------------------
add_library(GooeyGUI SHARED ${SOURCES})

# Set library properties with RPATH
set_target_properties(GooeyGUI PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "GooeyGUI-${PROJECT_VERSION_MAJOR}"
        BUILD_WITH_INSTALL_RPATH OFF
        SKIP_BUILD_RPATH OFF
)

# Set RPATH for Unix-like systems to handle dynamic dependencies
if(UNIX AND NOT APPLE)
    set_target_properties(GooeyGUI PROPERTIES
            BUILD_RPATH "\$ORIGIN:\$ORIGIN/../lib"
            INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib"
    )
elseif(APPLE)
    set_target_properties(GooeyGUI PROPERTIES
            BUILD_RPATH "@loader_path @loader_path/../lib"
            INSTALL_RPATH "@loader_path @loader_path/../lib"
    )
endif()

# Include directories
target_include_directories(GooeyGUI PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/internal
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cjson
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glps/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glps/internal
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/freetype/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/native_fdialog/src/
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/native_fdialog/src/include
)

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(GooeyGUI PRIVATE GOOEY_PLATFORM_WINDOWS)
elseif(LINUX)
    target_compile_definitions(GooeyGUI PRIVATE GOOEY_PLATFORM_LINUX)
    if(ENABLE_X11 AND X11_FOUND)
        target_compile_definitions(GooeyGUI PRIVATE GOOEY_HAS_X11)
    endif()
    if(ENABLE_WAYLAND AND WAYLAND_FOUND)
        target_compile_definitions(GooeyGUI PRIVATE GOOEY_HAS_WAYLAND)
    endif()
endif()

# ----------------------------
# Link dependencies DYNAMICALLY
# ----------------------------
target_link_libraries(GooeyGUI PRIVATE
        cjson
        GLPS
        freetype
        nfd
)

# ----------------------------
# System libraries
# ----------------------------
if(WIN32)
    target_link_libraries(GooeyGUI PRIVATE
            OpenGL::GL
            gdi32
            user32
            kernel32
            opengl32
            shell32  # For file dialogs
            comctl32 # For common controls
            dwmapi   # For desktop window manager
    )
else()
    target_link_libraries(GooeyGUI PRIVATE
            OpenGL::GL
            pthread
            dl
            ${LIBM}
    )

    # X11 libraries
    if(ENABLE_X11 AND X11_FOUND)
        target_link_libraries(GooeyGUI PRIVATE
                ${X11_LIBRARIES}
                ${X11_X11_LIB}
                ${X11_xcb_LIB}
        )
        target_include_directories(GooeyGUI PRIVATE ${X11_INCLUDE_DIR})
    endif()

    # Wayland libraries
    if(ENABLE_WAYLAND AND WAYLAND_FOUND)
        target_link_libraries(GooeyGUI PRIVATE
                ${WAYLAND_CLIENT_LIBRARIES}
                ${WAYLAND_EGL_LIBRARIES}
                ${WAYLAND_CURSOR_LIBRARIES}
        )
        target_include_directories(GooeyGUI PRIVATE
                ${WAYLAND_CLIENT_INCLUDE_DIRS}
                ${WAYLAND_EGL_INCLUDE_DIRS}
                ${WAYLAND_CURSOR_INCLUDE_DIRS}
        )
    endif()
endif()

# ----------------------------
# Install rules - Install both main library AND its dependencies
# ----------------------------
include(GNUInstallDirs)

# Install the main GooeyGUI library
install(TARGETS GooeyGUI
        EXPORT GooeyGUITargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install the dependency shared libraries with export sets
install(TARGETS cjson
        EXPORT GooeyGUITargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS GLPS
        EXPORT GooeyGUITargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GLPS
)

install(TARGETS freetype
        EXPORT GooeyGUITargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS nfd
        EXPORT GooeyGUITargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nfd
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ----------------------------
# Package config
# ----------------------------
# Create a simple config file if the .in file doesn't exist
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GooeyGUIConfig.cmake.in)
    file(WRITE ${CMAKE_BINARY_DIR}/GooeyGUIConfig.cmake
            "@PACKAGE_INIT@\n\n"
            "include(\"\${CMAKE_CURRENT_LIST_DIR}/GooeyGUITargets.cmake\")\n"
            "check_required_components(GooeyGUI)\n"
    )
    message(STATUS "Created basic GooeyGUIConfig.cmake")
else()
    configure_package_config_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GooeyGUIConfig.cmake.in
            ${CMAKE_BINARY_DIR}/GooeyGUIConfig.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GooeyGUI
    )
endif()

write_basic_package_version_file(
        ${CMAKE_BINARY_DIR}/GooeyGUIConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

install(EXPORT GooeyGUITargets
        FILE GooeyGUITargets.cmake
        NAMESPACE GooeyGUI::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GooeyGUI
)

install(FILES
        ${CMAKE_BINARY_DIR}/GooeyGUIConfig.cmake
        ${CMAKE_BINARY_DIR}/GooeyGUIConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GooeyGUI
)

# ----------------------------
# Post-build steps for Windows (optional: copy DLLs to output directory)
# ----------------------------
if(WIN32)
    add_custom_command(TARGET GooeyGUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:cjson>
            $<TARGET_FILE_DIR:GooeyGUI>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:GLPS>
            $<TARGET_FILE_DIR:GooeyGUI>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:freetype>
            $<TARGET_FILE_DIR:GooeyGUI>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:nfd>
            $<TARGET_FILE_DIR:GooeyGUI>
            COMMENT "Copying dependency DLLs to output directory"
    )
endif()

# ----------------------------
# Summary
# ----------------------------
message(STATUS "")
message(STATUS "GooeyGUI Configuration Summary:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Linking: DYNAMIC")
if(LINUX)
    message(STATUS "  X11 support: ${ENABLE_X11}")
    message(STATUS "  Wayland support: ${ENABLE_WAYLAND}")
endif()
message(STATUS "  Address Sanitizer: ${ENABLE_ASAN}")
message(STATUS "")