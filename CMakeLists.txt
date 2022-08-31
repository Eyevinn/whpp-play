cmake_minimum_required(VERSION 3.16.0)
project(whpp-play)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -DDEBUG -Wall -Wuninitialized -Wsign-compare -Wno-missing-braces")
set(CMAKE_CXX_FLAGS_RELEASE "-ggdb -O3 -DNDEBUG -Wall -Wuninitialized -Wsign-compare -Wno-missing-braces")

if (APPLE)
        message("OSX")
        set(ENV{PKG_CONFIG_PATH} "/opt/homebrew/Cellar/icu4c/70.1/lib/pkgconfig")
endif()

find_package(PkgConfig)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_WEBRTC REQUIRED gstreamer-webrtc-1.0)
pkg_check_modules(GSTREAMER_SDP REQUIRED gstreamer-sdp-1.0)
pkg_check_modules(SOUP REQUIRED libsoup-3.0)

set(FILES
        main.c
)

add_executable(whpp-play ${FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
        ${GSTREAMER_WEBRTC_INCLUDE_DIRS}
        ${GSTREAMER_SDP_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
        ${SOUP_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
        ${GLIB_LIBRARIES}
        ${GSTREAMER_LDFLAGS}
        ${GSTREAMER_WEBRTC_LDFLAGS}
        ${GSTREAMER_SDP_LDFLAGS}
        ${CURL_LDFLAGS}
        ${SOUP_LDFLAGS})