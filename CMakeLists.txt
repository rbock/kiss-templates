# Copyright (c) 2015-2015, Roland Bock
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

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(kiste VERSION 0.1 LANGUAGES CXX)
include(CMakeParseArguments)

enable_testing()

add_library(kiste INTERFACE)

target_include_directories(kiste INTERFACE
  $<BUILD_INTERFACE:${kiste_SOURCE_DIR}/include>
  )

if (UNIX AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.4)
target_compile_features(kiste INTERFACE
  cxx_auto_type
  cxx_constexpr
  cxx_decltype
  cxx_defaulted_functions
  cxx_defaulted_move_initializers
  cxx_deleted_functions
  cxx_explicit_conversions
  cxx_lambdas
  cxx_noexcept
  cxx_nullptr
  cxx_range_for
  cxx_right_angle_brackets
  cxx_rvalue_references
  cxx_static_assert
  cxx_trailing_return_types
  cxx_uniform_initialization
  cxx_template_template_parameters
  cxx_variadic_templates
  )
else()
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  if(COMPILER_SUPPORTS_CXX11)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
endif ()

function(add_kiss_templates KISTE_NAME)
  set(options REPORT_EXCEPTIONS NO_LINE_DIRECTIVES)
  set(oneValueArgs GENERATOR TARGET_FOLDER)
  set(multiValueArgs "")
  cmake_parse_arguments(KISTE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(report_transactions "")
  if (KISTE_REPORT_EXCEPTIONS)
    set(report_transactions "--report-exceptions")
  endif()

  set(no_line_directives "")
  if (KISTE_NO_LINE_DIRECTIVES)
    set(no_line_directives "--no-line-directives")
  endif()

  set(generator "kiste2cpp")
  if (KISTE_GENERATOR)
    set(generator ${KISTE_GENERATOR})
  endif()

  set(target_folder ${CMAKE_CURRENT_BINARY_DIR})
  if (KISTE_TARGET_FOLDER)
    set(target_folder ${CMAKE_CURRENT_BINARY_DIR}/${KISTE_TARGET_FOLDER})
	  file(MAKE_DIRECTORY ${target_folder})
  endif()

  set(templates "")
  foreach(kiste ${KISTE_UNPARSED_ARGUMENTS})
    get_filename_component(source ${kiste} ABSOLUTE)
    get_filename_component(basename ${kiste} NAME_WE)

    set(dest ${target_folder}/${basename}.h)

    set(templates ${templates} ${dest})
    add_custom_command(
      OUTPUT ${dest}
      COMMAND $<TARGET_FILE:${generator}> ${report_transactions} ${no_line_directives} ${source} > ${dest}
      DEPENDS ${source} ${generator}
      )
  endforeach()

  add_custom_target(${KISTE_NAME} DEPENDS ${templates})
endfunction()

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(examples)
add_subdirectory(tests)

#install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/kiste" DESTINATION include)

