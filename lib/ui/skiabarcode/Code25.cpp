#include "Code25.h"
#include "zint.h"
using namespace sojet::barcode;

Code25::Code25(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    setMode(STANDARD);
}

int Code25::getMode() {
    int format = symbol->symbology;
    switch (format) {
        case BARCODE_C25STANDARD: return STANDARD;
        case BARCODE_C25INTER: return INTERLEAVED;
        case BARCODE_C25IATA: return IATA;
        case BARCODE_C25LOGIC: return  DATA_LOGIC;
        case BARCODE_C25IND: return INDUSTRIAL;
        default: break; // TODO throw exception??
    }
    return STANDARD;
}

void Code25::setMode(int mode) {
    int formats[] = {BARCODE_C25STANDARD, BARCODE_C25INTER,
                     BARCODE_C25IATA, BARCODE_C25LOGIC, BARCODE_C25IND};
    symbol->symbology = formats[mode];
}

bool Code25::hasCheckDigit() {
    return symbol->option_2 == 1;
}

void Code25::setCheckDigit(bool en) {
    symbol->option_2 = en ? 1 : 0;
}

void Code25::setShowCheckDigits(bool showCheckDigits) {
    symbol->option_2 = (showCheckDigits) ? 2 : 1;
}

bool Code25::getShowCheckDigits() {
    return (symbol->option_2 == 2) ? true : false;
}

