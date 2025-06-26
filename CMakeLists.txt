cmake_minimum_required(VERSION 3.10)
project(GooeyGUI VERSION 1.0 LANGUAGES C)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build type and compiler flags
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
endif()

# Include dependencies
add_subdirectory(third_party/cjson)
add_subdirectory(third_party/glps)
add_subdirectory(third_party/freetype)

# TFT_eSPI configuration
option(ENABLE_TFT_ESPI "Enable TFT_eSPI backend" OFF)
if(ENABLE_TFT_ESPI)
    add_definitions(-DTFT_ESPI_ENABLED)
    find_library(TFT_ESPI_LIB TFT_eSPI)
    find_library(SPI_LIB SPI)

    if(NOT TFT_ESPI_LIB OR NOT SPI_LIB)
        message(WARNING "TFT_eSPI libraries not found - disabling TFT backend")
        set(ENABLE_TFT_ESPI OFF)
    else()
        message(STATUS "TFT_eSPI backend enabled")
        list(APPEND EXTRA_LIBS ${TFT_ESPI_LIB} ${SPI_LIB})
    endif()
endif()

set(SOURCES
        src/core/gooey.c
        src/core/gooey_window.c
        src/core/gooey_widget.c
        src/core/gooey_timers.c
        src/theme/gooey_theme.c
        src/widgets/gooey_drop_surface.c
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
        internal/core/gooey_timers_internal.c
        internal/widgets/gooey_appbar_internal.c
        internal/core/gooey_widget_internal.c
        internal/widgets/gooey_layout_internal.c
        internal/widgets/gooey_drop_surface_internal.c
        internal/widgets/gooey_button_internal.c
        internal/widgets/gooey_canvas_internal.c
        internal/widgets/gooey_checkbox_internal.c
        internal/widgets/gooey_dropdown_internal.c
        internal/widgets/gooey_label_internal.c
        internal/widgets/gooey_list_internal.c
        internal/widgets/gooey_menu_internal.c
        internal/widgets/gooey_radiobutton_internal.c
        internal/widgets/gooey_slider_internal.c
        internal/widgets/gooey_textbox_internal.c
        internal/widgets/gooey_plot_internal.c
        internal/widgets/gooey_image_internal.c
        internal/widgets/gooey_tabs_internal.c
        internal/widgets/gooey_container_internal.c
        internal/widgets/gooey_progressbar_internal.c
        internal/widgets/gooey_debug_overlay_internal.c
        internal/backends/utils/glad/glad.c
        internal/widgets/gooey_window_internal.c
        internal/backends/utils/backend_utils_internal.c
        internal/backends/glps_backend_internal.c
        internal/logger/pico_logger_internal.c
        internal/widgets/gooey_meter_internal.c
        internal/backends/tft_backend_internal.c
)

# Conditionally add TFT backend sources
if(ENABLE_TFT_ESPI)
    list(APPEND SOURCES
            backends/tft_espi/tft_espi.c
    )
endif()

# Main library target
add_library(GooeyGUI SHARED ${SOURCES})

# Include directories
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
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tft_espi/
)

# Link dependencies
target_link_libraries(GooeyGUI PRIVATE
        cjson
        GLPS
        freetype
        ${EXTRA_LIBS}
)

if(WIN32)
    target_link_libraries(GooeyGUI PRIVATE
            gdi32
            user32
            kernel32
    )
else()
    target_link_libraries(GooeyGUI PRIVATE
            GLESv2
    )
endif()

# Installation
install(TARGETS GooeyGUI
        EXPORT GooeyGUITargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include/Gooey)

# Generate package config
include(CMakePackageConfigHelpers)
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GooeyGUIConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/GooeyGUIConfig.cmake
        INSTALL_DESTINATION lib/cmake/GooeyGUI
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/GooeyGUIConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

install(EXPORT GooeyGUITargets
        FILE GooeyGUITargets.cmake
        NAMESPACE GooeyGUI::
        DESTINATION lib/cmake/GooeyGUI
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/GooeyGUIConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/GooeyGUIConfigVersion.cmake
        DESTINATION lib/cmake/GooeyGUI
)