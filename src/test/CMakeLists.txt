if(APPLE)
  cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
else()
  cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
endif()

project(JsonRpcCpp_Tests)

#------------------------------------------------------------------------------
# Required CPM Setup - See: http://github.com/bcachet/cpm
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
  if (EXISTS "${CPM_MODULE_CACHE_DIR}/github_bcachet_cpm")
    message(STATUS "Found cached version of CPM.")
    file(COPY "${CPM_MODULE_CACHE_DIR}/github_bcachet_cpm/" DESTINATION ${CPM_DIR})
  endif()
endif()
if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
  message(STATUS "Cloning repo (https://github.com/bcachet/cpm)")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" clone https://github.com/bcachet/cpm ${CPM_DIR}
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

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

ENABLE_TESTING()


INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
# ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)

set(CTEST_OUTPUT_ON_FAILURE TRUE)

file(GLOB test_source *.cpp)

message(${Boost_LIBRARIES})

add_executable(unit_testsuite ${test_source})
target_link_libraries(unit_testsuite ${Boost_LIBRARIES} ${CPM_LIBRARIES})


ADD_TEST(client ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=client)
ADD_TEST(server ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=server)
ADD_TEST(common ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=common)

ADD_TEST(all ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite)

