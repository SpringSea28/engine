#include "Ean.h"
#include "zint.h"
using namespace sojet::barcode;

Ean::Ean(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_EANX;
}

float Ean::getGuardPatternExtraHeight()
{
    return m_fGuardPatternExtraHeight;
}

void Ean::setGuardPatternExtraHeight(float guardPatternExtraHeight)
{
    m_fGuardPatternExtraHeight = guardPatternExtraHeight;
}

