project(WebboxManager)
cmake_minimum_required(VERSION 3.0)

message("Building Webbox Manager")

set(HTTP yes)
set(BROWSER yes)
set(IDLE yes)

if (HTTP)
    add_definitions(-DHTTP)
endif()

if (BROWSER)
    add_definitions(-DBROWSER)
endif()

if (IDLE)
    add_definitions(-DIDLE)
endif()

file(GLOB SOURCES *.c)

add_executable(WebboxManager ${SOURCES})

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
