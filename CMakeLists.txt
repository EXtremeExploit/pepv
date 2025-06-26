cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 20)

project(pepv VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/pepv.ui" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
add_subdirectory(external/tracy)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER})

# libalpm
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALPM REQUIRED libalpm)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_executable(pepv src/main.cpp src/pkgs.cpp src/utils.cpp src/events.cpp)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)

    if(supported)
        message(STATUS "IPO / LTO enabled")
        set_property(TARGET pepv PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO / LTO not supported: <${error}>")
    endif()
endif()

target_link_libraries(pepv PRIVATE ${GTK3_LIBRARIES} Tracy::TracyClient ${ALPM_LIBRARIES})
