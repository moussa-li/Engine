cmake_minimum_required(VERSION 3.14)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*.cpp"
    "${PROJECT_SOURCE_DIR}/include/*.h"
)

file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*.cpp"
)