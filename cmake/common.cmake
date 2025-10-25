function(gen_assets)
# the file should be embed
file (GLOB EMBED_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*"
)
# 输出文件目录
set (GEN_EMBED_OUTPUT_HDR_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/gen_inc")
set (GEN_EMBED_OUTPUT_SRC_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/gen_src")
file(MAKE_DIRECTORY ${GEN_EMBED_OUTPUT_HDR_DIR})
file(MAKE_DIRECTORY ${GEN_EMBED_OUTPUT_SRC_DIR})

# 依次处理文件
foreach(input_src ${EMBED_FILES})
    # 配置输出文件名
    file(SIZE ${input_src} embed_file_size)
    get_filename_component(embed_file ${input_src} NAME)
    set(gen_embed_file        "${GEN_EMBED_OUTPUT_SRC_DIR}/${embed_file}.cpp")
    set(gen_embed_file_header "${GEN_EMBED_OUTPUT_HDR_DIR}/${embed_file}.h")
    # 清空输出文件
    file(WRITE ${gen_embed_file} "")
    file(WRITE ${gen_embed_file_header} "")
    file(WRITE ${gen_embed_file} "#include \"${gen_embed_file_header}\" \n")
    # for c compatibility
    string(MAKE_C_IDENTIFIER ${embed_file} token)
    # to upper case
    string(TOUPPER ${token} token)
    # read hex data from file
    file(READ ${input_src} filedata HEX)
    # convert hex data for C compatibility
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
    # append data to output file
    file(APPEND ${gen_embed_file}
        "const unsigned char ${token}__DATA[] = {\n${filedata}0x00\n};\n"
        "const unsigned long ${token}__SIZE   = ${embed_file_size};\n")
    file(APPEND ${gen_embed_file_header}
        "extern const unsigned char ${token}__DATA[];\n"
        "extern const unsigned long ${token}__SIZE;\n")
    # 加入到生成文件列表
    list(APPEND GEN_EMBED_FILES
        ${gen_embed_file} 
        ${gen_embed_file_header}
    )
endforeach()

add_custom_target(
    embed_gen_files
    DEPENDS ${GEN_EMBED_FILES}
)

include_directories(${GEN_EMBED_OUTPUT_HDR_DIR})
set(MY_LIST ${GEN_EMBED_FILES} CACHE STRING "test")

endfunction()

macro(set_common_variable)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endmacro()