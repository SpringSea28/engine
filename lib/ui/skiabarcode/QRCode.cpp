#include "QRCode.h"
#include "zint.h"
using namespace sojet::barcode;

QRCode::QRCode(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension) {
    symbol->symbology = BARCODE_QRCODE;
}

int QRCode::getECLevel() {
    return symbol->option_1;
}

void QRCode::setECLevel(int level) {
    symbol->option_1 = level;
}

int QRCode::getVersion() {
    return symbol->option_2;
}

void QRCode::setVersion(int version) {
    symbol->option_2 = version;
}

bool QRCode::gs1Supported(){
    return true;
}

