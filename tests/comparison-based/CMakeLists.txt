# Copyright (c) 2015-2015, Roland Bock, Andreas Sommer
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#   Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
#   Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

find_package(PythonInterp)
if(NOT PYTHONINTERP_FOUND)
  message(WARNING "Ignoring tests because Python is not installed")
  return()
endif()

find_package(Boost)
if(NOT Boost_FOUND)
  message(WARNING "Ignoring tests because Boost is not installed")
  return()
endif()
if ("${Boost_VERSION}" STRLESS "105600")
	message(WARNING "Ignoring tests because Boost version is too old")
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
target_include_directories(ComparisonBasedTestRunner PRIVATE "${CMAKE_CURRENT_BINARY_DIR}" ${Boost_INCLUDE_DIRS})
target_link_libraries(ComparisonBasedTestRunner PRIVATE kiste)

add_test(
  NAME ComparisonBasedTest
  COMMAND ComparisonBasedTestRunner
)
