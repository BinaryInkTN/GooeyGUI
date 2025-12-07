cmake_minimum_required(VERSION 3.10)
project(GooeyGUI VERSION 1.0.2 LANGUAGES C)

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 2)

include(FetchContent)
include(CMakePackageConfigHelpers)

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
# Preprocessor compilation FLAGS: 
option(GLES_ON "Enable GLES3" ON)
if(GLES_ON)
    add_definitions(-DGLES_ON=1)
else()
    add_definitions(-DGLES_ON=0)
endif()
option(ENABLE_ASAN "Enable AddressSanitizer for Debug builds" OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
endif()

# Add AddressSanitizer flags for Debug builds if enabled
if(ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer -g")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer -g")
        set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
        message(STATUS "AddressSanitizer enabled for Debug build")
    else()
        message(WARNING "AddressSanitizer is not supported by the current compiler (${CMAKE_C_COMPILER_ID})")
    endif()
endif()

# ----------------------------
# Dependencies: cJSON, GLPS, FreeType
# ----------------------------
option(USE_BUNDLED_FREETYPE "Use bundled Freetype instead of system package" ON)

# Freetype handling - FIXED
if(USE_BUNDLED_FREETYPE)
    message(STATUS "Using bundled Freetype (forced)")
    add_subdirectory(third_party/freetype)
    # Create consistent target name for bundled version
    if(NOT TARGET Freetype::Freetype AND TARGET freetype)
        add_library(Freetype::Freetype ALIAS freetype)
    endif()
    set(FREETYPE_TARGET Freetype::Freetype)
    set(FREETYPE_INCLUDE_DIRS 
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/freetype/include
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/freetype/include
    )
else()
    # Try to find system Freetype first
    find_package(Freetype QUIET)
    
    if(Freetype_FOUND)
        message(STATUS "Using system Freetype")
        set(FREETYPE_TARGET Freetype::Freetype)
        set(FREETYPE_INCLUDE_DIRS ${Freetype_INCLUDE_DIRS})
    else()
        message(STATUS "System Freetype not found, using bundled version")
        add_subdirectory(third_party/freetype)
        # Create consistent target name for bundled version
        if(NOT TARGET Freetype::Freetype AND TARGET freetype)
            add_library(Freetype::Freetype ALIAS freetype)
        endif()
        set(FREETYPE_TARGET Freetype::Freetype)
        set(FREETYPE_INCLUDE_DIRS 
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/freetype/include
            ${CMAKE_CURRENT_BINARY_DIR}/third_party/freetype/include
        )
    endif()
endif()

add_subdirectory(third_party/cjson)
add_subdirectory(third_party/glps)
add_subdirectory(third_party/native_fdialog)

# Apply ASan flags to dependencies if enabled
if(ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cjson PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(cjson PRIVATE -fsanitize=address)
    target_compile_options(GLPS PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(GLPS PRIVATE -fsanitize=address)
    if(TARGET freetype)
        target_compile_options(freetype PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(freetype PRIVATE -fsanitize=address)
    endif()
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
    src/widgets/gooey_window_internal.c
    internal/backends/utils/backend_utils_internal.c
    src/backends/glps_backend_internal.c
    src/core/gooey_event.c
    #src/backends/glps_vk_backend_internal.c
    src/logger/pico_logger_internal.c
    src/widgets/gooey_meter_internal.c
    src/widgets/gooey_notifications_internal.c
    src/widgets/gooey_notifications.c
    src/widgets/gooey_webview.c
    src/widgets/gooey_webview_internal.c
    src/widgets/gooey_fdialog.c
    src/widgets/gooey_node_editor.c
    src/widgets/gooey_node_editor_internal.c
    src/widgets/gooey_ctxmenu_internal.c
    src/widgets/gooey_ctxmenu.c
    src/animations/gooey_animations.c
    src/animations/gooey_animations_internal.c
    
)
if (NOT GLES_ON)
set(SOURCES ${SOURCES}
    internal/backends/utils/glad/glad.c
)
endif()
# ----------------------------
# Main library
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

# Set RPATH for Unix-like systems
if(UNIX AND NOT APPLE)
    set_target_properties(GooeyGUI PROPERTIES
        BUILD_RPATH "\$ORIGIN"
        INSTALL_RPATH "\$ORIGIN"
    )
elseif(APPLE)
    set_target_properties(GooeyGUI PROPERTIES
        BUILD_RPATH "@loader_path"
        INSTALL_RPATH "@loader_path"
    )
endif()

# Include directories
# ----------------------------
target_include_directories(GooeyGUI PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(GooeyGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cjson
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glps/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glps/internal
    ${FREETYPE_INCLUDE_DIRS}  # Use the variable we set above
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/native_fdialog/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/native_fdialog/src/include
)

# ----------------------------
# Preprocessor flags 
# ----------------------------
target_compile_definitions(GooeyGUI PRIVATE GLES_ON=$<BOOL:${GLES_ON}>)


# ----------------------------
# Link dependencies 
# ----------------------------
target_link_libraries(GooeyGUI PRIVATE
    cjson
    GLPS
    ${FREETYPE_TARGET}  
    nfd
)

# ----------------------------
# Copy shared libraries to output directory
# ----------------------------
function(copy_shared_library target lib_target)
    get_target_property(lib_type ${lib_target} TYPE)
    if(lib_type STREQUAL "SHARED_LIBRARY")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${lib_target}>
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            COMMENT "Copying ${lib_target} to output directory"
        )
    endif()
endfunction()

copy_shared_library(GooeyGUI nfd)
copy_shared_library(GooeyGUI cjson)
# Only copy freetype if we're using the bundled version
if(NOT Freetype_FOUND OR USE_BUNDLED_FREETYPE)
    copy_shared_library(GooeyGUI freetype)
endif()
copy_shared_library(GooeyGUI GLPS)

# ----------------------------
# System libraries
# ----------------------------
if(WIN32)
    target_link_libraries(GooeyGUI PRIVATE gdi32 user32 kernel32 opengl32)
else()
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    target_link_libraries(GooeyGUI PRIVATE OpenGL::GL pthread dl X11::X11)
endif()

# ----------------------------
# Make all dependencies exportable
# ----------------------------
set_target_properties(cjson PROPERTIES EXPORT_NAME cjson)
set_target_properties(GLPS PROPERTIES EXPORT_NAME GLPS)
# Only set properties for freetype if we built it
if(NOT Freetype_FOUND OR USE_BUNDLED_FREETYPE)
    set_target_properties(freetype PROPERTIES EXPORT_NAME freetype)
endif()
set_target_properties(nfd PROPERTIES EXPORT_NAME native_fdialog)

# ----------------------------
# Install rules 
# ----------------------------
include(GNUInstallDirs)

# Always install our main library and dependencies we built
install(TARGETS GooeyGUI cjson GLPS nfd
    EXPORT GooeyGUITargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Only install bundled freetype if we built it
if(NOT Freetype_FOUND OR USE_BUNDLED_FREETYPE)
    install(TARGETS freetype
        EXPORT GooeyGUITargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Gooey)

# ----------------------------
# Package config
# ----------------------------
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