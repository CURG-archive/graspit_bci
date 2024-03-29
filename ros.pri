#
# Qt qmake integration with Google Protocol Buffers compiler protoc
#
# To compile protocol buffers with qt qmake, specify PROTOS variable and
# include this file
#
# Example:
# LIBS += /usr/lib/libprotobuf.so
# PROTOS = a.proto b.proto
# include(protobuf.pri)
#
# By default protoc looks for .proto files (including the imported ones) in
# the current directory where protoc is run. If you need to include additional
# paths specify the PROTOPATH variable
#


LIBS += /opt/ros/hydro/lib/*.so

INCLUDEPATH += ~/ros/bci_project_ws/devel/include/graspit_msgs
HEADERS += ~/ros/bci_project_ws/devel/include/graspit_msgs/

INCLUDEPATH += ~/ros/bci_project_ws/devel/include/moveit_trajectory_planner
HEADERS += /home/armuser/ros/bci_project_ws/devel/include/moveit_trajectory_planner/

INCLUDEPATH += ~/ros/bci_project_ws/devel/include/get_camera_origin
HEADERS += ~/ros/bci_project_ws/devel/include/get_camera_origin/
