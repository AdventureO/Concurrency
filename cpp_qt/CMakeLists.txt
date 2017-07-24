cmake_minimum_required(VERSION 3.3)

project(cpp_qt)

# set(CMAKE_VERBOSE_MAKEFILE ON)

enable_language(C CXX)

set(CMAKE_CXX_STANDARD 11)

# For FindPAPI.cmake and FindPCM modules
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}")

# https://stackoverflow.com/questions/2368811/how-to-set-warning-level-in-cmake
if(MSVC)
    # На старіших версіях Windows поки не плануємо запускати
    # а не все із використаного на них є.
    # define _WIN32_WINNT 0x0600
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_WIN32_WINNT=0x0600")

  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /wd4503")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

# Little less optimization, but simple life for profiles
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -g")
  IF(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0600")
  ENDIF(WIN32)
  # Потрібне для RUSAGE_THREAD але код може стати нестандартним з точки зору
  # POSIX!
  add_definitions(-D_GNU_SOURCE)
endif()

find_package (Threads)

# Find includes in corresponding build directories
# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)
find_package(Qt5Concurrent)

get_target_property(QtCore_location Qt5::Core LOCATION)

set(Q_COMMON_FILES  ../cxx/aux_tools.hpp ../cxx/aux_tools.cpp
                    ../cxx/measurements.cpp ../cxx/measurements.hpp
                    q_clean_words.hpp q_clean_words.cpp)

add_executable(QtConditionalQueue QtConditionalQueue.cpp ${Q_COMMON_FILES})
add_executable(QtRawThreads QtRawThreads.cpp ${Q_COMMON_FILES})
add_executable(QtConcurrentP QtConcurrentP.cpp ${Q_COMMON_FILES})
add_executable(q_seq_etalon q_seq_etalon.cpp ${Q_COMMON_FILES})

target_link_libraries(QtConditionalQueue Qt5::Core)
target_link_libraries(QtRawThreads Qt5::Core)
target_link_libraries(QtConcurrentP Qt5::Core Qt5::Concurrent)
target_link_libraries(q_seq_etalon Qt5::Core)

IF(WIN32)
TARGET_LINK_LIBRARIES(QtConditionalQueue psapi pdh)
TARGET_LINK_LIBRARIES(QtRawThreads psapi pdh)
TARGET_LINK_LIBRARIES(QtConcurrentP psapi pdh)
TARGET_LINK_LIBRARIES(q_seq_etalon psapi pdh)

# Define if you want to use Concurency Visualizer for Visual Studio
# Install corresponding SDK first!
# add_definitions(-DUSE_WINDOWS_CONCURRENCY_VIS_FLAGS)
ENDIF(WIN32)

IF(UNIX)
TARGET_LINK_LIBRARIES(QtConditionalQueue rt)
TARGET_LINK_LIBRARIES(QtRawThreads rt)
TARGET_LINK_LIBRARIES(QtConcurrentP rt)
TARGET_LINK_LIBRARIES(q_seq_etalon rt)
ENDIF(UNIX)

target_link_libraries (QtConditionalQueue ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (QtRawThreads ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (QtConcurrentP ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (q_seq_etalon ${CMAKE_THREAD_LIBS_INIT})
