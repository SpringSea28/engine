#ifndef __ITF14_H__
#define __ITF14_H__

#include "BarcodeCommon.h"
#include "SkiaBarcode.h"

namespace sojet {
namespace barcode {

class ITF14 : public SkiaBarcode {
 public:
  ITF14(float barHeight, float xdimension);
  ~ITF14(){};

  void setBearerBarType(int type) override;

  void setBearerBarThickness(int thickness) override;

 private:
  BearerBarType m_bearer_bar_type = BearerBarType::BEARER_BAR_NONE;
};

}  // namespace barcode
}  // namespace sojet

#endif  //__ITF14_H__
