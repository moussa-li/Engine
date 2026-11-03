cmake_minimum_required(VERSION 3.14)
project(Common)

set_common_variable()

if(BUILD_COMMON_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

if(WIN32)
add_definitions(-DWNT)
endif()

file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_library(${PROJECT_NAME} ${LIB_TYPE}
    ${CPP_FILES}
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include/Common
)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads Assets)

if(WIN32 AND BUILD_COMMON_SHARED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE EgLabCommon)
endif()
