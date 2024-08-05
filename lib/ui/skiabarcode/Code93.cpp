#include "Code93.h"
#include "zint.h"
using namespace sojet::barcode;

Code93::Code93(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_CODE93;
}

void Code93::setShowCheckDigits(bool showCheckDigits) {
    symbol->option_2 = (showCheckDigits) ? 1 : 0;
}

bool Code93::getShowCheckDigits() {
    return (symbol->option_2 == 1) ? true : false;
}

