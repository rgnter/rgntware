cmake_minimum_required(VERSION 3.15)
project(rgntware CXX)

set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/cmake/conan.cmake")
endif()
include("${CMAKE_BINARY_DIR}/cmake/conan.cmake")


if (UNIX)
    set(APP_PLATFORM "lnx")
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(APP_PLATFORM "win")
    set(APP_BINARES "${CMAKE_SOURCE_DIR}/vendor/win/64/debug")
else ()
    message(FATAL_ERROR "Unsupported OS '${CMAKE_SYSTEM_NAME}'")
endif ()

conan_cmake_configure(REQUIRES spdlog/1.9.2 nlohmann_json/3.9.1 cxxopts/2.2.1
        GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE .
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings})

# Find libaries
find_package(spdlog)
find_package(nlohmann_json)
find_package(cxxopts)

if(APP_PLATFORM MATCHES "win")
    add_library(dpp::dpp SHARED IMPORTED src/systems/ticket_system.cpp src/systems/ticket_system.hpp)
    set_target_properties(dpp::dpp PROPERTIES LINKER_LANGUAGE CXX
            IMPORTED_LOCATION ${APP_BINARES}/bin/dpp.dll
            IMPORTED_IMPLIB ${APP_BINARES}/lib/dpp.lib)
else ()
    message(FATAL_ERROR "todo")
endif()
# Link Libraries
link_libraries(spdlog::spdlog nlohmann_json::nlohmann_json cxxopts::cxxopts dpp::dpp)

# Include vendor headers
include_directories(vendor/include)
include_directories(src/)

add_executable(rgntware
        src/main.cpp
        src/bot/bot.cpp src/bot/bot.hpp src/bot/modules.cpp src/bot/modules.hpp)

if(APP_PLATFORM MATCHES "win")
    add_custom_command(
            TARGET rgntware POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${APP_BINARES}/bin"
            "$<TARGET_FILE_DIR:rgntware>")
endif()