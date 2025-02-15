cmake_minimum_required(VERSION 3.16)

# Set the plugin name to build
project(iec61850)

# Supported options:
# -DFLEDGE_INCLUDE
# -DFLEDGE_LIB
# -DFLEDGE_SRC
# -DFLEDGE_INSTALL
#
# If no -D options are given and FLEDGE_ROOT environment variable is set
# then Fledge libraries and header files are pulled from FLEDGE_ROOT path.

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

message(STATUS ${CMAKE_CXX_FLAGS})

if (${CMAKE_BUILD_TYPE} STREQUAL Coverage)
  message("Coverage is going to be generated")
  enable_testing()
  add_subdirectory(tests/unitTests)
  include(CodeCoverage)
  append_coverage_compiler_flags()
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
  set(GCOVR_ADDITIONAL_ARGS "--exclude-unreachable-branches" "--exclude-throw-branches" )

  setup_target_for_coverage_gcovr_sonar(NAME "${PROJECT_NAME}_coverage_sonar"
                                      EXECUTABLE RunTests
                                      DEPENDENCIES RunTests
                                      BASE_DIRECTORY "${PROJECT_SOURCE_DIR}"
                                      EXCLUDE "tests/*"
  )

  setup_target_for_coverage_gcovr_html(NAME "${PROJECT_NAME}_coverage_html"
                                       EXECUTABLE RunTests
                                       DEPENDENCIES RunTests
                                       BASE_DIRECTORY "${PROJECT_SOURCE_DIR}"
                                       EXCLUDE "tests/*"
  )
else()
  message("Build without Coverage")
  set(CMAKE_CXX_FLAGS "-std=c++14 -O3 -Wall")
endif()

# Generation version header file
set_source_files_properties(version.h PROPERTIES GENERATED TRUE)
add_custom_command(
  OUTPUT version.h
  DEPENDS ${CMAKE_SOURCE_DIR}/VERSION
  COMMAND ${CMAKE_SOURCE_DIR}/mkversion ${CMAKE_SOURCE_DIR}
  COMMENT "Generating version header"
  VERBATIM
)
include_directories(${CMAKE_BINARY_DIR})


# Set plugin type (south, north, filter)
set(PLUGIN_TYPE "south")
# Add here all needed Fledge libraries as list
set(NEEDED_FLEDGE_LIBS common-lib)

# Find source files
file(GLOB SOURCES src/*.cpp)

# Find Fledge includes and libs, by including FindFledge.cmak file
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
find_package(Fledge)
# If errors: make clean and remove Makefile
if (NOT FLEDGE_FOUND)
	if (EXISTS "${CMAKE_BINARY_DIR}/Makefile")
		execute_process(COMMAND make clean WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
		file(REMOVE "${CMAKE_BINARY_DIR}/Makefile")
	endif()
	# Stop the build process
	message(FATAL_ERROR "Fledge plugin '${PROJECT_NAME}' build error.")
endif()
# On success, FLEDGE_INCLUDE_DIRS and FLEDGE_LIB_DIRS variables are set


# Add ./include
include_directories(include)
# Add Fledge include dir(s)
include_directories(${FLEDGE_INCLUDE_DIRS})

# Add Fledge lib path
link_directories(${FLEDGE_LIB_DIRS})

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES} version.h)

# Add Fledge library names
target_link_libraries(${PROJECT_NAME} ${NEEDED_FLEDGE_LIBS})

# Add the libiec61850
find_library(LIBIEC61850 libiec61850.a)
if (NOT LIBIEC61850)
    message(FATAL_ERROR "The 61850 library 'libiec61850' was not found (in the standard lib dir)\n"
			"Please build and install the libiec61850 library")
	return()
endif()

target_link_libraries(${PROJECT_NAME} -L/usr/local/lib -liec61850)


# Add additional libraries
target_link_libraries(${PROJECT_NAME} -lpthread -ldl)

# Set the build version
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION 1)

set(FLEDGE_INSTALL "" CACHE INTERNAL "")
# Install library
if (FLEDGE_INSTALL)
	message(STATUS "Installing ${PROJECT_NAME} in ${FLEDGE_INSTALL}/plugins/${PLUGIN_TYPE}/${PROJECT_NAME}")
	install(TARGETS ${PROJECT_NAME} DESTINATION ${FLEDGE_INSTALL}/plugins/${PLUGIN_TYPE}/${PROJECT_NAME})
endif()

# Doc with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(docs/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMENT "Generating documentation with Doxygen" VERBATIM)
endif(DOXYGEN_FOUND)

# Functional tests
if(FUNCTIONAL_TESTS)
    add_subdirectory(tests/functionalTests)
endif()
