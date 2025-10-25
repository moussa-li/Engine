# add gtest
add_subdirectory(${CMAKE_SOURCE_DIR}/3rd/gtest ${CMAKE_BINARY_DIR}/3rd/gtest_build) 

set_common_variable()

add_executable(engine_tests
    ${MY_LIST}
    test_main.cpp
    test_normal.cpp
    test_common.cpp
)

if(WIN32)
    message(STATUS "Configuring for Windows")
    if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    endif()
elseif(UNIX)
    message(STATUS "Configuring for Linux")
endif()

target_include_directories(engine_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/3rd/gtest/googletest/include
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(engine_tests PRIVATE
    gtest
    gtest_main
    Common
)


#add_dependencies(engine_tests embed_gen_files)

add_test(NAME engine_tests COMMAND engine_tests)