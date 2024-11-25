cmake_minimum_required(VERSION 3.16.0)

project(dsm_examples VERSION 1.3.10 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the C++ flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  string(APPEND CMAKE_CXX_FLAGS "-Wall -Wextra -O0 -g")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  string(APPEND CMAKE_CXX_FLAGS "-O3")
endif()

find_package(dsm REQUIRED)

include(FetchContent)
# Get spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.0
)
FetchContent_MakeAvailable(spdlog)

# Set the folder for the executable
set(EXECUTABLE_OUTPUT_PATH ../)

# add as executable all cpp files into '.' folder
file(GLOB SOURCES "*.cpp")

# Loop through each source file and create an executable
foreach(SOURCE ${SOURCES})
    get_filename_component(EXE_NAME ${SOURCE} NAME_WE)
	add_executable(${EXE_NAME}.out ${SOURCE} ${SRC_SOURCES})
    target_link_libraries(${EXE_NAME}.out PRIVATE dsm::dsm spdlog::spdlog)
    target_include_directories(${EXE_NAME}.out PRIVATE ../src/dsm/headers/ ../src/dsm/utility/TypeTraits/ ../src/dsm/utility/)
endforeach()
