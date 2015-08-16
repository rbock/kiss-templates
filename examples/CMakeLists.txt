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

function(add_kiss_templates KISTE_NAME)
  set(templates "")
  foreach(kiste ${ARGN})
    get_filename_component(source ${kiste} ABSOLUTE)
    get_filename_component(basename ${kiste} NAME_WE)

    set(dest ${CMAKE_CURRENT_BINARY_DIR}/${basename}.h)

    set(templates ${templates} ${dest})
		add_custom_command(
			OUTPUT ${dest}
			COMMAND ${CMAKE_BINARY_DIR}/src/kiste2cpp ${source} > ${dest}
			DEPENDS ${source} kiste2cpp
			)
  endforeach()

	add_custom_target(${KISTE_NAME} DEPENDS ${templates})
endfunction()

add_subdirectory(0_hello_world)
add_subdirectory(1_functions)
add_subdirectory(2_cpp)
add_subdirectory(3_inheritance)
add_subdirectory(4_composition)
add_subdirectory(5_html)

