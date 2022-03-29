cmake_minimum_required(VERSION 3.15)
project(build/rgntware CXX)

set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/cmake/conan.cmake")
endif()
include("${CMAKE_BINARY_DIR}/cmake/conan.cmake")


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

add_subdirectory(vendor/dpp)

# Link Libraries
link_libraries(spdlog::spdlog nlohmann_json::nlohmann_json cxxopts::cxxopts dpp)

# Include vendor headers
include_directories(vendor/DPP/include)
include_directories(src/)

add_executable(build/rgntware
        src/main.cpp
        src/bot/bot.cpp src/bot/bot.hpp src/bot/modules.cpp src/bot/modules.hpp)

if(WIN32)
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/vendor/dpp/win32/bin"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE_DIR:dpp>/dpp.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
endif()