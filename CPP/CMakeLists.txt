cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 11)
# https://stackoverflow.com/questions/2368811/how-to-set-warning-level-in-cmake
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

# Uncomment to use std::unordered_map instead of std::map
add_definitions(-DUSE_UNORDERED_MAP)

project(cpp_concurrency)

set(AUX_TOOLS_FILES "aux_tools.cpp" "aux_tools.hpp")
set(CLEAN_WORDS_FILES "clean_words.cpp" "clean_words.hpp")


add_executable(ConditionalQueue "ConditionalQueue.cpp" ${AUX_TOOLS_FILES} ${CLEAN_WORDS_FILES})

add_executable(seq_etalon "seq_etalon.cpp" ${AUX_TOOLS_FILES} ${CLEAN_WORDS_FILES})

add_executable(RawThreads "RawThreads.cpp"  ${AUX_TOOLS_FILES} ${CLEAN_WORDS_FILES})
