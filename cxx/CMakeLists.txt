cmake_minimum_required(VERSION 3.3)

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
  IF(WIN32) # ? use IF(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0600")
  ENDIF(WIN32)
  # Потрібне для RUSAGE_THREAD але код може стати нестандартним з точки зору
  # POSIX!
  add_definitions(-D_GNU_SOURCE)
endif()

find_package (Threads)

# Uncomment to use std::unordered_map instead of std::map
add_definitions(-DUSE_UNORDERED_MAP)

# Neccessery for clang+cygwin, good thing everywhere
add_definitions(-D__STRICT_ANSI__)

project(cpp_concurrency)

set(AUX_TOOLS_FILES "aux_tools.cpp" "aux_tools.hpp" "measurements.cpp" "measurements.hpp" )
set(CLEAN_WORDS_FILES "clean_words.cpp" "clean_words.hpp")


add_executable(ConditionalQueue "ConditionalQueue.cpp" ${AUX_TOOLS_FILES} ${CLEAN_WORDS_FILES})

add_executable(seq_etalon "seq_etalon.cpp" ${AUX_TOOLS_FILES} ${CLEAN_WORDS_FILES})

add_executable(RawThreads "RawThreads.cpp" ${AUX_TOOLS_FILES} ${CLEAN_WORDS_FILES})

add_executable(Async "Async.cpp" ${AUX_TOOLS_FILES} ${CLEAN_WORDS_FILES})

add_executable(FuturePromise "FuturePromise.cpp" ${AUX_TOOLS_FILES} ${CLEAN_WORDS_FILES})
 
IF(WIN32)
TARGET_LINK_LIBRARIES(ConditionalQueue psapi pdh)
TARGET_LINK_LIBRARIES(seq_etalon psapi pdh)
TARGET_LINK_LIBRARIES(RawThreads psapi pdh)
TARGET_LINK_LIBRARIES(Async psapi pdh)
TARGET_LINK_LIBRARIES(FuturePromise psapi pdh)

# Define if you want to use Concurency Visualizer for Visual Studio
# Install corresponding SDK first!
# add_definitions(-DUSE_WINDOWS_CONCURRENCY_VIS_FLAGS)
ENDIF(WIN32)

IF(UNIX)
TARGET_LINK_LIBRARIES(ConditionalQueue rt)
TARGET_LINK_LIBRARIES(seq_etalon rt)
TARGET_LINK_LIBRARIES(RawThreads rt)
TARGET_LINK_LIBRARIES(Async rt)
TARGET_LINK_LIBRARIES(FuturePromise rt)
ENDIF(UNIX)


# Experimental, MSVC and Linux-x86 only. Rather recent x86 only.
find_package(PCM)
# Experimental, Linux only, x86, ARM, etc.
find_package(PAPI)

if(PCM_FOUND)
    include_directories(${PCM_INCLUDE_DIRS})
    target_link_libraries (ConditionalQueue ${PCM_LIBRARIES})
    target_link_libraries (seq_etalon ${PCM_LIBRARIES})
    target_link_libraries (RawThreads ${PCM_LIBRARIES})
    target_link_libraries (Async ${PCM_LIBRARIES})
    target_link_libraries (FuturePromise ${PCM_LIBRARIES})
    add_definitions(-DEXPERIMENTAL_USE_PCM)

    # Потрібне лише за випадку використання PCM
    target_link_libraries (seq_etalon ${CMAKE_THREAD_LIBS_INIT})

elseif(PAPI_FOUND)
    include_directories(${PAPI_INCLUDE_DIRS})
    target_link_libraries (ConditionalQueue ${PAPI_LIBRARIES})
    target_link_libraries (seq_etalon ${PAPI_LIBRARIES})
    target_link_libraries (RawThreads ${PAPI_LIBRARIES})
    target_link_libraries (Async ${PAPI_LIBRARIES})
    target_link_libraries (FuturePromise ${PAPI_LIBRARIES})
    add_definitions(-DEXPERIMENTAL_USE_PAPI)
endif()

target_link_libraries (Async ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (ConditionalQueue ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (RawThreads ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (FuturePromise ${CMAKE_THREAD_LIBS_INIT})
