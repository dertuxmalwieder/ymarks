cmake_minimum_required(VERSION 3.9)
project(ymarks)

set(CMAKE_BUILD_TYPE Release)

# Downloading the Conan wrapper if required:
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake" 
                 TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

# Fetching dependencies:
conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

# Building and linking:
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
add_executable(ymarks src/main.c src/backup.c)

find_package(cJSON)
find_package(SQLite3)

include_directories(${cJSON_INCLUDE_DIRS} ${SQLite_INCLUDE_DIRS}) 

target_link_libraries(ymarks ${cJSON_LIBRARIES} ${SQLite_LIBRARIES})
