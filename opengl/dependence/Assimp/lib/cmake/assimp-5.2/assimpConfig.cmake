
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was assimp-hunter-config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

find_package(RapidJSON     CONFIG REQUIRED)
find_package(ZLIB          CONFIG REQUIRED)
find_package(utf8cpp       CONFIG REQUIRED)
find_package(minizip       CONFIG REQUIRED)
find_package(openddlparser CONFIG REQUIRED)
find_package(poly2tri      CONFIG REQUIRED)
find_package(polyclipping  CONFIG REQUIRED)
find_package(zip           CONFIG REQUIRED)
find_package(pugixml       CONFIG REQUIRED)
find_package(stb           CONFIG REQUIRED)

if(OFF)
  find_package(draco CONFIG REQUIRED)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/AssimpTargets.cmake")
check_required_components("Assimp")
