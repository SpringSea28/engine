#include "Code128.h"
#include "zint.h"
using namespace sojet::barcode;

Code128::Code128(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_CODE128;
}

bool Code128::gs1Supported() {
    return true;
}

