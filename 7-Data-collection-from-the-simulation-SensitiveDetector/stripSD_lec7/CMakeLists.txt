
#----------------------------------------------------------------------------
# Setup the project
#

cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
SET(CMAKE_CXX_FLAGS "-std=c++17")
project(stripSD_lec7)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default

find_package(Geant4 REQUIRED ui_all vis_all)


#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)


link_directories( ${LINK_DIRECTORIES})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(stripSD_lec7 stripSD_lec7.cc ${sources} ${headers})
target_link_libraries(stripSD_lec7 ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(SCRIPTS
  vis.mac
  )
foreach(_script ${SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()
#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS stripSD_lec7 DESTINATION bin)
