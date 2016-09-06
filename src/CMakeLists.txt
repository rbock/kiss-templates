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

set(sources kiste2cpp.cpp parse_context.cpp line.cpp)
set(templates KisteTemplate.kiste ClassTemplate.kiste LineTemplate.kiste)

# code generator base
add_executable(kiste2cpp_base
	${sources}
	)
target_include_directories(kiste2cpp_base PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/prepared)
target_link_libraries(kiste2cpp_base PRIVATE kiste)

# bootstrap-iteration
function(bootstrap old_suffix new_suffix)
  set(options FINAL)
  set(oneValueArgs "")
  set(multiValueArgs "")
  cmake_parse_arguments(BOOTSTRAP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(target "kiste2cpp_${new_suffix}")
  if (BOOTSTRAP_FINAL)
    set(target "kiste2cpp")
  endif()

	add_kiss_templates(kiste2cpp_templates_${new_suffix}
			GENERATOR kiste2cpp_${old_suffix}
			NO_LINE_DIRECTIVES
			TARGET_FOLDER ${new_suffix}
			${templates}
			)

	add_executable(${target}
		${sources}
		)
	target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${new_suffix})
	add_dependencies(${target} kiste2cpp_templates_${new_suffix})
	target_link_libraries(${target} PRIVATE kiste)
endfunction()

# bootstrapping sequence towards final
bootstrap(base quarter)
bootstrap(quarter half)
bootstrap(half three_quarters)
bootstrap(three_quarters full FINAL)

install(TARGETS kiste2cpp RUNTIME DESTINATION bin)
