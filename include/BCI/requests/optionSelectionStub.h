#ifndef OPTIONSELECTIONSTUB_H
#define OPTIONSELECTIONSTUB_H

#include "rpcz/rpc_channel.hpp"
#include "rpcz/rpc_channel_impl.hpp"
#include "check_grasp_reachability.pb.h"
#include "check_grasp_reachability.rpcz.h"

#include "BCI/requests/requestStub.h"
#include <vector>
#include <QString>
#include <QImage>



class OptionSelectionStub : public Request
{
    Q_OBJECT
public:
    OptionSelectionStub(rpcz::rpc_channel * channel);
    void buildRequest(const std::vector<QString> & stringList, const std::vector<QImage> & imageList);

protected:
    virtual void sendRequestImpl();
    virtual void callbackImpl();

private:
    graspit_rpcz::GetOptionSelectionService_Stub optionSelection_stub;
    graspit_rpcz::GetOptionSelectionRequest request;
    graspit_rpcz::GetOptionSelectionResponse response;
};

#endif // OPTIONSELECTIONSTUB
