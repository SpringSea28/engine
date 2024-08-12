#include "HanXin.h"
#include "zint.h"
using namespace sojet::barcode;

HanXin::HanXin(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_HANXIN;
}


