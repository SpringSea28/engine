#include "ITF14.h"
#include "zint.h"
#include "BarcodeCommon.h"
using namespace sojet::barcode;

ITF14::ITF14(float barHeight, float xdimension) :
    SkiaBarcode(barHeight, xdimension)
{
    symbol->symbology = BARCODE_ITF14;
    symbol->output_options |= BARCODE_NO_QUIET_ZONES;
}


void ITF14::setBearerBarType(int type) {
  switch (type) {
    case BEARER_BAR_RECT:
      symbol->output_options |= BARCODE_BOX;
      m_bearer_bar_type = BEARER_BAR_RECT;
      break;
    case BEARER_BAR_BIND:
      symbol->output_options |= BARCODE_BIND;
      m_bearer_bar_type = BEARER_BAR_BIND;
      break;
    case BEARER_BAR_NONE:
      symbol->output_options |= BARCODE_BOX;
      m_bearer_bar_type = BEARER_BAR_NONE;
      break;
    default:
      break;
  }
}

void ITF14::setBearerBarThickness(int thickness) {
  symbol->border_width = thickness;
  if( m_bearer_bar_type == BEARER_BAR_NONE){
    symbol->border_width = 0;
  }
}

