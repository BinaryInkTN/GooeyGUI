cmake_minimum_required(VERSION 3.10)

project(GooeyGUI VERSION 1.0 LANGUAGES C)

# Set output directories for libraries and executables
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# Source files
set(SOURCES
    src/core/gooey.c
    src/core/gooey_window.c
    src/utils/logger/gooey_logger.c
    src/theme/gooey_theme.c
    src/utils/glad/glad.c
    src/utils/backends/backend_utils.c
    src/backends/glps_backend.c
    src/widgets/gooey_drop_surface.c
    src/widgets/gooey_button.c
    src/widgets/gooey_canvas.c
    src/widgets/gooey_checkbox.c
    src/widgets/gooey_dropdown.c
    src/widgets/gooey_label.c
    src/widgets/gooey_layout.c
    src/widgets/gooey_list.c
    src/widgets/gooey_menu.c
    src/widgets/gooey_messagebox.c
    src/widgets/gooey_radiobutton.c
    src/widgets/gooey_slider.c
    src/widgets/gooey_textbox.c
    src/widgets/gooey_plot.c
    src/widgets/gooey_image.c
    src/signals/gooey_signals.c
)

# Header files
set(HEADERS
    include/core/gooey_window.h
    include/core/gooey_backend.h
    include/utils/backends/backend_utils.h
    include/utils/glad/glad.h
    include/utils/linmath/linmath.h
    include/utils/logger/gooey_logger.h
    include/utils/stb_image/stb_image.h
    include/theme/gooey_theme.h
    include/event/gooey_event.h
    include/common/gooey_common.h
    include/widgets/gooey_drop_surface.h
    include/widgets/gooey_button.h
    include/widgets/gooey_canvas.h
    include/widgets/gooey_checkbox.h
    include/widgets/gooey_dropdown.h
    include/widgets/gooey_label.h
    include/widgets/gooey_layout.h
    include/widgets/gooey_list.h
    include/widgets/gooey_menu.h
    include/widgets/gooey_messagebox.h
    include/widgets/gooey_radiobutton.h
    include/widgets/gooey_slider.h
    include/widgets/gooey_textbox.h
    include/widgets/gooey_plot.h
    include/widgets/gooey_image.h
    include/signals/gooey_signals.h
    include/assets/drop_surface_image.h
    include/gooey.h
)

# Set compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -g3 -fPIC -fsanitize=address,undefined")

# Enable debugging
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

# Add third-party dependencies
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/cjson)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/glps)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/freetype)

# Define the library as SHARED
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    cjson
    GLPS
    freetype
)

# Include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/Gooey>
    PRIVATE
    ${PROJECT_SOURCE_DIR}/third_party/freetype/include
    ${PROJECT_SOURCE_DIR}/third_party/glps/include
    ${PROJECT_SOURCE_DIR}/third_party/cjson/include
)

# Ensure position-independent code (PIC) is enabled for shared libraries
set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Install all public headers under include/Gooey
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include/Gooey)


# Install the shared library
install(TARGETS ${PROJECT_NAME}
    EXPORT GooeyGUIConfig
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Create and install the CMake package configuration
install(EXPORT GooeyGUIConfig
    FILE GooeyGUIConfig.cmake
    NAMESPACE GooeyGUI::
    DESTINATION lib/cmake/GooeyGUI
)

# Optionally build examples
option(BUILD_EXAMPLES "Build example programs" ON)
if (BUILD_EXAMPLES)
    add_executable(example_basic examples/showcase.c)
    target_link_libraries(example_basic PRIVATE ${PROJECT_NAME})
    target_include_directories(example_basic
        PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
endif()

# Enable testing (if applicable)
enable_testing()