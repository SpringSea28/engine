#include "DataMatrix.h"
#include "zint.h"
using namespace sojet::barcode;

DataMatrix::DataMatrix(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_DATAMATRIX;
}

int DataMatrix::getVersion()
{
    return symbol->option_2;
}

void DataMatrix::setVersion(int version)
{
    if (version == DMATRIX_SIZE_AUTO_SQUARE) {
        symbol->option_2 = 0;
        symbol->option_3 = DM_SQUARE;
    } else if (version == DMATRIX_SIZE_AUTO_RECT) {
        symbol->option_2 = 0;
        symbol->option_3 = DM_DMRE;
    } else {
        symbol->option_2 = version-1;
    }
}

bool DataMatrix::gs1Supported()
{
    return true;
}

bool DataMatrix::getGSSeparator()
{
    return (symbol->output_options & GS1_GS_SEPARATOR);
}

void DataMatrix::setGSSeparator(bool gsSep)
{
    if(gsSep) {
        symbol->output_options |= GS1_GS_SEPARATOR;
    }
}


