cmake_minimum_required(VERSION 3.0)
project(cool C CXX)
message("CMAKE_CURRENT_SOURCE_DIR == ${CMAKE_CURRENT_SOURCE_DIR}")
#----------------  COOL ----------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cool/)
source_group(VM 
  FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_asm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_ast.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_io.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_khash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_limits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_obj.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_vm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_asm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_ast.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_io.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_asm_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_bcode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_obj.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_vm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
  REGULAR_EXPRESSION ".*vm.*") 

set(cool_utils_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
set(cool_hdrs 
  #${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_types.h
  #${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.h
)
set(cool_srcs 
  ${cool_utils_srcs}
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_asm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_ast.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_bcode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_compiler.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_hash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_io.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_khash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_lexer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_limits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_list.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_mem.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_murmur3.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_node.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_obj.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_parser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_queue.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_shunt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_symtab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_uthash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_vm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_vm_defs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_vm_undefs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_asm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_ast.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_bcode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_compiler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_hash.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_io.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_lexer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_list.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_mem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_node.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_obj.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_queue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_shunt.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_stack.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_symtab.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_vm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_murmur3.cpp
)
add_executable(cool_asm_test
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_asm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_io.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_limits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_obj.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_asm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_io.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_asm_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_bcode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_obj.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_vm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
add_executable(cool_ast_test
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_ast.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_io.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_limits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_symtab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_ast_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_ast.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_io.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_symtab.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
add_executable(cool_bcode_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_bcode_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_bcode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_vm_defs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_vm_undefs.h
)
add_executable(cool_compiler_test
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_khash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_compiler_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_lexer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_vm.cpp
)
add_executable(cool_hash_test
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_hash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_khash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_hash.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_hash_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_murmur3.cpp
)
add_executable(cool_lexer_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_lexer_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_khash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_symtab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_symtab.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_lexer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
add_executable(cool_list_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_list_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
add_executable(cool_mem_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_mem_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_mem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_node.cpp
)
add_executable(cool_node_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_node_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_node.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
add_executable(cool_obj_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_obj_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_obj.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_vm_defs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_vm_undefs.h
)
add_executable(cool_parser_test
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_khash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_parser_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_lexer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
add_executable(cool_queue_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_queue_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
add_executable(cool_shunt_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_shunt_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
add_executable(cool_stack_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_stack_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
add_executable(cool_symtab_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_symtab_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
)
add_executable(cool_vm_test
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/tests/cool_vm_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cool/cool_bcode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_vm_defs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../inc/cool/cool_vm_undefs.h
)

add_test(test_c_asm_test      cool_asm_test      )
add_test(test_c_ast_test      cool_ast_test      )
add_test(test_c_bcode_test    cool_bcode_test    )
add_test(test_c_obj_test      cool_obj_test      )
add_test(test_c_compiler_test cool_compiler_test )
add_test(test_c_hash_test     cool_hash_test     )
add_test(test_c_lexer_test    cool_lexer_test    )
add_test(test_c_list_test     cool_list_test     )
add_test(test_c_mem_test      cool_mem_test      )
add_test(test_c_node_test     cool_node_test     )
add_test(test_c_parser_test   cool_parser_test   )
add_test(test_c_queue_test    cool_queue_test    )
add_test(test_c_shunt_test    cool_shunt_test    )
add_test(test_c_stack_test    cool_stack_test    )
add_test(test_c_symtab_test   cool_symtab_test   )
add_test(test_c_vm_test       cool_vm_test       )

add_library(cool ${cool_srcs} ${cool_hdrs})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
target_link_libraries(cool_asm_test      cool) 
#target_link_libraries(cool_ast_test      cool) 
target_link_libraries(cool_bcode_test    cool) 
target_link_libraries(cool_obj_test      cool) 
target_link_libraries(cool_compiler_test cool) 
target_link_libraries(cool_hash_test     "-lapr-1 -laprutil-1") 
target_link_libraries(cool_hash_test     cool) 
target_link_libraries(cool_lexer_test    cool) 
target_link_libraries(cool_list_test     cool) 
target_link_libraries(cool_mem_test      cool) 
target_link_libraries(cool_parser_test   "-lapr-1 -laprutil-1") 
target_link_libraries(cool_parser_test   cool) 
target_link_libraries(cool_queue_test    cool) 
target_link_libraries(cool_shunt_test    cool) 
target_link_libraries(cool_stack_test    cool) 
target_link_libraries(cool_symtab_test   cool) 
target_link_libraries(cool_vm_test       cool) 

#message("CMAKE_BINARY_DIR == ${CMAKE_BINARY_DIR}") 
#message("CMAKE_COMMAND == ${CMAKE_COMMAND}") 
#message("CMAKE_CURRENT_BINARY_DIR == ${CMAKE_CURRENT_BINARY_DIR}") 
#message("CMAKE_CURRENT_LIST_FILE == ${CMAKE_CURRENT_LIST_FILE}") 
#message("CMAKE_CURRENT_LIST_DIR == ${CMAKE_CURRENT_LIST_DIR}") 
#message("CMAKE_CURRENT_LIST_LINE == ${CMAKE_CURRENT_LIST_LINE}") 
#message("CMAKE_CURRENT_SOURCE_DIR == ${CMAKE_CURRENT_SOURCE_DIR}") 
#message("CMAKE_FILES_DIRECTORY == ${CMAKE_FILES_DIRECTORY}") 
#message("CMAKE_MODULE_PATH == ${CMAKE_MODULE_PATH}") 
#message("CMAKE_ROOT == ${CMAKE_ROOT}") 
#message("CMAKE_SOURCE_DIR == ${CMAKE_SOURCE_DIR}") 
#message("EXECUTABLE_OUTPUT_PATH == ${EXECUTABLE_OUTPUT_PATH}") 
#message("LIBRARY_OUTPUT_PATH == ${LIBRARY_OUTPUT_PATH}") 
#message("PROJECT_NAME == ${PROJECT_NAME}") 
#message("CMAKE_PROJECT_NAME == ${CMAKE_PROJECT_NAME}") 
#message("PROJECT_BINARY_DIR == ${PROJECT_BINARY_DIR}") 
#message("PROJECT_SOURCE_DIR == ${PROJECT_SOURCE_DIR}") 
#message("-- VARIABLES --") 
