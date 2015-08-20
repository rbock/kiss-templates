find_package(PythonInterp)
if(NOT PYTHONINTERP_FOUND)
  message(WARNING "Ignoring tests because Python is not installed")
  return()
endif()

file(GLOB ComparisonBasedTestInputs "*.kiste")
set(ComparisonBasedTestOutputs "")
foreach(ComparisonBasedTestInput ${ComparisonBasedTestInputs})
  string(REGEX REPLACE "\\.kiste$" ".data" ComparisonBasedTestData "${ComparisonBasedTestInput}")
  if(NOT EXISTS "${ComparisonBasedTestData}")
    set(ComparisonBasedTestData "")
  endif()

  string(REGEX REPLACE "\\.kiste$" ".expected" ComparisonBasedTestExpected "${ComparisonBasedTestInput}")
  get_filename_component(filename "${ComparisonBasedTestInput}" NAME)
  string(REGEX REPLACE "\\.kiste$" ".h" ComparisonBasedTestInputGeneratedHeader "${filename}")
  set(output "${CMAKE_CURRENT_BINARY_DIR}/${ComparisonBasedTestInputGeneratedHeader}")
  add_custom_command(
    OUTPUT "${output}"
    COMMAND kiste2cpp --output "${output}" "${ComparisonBasedTestInput}"
    DEPENDS kiste2cpp "${ComparisonBasedTestInput}" "${ComparisonBasedTestExpected}" "${ComparisonBasedTestData}"
  )
  set(ComparisonBasedTestOutputs ${ComparisonBasedTestOutputs} "${output}")
endforeach()

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ComparisonBasedTestRunnerAllHeaders.src.h"
  COMMAND
    "${CMAKE_CURRENT_SOURCE_DIR}/generate.py"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/ComparisonBasedTestRunnerAllHeaders.src.h"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generate.py" ${ComparisonBasedTestOutputs}
)

add_executable(
  ComparisonBasedTestRunner
  ComparisonBasedTestRunner.cpp
  "${CMAKE_CURRENT_BINARY_DIR}/ComparisonBasedTestRunnerAllHeaders.src.h"
)
target_include_directories(ComparisonBasedTestRunner PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ComparisonBasedTestRunner PRIVATE kiste)

add_test(
  NAME ComparisonBasedTest
  COMMAND ComparisonBasedTestRunner
)
