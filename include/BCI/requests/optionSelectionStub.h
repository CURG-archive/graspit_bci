#ifndef OPTIONSELECTIONSTUB_H
#define OPTIONSELECTIONSTUB_H

#include "rpcz/rpc_channel.hpp"
#include "rpcz/rpc_channel_impl.hpp"
#include "run_recognition.rpcz.h"
#include "run_recognition.pb.h"
#include "get_option_selection.pb.h"
#include "get_option_selection.rpcz.h"
#include "BCI/requests/requestStub.h"
#include <vector>
#include <QString>
#include <QImage>

using namespace graspit_rpcz;

class OptionSelectionStub : public Request {
Q_OBJECT

public:
    OptionSelectionStub(rpcz::rpc_channel *channel);

    void buildRequest(const std::vector<QImage *> &imageList, const std::vector<QString> &stringList, const std::vector<float> &imageCosts,
            const std::vector<QString> &descriptionList, const float minimumConfidence);

protected:
    virtual void sendRequestImpl();

    virtual void callbackImpl();

private:
    graspit_rpcz::GetOptionSelectionService_Stub optionSelection_stub;
    graspit_rpcz::GetOptionSelectionRequest request;
    graspit_rpcz::GetOptionSelectionResponse response;

};

#endif // OPTIONSELECTIONSTUB
