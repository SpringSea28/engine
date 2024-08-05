#include "MaxiCode.h"
#include "zint.h"
using namespace sojet::barcode;

MaxiCode::MaxiCode(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_MAXICODE;
}

