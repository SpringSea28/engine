#include "Code39.h"
#include "zint.h"
using namespace sojet::barcode;

Code39::Code39(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_EXCODE39;
}

bool Code39::hasCheckDigit() {
    return symbol->option_2 == 1;
}

void Code39::setCheckDigit(bool en) {
    symbol->option_2 = en ? 1 : 0;
}

int Code39::getMode() {
    int format = symbol->symbology;

    switch (format) {
        case BARCODE_CODE39: return STANDARD; break;
        case BARCODE_EXCODE39: return EXTENDED; break;
        default: break; // TODO throw exception??
    }
    return STANDARD;
}

void Code39::setMode(int mode) {
    int formats[] = {BARCODE_CODE39, BARCODE_EXCODE39};
    symbol->symbology = formats[mode];
}

