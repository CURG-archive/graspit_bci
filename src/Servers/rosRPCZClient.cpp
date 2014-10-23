#include "rosRPCZClient.h"

#include "BCI/bciService.h"
#include "EGPlanner/searchState.h"
#include "debug.h"

RosRPCZClient::RosRPCZClient():
    _application(NULL),
    graspReachabilityStub(NULL),
    objectRecognitionStub(NULL),
    cameraOriginStub(NULL),
    executeGraspStub(NULL)
{
    DBGA("Created RosRPCZClient");
    _application = new rpcz::application();
    rpcz::rpc_channel * channel = _application->create_rpc_channel("tcp://localhost:5561");
    if(!channel)
    {
        DBGA("Failed to create channel");
    }
    graspReachabilityStub = new GraspReachabilityStub(channel);
    sleep(.1);
    channel = _application->create_rpc_channel("tcp://localhost:5562");
    if(!channel)
    {
        DBGA("Failed to create channel");
    }
    sleep(.1);
    objectRecognitionStub = new ObjectRecognitionStub(channel);


    channel = _application->create_rpc_channel("tcp://localhost:5563");
    if(!channel)
    {
        DBGA("Failed to create channel");
    }
    cameraOriginStub = new CameraOriginStub(channel);
    sleep(.1);

    channel = _application->create_rpc_channel("tcp://localhost:5564");
    if(!channel)
    {
        DBGA("Failed to create channel");
    }

    executeGraspStub = new ExecuteGraspStub(channel);
    sleep(.3);
    DBGA("Finished building RosRPCZClient");

}


bool RosRPCZClient::runObjectRecognition(QObject * callbackReceiver, const char * slot)
{
    if(!objectRecognitionStub)
    {
        DBGA("Tried to send invalid objectRecognitionStub");
        return false;
    }
    DBGA("Sent objectRecognitionStub");

    return objectRecognitionStub->sendRequest(callbackReceiver, slot);
}

bool RosRPCZClient::getCameraOrigin(QObject * callbackReceiver, const char *slot)
{
    if(!cameraOriginStub)
    {
        DBGA("Tried to send invalid getCameraOrigin");
        return false;
    }
    DBGA("Sent Camera Origin");
    return cameraOriginStub->sendRequest(callbackReceiver, slot);
}

bool RosRPCZClient::checkGraspReachability(const GraspPlanningState * gps, QObject * callbackReceiver, const char * slot)
{    
    if(!graspReachabilityStub)
    {
        DBGA("Tried to send invalid graspReachability");
        return false;
    }
    DBGA("Sent GraspReachability");
    graspReachabilityStub->buildRequest(gps);
    return graspReachabilityStub->sendRequest(callbackReceiver, slot);
}


bool RosRPCZClient::executeGrasp(const GraspPlanningState * gps, QObject * callbackReceiver, const char * slot)
{
  if(!executeGraspStub)
  {
    DBGA("Tried to send invalid executeGraspStub");
    return false;
  }
  DBGA("Sent execute grasp");
  executeGraspStub->buildRequest(gps);
  return executeGraspStub->sendRequest(callbackReceiver, slot);
}


RosRPCZClient::~RosRPCZClient()
{
    delete _application;
    delete graspReachabilityStub;
    delete objectRecognitionStub;
    delete cameraOriginStub;
    delete executeGraspStub;
}

