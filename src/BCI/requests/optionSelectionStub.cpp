#include "BCI/requests/optionSelectionStub.h"
#include <QString>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <BCI/bciService.h>


OptionSelectionStub::OptionSelectionStub(rpcz::rpc_channel * channel)
    :optionSelection_stub(channel, "GetOptionSelectionService", true)
{
}


void OptionSelectionStub::buildRequest(const std::vector<QImage*> & imageList,
                                       const std::vector<QString> & stringList,
                                       const std::vector<float> imageCosts,
                                       const std::vector<QString> & descriptionList,
                                       const float minimumConfidence)
{
  // Check that the number of descriptions matches the number of options
  DBGA(imageList.size());
  DBGA(stringList.size());
  DBGA(descriptionList.size());
  assert(imageList.size() + stringList.size() == descriptionList.size());

  request.clear_compressedimageoptions();
  request.clear_imageoptions();
  request.clear_stringoptions();



  for(int i = 0; i < imageList.size(); ++i)
  {
      QImage * img = imageList[i];
      QByteArray ba;
      QBuffer buffer(&ba);
      buffer.open(QIODevice::WriteOnly);
      img->save(&buffer, "PNG");
      img->save(QString("/tmp/temp") + QString::number(i) + QString(".png"));
      request.add_compressedimageoptions();
      graspit_rpcz::GetOptionSelectionRequest_CompressedImageOption * cio = request.mutable_compressedimageoptions(i);

      std::string data(ba.constData(), ba.length());

      cio->mutable_option()->set_data(data);
      cio->mutable_option()->set_format("png");
      cio->mutable_description()->set_description(descriptionList[i].toStdString());
      cio->mutable_description()->set_cost(imageCosts[i]);
      cio->mutable_description()->set_id(i);
  }
  request.clear_similaritymatrix();
  for(size_t i = 0; i < imageCosts.size(); ++i)
    request.add_similaritymatrix(imageCosts[i]);
  request.set_minimumconfidencelevel(minimumConfidence);
}

void OptionSelectionStub::sendRequestImpl()
{
    optionSelection_stub.run(request, &response, _rpc, rpcz::new_callback<OptionSelectionStub>(this, &OptionSelectionStub::callback));
}

void OptionSelectionStub::callbackImpl()
{
    std::vector<float> interestLevel;
    for (int i = 0; i < response.interestlevel_size(); ++i)
        interestLevel.push_back(response.interestlevel().Get(i));
    unsigned int option = response.selectedoption();
    float confidence = response.confidence();
    BCIService::getInstance()->emitOptionChoice(option, confidence, interestLevel);
    return;
}
