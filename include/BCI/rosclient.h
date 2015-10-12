#ifndef ROSCLIENT_H
#define ROSCLIENT_H

#include <QObject>
#include "graspit_msgs/GetObjectInfo.h"
#include "graspit_msgs/ObjectInfo.h"
#include "ros/ros.h"

class RosClient:public QObject
{

    Q_OBJECT

public:
    RosClient();

    void runObjectRecognition();

signals:
    void addToWorld(const QString model_filename, const QString object_name, const QString object_pose);
    void clearGB();

private:

    ros::NodeHandle n;

    //ros services
    ros::ServiceClient run_recognition_client;

    //helper for run_recognition.
    void addObject(graspit_msgs::ObjectInfo object);

};

#endif // ROSCLIENT_H
