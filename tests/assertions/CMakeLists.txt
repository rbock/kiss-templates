add_kiss_templates(calling_non_void_templates calling_non_void.kiste)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}/../../include)
add_executable(calling_non_void EXCLUDE_FROM_ALL calling_non_void.cpp ${CMAKE_CURRENT_BINARY_DIR}/calling_non_void.h)
add_dependencies(calling_non_void calling_non_void_templates)
target_link_libraries(calling_non_void PRIVATE kiste)

add_test(NAME calling_non_void_test
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target calling_non_void
	)
set_property(TEST calling_non_void_test PROPERTY PASS_REGULAR_EXPRESSION "call{} requires void expression")

