cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 11)
# GCC Only -- see also https://stackoverflow.com/questions/2368811/how-to-set-warning-level-in-cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

# Uncomment to use std::unordered_map instead of std::map
add_definitions(-DUSE_UNORDERED_MAP)

project(ConditionalQueue)

set(AUX_TOOLS_FILES "aux_tools.cpp" "aux_tools.hpp")
set(CLEAN_WORDS_FILES "clean_words.cpp" "clean_words.hpp")


add_executable(${PROJECT_NAME} "ConditionalQueue.cpp" ${AUX_TOOLS_FILES} ${CLEAN_WORDS_FILES})

#add_executable(seq_etalon "seq_etalon.cpp")
