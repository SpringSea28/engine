#include "GridMatrix.h"
#include "zint.h"
using namespace sojet::barcode;

GridMatrix::GridMatrix(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_GRIDMATRIX;
}

int GridMatrix::getECLevel()
{
    return symbol->option_1;
}

void GridMatrix::setECLevel(int level)
{
    symbol->option_1 = level;
}

int GridMatrix::getVersion()
{
    return symbol->option_2;
}

void GridMatrix::setVersion(int version)
{
    symbol->option_2 = version;
}

