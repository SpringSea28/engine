#ifndef __HanXin_H__
#define __HanXin_H__

#include "SkiaBarcode.h"

namespace sojet {
namespace barcode {

class HanXin : public SkiaBarcode {
 public:
  HanXin(float barHeight, float xdimension);
  ~HanXin(){};
};

}  // namespace barcode
}  // namespace sojet

#endif  //__HanXin_H__
