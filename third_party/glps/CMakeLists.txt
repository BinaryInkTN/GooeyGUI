cmake_minimum_required(VERSION 3.10)

project(GLPS VERSION 1.0 LANGUAGES C)

set(CMAKE_CROSSCOMPILING 1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(WAYLAND wayland-client wayland-egl egl)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_C_COMPILER "/usr/bin/x86_64-w64-mingw32-gcc-win32")
    message(STATUS "Building for Windows")

    set(GLPS_SOURCES
        src/glps_wgl_context.c
        src/glps_win32.c
        src/glps_window_manager.c
        src/utils/logger/pico_logger.c
        src/glps_thread.c
    )

    set(GLPS_HEADERS
        internal/glps_wgl_context.h
        include/glps_window_manager.h
        include/glps_thread.h
        internal/glps_win32.h
        internal/glps_common.h
        internal/utils/logger/pico_logger.h
    )

    add_library(${PROJECT_NAME} SHARED ${GLPS_SOURCES} ${GLPS_HEADERS})

    target_compile_definitions(${PROJECT_NAME} PRIVATE GLPS_USE_WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE -mwindows)
    target_link_libraries(${PROJECT_NAME} PRIVATE gdi32 opengl32)

elseif(UNIX AND NOT APPLE)
    if(WAYLAND_FOUND)
        message(STATUS "Building for Linux Wayland")
        set(WAYLAND_PROTOCOLS_INSTALLER "${PROJECT_SOURCE_DIR}/scripts/waylandProtolcsInst.sh")
        execute_process(
          COMMAND bash ${WAYLAND_PROTOCOLS_INSTALLER}
        )
        set( GLPS_SOURCES
            src/glps_wayland.c
            src/glps_window_manager.c
            src/utils/logger/pico_logger.c
            src/glps_egl_context.c
            src/glps_thread.c

            # src/xdg/wlr-data-control-unstable-v1.c
            # src/xdg/xdg-decorations.c
            # src/xdg/xdg-dialog.c
            # src/xdg/xdg-shell.c
        )
        
        file(GLOB XDG_GLPS_SOURCES "src/xdg/*.c")
        list(APPEND GLPS_SOURCES ${XDG_GLPS_SOURCES})


        set(GLOB GLPS_HEADERS
            internal/glps_wayland.h
            include/glps_window_manager.h
            internal/glps_egl_context.h
            internal/glps_common.h
            internal/utils/logger/pico_logger.h
            include/glps_thread.h

            # internal/xdg/wlr-data-control-unstable-v1.h
            # internal/xdg/xdg-decorations.h
            # internal/xdg/xdg-dialog.h
            # internal/xdg/xdg-shell.h
        )
        
        file(GLOB XDG_GLPS_HEADERS "internal/xdg/*.h")
        list(APPEND GLPS_HEADERS ${XDG_GLPS_HEADERS})

        add_library(${PROJECT_NAME} SHARED ${GLPS_SOURCES} ${GLPS_HEADERS})

        target_compile_definitions(${PROJECT_NAME} PRIVATE GLPS_USE_WAYLAND)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -g3 -fsanitize=address,undefined)
        target_link_libraries(${PROJECT_NAME} PRIVATE m EGL wayland-client wayland-server wayland-cursor wayland-egl xkbcommon pthread)
        
if(UNIX AND NOT APPLE)
include_directories(SYSTEM
    /usr/include/glib-2.0
    /usr/lib/glib-2.0/include
    /usr/include/pixman-1
    /usr/include/libdrm
    /usr/include/libpng12
    /usr/include
)
endif()
    else()
        message(STATUS "Building for X11")
        set(GLPS_SOURCES
        src/glps_egl_context.c
        src/glps_x11.c
        src/glps_window_manager.c
        src/utils/logger/pico_logger.c
        src/glps_thread.c

        )

        set(GLPS_HEADERS
        internal/glps_egl_context.h
        internal/glps_x11.h
        internal/glps_common.h
        internal/utils/logger/pico_logger.h
        include/glps_window_manager.h
        include/glps_thread.h

        )

        add_library(${PROJECT_NAME} SHARED ${GLPS_SOURCES} ${GLPS_HEADERS})

        target_compile_definitions(${PROJECT_NAME} PRIVATE GLPS_USE_X11)
       target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -g3 -fsanitize=address,undefined)
        target_link_libraries(${PROJECT_NAME} PRIVATE X11 EGL pthread)
        if(UNIX AND NOT APPLE)
include_directories(SYSTEM
    /usr/include/glib-2.0
    /usr/lib/glib-2.0/include
    /usr/include/pixman-1
    /usr/include/libdrm
    /usr/include/libpng12
    /usr/include
)
endif()
    endif()
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/internal)


install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include/GLPS/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/internal/ DESTINATION include/GLPS/)

install(TARGETS ${PROJECT_NAME}
    EXPORT GLPSConfig
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(EXPORT GLPSConfig
    FILE GLPSConfig.cmake
    NAMESPACE GLPS::
    DESTINATION lib/cmake/GLPS
)

enable_testing()
