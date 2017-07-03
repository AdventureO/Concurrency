cmake_minimum_required(VERSION 3.3)

project(cpp_qt)

set(CMAKE_CXX_STANDARD 11)

# Find includes in corresponding build directories
# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)

get_target_property(QtCore_location Qt5::Core LOCATION)


add_executable(QtConditionalQueue QtConditionalQueue.cpp
        ../cxx/aux_tools.hpp ../cxx/aux_tools.cpp
        q_clean_words.hpp q_clean_words.cpp )
add_executable(q_seq_etalon q_seq_etalon.cpp
        ../cxx/aux_tools.hpp ../cxx/aux_tools.cpp
        q_clean_words.hpp q_clean_words.cpp)

target_link_libraries(QtConditionalQueue Qt5::Core)
target_link_libraries(q_seq_etalon Qt5::Core)
