#include "UPC.h"
#include "zint.h"
using namespace sojet::barcode;

UPC::UPC(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension) {
    setMode(UPCA);
}

int UPC::getMode() {
    switch (symbol->symbology) {
        case BARCODE_UPCA: return UPCA;
        case BARCODE_UPCE: return UPCE;
        default: return UPCA;
    }
}

void UPC::setMode(int mode) {
    switch (mode) {
        case UPCA: symbol->symbology = BARCODE_UPCA; break;
        case UPCE: symbol->symbology = BARCODE_UPCE; break;
    }
}

float UPC::getGuardPatternExtraHeight() {
	return m_fGuardPatternExtraHeight;
}

void UPC::setGuardPatternExtraHeight(float guardPatternExtraHeight) {
    m_fGuardPatternExtraHeight = guardPatternExtraHeight;
}
