#include "BCI/rosclient.h"

#include "graspitGUI.h"
#include "ivmgr.h"
#include "world.h"

RosClient::RosClient()
{
}


void RosClient::runObjectRecognition()
{
    //rosRPCZServer.runObjectRecognition(callbackReceiver, slot);
    //build service request
    graspit_msgs::GetObjectInfo srv;

    //call service
    run_recognition_client.call(srv);

    // ask bodies to be cleared
    emit clearGB();

    // clear any existing graspable bodies
    while(graspItGUI->getIVmgr()->getWorld()->getNumGB())
        usleep(10000);


    // add each of the new bodies
    std::for_each(srv.response.object_info.begin(),
                  srv.response.object_info.end(),
                  boost::bind(&RosClient::addObject, this, _1));
}


void RosClient::addObject(graspit_msgs::ObjectInfo object)
{
    QString  modelName(QString::fromStdString(object.model_name) + ".xml");
    QString objectName(QString::fromStdString(object.object_name));

    transf object_pose = transf(
                Quaternion(
                    object.object_pose.orientation.w,
                    object.object_pose.orientation.x,
                    object.object_pose.orientation.y,
                    object.object_pose.orientation.z),
                vec3(
                    object.object_pose.position.x*1000.0,
                    object.object_pose.position.y*1000.0,
                    object.object_pose.position.z*1000.0
                    ));

    std::stringstream s;
    s << object_pose;
    QString stringPose(QString::fromStdString(s.str()));

    emit addToWorld(modelName, objectName, stringPose);
}
