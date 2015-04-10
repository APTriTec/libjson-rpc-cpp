cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall --coverage")
SET(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG} -Wall --coverage")

SET(MAJOR_VERSION 0)
SET(MINOR_VERSION 3)
SET(PATCH_VERSION 2)
SET(SO_VERSION    0)

#possible modules that can be enabled/disabled
SET(HTTP_SERVER_MONGOOSE YES CACHE BOOL "Include HTTP server using mongoose")
SET(HTTP_CLIENT_CURL YES CACHE BOOL "Include HTTP client support using curl")
SET(SOCKET_SERVER NO CACHE BOOL "Include Server Socket support")
SET(SOCKET_CLIENT NO CACHE BOOL "Include Client Socket support")

SET(COMPILE_TESTS YES CACHE BOOL "Compile test framework")
SET(COMPILE_STUBGEN YES CACHE BOOL "Compile the stubgenerator")
SET(COMPILE_EXAMPLES YES CACHE BOOL "Compile example programs")


#-----------------------------------------------------------------------
# CPM configuration
#-----------------------------------------------------------------------
set(CPM_MODULE_NAME libjson-rpc-cpp)
set(CPM_LIB_TARGET_NAME ${CPM_MODULE_NAME})

if ((DEFINED CPM_DIR) AND (DEFINED CPM_UNIQUE_ID) AND (DEFINED CPM_TARGET_NAME))
  set(CPM_LIB_TARGET_NAME ${CPM_TARGET_NAME})
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CPM_DIR})
  include(CPM)
else()
  set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
  find_package(Git)
  if(NOT GIT_FOUND)
    message(FATAL_ERROR "CPM requires Git.")
  endif()
  if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" clone https://github.com/iauns/cpm ${CPM_DIR}
      RESULT_VARIABLE error_code
      OUTPUT_VARIABLE head_sha)
    if(error_code)
      message(FATAL_ERROR "CPM failed to get the hash for HEAD")
    endif()
  endif()
  include(${CPM_DIR}/CPM.cmake)
endif()

CPM_AddModule("jsoncpp"
    GIT_REPOSITORY "https://github.com/bcachet/cpm-jsoncpp"
    GIT_TAG origin/feature/ADD_INtimeSupport
    EXPORT_MODULE TRUE)

CPM_InitModule(${CPM_MODULE_NAME})


file(GLOB jsonrpc_source_common src/jsonrpccpp/common/*.c*)
file(GLOB jsonrpc_source_client src/jsonrpccpp/client/*.c*)
file(GLOB jsonrpc_source_server src/jsonrpccpp/server/*.c*)
list(APPEND jsonrpc_source_server src/jsonrpccpp/server/abstractserver.h)

file(GLOB jsonrpc_header_common src/jsonrpccpp/common/*.h)
file(GLOB jsonrpc_header_client
    src/jsonrpccpp/client/batchcall.h
    src/jsonrpccpp/client/batchresponse.h
    src/jsonrpccpp/client/client.h
    src/jsonrpccpp/client/iclientconnector.h
)
file(GLOB jsonrpc_header_server
    src/jsonrpccpp/server/requesthandlerfactory.h
    src/jsonrpccpp/server/abstractserver.h
    src/jsonrpccpp/server/abstractserverconnector.h
    src/jsonrpccpp/server/iprocedureinvokationhandler.h
    src/jsonrpccpp/server/iclientconnectionhandler.h
)
file(GLOB jsonrpc_header src/jsonrpccpp/*.h)

set(client_connector_source "")
set(client_connector_header "")
set(client_connector_libs   "")
set(server_connector_source "")
set(server_connector_header "")
set(server_connector_libs   "")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsonrpccpp/version.h.in"
  "${PROJECT_BINARY_DIR}/version.h"
)
install(FILES ${PROJECT_BINARY_DIR}/version.h DESTINATION include/jsonrpccpp) 

IF(HTTP_CLIENT_CURL)
    find_package(CURL REQUIRED)
    list(APPEND client_connector_header "src/jsonrpccpp/client/connectors/httpclient.h")
    list(APPEND client_connector_source "src/jsonrpccpp/client/connectors/httpclient.cpp")
    list(APPEND client_connector_libs ${CURL_LIBRARIES})
ENDIF()

IF(HTTP_SERVER_MONGOOSE)
    message(STATUS "Activated Mongoose HTTP Server")
    list(APPEND server_connector_header "src/jsonrpccpp/server/connectors/mongoose.h")
    list(APPEND server_connector_source "src/jsonrpccpp/server/connectors/mongoose.c")
    list(APPEND server_connector_header "src/jsonrpccpp/server/connectors/httpserver.h")
    list(APPEND server_connector_source "src/jsonrpccpp/server/connectors/httpserver.cpp")
    list(APPEND server_connector_libs ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

IF(SOCKET_CLIENT)
    find_package(Threads REQUIRED)
    list(APPEND client_connector_header "src/jsonrpccpp/client/connectors/socketclient.h")
    list(APPEND client_connector_source "src/jsonrpccpp/client/connectors/socketclient.cpp")
ENDIF()

IF(SOCKET_SERVER)
    find_package(Threads REQUIRED)
    list(APPEND server_connector_header "src/jsonrpccpp/server/connectors/socketserver.h")
    list(APPEND server_connector_source "src/jsonrpccpp/server/connectors/socketserver.cpp")
    list(APPEND server_connector_libs ${CMAKE_THREAD_LIBS_INIT})

ENDIF()

IF(SOCKET_CLIENT OR SOCKET_SERVER)
    list(APPEND jsonrpc_helper_header_common "src/jsonrpccpp/common/helper/socket.h")
    list(APPEND jsonrpc_helper_header_common "src/jsonrpccpp/common/helper/threads.h")
    list(APPEND jsonrpc_helper_source_common "src/jsonrpccpp/common/helper/threads.cpp")
ENDIF()

add_library(${CPM_LIB_TARGET_NAME}
      ${jsonrpc_source_common}
      ${jsonrpc_header}
      ${jsonrpc_helper_source_common}
      ${jsonrpc_source_client}
      ${jsonrpc_header_client}
      ${client_connector_source}
      ${jsonrpc_source_server}
      ${jsonrpc_header_server}
      ${server_connector_source})
target_link_libraries(${CPM_LIB_TARGET_NAME} ${CPM_LIBRARIES})

CPM_ExportAdditionalIncludeDir("${CMAKE_CURRENT_SOURCE_DIR}/src/")

set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
set_target_properties(${CPM_LIB_TARGET_NAME} PROPERTIES VERSION "${VERSION_STRING}" SOVERSION "${SO_VERSION}")

install(FILES ${jsonrpc_header}                 DESTINATION include/jsonrpccpp)
install(FILES ${jsonrpc_header_common}          DESTINATION include/jsonrpccpp/common)
install(FILES ${jsonrpc_helper_header_common}   DESTINATION include/jsonrpccpp/common/helper)
install(FILES ${jsonrpc_header_client}          DESTINATION include/jsonrpccpp/client)
install(FILES ${jsonrpc_header_server}          DESTINATION include/jsonrpccpp/server)
install(FILES ${client_connector_header}        DESTINATION include/jsonrpccpp/client/connectors)
install(FILES ${server_connector_header}        DESTINATION include/jsonrpccpp/server/connectors)

install(TARGETS ${CPM_LIB_TARGET_NAME}
                      LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall
COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
