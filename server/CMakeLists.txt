cmake_minimum_required(VERSION 3.9)
project(ymarks)

set(CMAKE_BUILD_TYPE Release)

# Downloading the Conan wrapper if required:
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

# Fetching dependencies:
conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}
                    BUILD missing
                    SETTINGS ${settings})

# Building and linking:
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
add_executable(ymarks src/main.c src/backup.c)

find_package(cJSON)
find_package(SQLite3)

include_directories(${cJSON_INCLUDE_DIRS} ${SQLite_INCLUDE_DIRS}) 

target_link_libraries(ymarks ${cJSON_LIBRARIES} ${SQLite_LIBRARIES})
