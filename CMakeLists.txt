cmake_minimum_required(VERSION 3.10)

# Set project info
project(GooeyGUI VERSION 1.0 LANGUAGES C)




# Output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter")

if(WIN32)
    message(STATUS "Compiling on Windows")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
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
    src/widgets/gooey_meter.c
    src/signals/gooey_signals.c
    src/animations/gooey_animations.c
    internal/core/gooey_timers_internal.c
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
    internal/widgets/gooey_progressbar_internal.c
    internal/widgets/gooey_debug_overlay_internal.c
        internal/backends/utils/glad/glad.c
    internal/widgets/gooey_window_internal.c
    internal/backends/utils/backend_utils_internal.c
    internal/backends/glps_backend_internal.c
    internal/logger/pico_logger_internal.c
    internal/widgets/gooey_meter_internal.c
)


add_subdirectory(third_party/cjson)
add_subdirectory(third_party/glps)
add_subdirectory(third_party/freetype)

add_dependencies(cjson GLPS freetype)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/Gooey>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/internal
        ${PROJECT_SOURCE_DIR}/third_party/freetype/include
        ${PROJECT_SOURCE_DIR}/third_party/glps/include
        ${PROJECT_SOURCE_DIR}/third_party/glps/internal
        ${PROJECT_SOURCE_DIR}/third_party/cjson
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            cjson
            GLPS
            freetype
            #glad
            gdi32
            user32
            kernel32
    )
else()
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            cjson
            GLPS
            freetype
            GLESv2
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include/Gooey FILES_MATCHING PATTERN "*.h")

install(TARGETS ${PROJECT_NAME}
    EXPORT GooeyGUIConfig
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(EXPORT GooeyGUIConfig
    FILE GooeyGUIConfig.cmake
    NAMESPACE GooeyGUI::
    DESTINATION lib/cmake/GooeyGUI
)

enable_testing()
