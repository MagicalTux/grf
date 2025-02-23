cmake_minimum_required(VERSION 3.10)

# Set the project name and version (adjust as needed).
project(grf VERSION 0.1.30 LANGUAGES C)

# Enable an option to choose between building a static or shared library.
option(BUILD_SHARED_LIB "Build as a shared library instead of static" ON)
option(BUILD_TESTS "Build the test executable" ON)

# Find ZLIB library - this makes ZLIB::ZLIB available.
find_package(ZLIB REQUIRED)

# List out all source files from the src directory.
set(SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/euc_kr.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/grf.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hash_tables.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/zlib.c"
)

# Decide how the library will be built (STATIC or SHARED).
if(BUILD_SHARED_LIB)
    add_library(grf SHARED ${SRC_FILES})
else()
    add_library(grf STATIC ${SRC_FILES})
endif()

# Optionally set a C standard (e.g., C99).
# You can remove or update this to whatever standard you need.
set_property(TARGET grf PROPERTY C_STANDARD 99)

# Link against zlib.
target_link_libraries(grf PUBLIC ZLIB::ZLIB)

# If you have library headers you want other projects to use, specify them here:
target_include_directories(grf PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/includes"
)

if(BUILD_TESTS)
    # We use CMake's testing features.
    include(CTest)
    enable_testing()

    # Create an executable from test.c
    add_executable(grf_test "${CMAKE_CURRENT_SOURCE_DIR}/src/test.c")

    # Link the test executable against our library
    target_link_libraries(grf_test PRIVATE grf)

    # Optionally, add a test to be run with "ctest" command
    add_test(NAME run_grf_test COMMAND grf_test)
endif()

# If you have headers to install as well, list them here:
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/includes/libgrf.h" DESTINATION include)

