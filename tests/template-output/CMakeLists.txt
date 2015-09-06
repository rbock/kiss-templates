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
