#include "BCI/requests/optionSelectionStub.h"
#include <QString>
#include <QImage>
#include <QByteArray>
#include <QBuffer>


OptionSelectionStub::OptionSelectionStub(rpcz::rpc_channel * channel)
    :optionSelection_stub(channel, "OptionSelectionService", true)
{
}


void OptionSelectionStub::buildRequest(const std::vector<QString> & stringList,
                                       const std::vector<QImage*> & imageList,
                                       const std::vector<QString> descriptionList,
                                       const float minimumConfidence)
{
  // Check that the number of descriptions matches the number of options
  if(imageList.size() + stringList.size() != descriptionList.size())
      assert(0);
  request.clear_compressedimageoptions();
  request.clear_imageoptions();
  request.clear_stringoptions();

  unsigned int descriptionIterator;

  for(int i = 0; i < imageList.size(); ++i)
  {
      QImage * img = imageList[i];
      QByteArray ba;
      QBuffer buffer(&ba);
      buffer.open(QIODevice::WriteOnly);
      img->save(buffer, "PNG");
      request.add_compressedimageoptions();
      graspit_rpcz::GetOptionSelectionRequest_CompressedImageOption * cio = request.mutable_compressedimageoptions(i);
      std::string * image_data = cio->mutable_option()->mutable_data();
      image_data->copy(ba.data(),ba.size());
      *(cio->mutable_option()->mutable_format()) = "png";
      cio->mutable_description()->mutable_description()->copy(descriptionList[descriptionIterator++]);
  }
  request.set_minimumconfidencelevel(minimumConfidence);
}

void OptionSelectionStub::sendRequestImpl()
{
    optionSelection_stub.run(request, &response, _rpc, rpcz::new_callback<OptionSelectionStub>(this, &OptionSelectionStub::callback));
}

void OptionSelectionStub::callbackImpl()
{
return;
}
