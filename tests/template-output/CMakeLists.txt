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

file(GLOB TemplateOutputTestInputs "*.kiste")
foreach(TemplateOutputTestInput ${TemplateOutputTestInputs})
  get_filename_component(filename "${TemplateOutputTestInput}" NAME)
  get_filename_component(filename_we "${TemplateOutputTestInput}" NAME_WE)
  string(REGEX REPLACE "\\.kiste$" ".h.actual" TemplateOutputTestInputGeneratedHeader "${filename}")
  string(REGEX REPLACE "\\.kiste$" ".h.expected" TemplateOutputTestInputExpectedHeader "${filename}")
  string(REGEX REPLACE "\\.kiste$" ".params" TemplateOutputTestInputParams "${filename}")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TemplateOutputTestInputParams}")
    file(STRINGS "${TemplateOutputTestInputParams}" params)
  else()
    set(params "")
  endif()
  set(output "${CMAKE_CURRENT_SOURCE_DIR}/${TemplateOutputTestInputGeneratedHeader}")
  add_custom_target(
    "TemplateOutputTest_generate_${filename_we}"
    ALL
    # Pass relative filenames so that "#line" pragmas are repeatable on all developers' machines
    COMMAND kiste2cpp ${params} --output "${output}" "${filename}"
    DEPENDS kiste2cpp "${TemplateOutputTestInput}" "${TemplateOutputTestInputExpectedHeader}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  add_test(
    NAME "TemplateOutputTest_${filename_we}"
    COMMAND
      python -B
      "${CMAKE_CURRENT_SOURCE_DIR}/../differ.py"
      "${TemplateOutputTestInput}"
      "${output}"
      "${TemplateOutputTestInputExpectedHeader}"
    WORKING_DIRECTORY
      "${CMAKE_CURRENT_SOURCE_DIR}"
  )
endforeach()
