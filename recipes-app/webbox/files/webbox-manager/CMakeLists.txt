project(WebboxManager)
cmake_minimum_required(VERSION 3.0)

message("Building Webbox Manager")

#set(VLC ${CMAKE_CROSSCOMPILING})
set(VLC true)

add_definitions(-DVLC=${VLC})

# lazy way to grap all sources
file(GLOB SOURCES *.c)

add_executable(WebboxManager ${SOURCES})

if (VLC)
#    add_definitions(-DVLC=true)
    target_link_libraries(WebboxManager libvlc.so)
endif()

target_link_libraries(WebboxManager pthread.so)

# just to simplify development
if (CMAKE_CROSSCOMPILING)
    add_definitions(-DWEBBOX_LIB_PATH="/usr/lib/webbox-manager")
else()
    add_definitions(-DWEBBOX_LIB_PATH="..")
endif()

# debug print defines
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
