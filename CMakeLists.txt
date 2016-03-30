project(cool CXX)
cmake_minimum_required(VERSION 3.0)
include(CTest)
enable_testing()
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
##set(CMAKE_DISABLE_SOURCE_CHANGES ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
message("-- VARIABLES --") 
message("CMAKE_BINARY_DIR == ${CMAKE_BINARY_DIR}") 
message("CMAKE_COMMAND == ${CMAKE_COMMAND}") 
message("CMAKE_CURRENT_BINARY_DIR == ${CMAKE_CURRENT_BINARY_DIR}") 
message("CMAKE_CURRENT_LIST_FILE == ${CMAKE_CURRENT_LIST_FILE}") 
message("CMAKE_CURRENT_LIST_DIR == ${CMAKE_CURRENT_LIST_DIR}") 
message("CMAKE_CURRENT_LIST_LINE == ${CMAKE_CURRENT_LIST_LINE}") 
message("CMAKE_CURRENT_SOURCE_DIR == ${CMAKE_CURRENT_SOURCE_DIR}") 
message("CMAKE_FILES_DIRECTORY == ${CMAKE_FILES_DIRECTORY}") 
message("CMAKE_MODULE_PATH == ${CMAKE_MODULE_PATH}") 
message("CMAKE_ROOT == ${CMAKE_ROOT}") 
message("CMAKE_SOURCE_DIR == ${CMAKE_SOURCE_DIR}") 
message("EXECUTABLE_OUTPUT_PATH == ${EXECUTABLE_OUTPUT_PATH}") 
message("LIBRARY_OUTPUT_PATH == ${LIBRARY_OUTPUT_PATH}") 
message("PROJECT_NAME == ${PROJECT_NAME}") 
message("CMAKE_PROJECT_NAME == ${CMAKE_PROJECT_NAME}") 
message("PROJECT_BINARY_DIR == ${PROJECT_BINARY_DIR}") 
message("PROJECT_SOURCE_DIR == ${PROJECT_SOURCE_DIR}") 
message("-- VARIABLES --") 
#-fsanitize=thread
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3 -Wall -pedantic -std=c11 -D_FORTIFY_SOURCE=2 -Weverything -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Wno-unneeded-internal-declaration -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -Wunused-function -Wstack-protector -fstack-protector") 


#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG -g -O3 -Wall -pedantic-errors -std=c11 -D_FORTIFY_SOURCE=2 -Weverything -Wextra -Wno-padded -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Wno-unneeded-internal-declaration -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter  -Wstack-protector -fstack-protector")set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -dNDEBUG -g -O3 -Wall -pedantic-errors -std=c11 -D_FORTIFY_SOURCE=2 -Weverything -Wno-padded -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Wno-unneeded-internal-declaration -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter") 
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -pedantic-errors -std=c11 -D_FORTIFY_SOURCE=2 -Weverything -Wno-float-equal -Wno-padded -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Wno-missing-noreturn -Wno-unneeded-internal-declaration -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -Wno-unused -Wno-unused-macros -Wno-switch-enum -Wno-switch -Wno-reserved-id-macro") 

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -pedantic-errors -std=c++11 -D_FORTIFY_SOURCE=2 -Weverything -Wno-float-equal -Wno-padded -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Wno-missing-noreturn -Wno-unneeded-internal-declaration -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -Wno-unused -Wno-unused-macros -Wno-switch-enum -Wno-switch -Wno-reserved-id-macro") 
set(COOL_INCLUDES ./inc)
include_directories(${COOL_INCLUDES})
add_subdirectory(./src)
#add_subdirectory(./docs)

#find_package(Doxygen)
#option(BUILD_DOCUMENTATION "Create and install the HTML based API        
#documentation (requires Doxygen)" ${DOXYGEN_FOUND})
#add_custom_command( OUTPUT doxy.log
#                    COMMAND doxygen ./docs/doxygen.conf > doxy.log
#                            DEPENDS details/part.out 
#                  )
#
