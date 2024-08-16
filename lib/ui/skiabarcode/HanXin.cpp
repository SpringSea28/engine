#include "HanXin.h"
#include "zint.h"
using namespace sojet::barcode;

HanXin::HanXin(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_HANXIN;
}

int HanXin::getECLevel() {
  return symbol->option_1;
}

void HanXin::setECLevel(int level) {
  symbol->option_1 = level;
}

int HanXin::getVersion() {
  return symbol->option_2;
}

void HanXin::setVersion(int version) {
  symbol->option_2 = version;
}