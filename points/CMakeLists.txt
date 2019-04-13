cmake_minimum_required (VERSION 3.5)

project(Coding_Test)

set(CMAKE_PROJECT_NAME Coding Test)
set (CMAKE_CXX_STANDARD 11)

set(MAIN_DIR ${CMAKE_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

MACRO( source_files_tree path files)
        FILE( GLOB all_files ${path}/* )

        FOREACH( each ${all_files} )
                IF( NOT IS_DIRECTORY ${each} )
                        GET_FILENAME_COMPONENT( file_name ${each} NAME )
                        LIST( APPEND ${files} "${path}/${file_name}" )
                ENDIF()
        ENDFOREACH()

        FILE( GLOB directories ${path}/*/ )
        FOREACH( each ${directories} )
                IF( IS_DIRECTORY ${each} )
                        GET_FILENAME_COMPONENT( file_name ${each} NAME )
                        source_files_tree("${path}/${file_name}" ${files})
                ENDIF()
        ENDFOREACH()
ENDMACRO()

MACRO( source_group_tree )
        FILE( GLOB all_files ${ARGV1}/* )
        SET( files "" )
        FOREACH( each ${all_files} )
                IF( NOT IS_DIRECTORY ${each} )
                        GET_FILENAME_COMPONENT( file_name ${each} NAME )
                        LIST( APPEND files "${ARGV1}/${file_name}" )
                ENDIF()
        ENDFOREACH()
        LIST( LENGTH files files_len )
        IF( ${files_len} GREATER 0 )
                STRING( REPLACE "/" "\\\\" group_name ${ARGV0} )
                SOURCE_GROUP("${group_name}" FILES ${files})
        ENDIF()

        FILE( GLOB directories ${ARGV1}/*/ )
        FOREACH( each ${directories} )
                IF( IS_DIRECTORY ${each} )
                        GET_FILENAME_COMPONENT( file_name ${each} NAME )
                        source_group_tree("${ARGV0}/${file_name}" "${ARGV1}/${file_name}")
                ENDIF()
        ENDFOREACH()
ENDMACRO()

find_package(Threads)

SET( HEADER "" )
source_files_tree(include HEADER)

SET( SOURCE "" )
source_files_tree(src SOURCE)

add_executable(Coding_Test ${SOURCE} ${HEADER})
target_link_libraries(Coding_Test ${CMAKE_THREAD_LIBS_INIT})

source_group_tree("Header" include)
source_group_tree("Source" src)
