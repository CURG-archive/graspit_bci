TEMPLATE	= app
LANGUAGE	= C++

#-------------------------options--------------------------

#comment out this line for compiling in Release mode
#usually, compiling in Release mode delivers a significant gain in performance
#in MS Visual Studio *also* set the project mode to Release
#CONFIG += graspitdbg

#select collision detection engine
#possible values: graspit_collision
COLLISION = graspit_collision

#select blas and lapack libraries, Windows-only
#possible values: mkl clapack
LAPACK = clapack

#build and use interface with Columbia Grasp Database
CONFIG += cgdb

#link against the Hardware library (included with this distribution) which 
#provides access to various hardware. Hardware library must be compiled
#separately and is Windows-only.
#CONFIG += hardwarelib

#link against Mosek QP solver (must be installed separately)
#CONFIG += mosek

#enable linking against boost (must be installed separately)
#CONFIG += boost

#------------------system-specific libraries---------------
CONFIG +=  $$COLLISION $$LAPACK
ros {
  include(ros.pri)

}
CONFIG += ros

graspitdbg {
    CONFIG += debug
} else {
    CONFIG += release
	DEFINES += GRASPIT_RELEASE
}

win32 {
	include(graspit-lib-WINDOWS.pro)
} else {
	include(graspit-lib-LINUX.pro)
}

CONFIG += bci_experiment

#---------------------------ROS-----------------------------
ros {
  include(ros.pri)

}

LIBS += /opt/ros/hydro/lib/*.so

INCLUDEPATH += /home/jvarley/ros/bci_project_ws/devel/include/graspit_msgs

HEADERS += /home/jvarley/ros/bci_project_ws/devel/include/graspit_msgs/ObjectInfo.h

#------------------GraspIt! core files---------------------
CONFIG += protobuf
CONFIG += rpcz


include(graspit-core.pro)




