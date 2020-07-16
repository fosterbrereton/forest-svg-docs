cmake_minimum_required (VERSION 3.0)

project (fvg)

set(GCC_COVERAGE_COMPILE_FLAGS "-stdlib=libc++ -std=c++17")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

####################################################################################################
# Boost - sources disabled for now; haven't needed 'em yet.

#file(GLOB BOOST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/*/src/*.cpp)
#file(GLOB BOOST_THREAD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/thread/src/pthread/*.cpp)
#add_library(boost_sources STATIC ${BOOST_SRC} ${BOOST_THREAD_SRC})

get_filename_component(BOOST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/boost ABSOLUTE)

# target_include_directories(boost_sources PUBLIC ${BOOST_PATH})

####################################################################################################
# fvg header paths and sources

file(GLOB APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp)

get_filename_component(JSON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/json ABSOLUTE)

get_filename_component(ASL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/adobe_source_libraries ABSOLUTE)

get_filename_component(STLAB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/stlab ABSOLUTE)

get_filename_component(APP_HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/headers ABSOLUTE)

include_directories(fvg AFTER
                    ${ASL_PATH}
                    ${STLAB_PATH}
                    ${BOOST_PATH}
                    ${APP_HEADERS_PATH}
                    ${JSON_PATH}
                    )

add_executable(fvg ${APP_SRC})

#add_dependencies(fvg boost_sources)
#target_link_libraries(fvg PUBLIC boost_sources)

set_target_properties(fvg PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++17)
set_target_properties(fvg PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY libc++)

####################################################################################################
# forest unit test header paths and sources

file(GLOB FOREST_TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/forest_test/*.cpp)

add_executable(forest_test ${FOREST_TEST_SRC})

