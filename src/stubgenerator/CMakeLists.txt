if(APPLE)
  cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
else()
  cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
endif()

project(JsonRpcCpp_StubGenerator)

#------------------------------------------------------------------------------
# Required CPM Setup - See: http://github.com/iauns/cpm
#------------------------------------------------------------------------------
set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "CPM requires Git.")
endif()
if ((NOT DEFINED CPM_MODULE_CACHE_DIR) AND (NOT "$ENV{CPM_CACHE_DIR}" STREQUAL ""))
  set(CPM_MODULE_CACHE_DIR "$ENV{CPM_CACHE_DIR}")
endif()
if ((NOT EXISTS ${CPM_DIR}/CPM.cmake) AND (DEFINED CPM_MODULE_CACHE_DIR))
  if (EXISTS "${CPM_MODULE_CACHE_DIR}/github_iauns_cpm")
    message(STATUS "Found cached version of CPM.")
    file(COPY "${CPM_MODULE_CACHE_DIR}/github_iauns_cpm/" DESTINATION ${CPM_DIR})
  endif()
endif()
if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
  message(STATUS "Cloning repo (https://github.com/iauns/cpm)")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" clone https://github.com/iauns/cpm ${CPM_DIR}
    RESULT_VARIABLE error_code
    OUTPUT_QUIET ERROR_QUIET)
  if(error_code)
    message(FATAL_ERROR "CPM failed to get the hash for HEAD")
  endif()
endif()
include(${CPM_DIR}/CPM.cmake)

#-----------------------------------------------------------------------
# CPM Modules
#-----------------------------------------------------------------------
# ++ MODULE: socketpp
CPM_AddModule("libjson-rpc-cpp"
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.."
  EXPORT_MODULE TRUE)

CPM_AddModule("argtable3"
    GIT_REPOSITORY "https://github.com/bcachet/cpm-argtable3.git"
    GIT_TAG origin/master)

CPM_Finish()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

file(GLOB jsonrpcstub_source *.cpp client/*.cpp helper/*.cpp server/*.cpp)

add_executable(jsonrpcstub ${jsonrpcstub_source})
target_link_libraries(jsonrpcstub ${CPM_LIBRARIES})

#Generate manpage
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_custom_command(OUTPUT jsonrpcstub.1.gz
                   COMMAND gzip -c ${CMAKE_SOURCE_DIR}/doc/manpage > ${CMAKE_BINARY_DIR}/jsonrpcstub.1.gz
                   MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/doc/manpage
                   COMMENT "Generating man pages"
                   VERBATIM)

    add_custom_target(manpage ALL DEPENDS jsonrpcstub.1.gz)
    install(FILES ${CMAKE_BINARY_DIR}/jsonrpcstub.1.gz DESTINATION share/man/man1/)
    add_dependencies(jsonrpcstub manpage)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

install(TARGETS jsonrpcstub LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)
