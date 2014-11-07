#include <QString>

#include "BCI/requests/optionSelectionStub.h"


OptionSelectionStub::OptionSelectionStub(rpcz::rpc_channel * channel)
  :optionSelection_stub(channel, "OptionSelectionService")
{
}


void OptionSelectionStub::buildRequest(const std::vector<QString> & stringList, 
				       const std::vector<QImage> & imageList)
{
  
}
