cmake_minimum_required(VERSION 2.8.9)
project(graspit_core)

###### ---[ Add modules for finding libraries
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/" ${CMAKE_MODULE_PATH})




###########################################
#User Configurable OPTIONS
###########################################
set(GRASPIT_CORE_COLLISION_LIBRARY "graspit_collision" CACHE STRING "Which collision library to use. Valid options are {graspit_collision|pqp}")

if(win32)
  #select blas and lapack libraries, Windows-only
  #possible values: mkl clapack
  set(GRASPIT_CORE_GRASPIT_LAPACK_LIBRARY clapack CACHE STRING "Lapack library to use. Currently allowed types are {mkl|clapack} - WINDOWS ONLY")
  #link against the Hardware library (included with this distribution) which 
  #provides access to various hardware. Hardware library must be compiled
  #separately and is Windows-only.
  set(GRASPIT_CORE_BUILD_HARDWARELIB false CACHE BOOL "Whether to build the hardware library - WINDOWS ONLY")
else(win32) 
endif()


set(GRASPIT_CORE_USE_MOSEK false CACHE BOOL "Whether to use the mosek QP library (Must be installed separately)")

set(GRASPIT_CORE_USE_CGAL false CACHE BOOL "Whether to use the CGAL QP library (Must be installed separately)")


set(GRASPIT_CORE_USE_GRASPIT_CGDB true CACHE BOOL "Compile with support for the columbia grasp database")


set(GRASPIT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(LOCAL_QHULL false CACHE BOOL "Whether to use the local qhull library or the system one")


set(QT_USE_QT3SUPPORT TRUE)
set(QT_USE_QTOPENGL TRUE)
#set(QT_USE_QTNETWORK TRUE)

if(LOCAL_QHULL)
  #FIXME - Test this
  set(QHULL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty")
endif(LOCAL_QHULL)





######################################
# Include libraries 
######################################
find_package(Qt4 REQUIRED)
find_package(Qhull REQUIRED)
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)
find_package(Threads REQUIRED)
find_package(SoQt REQUIRED)
#find_package(Coin3D REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)


include(${QT_USE_FILE})




######################################
#Use c++0x - This magic may be deprecated in later versions of CMakeFiles
######################################
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#########################################################
#Disable excessive typedef checking in some CXX versions#
#########################################################
if (CMAKE_COMPILER_IS_GNUCXX)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
    endif()
endif()


################
#Core sources  #
################
set(GRASPIT_CORE_INCLUDE_DIRS src src/Collision include include/math include/Planner include/EGPlanner ui ui/Planner ui/EGPlanner include/Servers)
set(GRASPIT_CORE_SOURCE_DIRS src src/Collision src/Planner src/Servers src/EGPlanner ui ui/Planner ui/EGPlanner)
set(GRASPIT_CORE_FORM_DIRS ui ui/Planner ui/EGPlanner)
set(GRASPIT_CORE_RESOURCES ${GRASPIT_DIR}/images.qrc)

#############
#XML library
#############
list(APPEND GRASPIT_CORE_SOURCE_DIRS tinyxml)
list(APPEND GRASPIT_CORE_INCLUDE_DIRS tinyxml)

############
#CGDB
#########
if(GRASPIT_CORE_USE_GRASPIT_CGDB)
  set(QT_USE_QTSQL true)
  if($ENV{CGDB_MODEL_ROOT})
    set(GRASPIT_CORE_CGDB_DIR "$ENV{CGDB_MODEL_ROOT}" CACHE STRING "The path of the CGDB database")
  else($ENV{CGDB_MODEL_ROOT})
    set(GRASPIT_CORE_CGDB_DIR "${GRASPIT_DIR}/cgdb" CACHE STRING "The path of the CGDB database")    
  endif($ENV{CGDB_MODEL_ROOT})
  if(NOT IS_DIRECTORY "${GRASPIT_CORE_CGDB_DIR}")
    message(FATAL_ERROR "Failed to find CGDB directory: ${GRASPIT_CORE_CGDB_DIR} - cannot build")
  endif(NOT IS_DIRECTORY "${GRASPIT_CORE_CGDB_DIR}")
  list(APPEND GRASPIT_CORE_INCLUDE_DIRS src/DBase src/DBase/DBPlanner)
  list(APPEND GRASPIT_CORE_SOURCE_DIRS src/DBase src/DBase/DBPlanner)
  list(APPEND GRASPIT_CORE_FORM_DIRS src/DBase)

  set(GRASPIT_CORE_CGDB_BATCH_PROCESSING true CACHE BOOL "define BATCH_PROCESSING in order to supress error output which requires user attention")
  add_definitions(-DCGDB_ENABLED)

  if(${GRASPIT_CORE_CGDB_BATCH_PROCESSING})
    add_definitions(-DBATCH_PROCESSING)
  endif(${GRASPIT_CORE_CGDB_BATCH_PROCESSING})
endif(GRASPIT_CORE_USE_GRASPIT_CGDB)


if (${GRASPIT_CORE_COLLISION_LIBRARY} STREQUAL "graspit_collision")
  add_definitions(-DGRASPIT_COLLISION)
  list(APPEND GRASPIT_CORE_SOURCE_DIRS src/Collision/Graspit)
  list(APPEND GRASPIT_CORE_INCLUDE_DIRS src/Collision/Graspit)
endif()


#########################
#Add the matrix library
#########################
list(APPEND GRASPIT_CORE_HEADERS ${GRASPIT_DIR}/include/math/matrix.h)
list(APPEND GRASPIT_CORE_SOURCES ${GRASPIT_DIR}/src/math/matrix.cpp)
list(APPEND GRASPIT_CORE_INCLUDE_DIRS src)


########################################
#Mosek quadratic optimization library
########################################
if (${GRASPIT_CORE_USE_MOSEK})
  add_definitions(-DMOSEK_QP)
  list(APPEND GRASPIT_CORE_SOURCES ${GRASPIT_DIR}/src/math/mosek_qp.cpp})
  list(APPEND GRASPIT_CORE_HEADERS ${GRASPIT_DIR}/src/math/mosek_qp.h})
endif()

########################################
#CGAL quadratic optimization library
########################################
if (${GRASPIT_CORE_USE_CGAL})
  add_definitions(-DCGAL_QP)
  list(APPEND GRASPIT_CORE_SOURCES ${GRASPIT_DIR}/src/math/cgal_qp.cpp)
  list(APPEND GRASPIT_CORE_HEADERS ${GRASPIT_DIR}/src/math/cgal_qp.h)
endif()

####################################
#BCI stuff
####################################
list(APPEND GRASPIT_CORE_FORM_DIRS ui/BCI ui/BCI/state_views ui/BCI/control_views)
list(APPEND GRASPIT_CORE_INCLUDE_DIRS include/BCI include/BCI/states include/BCI/state_views include/BCI/requests include/BCI/utils ui/BCI ui/BCI/state_views ui/BCI/control_views)
list(APPEND GRASPIT_CORE_SOURCE_DIRS src/BCI src/BCI/requests src/Servers src/BCI/states src/BCI/utils ui/BCI ui/BCI/control_views ui/BCI/state_views)


add_subdirectory(protocols)
list(APPEND GRASPIT_CORE_SUBDIR_LIBS graspit_proto_msgs)
list(APPEND GRASPIT_CORE_INCLUDE_DIRS protocols/gen_proto)

add_subdirectory(rpcz)
list(APPEND GRASPIT_CORE_SUBDIR_LIBS rpcz graspit_pb boost_thread boost_program_options zmq)
list(APPEND GRASPIT_CORE_INCLUDE_DIRS rpcz/src rpcz/include rpcz/src/rpcz ${CMAKE_CURRENT_BINARY_DIR}/rpcz/graspit_rcpz/cpp ${CMAKE_CURRENT_BINARY_DIR}/rpcz/src)


foreach(SOURCE_DIR_ITER ${GRASPIT_CORE_SOURCE_DIRS})
  file(GLOB GLOB_TEMP "${GRASPIT_DIR}/${SOURCE_DIR_ITER}/*.cpp")
  list(APPEND GRASPIT_CORE_SOURCES_GLOB ${GLOB_TEMP})
  #message("Graspit core source temp: ${GRASPIT_DIR}/${SOURCE_DIR_ITER} - ${GLOB_TEMP}")
endforeach(SOURCE_DIR_ITER)

foreach(INCLUDE_DIR_ITER ${GRASPIT_CORE_INCLUDE_DIRS})
  file(GLOB GLOB_TEMP "${GRASPIT_DIR}/${INCLUDE_DIR_ITER}/*.h")
  list(APPEND GRASPIT_CORE_HEADERS_GLOB ${GLOB_TEMP})
endforeach(INCLUDE_DIR_ITER)

foreach(FORM_DIR_ITER ${GRASPIT_CORE_FORM_DIRS})
  file(GLOB GLOB_TEMP "${GRASPIT_DIR}/${FORM_DIR_ITER}/*.ui")
  list(APPEND GRASPIT_CORE_FORMS_GLOB ${GLOB_TEMP})
endforeach(FORM_DIR_ITER)


set(GRASPIT_CORE_UI_DIR ${GRASPIT_DIR}/ui)

list(APPEND GRASPIT_CORE_HEADERS ${GRASPIT_CORE_HEADERS_GLOB})
list(APPEND GRASPIT_CORE_SOURCES ${GRASPIT_CORE_SOURCES_GLOB})
list(APPEND GRASPIT_CORE_FORMS ${GRASPIT_CORE_FORMS_GLOB})

list(REMOVE_ITEM GRASPIT_CORE_HEADERS ${GRASPIT_CORE_UI_DIR}/sensorInputDlg.h)

message("Graspit headers ${GRASPIT_CORE_HEADERS}")





include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
ADD_DEFINITIONS(${QT_DEFINITIONS} -DQT3_SUPPORT)
QT4_ADD_RESOURCES(GRASPIT_CORE_RESOURCES_CPP ${GRASPIT_CORE_RESOURCES})
QT4_WRAP_UI(GRASPIT_CORE_FORMS_HEADERS ${GRASPIT_CORE_FORMS})

set(CMAKE_AUTOMOC ON)

#In new cmake, do this
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)
#in old cmake, do this
list(APPEND GRASPIT_CORE_SOURCES ${GRASPIT_CORE_RESOURCES_CPP} ${GRASPIT_CORE_FORMS_HEADERS})


#Remove old unused code.
list(REMOVE_ITEM GRASPIT_CORE_HEADERS ${GRASPIT_CORE_UI_DIR}/sensorInputDlg.h) 
list(REMOVE_ITEM GRASPIT_CORE_SOURCES ${GRASPIT_CORE_UI_DIR}/sensorInputDlg.cpp) 
list(REMOVE_ITEM GRASPIT_CORE_SOURCES ${GRASPIT_DIR}/src/pincer.cpp)
list(REMOVE_ITEM GRASPIT_CORE_SOURCES ${GRASPIT_CORE_UI_DIR}/BCI/handViewWindow.cpp)



ADD_EXECUTABLE(graspit ${GRASPIT_CORE_SOURCES}  ${GRASPIT_CORE_HEADERS})

message("QHull Libraries ${QHULL_LIBRARIES}")
TARGET_LINK_LIBRARIES(graspit optimized ${GRASPIT_CORE_SUBDIR_LIBS} ${QHULL_LIBRARIES} ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${SoQt_LIBRARIES} ${QT_LIBRARIES} ${Boost_LIBRARIES} Coin GL)


INCLUDE_DIRECTORIES(graspit ${QHULL_INCLUDE_DIR} ${GRASPIT_CORE_INCLUDE_DIRS} ${QT_INCLUDES} ${SoQt_INCLUDE_DIRS} ${COIN3D_Include_DIRS})

SET_TARGET_PROPERTIES(graspit PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${GRASPIT_DIR}/bin)
#message("Graspit core source:  ${GRASPIT_CORE_SOURCES}")

