cmake_minimum_required(VERSION 3.10)
project(GooeyGUI VERSION 1.0.2 LANGUAGES C)
# set project version 
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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
endif()

# ----------------------------
# Dependencies: cJSON, GLPS, FreeType
# ----------------------------
add_subdirectory(third_party/cjson)
add_subdirectory(third_party/glps)
add_subdirectory(third_party/freetype)

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
        src/widgets/gooey_messagebox.c
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
        internal/backends/utils/glad/glad.c
        src/widgets/gooey_window_internal.c
        internal/backends/utils/backend_utils_internal.c
        src/backends/glps_backend_internal.c
        src/logger/pico_logger_internal.c
        src/widgets/gooey_meter_internal.c
        src/widgets/gooey_webview.c
        src/widgets/gooey_webview_internal.c
)

# ----------------------------
# Main library
# ----------------------------
add_library(GooeyGUI SHARED ${SOURCES})

# Set library properties 
set_target_properties(GooeyGUI PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "GooeyGUI-${PROJECT_VERSION_MAJOR}"
)    
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
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/freetype/include
)

# ----------------------------
# Link dependencies
# ----------------------------
target_link_libraries(GooeyGUI PRIVATE
    cjson
    GLPS
    freetype
)

# ----------------------------
# Ultralight Configuration
# ----------------------------
set(ULTRALIGHT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ultralight/include)
set(ULTRALIGHT_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ultralight/bin)

# Check if Ultralight directory exists
if(EXISTS ${ULTRALIGHT_INCLUDE_DIR} AND EXISTS ${ULTRALIGHT_BIN_DIR})
    message(STATUS "Ultralight found, enabling webview support")
    
    target_include_directories(GooeyGUI PRIVATE ${ULTRALIGHT_INCLUDE_DIR})
     set(ULTRALIGHT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ultralight/resources)
    if(EXISTS ${ULTRALIGHT_RESOURCES_DIR})
        add_custom_command(TARGET GooeyGUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${ULTRALIGHT_RESOURCES_DIR}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources
        )
        message(STATUS "Ultralight resources will be copied to output directory")
    endif()
    if(WIN32)
        # Windows configuration
        target_link_libraries(GooeyGUI PRIVATE
            ${ULTRALIGHT_BIN_DIR}/Ultralight.lib
            ${ULTRALIGHT_BIN_DIR}/UltralightCore.lib
            ${ULTRALIGHT_BIN_DIR}/AppCore.lib
            ${ULTRALIGHT_BIN_DIR}/WebCore.lib
        )
        
        # Copy DLLs to output directory
        add_custom_command(TARGET GooeyGUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${ULTRALIGHT_BIN_DIR}/Ultralight.dll
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
        add_custom_command(TARGET GooeyGUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${ULTRALIGHT_BIN_DIR}/UltralightCore.dll
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
        add_custom_command(TARGET GooeyGUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${ULTRALIGHT_BIN_DIR}/AppCore.dll
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
        add_custom_command(TARGET GooeyGUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${ULTRALIGHT_BIN_DIR}/WebCore.dll
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
        
    elseif(UNIX AND NOT APPLE)
        # Linux configuration
        target_link_libraries(GooeyGUI PRIVATE
            ${ULTRALIGHT_BIN_DIR}/libUltralight.so
            ${ULTRALIGHT_BIN_DIR}/libUltralightCore.so
            ${ULTRALIGHT_BIN_DIR}/libAppCore.so
            ${ULTRALIGHT_BIN_DIR}/libWebCore.so
        )
        
        # Set RPATH for runtime
        set_target_properties(GooeyGUI PROPERTIES
            BUILD_RPATH "${ULTRALIGHT_BIN_DIR}"
            INSTALL_RPATH "${ULTRALIGHT_BIN_DIR}"
        )
        
    elseif(APPLE)
        # macOS configuration
        target_link_libraries(GooeyGUI PRIVATE
            ${ULTRALIGHT_BIN_DIR}/libUltralight.dylib
            ${ULTRALIGHT_BIN_DIR}/libUltralightCore.dylib
            ${ULTRALIGHT_BIN_DIR}/libAppCore.dylib
            ${ULTRALIGHT_BIN_DIR}/libWebCore.dylib
        )
        
        # Set RPATH for macOS
        set_target_properties(GooeyGUI PROPERTIES
            BUILD_RPATH "@loader_path/../Frameworks"
            INSTALL_RPATH "@loader_path/../Frameworks"
        )
    endif()
    
    # Add Ultralight definition
    target_compile_definitions(GooeyGUI PRIVATE -DULTRALIGHT_ENABLED=1)
else()
    message(WARNING "Ultralight not found, webview support will be disabled")
    target_compile_definitions(GooeyGUI PRIVATE -DULTRALIGHT_ENABLED=0)
endif()

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
# Install rules
# ----------------------------
include(GNUInstallDirs)

install(TARGETS GooeyGUI
        EXPORT GooeyGUITargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ----------------------------
# Package config
# ----------------------------
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GooeyGUIConfig.cmake.in
        ${CMAKE_BINARY_DIR}/GooeyGUIConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GooeyGUI
)

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