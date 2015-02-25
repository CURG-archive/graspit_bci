#include "rosRPCZClient.h"

#include "BCI/bciService.h"
#include "EGPlanner/searchState.h"
#include "debug.h"
#include <string>

RosRPCZClient::RosRPCZClient():
    _application(NULL),
    graspReachabilityStub(NULL),
    objectRecognitionStub(NULL),
    cameraOriginStub(NULL),
    executeGraspStub(NULL),
    _connected(true)
{
    DBGA("Created RosRPCZClient");
    _application = new rpcz::application();
    std::string urlString = "tcp://192.168.11.200:5561";

    rpcz::rpc_channel * channel = _application->create_rpc_channel(urlString);

//    if(!channel)
//    {
//        DBGA("Failed to create channel");
//    }
//    graspReachabilityStub = new GraspReachabilityStub(channel);
//    sleep(.1);
//    channel = _application->create_rpc_channel(urlString);
//    if(!channel)
//    {
//        DBGA("Failed to create channel");
//    }
//    sleep(.1);
//    objectRecognitionStub = new ObjectRecognitionStub(channel);


//    channel = _application->create_rpc_channel(urlString);
//    if(!channel)
//    {
//        DBGA("Failed to create channel");
//    }
//    sleep(.1);
//    objectRetrievalStub = new ObjectRecognitionStub(channel,"RetrieveObjectsService");


//    channel = _application->create_rpc_channel(urlString);
//    if(!channel)
//    {
//        DBGA("Failed to create channel");
//    }
//    cameraOriginStub = new CameraOriginStub(channel);
//    sleep(.1);

//    channel = _application->create_rpc_channel(urlString);
//    if(!channel)
//    {
//        DBGA("Failed to create channel");
//    }

    executeGraspStub = new ExecuteGraspStub(channel);
    sleep(.3);

    channel = _application->create_rpc_channel(urlString);
    if(!channel)
    {
        DBGA("Failed to create channel");
    }

//    optionSelectionStub = new OptionSelectionStub(channel);
//    sleep(.3);


    DBGA("Finished building RosRPCZClient");

}

bool RosRPCZClient::runObjectRetrieval(QObject * callbackReceiver, const char * slot)
{
    if(!_connected || !objectRetrievalStub)
    {
        DBGA("Tried to send invalid objectRetrievalStub");
        return false;
    }
    DBGA("Sent objectRetrievalStub");

    return objectRetrievalStub->sendRequest(callbackReceiver, slot);
}

bool RosRPCZClient::runObjectRecognition(QObject * callbackReceiver, const char * slot)
{
    if(!_connected || !objectRecognitionStub)
    {
        DBGA("Tried to send invalid objectRecognitionStub");
        return false;
    }
    DBGA("Sent objectRecognitionStub");

    return objectRecognitionStub->sendRequest(callbackReceiver, slot);
}

bool RosRPCZClient::getCameraOrigin(QObject * callbackReceiver, const char *slot)
{
    if(!_connected || !cameraOriginStub)
    {
        DBGA("Tried to send invalid getCameraOrigin");
        return false;
    }
    DBGA("Sent Camera Origin");
    return cameraOriginStub->sendRequest(callbackReceiver, slot);
}

bool RosRPCZClient::checkGraspReachability(const GraspPlanningState * gps, QObject * callbackReceiver, const char * slot)
{    
    if(!_connected || !graspReachabilityStub)
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
    if(!_connected)
    {
        DBGA("NOT connected");
    }
    if (!executeGraspStub)
    {
        DBGA("NOT executeGraspStub");
    }
  if(!_connected || !executeGraspStub)
  {
    DBGA("Tried to send invalid executeGraspStub");
    return false;
  }
  DBGA("Sent execute grasp");
  executeGraspStub->buildRequest(gps);
  return executeGraspStub->sendRequest(callbackReceiver, slot);
}

bool RosRPCZClient::sendOptionChoices(std::vector<QImage*> &imageOptions, std::vector<QString> &imageDescriptions, const std::vector<float> & imageCosts,
                                      float minimumConfidence,  QObject * callbackReceiver, const char * slot)
{
    if(!_connected || !optionSelectionStub)
    {
      DBGA("Tried to send invalid executeGraspStub");
      return false;
    }

    std::vector<QString> stringList;
    optionSelectionStub->buildRequest(imageOptions, stringList, imageCosts, imageDescriptions, minimumConfidence);
    return optionSelectionStub->sendRequest(callbackReceiver, slot);
}

RosRPCZClient::~RosRPCZClient()
{
    delete _application;
    delete graspReachabilityStub;
    delete objectRecognitionStub;
    delete cameraOriginStub;
    delete executeGraspStub;
}

