#include "Pdf417.h"
#include "zint.h"
using namespace sojet::barcode;

Pdf417::Pdf417(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension) {
    setMode(NORMAL);
    setDataColumns(0);
    setECLevel(-1);
}

int Pdf417::getMode() {
    switch(symbol->symbology) {
        case BARCODE_PDF417: return NORMAL;
        case BARCODE_MICROPDF417: return MICRO;
        case BARCODE_PDF417TRUNC: return TRUNCATED;
        default: break; // TODO throw exception??
    }
    return NORMAL;
}

void Pdf417::setMode(int mode) {
    int formats[] = {BARCODE_PDF417, BARCODE_MICROPDF417, BARCODE_PDF417TRUNC};
    symbol->symbology = formats[mode];
}

int Pdf417::getDataColumns() {
    return symbol->option_2;
}

void Pdf417::setDataColumns(int columns) {
    symbol->option_2 = columns;
}

int Pdf417::getBarcodeRows() {
    return symbol->rows;
}

void Pdf417::setBarcodeRows(int rows) {
    symbol->rows = rows;
}

int Pdf417::getECLevel() {
    return symbol->option_1;
}

void Pdf417::setECLevel(int level) {
    symbol->option_1 = level;
}

