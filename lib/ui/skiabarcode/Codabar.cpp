#include "Codabar.h"
#include "zint.h"
using namespace sojet::barcode;

Codabar::Codabar(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_CODABAR;
}

Codabar::~Codabar()
{

}

bool Codabar::hasCheckDigit() {
    return symbol->option_2 == 1;
}

void Codabar::setCheckDigit(bool en) {
    symbol->option_2 = en ? 1 : 0;
}

void Codabar::setShowCheckDigits(bool showCheckDigits) {
    symbol->option_2 = (showCheckDigits) ? 2 : 1;
}


bool Codabar::getShowCheckDigits() {
    return (symbol->option_2 == 2) ? true : false;
}

