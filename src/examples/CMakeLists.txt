if(APPLE)
  cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
else()
  cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
endif()

project(JsonRpcCpp_Examples)

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
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../"
  EXPORT_MODULE TRUE)

CPM_Finish()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/gen)

add_custom_command(OUTPUT abstractstubserver.h
                   COMMAND jsonrpcstub ARGS ${CMAKE_CURRENT_SOURCE_DIR}/spec.json --cpp-server=AbstractStubServer --cpp-server-file=${CMAKE_BINARY_DIR}/gen/abstractsubserver.h
                   MAIN_DEPENDENCY spec.json
                   DEPENDS jsonrpcstub
                   COMMENT "Generating Server Stubfiles"
                   VERBATIM)

add_custom_command(OUTPUT stubclient.h
                   COMMAND jsonrpcstub ARGS ${CMAKE_CURRENT_SOURCE_DIR}/spec.json --cpp-client=StubClient --cpp-client-file=${CMAKE_BINARY_DIR}/gen/stubclient.h
                   MAIN_DEPENDENCY spec.json
                   DEPENDS jsonrpcstub
                   COMMENT "Generating Client Stubfile"
                   VERBATIM)

add_custom_command(OUTPUT xbmcremote.h
                   COMMAND jsonrpcstub ARGS ${CMAKE_CURRENT_SOURCE_DIR}/xbmc_remote.json --cpp-client=XbmcRemoteClient --cpp-client-file=${CMAKE_BINARY_DIR}/gen/xbmcremote.h
                   MAIN_DEPENDENCY xbmc_remote.json
                   DEPENDS jsonrpcstub
                   COMMENT "Generating Client XBMC Stubfile"
                   VERBATIM)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

IF(HTTP_SERVER_MONGOOSE AND HTTP_CLIENT_CURL)
  add_executable(simpleserversample simpleserver.cpp)
  target_link_libraries(simpleserversample ${CPM_LIBRARIES})
  
  add_executable(simpleclientsample simpleclient.cpp)
  target_link_libraries(simpleclientsample ${CPM_LIBRARIES})
ENDIF()

IF(COMPILE_STUBGEN)
    add_executable(stubclientsample stubclient.cpp stubclient.h)
    target_link_libraries(stubclientsample ${CPM_LIBRARIES}) 

    add_executable(stubserversample stubserver.cpp abstractstubserver.h)
    target_link_libraries(stubserversample ${CPM_LIBRARIES})

    add_executable(xbmcremote xbmcremote.cpp xbmcremote.h)
    target_link_libraries(xbmcremote ${CPM_LIBRARIES})
ENDIF()
