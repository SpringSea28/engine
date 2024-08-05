#include <assert.h>
#include <iostream>
#include <algorithm>
#include "SkiaBarcode.h"
#include "zint.h"
#include "SkPath.h"
#include "SkColor.h"
#include "SkPictureRecorder.h"
#include "BarcodeCommon.h"

#include "flutter/lib/ui/painting/paint.h"
#include "flutter/display_list/dl_op_flags.h"
#include "flutter/lib/ui/floating_point.h"
#include "flutter/fml/logging.h"
#include "third_party/icu/source/common/unicode/ustring.h"
#include "third_party/icu/source/common/cmemory.h"
#include "flutter/lib/ui/text/font_collection.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/platform_configuration.h"
#include "flutter/third_party/txt/src/txt/paragraph_builder.h"
#include "third_party/skia/modules/skparagraph/include/Metrics.h"
#include "flutter/display_list/dl_color.h"

using namespace sojet::barcode;

/* `is_sane()` flags */
#define IS_SPC_F    0x0001 /* Space */
#define IS_HSH_F    0x0002 /* Hash sign # */
#define IS_AST_F    0x0004 /* Asterisk sign * */
#define IS_PLS_F    0x0008 /* Plus sign + */
#define IS_MNS_F    0x0010 /* Minus sign - */
#define IS_NUM_F    0x0020 /* Number 0-9 */
#define IS_UPO_F    0x0040 /* Uppercase letter, apart from A-F and X */
#define IS_UHX_F    0x0080 /* Uppercase hex A-F */
#define IS_UX__F    0x0100 /* Uppercase X */
#define IS_LWO_F    0x0200 /* Lowercase letter, apart from a-f and x */
#define IS_LHX_F    0x0400 /* Lowercase hex a-f */
#define IS_LX__F    0x0800 /* Lowercase x */
#define IS_C82_F    0x1000 /* CSET82 punctuation (apart from *, + and -) */
#define IS_SIL_F    0x2000 /* SILVER/TECHNETIUM punctuation .$/% (apart from space, + and -) */
#define IS_CLI_F    0x4000 /* CALCIUM INNER punctuation $:/. (apart from + and -) (Codabar) */
#define IS_ARS_F    0x8000 /* ARSENIC uppercase subset (VIN) */
#define IS_CLS_F    (IS_CLI_F | IS_SIL_F)
#define SSET_F  (IS_NUM_F | IS_UHX_F) /* SSET "0123456789ABCDEF" */
#define z_islower(c) ((c) >= 'a' && (c) <= 'z')

/* Is float integral value? (https://stackoverflow.com/a/40404149) */
#define isfintf(arg) (fmodf(arg, 1.0f) == 0.0f)

static const unsigned short flgs[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*00-1F*/
               IS_SPC_F,            IS_C82_F,            IS_C82_F,            IS_HSH_F, /*20-23*/ /*  !"# */
               IS_CLS_F, IS_SIL_F | IS_C82_F,            IS_C82_F,            IS_C82_F, /*24-27*/ /* $%&' */
               IS_C82_F,            IS_C82_F,            IS_AST_F,            IS_PLS_F, /*28-2B*/ /* ()*+ */
               IS_C82_F,            IS_MNS_F, IS_CLS_F | IS_C82_F, IS_CLS_F | IS_C82_F, /*2B-2F*/ /* ,-./ */
               IS_NUM_F,            IS_NUM_F,            IS_NUM_F,            IS_NUM_F, /*30-33*/ /* 0123 */
               IS_NUM_F,            IS_NUM_F,            IS_NUM_F,            IS_NUM_F, /*34-37*/ /* 4567 */
               IS_NUM_F,            IS_NUM_F, IS_CLI_F | IS_C82_F,            IS_C82_F, /*38-3B*/ /* 89:; */
               IS_C82_F,            IS_C82_F,            IS_C82_F,            IS_C82_F, /*3B-3F*/ /* <=>? */
                      0, IS_UHX_F | IS_ARS_F, IS_UHX_F | IS_ARS_F, IS_UHX_F | IS_ARS_F, /*40-43*/ /* @ABC */
    IS_UHX_F | IS_ARS_F, IS_UHX_F | IS_ARS_F, IS_UHX_F | IS_ARS_F, IS_UPO_F | IS_ARS_F, /*44-47*/ /* DEFG */
    IS_UPO_F | IS_ARS_F,            IS_UPO_F, IS_UPO_F | IS_ARS_F, IS_UPO_F | IS_ARS_F, /*48-4B*/ /* HIJK */
    IS_UPO_F | IS_ARS_F, IS_UPO_F | IS_ARS_F, IS_UPO_F | IS_ARS_F,            IS_UPO_F, /*4B-4F*/ /* LMNO */
    IS_UPO_F | IS_ARS_F,            IS_UPO_F, IS_UPO_F | IS_ARS_F, IS_UPO_F | IS_ARS_F, /*50-53*/ /* PQRS */
    IS_UPO_F | IS_ARS_F, IS_UPO_F | IS_ARS_F, IS_UPO_F | IS_ARS_F, IS_UPO_F | IS_ARS_F, /*53-57*/ /* TUVW */
    IS_UX__F | IS_ARS_F, IS_UPO_F | IS_ARS_F, IS_UPO_F | IS_ARS_F,                   0, /*58-5B*/ /* XYZ[ */
                      0,                   0,                   0,            IS_C82_F, /*5B-5F*/ /* \]^_ */
                      0,            IS_LHX_F,            IS_LHX_F,            IS_LHX_F, /*60-63*/ /* `abc */
               IS_LHX_F,            IS_LHX_F,            IS_LHX_F,            IS_LWO_F, /*64-67*/ /* defg */
               IS_LWO_F,            IS_LWO_F,            IS_LWO_F,            IS_LWO_F, /*68-6B*/ /* hijk */
               IS_LWO_F,            IS_LWO_F,            IS_LWO_F,            IS_LWO_F, /*6B-6F*/ /* lmno */
               IS_LWO_F,            IS_LWO_F,            IS_LWO_F,            IS_LWO_F, /*70-73*/ /* pqrs */
               IS_LWO_F,            IS_LWO_F,            IS_LWO_F,            IS_LWO_F, /*74-77*/ /* tuvw */
               IS_LX__F,            IS_LWO_F,            IS_LWO_F,                   0, /*78-7B*/ /* xyz{ */
                      0,                   0,                   0,                   0, /*7B-7F*/ /* |}~D */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*80-9F*/
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*A0-BF*/
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*C0-DF*/
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*E0-FF*/
};

static float stripf(const float arg) {
    return *((volatile const float *) &arg);
}

static int is_matrix(const int symbology) {
    /* Returns 1 if symbology is a matrix design */
    int result = 0;

    switch (symbology) {
        case BARCODE_QRCODE:
        case BARCODE_DATAMATRIX:
        case BARCODE_MICROQR:
        case BARCODE_HIBC_DM:
        case BARCODE_AZTEC:
        case BARCODE_HIBC_QR:
        case BARCODE_HIBC_AZTEC:
        case BARCODE_AZRUNE:
        case BARCODE_CODEONE:
        case BARCODE_GRIDMATRIX:
        case BARCODE_HANXIN:
        case BARCODE_DOTCODE:
        case BARCODE_UPNQR:
        case BARCODE_RMQR:
            result = 1;
            break;
    }

    return result;
}

static int is_composite(int symbology) {
    return symbology >= BARCODE_EANX_CC && symbology <= BARCODE_DBAR_EXPSTK_CC;
}

static int module_is_set(const struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    if (symbol->symbology == BARCODE_ULTRA) {
        return symbol->encoded_data[y_coord][x_coord];
    } else {
        return (symbol->encoded_data[y_coord][x_coord / 8] >> (x_coord % 8)) & 1;
    }
}

/* Return true (1-8) if a module is colour, otherwise false (0) */
static int module_colour_is_set(const struct zint_symbol *symbol, const int y_coord, const int x_coord) {
    return symbol->encoded_data[y_coord][x_coord];
}

/* Return minimum quiet zones for each symbology */
static int out_quiet_zones(const struct zint_symbol *symbol, const int hide_text,
                            float *left, float *right, float *top, float *bottom) {
    int done = 0;
    *left = *right = *top = *bottom = 0.0f;

    /* These always have quiet zones set (previously used whitespace_width) */
    switch (symbol->symbology) {
        case BARCODE_CODE16K:
            /* BS EN 12323:2005 Section 4.5 (c) */
            if (!(symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
                *left = 10.0f;
                *right = 1.0f;
            }
            done = 1;
            break;
        case BARCODE_CODE49:
            /* ANSI/AIM BC6-2000 Section 2.4 */
            if (!(symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
                *left = 10.0f;
                *right = 1.0f;
            }
            done = 1;
            break;
        case BARCODE_CODABLOCKF:
        case BARCODE_HIBC_BLOCKF:
            /* AIM ISS-X-24 Section 4.6.1 */
            if (!(symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
                *left = *right = 10.0f;
            }
            done = 1;
            break;
        case BARCODE_ITF14:
            /* GS1 General Specifications 21.0.1 Section 5.3.2.2 */
            if (!(symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
                *left = *right = 10.0f;
            }
            done = 1;
            break;
        case BARCODE_EANX:
        case BARCODE_EANX_CHK:
        case BARCODE_EANX_CC:
        case BARCODE_ISBNX:
            /* GS1 General Specifications 21.0.1 Section 5.2.3.4 */
            switch (strlen((const char*)symbol->text)) {
                case 13: /* EAN-13 */
                    if (!(symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
                        *left = 11.0f;
                        *right = 7.0f;
                    } else if (!hide_text) {
                        *left = 11.0f; /* Need for outside left digit */
                    }
                    break;
                case 16: /* EAN-13/ISBN + 2 digit addon */
                case 19: /* EAN-13/ISBN + 5 digit addon */
                    if (!(symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
                        *left = 11.0f;
                        *right = 5.0f;
                    } else if (!hide_text) {
                        *left = 11.0f; /* Need for outside left digit */
                    }
                    break;
                case 5: /* EAN-5 addon */
                case 2: /* EAN-2 addon */
                    if (!(symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
                        *left = 7.0f;
                        *right = 5.0f;
                    }
                    break;
                default: /* EAN-8 (+/- 2/5 digit addon) */
                    if (!(symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
                        *left = *right = 7.0f;
                    }
                    break;
            }
            done = 1;
            break;
        case BARCODE_UPCA:
        case BARCODE_UPCA_CHK:
        case BARCODE_UPCA_CC:
            /* GS1 General Specifications 21.0.1 Section 5.2.3.4 */
            if (!(symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
                *left = 9.0f;
                if (strlen((const char*)symbol->text) > 12) { /* UPC-A + addon */
                    *right = 5.0f;
                } else {
                    *right = 9.0f;
                }
            } else if (!hide_text) {
                *left = 9.0f; /* Need for outside left digit */
                if (strlen((const char*)symbol->text) <= 12) { /* No addon */
                    *right = 9.0f; /* Need for outside right digit */
                }
            }
            done = 1;
            break;
        case BARCODE_UPCE:
        case BARCODE_UPCE_CHK:
        case BARCODE_UPCE_CC:
            /* GS1 General Specifications 21.0.1 Section 5.2.3.4 */
            if (!(symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
                *left = 9.0f;
                if (strlen((const char*)symbol->text) > 8) { /* UPC-E + addon */
                    *right = 5.0f;
                } else {
                    *right = 7.0f;
                }
            } else if (!hide_text) {
                *left = 9.0f; /* Need for outside left digit */
                if (strlen((const char*)symbol->text) <= 8) { /* No addon */
                    *right = 7.0f; /* Need for outside right digit */
                }
            }
            done = 1;
            break;
    }

    if (done) {
        return done;
    }

    /* Only do others if flag set */
    if (!(symbol->output_options & BARCODE_QUIET_ZONES) || (symbol->output_options & BARCODE_NO_QUIET_ZONES)) {
        return 0;
    }

    switch (symbol->symbology) {
        case BARCODE_CODE11:
            /* No known standard. Following ITF-14, set to 10X */
            *left = *right = 10.0f;
            done = 1;
            break;

        case BARCODE_C25INTER:
            /* ISO/IEC 16390:2007 Section 4.4 10X */
            *left = *right = 10.0f;
            done = 1;
            break;
        case BARCODE_C25STANDARD:
        case BARCODE_C25IATA:
        case BARCODE_C25LOGIC:
        case BARCODE_C25IND:
            /* No known standards. Following C25INTER, set to 10X */
            *left = *right = 10.0f;
            done = 1;
            break;

        case BARCODE_CODE39:
        case BARCODE_EXCODE39:
        case BARCODE_LOGMARS:
        case BARCODE_PZN:
        case BARCODE_VIN:
        case BARCODE_HIBC_39:
        case BARCODE_CODE32:
            /* ISO/IEC 16388:2007 Section 4.4 (d) */
            *left = *right = 10.0f;
            done = 1;
            break;
        case BARCODE_GS1_128: /* GS1-128 */
        case BARCODE_GS1_128_CC:
        case BARCODE_EAN14:
            /* GS1 General Specifications 21.0.1 Section 5.4.4.2 */
            *left = *right = 10.0f;
            done = 1;
            break;
        case BARCODE_CODABAR:
            /* BS EN 798:1995 Section 4.4.1 (d) */
            *left = *right = 10.0f;
            done = 1;
            break;
        case BARCODE_CODE128:
        case BARCODE_CODE128B:
        case BARCODE_HIBC_128:
        case BARCODE_NVE18:
            /* ISO/IEC 15417:2007 4.4.2 */
            *left = *right = 10.0f;
            done = 1;
            break;
        case BARCODE_DPLEIT:
        case BARCODE_DPIDENT:
            /* Using CODE39 values TODO: Find doc */
            *left = *right = 10.0f;
            done = 1;
            break;

        case BARCODE_CODE93:
            /* ANSI/AIM BC5-1995 Section 2.4 */
            *left = *right = 10.0f;
            done = 1;
            break;

        case BARCODE_FLAT:
            /* TODO: Find doc (application defined according to TEC-IT) */
            break;

        case BARCODE_DBAR_OMN: /* GS1 Databar Omnidirectional */
        case BARCODE_DBAR_LTD: /* GS1 Databar Limited */
        case BARCODE_DBAR_EXP: /* GS1 Databar Expanded */
        case BARCODE_DBAR_STK: /* GS1 DataBar Stacked */
        case BARCODE_DBAR_OMNSTK: /* GS1 DataBar Stacked Omnidirectional */
        case BARCODE_DBAR_EXPSTK: /* GS1 Databar Expanded Stacked */
            /* GS1 General Specifications 21.0.1 Section 5.5.1.1 - Quiet Zones: None required */
            done = 1;
            break;
        case BARCODE_DBAR_OMN_CC:
        case BARCODE_DBAR_LTD_CC:
        case BARCODE_DBAR_EXP_CC:
        case BARCODE_DBAR_STK_CC:
        case BARCODE_DBAR_OMNSTK_CC:
        case BARCODE_DBAR_EXPSTK_CC:
            /* GS1 General Specifications 21.0.1 Sections 5.11.2.1 (CC-A) & 5.11.2.2 (CC-B) */
            *left = *right = 1.0f;
            done = 1;
            break;

        case BARCODE_TELEPEN:
        case BARCODE_TELEPEN_NUM:
            /* Appears to be ~10X from diagram in Telepen SkiaBarcode Symbology information and History */
            /* TODO: Find better doc */
            *left = *right = 10.0f;
            done = 1;
            break;

        case BARCODE_POSTNET:
        case BARCODE_PLANET:
            /* USPS DMM 300 2006 (2011) 5.7 SkiaBarcode in Address Block
               left/right 0.125" / 0.025" (X max) = 5, top/bottom 0.04" / 0.025" (X max) = 1.6 */
            *left = *right = 5.0f;
            *top = *bottom = 1.6f;
            done = 1;
            break;
        case BARCODE_CEPNET:
            /* CEPNet e Código Bidimensional Datamatrix 2D (26/05/2021) 3.8 Localização */
            *left = *right = 10.0f;
            *top = *bottom = 1.6f; /* As POSTNET (1.016mm == 0.025") */
            done = 1;
            break;

        case BARCODE_MSI_PLESSEY:
            /* TODO Find doc (TEC-IT says 12X so use that for the moment) */
            *left = *right = 12.0f;
            done = 1;
            break;

        case BARCODE_FIM:
            /* USPS DMM 300 2006 (2011) 708.9.3 (top/bottom zero)
               right 0.125" (min) / 0.03925" (X max) ~ 3.18, left 1.25" - 0.66725" (max width of barcode)
               - 0.375 (max right) = 0.20775" / 0.03925" (X max) ~ 5.29 */
            *right = (float) (0.125 / 0.03925);
            *left = (float) (0.20775 / 0.03925);
            done = 1;
            break;
        case BARCODE_PHARMA:
        case BARCODE_PHARMA_TWO:
            /* Laetus Pharmacode Guide 2.2 from 6mm depending on context, 6mm / 1mm (Pharma Two X) = 6 */
            *left = *right = 6.0f;
            done = 1;
            break;
        case BARCODE_PDF417:
        case BARCODE_PDF417COMP:
        case BARCODE_HIBC_PDF:
            /* ISO/IEC 15438:2015 Section 5.8.3 */
            *left = *right = *top = *bottom = 2.0f;
            done = 1;
            break;
        case BARCODE_MICROPDF417:
        case BARCODE_HIBC_MICPDF:
            /* ISO/IEC 24728:2006 Section 5.8.3 */
            *left = *right = *top = *bottom = 1.0f;
            done = 1;
            break;
        case BARCODE_MAXICODE:
            /* ISO/IEC 16023:2000 Section 4.11.5 */
            *left = *right = *top = *bottom = 1.0f;
            done = 1;
            break;
        case BARCODE_QRCODE:
        case BARCODE_UPNQR:
        case BARCODE_HIBC_QR:
            /* ISO/IEC 18004:2015 Section 9.1 */
            *left = *right = *top = *bottom = 4.0f;
            done = 1;
            break;
        case BARCODE_DPD:
            /* Specification DPD and primetime Parcel Despatch 4.0.2 Section 5.5.1 5mm / 0.4mm (X max) = 12.5 */
            *left = *right = 12.5f;
            done = 1;
            break;
        case BARCODE_MICROQR:
            /* ISO/IEC 18004:2015 Section 9.1 */
            *left = *right = *top = *bottom = 2.0f;
            done = 1;
            break;
        case BARCODE_RMQR:
            /* ISO/IEC JTC1/SC31N000 Section 6.3.10 */
            *left = *right = *top = *bottom = 2.0f;
            done = 1;
            break;
        case BARCODE_AUSPOST:
        case BARCODE_AUSREPLY:
        case BARCODE_AUSROUTE:
        case BARCODE_AUSREDIRECT:
            /* Customer SkiaBarcode Technical Specifications (2012) left/right 6mm / 0.6mm = 10,
               top/bottom 2mm / 0.6mm ~ 3.33 (X max) */
            *left = *right = 10.0f;
            *top = *bottom = (float) (2.0 / 0.6);
            done = 1;
            break;
        case BARCODE_RM4SCC:
            /* Royal Mail Know How User's Manual Appendix C: using CBC, same as MAILMARK, 2mm all round,
               use X max (25.4mm / 39) i.e. 20 bars per 25.4mm */
            *left = *right = *top = *bottom = (float) ((2.0 * 39.0) / 25.4); /* ~ 3.07 */
            done = 1;
            break;
        case BARCODE_DATAMATRIX:
        case BARCODE_HIBC_DM:
            /* ISO/IEC 16022:2006 Section 7.1 */
            *left = *right = *top = *bottom = 1.0f;
            done = 1;
            break;
        case BARCODE_JAPANPOST:
            /* Japan Post Zip/SkiaBarcode Manual p.13 2mm all round, X 0.6mm, 2mm / 0.6mm ~ 3.33 */
            *left = *right = *top = *bottom = (float) (2.0 / 0.6);
            done = 1;
            break;

        case BARCODE_KOREAPOST:
            /* TODO Find doc (TEC-IT uses 10X but says not exactly specified - do the same for the moment) */
            *left = *right = 10.0f;
            done = 1;
            break;

        case BARCODE_USPS_IMAIL:
            /* USPS-B-3200 (2015) Section 2.3.2 left/right 0.125", top/bottom 0.026", use X max (1 / 39)
               i.e. 20 bars per inch */
            *left = *right = 0.125f * 39.0f; /* 4.875 */
            *top = *bottom = 0.026f * 39.0f; /* 1.014 */
            done = 1;
            break;

        case BARCODE_PLESSEY:
            /* TODO Find doc (see MSI_PLESSEY) */
            *left = *right = 12.0f;
            done = 1;
            break;

        case BARCODE_KIX:
            /* Handleiding KIX code brochure - same as RM4SCC/MAILMARK */
            *left = *right = *top = *bottom = (float) ((2.0 * 39.0) / 25.4); /* ~ 3.07 */
            done = 1;
            break;
        case BARCODE_AZTEC:
        case BARCODE_HIBC_AZTEC:
        case BARCODE_AZRUNE:
            /* ISO/IEC 24778:2008 Section 4.1 (c) & Annex A.1 (Rune) - no quiet zone required */
            done = 1;
            break;
        case BARCODE_DAFT:
            /* Generic so unlikely to be defined */
            done = 1;
            break;
        case BARCODE_DOTCODE:
            /* ISS DotCode Rev. 4.0 Section 4.1 (3) (c) */
            *left = *right = *top = *bottom = 3.0f;
            done = 1;
            break;
        case BARCODE_HANXIN:
            /* ISO/IEC DIS 20830:2019 Section 4.2.8 (also Section 6.2) */
            *left = *right = *top = *bottom = 3.0f;
            done = 1;
            break;
        case BARCODE_MAILMARK:
            /* Royal Mail Mailmark SkiaBarcode Definition Document Section 3.5.2, 2mm all round, use X max (25.4mm / 39)
               i.e. 20 bars per 25.4mm */
            *left = *right = *top = *bottom = (float) ((2.0 * 39.0) / 25.4); /* ~ 3.07 */
            done = 1;
            break;
        case BARCODE_CHANNEL:
            /* ANSI/AIM BC12-1998 Section 4.4 (c) */
            *left = 1.0f;
            *right = 2.0f;
            done = 1;
            break;

        case BARCODE_CODEONE:
            /* USS Code One AIM 1994 Section 2.2.4 No quiet zone required for Versions A to H */
            if (symbol->option_2 == 9 || symbol->option_2 == 10) { /* Section 2.3.2 Versions S & T */
                *left = *right = 1.0f;
            }
            done = 1;
            break;

        case BARCODE_GRIDMATRIX:
            /* AIMD014 (v 1.63) Section 7.1 */
            *left = *right = *top = *bottom = 6.0f;
            done = 1;
            break;
        case BARCODE_ULTRA:
            /* AIMD/TSC15032-43 (v 0.99c) Section 9.2 */
            *left = *right = *top = *bottom = 1.0f;
            done = 1;
            break;

        case BARCODE_BC412:
            /* SEMI T1-95 Table 4 */
            *left = *right = 10.0f;
            done = 1;
            break;
    }

    return done; /* For self-checking */
}

static float out_large_bar_height(struct zint_symbol *symbol, int si, int *row_heights_si, int *symbol_height_si) {
    float fixed_height = 0.0f;
    int zero_count = 0;
    int round_rows = 0;
    int i;
    float large_bar_height = 0.0f; /* Not used if zero_count zero */

    if (si) {
        for (i = 0; i < symbol->rows; i++) {
            if (symbol->row_height[i]) {
                fixed_height += symbol->row_height[i];
                if (!round_rows && !isfintf(symbol->row_height[i] * si)) {
                    round_rows = 1;
                }
            } else {
                zero_count++;
            }
        }

        if (zero_count) {
            large_bar_height = stripf((symbol->height - fixed_height) / zero_count);
            assert(large_bar_height >= 0.5f); /* Min row height as set by `set_height()` */
            if (!isfintf(large_bar_height * si)) {
                large_bar_height = stripf((float) floor(large_bar_height * si) / si);
            }
            symbol->height = stripf(large_bar_height * zero_count + fixed_height);
            /* Note should never happen that have both zero_count and round_rows */
        } else {
            if (round_rows) {
                float total_height = 0.0f;
                for (i = 0; i < symbol->rows; i++) {
                    if (!isfintf(symbol->row_height[i] * si)) {
                        symbol->row_height[i] = (float) floor(symbol->row_height[i] * si) / si;
                    }
                    total_height += symbol->row_height[i];
                }
                symbol->height = stripf(total_height);
            }
        }

        if (row_heights_si) {
            assert(symbol_height_si);
            *symbol_height_si = 0;
            for (i = 0; i < symbol->rows; i++) {
                if (symbol->row_height[i]) {
                    row_heights_si[i] = (int) floor(symbol->row_height[i] * si);
                } else {
                    row_heights_si[i] = (int) floor(large_bar_height * si);
                }
                *symbol_height_si += row_heights_si[i];
            }
        }
    } else {
        for (i = 0; i < symbol->rows; i++) {
            if (symbol->row_height[i]) {
                fixed_height += symbol->row_height[i];
            } else {
                zero_count++;
            }
        }
        if (zero_count) {
            large_bar_height = stripf((symbol->height - fixed_height) / zero_count);
            assert(large_bar_height >= 0.5f); /* Min row height as set by `set_height()` */
            symbol->height = stripf(large_bar_height * zero_count + fixed_height);
        }
    }

    return large_bar_height;
}

static int is_extendable(const int symbology) {
    switch (symbology) {
        case BARCODE_EANX:
        case BARCODE_EANX_CHK:
        case BARCODE_UPCA:
        case BARCODE_UPCA_CHK:
        case BARCODE_UPCE:
        case BARCODE_UPCE_CHK:
        case BARCODE_ISBNX:
        case BARCODE_EANX_CC:
        case BARCODE_UPCA_CC:
        case BARCODE_UPCE_CC:
            return 1;
    }

    return 0;
}

/* Set left (x), top (y), right and bottom offsets for whitespace */
static void out_set_whitespace_offsets(const struct zint_symbol *symbol, const int hide_text,
                float *xoffset, float *yoffset, float *roffset, float *boffset, const float scaler,
                int *xoffset_si, int *yoffset_si, int *roffset_si, int *boffset_si) {
    float qz_left, qz_right, qz_top, qz_bottom;

    out_quiet_zones(symbol, hide_text, &qz_left, &qz_right, &qz_top, &qz_bottom);

    *xoffset = symbol->whitespace_width + qz_left;
    *roffset = symbol->whitespace_width + qz_right;
    if (symbol->output_options & BARCODE_BOX) {
        *xoffset += symbol->border_width;
        *roffset += symbol->border_width;
    }

    *yoffset = symbol->whitespace_height + qz_top;
    *boffset = symbol->whitespace_height + qz_bottom;
    if (symbol->output_options & (BARCODE_BOX | BARCODE_BIND)) {
        *yoffset += symbol->border_width;
        *boffset += symbol->border_width;
    }

    if (scaler) {
        if (xoffset_si) {
            *xoffset_si = (int) (*xoffset * scaler);
        }
        if (yoffset_si) {
            *yoffset_si = (int) (*yoffset * scaler);
        }
        if (roffset_si) {
            *roffset_si = (int) (*roffset * scaler);
        }
        if (boffset_si) {
            *boffset_si = (int) (*boffset * scaler);
        }
    }
}


/* Set composite offset and main width excluding addon (for start of addon calc) and addon text, returning
   UPC/EAN type */
static int out_process_upcean(const struct zint_symbol *symbol, int *p_main_width, int *p_comp_xoffset,
                unsigned char addon[6], int *p_addon_gap) {
    int main_width; /* Width of main linear symbol, excluding addon */
    int comp_xoffset; /* Whitespace offset (if any) of main linear symbol due to having composite */
    int upceanflag; /* UPC/EAN type flag */
    int i, j, latch;
    int text_length = (int) strlen((const char*)symbol->text);

    latch = 0;
    j = 0;
    /* Isolate add-on text */
    for (i = 6; i < text_length && j < 5; i++) {
        if (latch == 1) {
            addon[j] = symbol->show_hrt ? symbol->text[i] : ' '; /* Use dummy space-filled addon if no hrt */
            j++;
        } else if (symbol->text[i] == '+') {
            latch = 1;
        }
    }
    addon[j] = '\0';
    if (latch) {
        if (symbol->symbology == BARCODE_UPCA || symbol->symbology == BARCODE_UPCA_CHK
                || symbol->symbology == BARCODE_UPCA_CC) {
            *p_addon_gap = symbol->option_2 >= 9 && symbol->option_2 <= 12 ? symbol->option_2 : 9;
        } else {
            *p_addon_gap = symbol->option_2 >= 7 && symbol->option_2 <= 12 ? symbol->option_2 : 7;
        }
    }

    /* Calculate composite offset */
    comp_xoffset = 0;
    if (is_composite(symbol->symbology)) {
        while (!(module_is_set(symbol, symbol->rows - 1, comp_xoffset))) {
            comp_xoffset++;
        }
    }

    upceanflag = 0;
    main_width = symbol->width;
    if ((symbol->symbology == BARCODE_EANX) || (symbol->symbology == BARCODE_EANX_CHK)
            || (symbol->symbology == BARCODE_EANX_CC) || (symbol->symbology == BARCODE_ISBNX)) {
        switch (text_length) {
            case 13: /* EAN-13 */
            case 16: /* EAN-13 + EAN-2 */
            case 19: /* EAN-13 + EAN-5 */
                main_width = 95 + comp_xoffset; /* EAN-13 main symbol 95 modules wide */
                upceanflag = 13;
                break;
            case 2:
                /* EAN-2 can't have addon or be composite */
                upceanflag = 2;
                break;
            case 5:
                /* EAN-5 can't have addon or be composite */
                upceanflag = 5;
                break;
            default:
                main_width = 68 + comp_xoffset; /* EAN-8 main symbol 68 modules wide */
                upceanflag = 8;
                break;
        }
    } else if ((symbol->symbology == BARCODE_UPCA) || (symbol->symbology == BARCODE_UPCA_CHK)
            || (symbol->symbology == BARCODE_UPCA_CC)) {
        main_width = 95 + comp_xoffset; /* UPC-A main symbol 95 modules wide */
        upceanflag = 12;
    } else if ((symbol->symbology == BARCODE_UPCE) || (symbol->symbology == BARCODE_UPCE_CHK)
            || (symbol->symbology == BARCODE_UPCE_CC)) {
        main_width = 51 + comp_xoffset; /* UPC-E main symbol 51 modules wide */
        upceanflag = 6;
    }

    *p_comp_xoffset = comp_xoffset;
    *p_main_width = main_width;

    return upceanflag;
}

static void out_upcean_split_text(int upceanflag, unsigned char text[],
                unsigned char textpart1[5], unsigned char textpart2[7], unsigned char textpart3[7],
                unsigned char textpart4[2]) {
    int i;

    if (upceanflag == 6) { /* UPC-E */
        textpart1[0] = text[0];
        textpart1[1] = '\0';

        for (i = 0; i < 6; i++) {
            textpart2[i] = text[i + 1];
        }
        textpart2[6] = '\0';

        textpart3[0] = text[7];
        textpart3[1] = '\0';

    } else if (upceanflag == 8) { /* EAN-8 */
        for (i = 0; i < 4; i++) {
            textpart1[i] = text[i];
        }
        textpart1[4] = '\0';

        for (i = 0; i < 4; i++) {
            textpart2[i] = text[i + 4];
        }
        textpart2[4] = '\0';

    } else if (upceanflag == 12) { /* UPC-A */
        textpart1[0] = text[0];
        textpart1[1] = '\0';

        for (i = 0; i < 5; i++) {
            textpart2[i] = text[i + 1];
        }
        textpart2[5] = '\0';

        for (i = 0; i < 5; i++) {
            textpart3[i] = text[i + 6];
        }
        textpart3[5] = '\0';

        textpart4[0] = text[11];
        textpart4[1] = '\0';

    } else if (upceanflag == 13) { /* EAN-13 */
        textpart1[0] = text[0];
        textpart1[1] = '\0';

        for (i = 0; i < 6; i++) {
            textpart2[i] = text[i + 1];
        }
        textpart2[6] = '\0';

        for (i = 0; i < 6; i++) {
            textpart3[i] = text[i + 7];
        }
        textpart3[6] = '\0';
    }
}

/* Indicates which symbologies can have row binding
 * Note: if change this must also change version in frontend/main.c */
static int is_stackable(const int symbology) {
    if (symbology < BARCODE_PDF417) {
        return 1;
    }

    switch (symbology) {
        case BARCODE_CODE128B:
        case BARCODE_ISBNX:
        case BARCODE_EAN14:
        case BARCODE_NVE18:
        case BARCODE_KOREAPOST:
        case BARCODE_PLESSEY:
        case BARCODE_TELEPEN_NUM:
        case BARCODE_ITF14:
        case BARCODE_CODE32:
        case BARCODE_CODABLOCKF:
        case BARCODE_HIBC_BLOCKF:
            return 1;
    }

    return 0;
}

static void error_tag(char error_string[], int error_number) {
    if (error_number != 0) {
        char error_buffer[100];
        strcpy(error_buffer, error_string);

        if (error_number > 4) {
            strcpy(error_string, "Error ");
        } else {
            strcpy(error_string, "Warning ");
        }

        strcat(error_string, error_buffer);
    }
}

static int is_dotty(const int symbology) {
    switch (symbology) {
        /* Note MAXICODE and ULTRA absent */
        case BARCODE_QRCODE:
        case BARCODE_DATAMATRIX:
        case BARCODE_MICROQR:
        case BARCODE_HIBC_DM:
        case BARCODE_AZTEC:
        case BARCODE_HIBC_QR:
        case BARCODE_HIBC_AZTEC:
        case BARCODE_AZRUNE:
        case BARCODE_CODEONE:
        case BARCODE_GRIDMATRIX:
        case BARCODE_HANXIN:
        case BARCODE_DOTCODE:
        case BARCODE_UPNQR:
        case BARCODE_RMQR:
            return 1;
            break;
    }

    return 0;
}

static int is_sane(const unsigned int flg, const unsigned char source[], const int length) {
    int i;
    for (i = 0; i < length; i++) {
        if (!(flgs[source[i]] & flg)) {
            return 0;
        }
    }
    return 1;
}

static void to_upper(unsigned char source[], const int length) {
    int i;
    for (i = 0; i < length; i++) {
        if (z_islower(source[i])) {
            source[i] &= 0x5F;
        }
    }
}

static int out_check_colour_options(struct zint_symbol *symbol) {
    int fg_len = (int) strlen(symbol->fgcolour);
    int bg_len = (int) strlen(symbol->bgcolour);

    if ((fg_len != 6) && (fg_len != 8)) {
        strcpy(symbol->errtxt, "651: Malformed foreground colour (6 or 8 characters only)");
        return ZINT_ERROR_INVALID_OPTION;
    }
    if ((bg_len != 6) && (bg_len != 8)) {
        strcpy(symbol->errtxt, "652: Malformed background colour (6 or 8 characters only)");
        return ZINT_ERROR_INVALID_OPTION;
    }

    to_upper((unsigned char *) symbol->fgcolour, fg_len);
    to_upper((unsigned char *) symbol->bgcolour, bg_len);

    if (!is_sane(SSET_F, (unsigned char *) symbol->fgcolour, fg_len)) {
        sprintf(symbol->errtxt, "653: Malformed foreground colour '%s' (hexadecimal only)", symbol->fgcolour);
        return ZINT_ERROR_INVALID_OPTION;
    }

    if (!is_sane(SSET_F, (unsigned char *) symbol->bgcolour, bg_len)) {
        sprintf(symbol->errtxt, "654: Malformed background colour '%s' (hexadecimal only)", symbol->bgcolour);
        return ZINT_ERROR_INVALID_OPTION;
    }

    return 0;
}

static int is_fixed_ratio(const int symbology) {
    if (is_dotty(symbology)) {
        return 1;
    }

    switch (symbology) {
        case BARCODE_MAXICODE:
        case BARCODE_ULTRA:
            return 1;
            break;
    }

    return 0;
}

static SkColor getColor(int colour) {
    switch (colour) {
        case 1: {// Cyan
            return SK_ColorCYAN;
        }
        case 2: {// Blue
            return SK_ColorBLUE;
        }
        case 3: {// Magenta
            return SK_ColorMAGENTA;
        }
        case 4: {// Red
            return SK_ColorRED;
        }
        case 5: {// Yellow
            return SK_ColorYELLOW;
        }
        case 6: {// Green
            return SK_ColorGREEN;
        }
        case 8: {// White
            return SK_ColorWHITE;
        }
        default: {
            return SK_ColorBLACK;
        }
    }
}

SkiaBarcode::SkiaBarcode(float barHeight, float xdimension)
{
    m_fGuardPatternExtraHeight = 35.0;
    m_iTextHeight = 80;
    m_fBarcodeWidth = 0.0f;
    m_fBarcodeHeight = 0.0f;
    m_iTextMargin = 0;

    symbol = ZBarcode_Create();
    m_XDimensions = xdimension;
    symbol->height = barHeight;
    m_Font.setSize(m_iTextHeight);
}

SkiaBarcode::~SkiaBarcode() {
    if (symbol) {
        ZBarcode_Delete(symbol);
    }
}

float SkiaBarcode::getXDimensions() {
    return m_XDimensions;
}

void SkiaBarcode::setXDimensions(float xdimensions) {
    m_XDimensions = xdimensions;
}

bool SkiaBarcode::isGS1NoCheck() {
    return symbol->input_mode & GS1NOCHECK_MODE;
}

int SkiaBarcode::getDataType() {
    return symbol->input_mode & 0x07;
}

void SkiaBarcode::setDataType(int dataType, bool bGS1NoCheck, bool bParseEscapes) {
    symbol->input_mode = dataType;
    if(dataType == GS1_MODE && bGS1NoCheck) {
        symbol->input_mode |= GS1NOCHECK_MODE;
    }

    if(bParseEscapes) {
        symbol->input_mode |= ESCAPE_MODE;
    }
}

float SkiaBarcode::getBarHeight() {
    return symbol->height;
}

void SkiaBarcode::setBarHeight(float barHeight) {
    symbol->height = barHeight;
}

int SkiaBarcode::getQuietZoneWidth() {
    return symbol->whitespace_width;
}

void SkiaBarcode::setQuietZoneWidth(int iQuietZoneWidth) {
    symbol->whitespace_width = iQuietZoneWidth;
}

int SkiaBarcode::getQuietZoneHeight() {
	return symbol->whitespace_height;
}

void SkiaBarcode::setQuietZoneHeight(int iQuietZoneHeight) {
    symbol->whitespace_height = iQuietZoneHeight;
}

float SkiaBarcode::getWidth() {
    return m_fBarcodeWidth;
}

float SkiaBarcode::getHeight() {
    return m_fBarcodeHeight;
}

int SkiaBarcode::getHumanReadableLocation() {
    return symbol->show_hrt;
}

void SkiaBarcode::setHumanReadableLocation(int humanReadableLocation) {
    symbol->show_hrt = humanReadableLocation;
}

int SkiaBarcode::getTextMargin() {
    return m_iTextMargin;
}

void SkiaBarcode::setTextMargin(int margin) {
    m_iTextMargin = margin;
}

int SkiaBarcode::getBearerBarType() {
    if (symbol->output_options & BARCODE_BOX)
        return BEARER_BAR_RECT;
    else if (symbol->output_options & BARCODE_BIND)
        return BEARER_BAR_BIND;
    return BEARER_BAR_NONE;
}

void SkiaBarcode::setBearerBarType(int type) {
    switch (type) {
        case BEARER_BAR_RECT:
            symbol->output_options |= BARCODE_BOX;
            break;
        case BEARER_BAR_BIND:
            symbol->output_options |= BARCODE_BIND;
            break;
        default:
            break;
    }
}

int SkiaBarcode::getBearerBarThickness() {
    return symbol->border_width;
}

void SkiaBarcode::setBearerBarThickness(int thickness) {
    symbol->border_width = thickness;
}

sk_sp<SkPicture> SkiaBarcode::getBarcodePicture() {
    return m_BarcodePicture;
}

void SkiaBarcode::setFont(const SkFont& font) {
    m_Font = font;
}

void SkiaBarcode::setPaint(const SkPaint& paint) {
    m_Paint = paint;
}

int SkiaBarcode::encode(const char* text, int length) {
    clearBarcode();
    return native_encode(text, length);
}

void SkiaBarcode::clearBarcode() {
    ZBarcode_Clear(symbol);
}

bool SkiaBarcode::gs1Supported() {
    return false;
}

const char* SkiaBarcode::getErrorInfo() {
    return symbol->errtxt;
}

int SkiaBarcode::native_encode(const char* text, int length) {
    if(symbol->input_mode & GS1_MODE) {
        std::string temp = text;
        replace(temp.begin(), temp.end(), '(', '[');
        replace(temp.begin(), temp.end(), ')', ']');

        if(symbol->symbology == BARCODE_CODE128) {
            symbol->symbology = BARCODE_GS1_128;
        }
		return encodeAndBufferVector((unsigned char*)temp.c_str(),length);
    }

    return encodeAndBufferVector((unsigned char*)text,length);
}

void SkiaBarcode::setTextHeight(int iTextHeight) {
    m_iTextHeight = iTextHeight;
}

int SkiaBarcode::encodeAndBufferVector(unsigned char *input, int length) {
    int error_number;

    error_number = ZBarcode_Encode(symbol, input, length);
    if (error_number >= ZINT_ERROR) {
        return error_number;
    }

    error_number = bufferVector();
    if (error_number >= ZINT_ERROR) {
        return error_number;
    }

    return 0;
}

int SkiaBarcode::bufferVector() {
    int error_number = 0;
    if (symbol->output_options & BARCODE_DOTTY_MODE) {
        if (!(is_matrix(symbol->symbology))) {
            strcpy(symbol->errtxt, "238: Selected symbology cannot be rendered as dots");
            error_tag(symbol->errtxt, ZINT_ERROR_INVALID_OPTION);
            return ZINT_ERROR_INVALID_OPTION;
        }
    }

    error_number = plot_vector();
    error_tag(symbol->errtxt, error_number);
    return error_number;
}

int SkiaBarcode::plot_vector() {
    int error_number;
    int main_width;
    int comp_xoffset = 0;
    unsigned char addon[6];
    int addon_gap = 0;
    float addon_text_yposn = 0.0f;
    float xoffset, yoffset, roffset, boffset;
    float textoffset;
    int upceanflag = 0;
    int addon_latch = 0;
    unsigned char textpart1[5], textpart2[7], textpart3[7], textpart4[2];
    int hide_text;
    int i, r;
    int block_width = 0;
    //int text_height; /* Font pixel size (so whole integers) */ //废弃
    float text_gap; /* Gap between barcode and text */
    //float guard_descent; //废弃
    const int is_codablockf = symbol->symbology == BARCODE_CODABLOCKF || symbol->symbology == BARCODE_HIBC_BLOCKF;

    float addon_row_height;
    float large_bar_height;
    //int upcae_outside_text_height; /* UPC-A/E outside digits font size */  //废弃
    float digit_ascent_factor = 0.25f; /* Assuming digit ascent roughly 25% less than font size */
    float dot_overspill = 0.0f;
    float dot_offset = 0.0f;
    //int rect_count = 0, last_row_start = 0; /* For UPC/EAN guard bars */ //废弃
    float yposn;
    int row_heights_si[200] = {0};
    int symbol_height_si;

    SkPictureRecorder picRecoder;
    SkCanvas* canvas = nullptr;
    float humanReadableTextWidth = 0.0f;
    float humanReadableTextHeight = 0.0f;

    /* Sanity check colours */
    error_number = out_check_colour_options(symbol);
    if (error_number != 0) {
        return error_number;
    }

    large_bar_height = out_large_bar_height(symbol, 0 /*si (scale and round)*/, NULL /*row_heights_si*/,NULL /*symbol_height_si*/);
    main_width = symbol->width;

    if (is_extendable(symbol->symbology)) {
        upceanflag = out_process_upcean(symbol, &main_width, &comp_xoffset, addon, &addon_gap);
    }

    hide_text = ((!symbol->show_hrt) || (strlen((const char*)symbol->text) == 0));
    if (!hide_text) {
        SkRect bounds;
        m_Font.measureText(symbol->text, strlen((const char*)symbol->text), SkTextEncoding::kUTF8, &bounds, &m_Paint);
        humanReadableTextWidth = bounds.width();
        humanReadableTextHeight = bounds.height();
    }
    out_set_whitespace_offsets(symbol, hide_text, &xoffset, &yoffset, &roffset, &boffset, 0 /*scaler*/,
        NULL, NULL, NULL, NULL);
#if 0
    /* Note font sizes scaled by 2 so really twice these values */
    if (upceanflag) {
        /* Note BOLD_TEXT ignored for UPCEAN by svg/emf/ps/qzint */
        text_height = symbol->output_options & SMALL_TEXT ? 7 : 10;
        upcae_outside_text_height = symbol->output_options & SMALL_TEXT ? 6 : 7;
        /* Negative to move close to barcode (less digit ascent, then add 0.5X) */
        text_gap = -text_height * digit_ascent_factor + 0.5f;
        /* Guard bar height (none for EAN-2 and EAN-5) */
        guard_descent = upceanflag != 2 && upceanflag != 5 ? symbol->guard_descent : 0.0f;
    } else {
        text_height = symbol->output_options & SMALL_TEXT ? 6 : 7;
        text_gap = text_height * 0.1f;
        guard_descent = 0.0f;
    }

    if (hide_text) {
        textoffset = guard_descent;
    } else {
        if (upceanflag) {
            if (text_height + 0.2f + text_gap > guard_descent) {
                textoffset = text_height + 0.2f + text_gap;
            } else {
                textoffset = guard_descent;
            }
        } else {
            textoffset = text_height * 1.25f + text_gap;
        }
    }
#endif

    if ((symbol->symbology != BARCODE_MAXICODE) && (symbol->output_options & BARCODE_DOTTY_MODE)) {
        if (symbol->dot_size < 1.0f) {
            dot_overspill = 0.0f;
            /* Offset (1 - dot_size) / 2 + dot_radius == (1 - dot_size + dot_size) / 2 == 1 / 2 */
            dot_offset = 0.5f;
        } else { /* Allow for exceeding 1X */
            dot_overspill = symbol->dot_size - 1.0f + 0.1f; /* Fudge for anti-aliasing */
            dot_offset = symbol->dot_size / 2.0f + 0.05f; /* Fudge for anti-aliasing */
        }
    }

    float barWidth = symbol->width + dot_overspill + (xoffset + roffset);
    float barHeight = symbol->height + dot_overspill + (yoffset + boffset);

    /* Plot Maxicode symbols */
    if (symbol->symbology == BARCODE_MAXICODE) {
        float bull_x, bull_y, bull_d_incr, bull_width;
        const float two_div_sqrt3 = 1.1547f; /* 2 / √3 */
        const float sqrt3_div_two = 0.866f; /* √3 / 2 == 1.5 / √3 */

        /* `hex_diameter` is short diameter, X in ISO/IEC 16023:2000 Figure 8 (same as W) */
        const float hex_diameter = 1.0f;
        const float hex_radius = hex_diameter / 2.0f;
        const float hex_ydiameter = two_div_sqrt3 * hex_diameter; /* Long diameter, V in Figure 8 */
        const float hex_yradius = hex_ydiameter / 2.0f;
        const float yposn_offset = sqrt3_div_two * hex_diameter; /* Vertical distance between rows, Y in Figure 8 */

        barWidth = 30 * hex_diameter + (xoffset + roffset);
        /* 32 rows drawn yposn_offset apart + final hexagon */
        barHeight = 32 * yposn_offset + hex_ydiameter + (yoffset + boffset);
        m_fBarcodeWidth = barWidth * m_XDimensions;
        m_fBarcodeHeight = barHeight * m_XDimensions;
        canvas = picRecoder.beginRecording(m_fBarcodeWidth, m_fBarcodeHeight);

        /* Bullseye (ISO/IEC 16023:2000 4.2.1.1 and 4.11.4) */
        bull_x = 14.5f * hex_diameter + xoffset; /* 14W right from leftmost centre = 14.5X */
        bull_y = barHeight / 2.0f; /* 16Y above bottom-most centre = halfway */
        /* Total finder diameter is 9X, so diametric increment for 5 diameters d2 to d6 is (9X - d1) / 5 */
        bull_d_incr = (hex_diameter * 9 - hex_ydiameter) / 5.0f;
        bull_width = bull_d_incr / 2.0f;

        SkPaint p = m_Paint;
        p.setStyle(SkPaint::Style::kStroke_Style);
        p.setStrokeWidth(bull_width * m_XDimensions);
        drawCircle(bull_x, bull_y, (hex_ydiameter + bull_d_incr * 5 - bull_width) / 2.0, p, canvas);
        drawCircle(bull_x, bull_y, (hex_ydiameter + bull_d_incr * 3 - bull_width) / 2.0, p, canvas);
        drawCircle(bull_x, bull_y, (hex_ydiameter + bull_d_incr - bull_width) / 2.0, p, canvas);

        /* Hexagons */
        for (r = 0; r < symbol->rows; r++) {
            const int odd_row = r & 1; /* Odd (reduced) row, even (full) row */
            const float hex_yposn = r * yposn_offset + hex_yradius + yoffset;
            const float xposn_offset = (odd_row ? hex_diameter : hex_radius) + xoffset;
            for (i = 0; i < symbol->width - odd_row; i++) {
                if (module_is_set(symbol, r, i)) {
                    const float hex_xposn = i * hex_diameter + xposn_offset;
                    drawPath(hex_xposn, hex_yposn, hex_diameter, m_Paint, canvas);
                }
            }
        }
    /* Dotty mode */
    } else if (symbol->output_options & BARCODE_DOTTY_MODE) {
        m_fBarcodeWidth = barWidth * m_XDimensions;
        m_fBarcodeHeight = barHeight * m_XDimensions;
        canvas = picRecoder.beginRecording(m_fBarcodeWidth, m_fBarcodeHeight);

        for (r = 0; r < symbol->rows; r++) {
            for (i = 0; i < symbol->width; i++) {
                if (module_is_set(symbol, r, i)) {
                    drawCircle(i + dot_offset + xoffset, r + dot_offset + yoffset, symbol->dot_size / 2.0, m_Paint, canvas);
                }
            }
        }
    /* Plot rectangles - most symbols created here */
    } else if (symbol->symbology == BARCODE_ULTRA) {
        m_fBarcodeWidth = barWidth * m_XDimensions;
        m_fBarcodeHeight = barHeight * m_XDimensions;
        canvas = picRecoder.beginRecording(m_fBarcodeWidth, m_fBarcodeHeight);

        yposn = yoffset;
        for (r = 0; r < symbol->rows; r++) {
            const float row_height = symbol->row_height[r];
            for (i = 0; i < symbol->width; i += block_width) {
                const int fill = module_colour_is_set(symbol, r, i);
                for (block_width = 1; (i + block_width < symbol->width)
                                        && module_colour_is_set(symbol, r, i + block_width) == fill; block_width++);
                if (fill) {
                    /* a colour block */
                    int icolour = module_colour_is_set(symbol, r, i);
                    SkPaint p = m_Paint;
                    p.setColor(getColor(icolour));
                    drawRect(i + xoffset, yposn, block_width, row_height, p, canvas);
                }
            }
            yposn += row_height;
        }

    } else if (upceanflag >= 6) { /* UPC-E, EAN-8, UPC-A, EAN-13 */
        m_fBarcodeWidth = barWidth * m_XDimensions;
        m_fBarcodeHeight = barHeight * m_XDimensions;
        if (hide_text) {
            //文本不显示的情况下，条码高度要加上护栏的高度，否则护栏会显示不出来
            m_fBarcodeHeight += m_fGuardPatternExtraHeight;
        } else {
            m_fBarcodeWidth = std::max(m_fBarcodeWidth, humanReadableTextWidth);
            m_fBarcodeHeight = m_fBarcodeHeight + m_iTextMargin + humanReadableTextHeight;
        }
        canvas = picRecoder.beginRecording(m_fBarcodeWidth, m_fBarcodeHeight);

        yposn = yoffset;
        int iGuardBar = 0;
        for (r = 0; r < symbol->rows; r++) {
            const float row_height = symbol->row_height[r] ? symbol->row_height[r] : large_bar_height;
            //last_row_start = rect_count;
            for (i = 0; i < symbol->width; i += block_width) {
                const int fill = module_is_set(symbol, r, i);
                for (block_width = 1; (i + block_width < symbol->width)
                                        && module_is_set(symbol, r, i + block_width) == fill; block_width++);

                if ((r == (symbol->rows - 1)) && (i > main_width) && (addon_latch == 0)) {
                    //addon_text_yposn = yposn + text_height - text_height * digit_ascent_factor;
                    addon_text_yposn = m_iTextHeight + yposn*m_XDimensions + m_iTextMargin;
                    if (addon_text_yposn < 0.0f) {
                        addon_text_yposn = 0.0f;
                    }
                    //addon_row_height = row_height - (addon_text_yposn - yposn) + text_gap;
                    addon_row_height = row_height;
                    if (upceanflag != 12 && upceanflag != 6) { /* UPC-A/E add-ons don't descend */
                        addon_row_height += (m_fGuardPatternExtraHeight / m_XDimensions);
                    }
                    if (addon_row_height < 0.5f) {
                        addon_row_height = 0.5f;
                    }
                    addon_latch = 1;
                }
                if (fill) {
                    float x, y, height, width;
                    /* a bar */
                    if (addon_latch) {
                        x = (i + xoffset) * m_XDimensions;
                        y = addon_text_yposn;
                        width = block_width * m_XDimensions;
                        height = addon_row_height * m_XDimensions - addon_text_yposn + yoffset * m_XDimensions * 2;
                    } else {
                        x = (i + xoffset) * m_XDimensions;
                        y = yposn * m_XDimensions;
                        width = block_width * m_XDimensions;
                        height = row_height * m_XDimensions;
                        if (symbol->rows == (r + 1)) { /* UPC-E, EAN-8, UPC-A, EAN-13 */
                            /* Guard bar extension */
                            if (upceanflag == 6) { /* UPC-E */
                                switch (iGuardBar) {
                                    case 0:
                                    case 1:
                                    case 14:
                                    case 15:
                                    case 16:
                                        height += (m_fGuardPatternExtraHeight + yoffset * m_XDimensions);
                                        break;
                                }
                            } else if (upceanflag == 8) { /* EAN-8 */
                                switch (iGuardBar) {
                                    case 0:
                                    case 1:
                                    case 10:
                                    case 11:
                                    case 20:
                                    case 21:
                                        height += (m_fGuardPatternExtraHeight + yoffset * m_XDimensions);
                                        break;
                                }
                            } else if (upceanflag == 12) { /* UPC-A */
                                switch (iGuardBar) {
                                    case 0:
                                    case 1:
                                    case 2:
                                    case 3:
                                    case 14:
                                    case 15:
                                    case 26:
                                    case 27:
                                    case 28:
                                    case 29:
                                        height += (m_fGuardPatternExtraHeight + yoffset * m_XDimensions);
                                        break;
                                }

                            } else { /* EAN-13 */
                                switch (iGuardBar) {
                                    case 0:
                                    case 1:
                                    case 14:
                                    case 15:
                                    case 28:
                                    case 29:
                                        height += (m_fGuardPatternExtraHeight + yoffset * m_XDimensions);
                                        break;
                                }
                            }
                        }
                        iGuardBar++;
                    }
                    drawRect(x / m_XDimensions, y / m_XDimensions, width / m_XDimensions, height / m_XDimensions, m_Paint, canvas);
                }
            }
            yposn += row_height;
        }

    } else {
        m_fBarcodeWidth = barWidth * m_XDimensions;
        m_fBarcodeHeight = barHeight * m_XDimensions;
        if (!hide_text) {
            m_fBarcodeWidth = std::max(m_fBarcodeWidth, humanReadableTextWidth);
            m_fBarcodeHeight = m_fBarcodeHeight + m_iTextMargin + humanReadableTextHeight;
        }
        canvas = picRecoder.beginRecording(m_fBarcodeWidth, m_fBarcodeHeight);

        yposn = yoffset;
        for (r = 0; r < symbol->rows; r++) {
            float row_height = symbol->row_height[r] ? symbol->row_height[r] : large_bar_height;
            for (i = 0; i < symbol->width; i += block_width) {
                const int fill = module_is_set(symbol, r, i);
                for (block_width = 1; (i + block_width < symbol->width)
                                        && module_is_set(symbol, r, i + block_width) == fill; block_width++);
                if (fill) {
                    /* a bar */
                    drawRect(i + xoffset, yposn, block_width, row_height, m_Paint, canvas);
                }
            }
            yposn += row_height;
        }
    }

#if 0
    if (m_fGuardPatternExtraHeight && upceanflag >= 6) { /* UPC-E, EAN-8, UPC-A, EAN-13 */
        /* Guard bar extension */
        if (upceanflag == 6) { /* UPC-E */
            i = 0;
            for(i = 0; i < m_vecRect.size(); i++) {
                switch (i - last_row_start) {
                    case 0:
                    case 1:
                    case 14:
                    case 15:
                    case 16:
                        m_vecRect[i].height += (m_fGuardPatternExtraHeight + yoffset*m_XDimensions);
                        break;
                }
            }
        } else if (upceanflag == 8) { /* EAN-8 */
            i = 0;
            for(i = 0; i < m_vecRect.size(); i++) {
                switch (i - last_row_start) {
                    case 0:
                    case 1:
                    case 10:
                    case 11:
                    case 20:
                    case 21:
                        m_vecRect[i].height += (m_fGuardPatternExtraHeight + yoffset*m_XDimensions);
                        break;
                }
            }
        } else if (upceanflag == 12) { /* UPC-A */
            i = 0;
            for(i = 0; i < m_vecRect.size(); i++) {
                switch (i - last_row_start) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 14:
                    case 15:
                    case 26:
                    case 27:
                    case 28:
                    case 29:
                        m_vecRect[i].height += (m_fGuardPatternExtraHeight + yoffset*m_XDimensions);
                        break;
                }
            }
        } else { /* EAN-13 */
            i = 0;
            for(i = 0; i < m_vecRect.size(); i++) {
                switch (i - last_row_start) {
                    case 0:
                    case 1:
                    case 14:
                    case 15:
                    case 28:
                    case 29:
                        m_vecRect[i].height += (m_fGuardPatternExtraHeight + yoffset*m_XDimensions);
                        break;
                }
            }
        }
    }
#endif
    /* Add the text */
    if (!hide_text) {
        float text_xposn;
        float text_yposn;
        xoffset += comp_xoffset;

        //text_yposn = yoffset + symbol->height + text_height + text_gap; /* Calculated to bottom of text */
        text_yposn = yoffset + symbol->height;

        if (symbol->border_width > 0 && (symbol->output_options & (BARCODE_BOX | BARCODE_BIND))) {
            text_yposn += symbol->border_width;
        }

        if (upceanflag >= 6) { /* UPC-E, EAN-8, UPC-A, EAN-13 */
            float textwidth;
            out_upcean_split_text(upceanflag, symbol->text, textpart1, textpart2, textpart3, textpart4);
            if (upceanflag == 6) { /* UPC-E */
                text_xposn = -5.0f + xoffset;
                textwidth = 6.2f;
                drawString((const char*)textpart1, text_xposn, text_yposn,textwidth, 0.7, m_Font, m_Paint, canvas);
                text_xposn = 24.0f + xoffset;
                textwidth = 6.0f * 8.5f;
                drawString((const char*)textpart2, text_xposn, text_yposn,textwidth, 1, m_Font, m_Paint, canvas);
                text_xposn = 51.0f + 3.0f + xoffset;
                textwidth = 6.2f;
                drawString((const char*)textpart3, text_xposn, text_yposn,textwidth, 0.7, m_Font, m_Paint, canvas);
                switch (strlen((const char*)addon)) {
                    case 2:
                        text_xposn = 61.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        drawString((const char*)addon, text_xposn, yoffset, textwidth, 1, m_Font, m_Paint, canvas);
                        break;
                    case 5:
                        text_xposn = 75.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        drawString((const char*)addon, text_xposn, yoffset, textwidth, 1, m_Font, m_Paint, canvas);
                        break;
                }

            } else if (upceanflag == 8) { /* EAN-8 */
                text_xposn = 17.0f + xoffset;
                textwidth = 4.0f * 8.5f;
                drawString((const char*)textpart1, text_xposn, text_yposn, textwidth, 1, m_Font, m_Paint, canvas);
                text_xposn = 50.0f + xoffset;
                drawString((const char*)textpart2, text_xposn, text_yposn, textwidth, 1, m_Font, m_Paint, canvas);
                switch (strlen((const char*)addon)) {
                    case 2:
                        text_xposn = 77.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        drawString((const char*)addon, text_xposn, yoffset, textwidth, 1, m_Font, m_Paint, canvas);
                        break;
                    case 5:
                        text_xposn = 91.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        drawString((const char*)addon, text_xposn, yoffset, textwidth, 1, m_Font, m_Paint, canvas);
                        break;
                }

            } else if (upceanflag == 12) { /* UPC-A */
                text_xposn = -5.0f + xoffset;
                textwidth = 6.2f;
                drawString((const char*)textpart1, text_xposn, text_yposn,textwidth, 0.7, m_Font, m_Paint, canvas);
                text_xposn = 27.0f + xoffset;
                textwidth = 5.0f * 8.5f;
                drawString((const char*)textpart2, text_xposn, text_yposn,textwidth, 1, m_Font, m_Paint, canvas);
                text_xposn = 67.0f + xoffset;
                drawString((const char*)textpart3, text_xposn, text_yposn,textwidth, 1, m_Font, m_Paint, canvas);
                text_xposn = 95.0f + 5.0f + xoffset;
                textwidth = 6.2f;
                drawString((const char*)textpart4, text_xposn, text_yposn,textwidth, 0.7, m_Font, m_Paint, canvas);
                switch (strlen((const char*)addon)) {
                    case 2:
                        text_xposn = 105.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;

                        drawString((const char*)addon, text_xposn, yoffset, textwidth, 1, m_Font, m_Paint, canvas);
                        break;
                    case 5:
                        text_xposn = 119.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        drawString((const char*)addon, text_xposn, yoffset, textwidth, 1, m_Font, m_Paint, canvas);
                        break;
                }

            } else { /* EAN-13 */
                text_xposn = -5.0f + xoffset;
                textwidth = 8.5f;
                drawString((const char*)textpart1, text_xposn, text_yposn, textwidth, 1, m_Font, m_Paint, canvas);
                text_xposn = 24.0f + xoffset;
                textwidth = 6.0f * 8.5f;
                drawString((const char*)textpart2, text_xposn, text_yposn, textwidth, 1, m_Font, m_Paint, canvas);
                text_xposn = 71.0f + xoffset;
                drawString((const char*)textpart3, text_xposn, text_yposn, textwidth, 1, m_Font, m_Paint, canvas);
                switch (strlen((const char*)addon)) {
                    case 2:
                        text_xposn = 105.0f + xoffset + addon_gap;
                        textwidth = 2.0f * 8.5f;
                        drawString((const char*)addon, text_xposn, yoffset, textwidth, 1, m_Font, m_Paint, canvas);
                        break;
                    case 5:
                        text_xposn = 119.0f + xoffset + addon_gap;
                        textwidth = 5.0f * 8.5f;
                        drawString((const char*)addon, text_xposn, yoffset, textwidth, 1, m_Font, m_Paint, canvas);
                        break;
                }
            }
        } else {
            /* Put normal human readable text at the bottom (and centered) */
            /* calculate start xoffset to center text */
            text_xposn = main_width / 2.0f + xoffset;
            drawString((const char*)symbol->text, text_xposn, text_yposn, symbol->width, 1, m_Font, m_Paint, canvas);
        }

        xoffset -= comp_xoffset; /* Restore xoffset */
    }

    /* Separator binding for stacked barcodes */
    if ((symbol->output_options & BARCODE_BIND) && (symbol->rows > 1) && is_stackable(symbol->symbology)) {
        float sep_xoffset = xoffset;
        float sep_width = symbol->width;
        float sep_height = 1.0f, sep_yoffset, sep_half_height;
        if (symbol->option_3 > 0 && symbol->option_3 <= 4) {
            sep_height = symbol->option_3;
        }
        sep_half_height = sep_height / 2.0f;
        sep_yoffset = yoffset - sep_half_height;
        if (is_codablockf) {
            /* Avoid 11-module start and 13-module stop chars */
            sep_xoffset += 11;
            sep_width -= 11 + 13;
        }

        for (r = 1; r < symbol->rows; r++) {
            const float row_height = symbol->row_height[r - 1] ? symbol->row_height[r - 1] : large_bar_height;
            drawRect(sep_xoffset, (r * row_height) + sep_yoffset,sep_width, sep_height, m_Paint, canvas);
        }
    }

    /* Bind/box */
    if (symbol->border_width > 0 && (symbol->output_options & (BARCODE_BOX | BARCODE_BIND))) {
        const int horz_outside = is_fixed_ratio(symbol->symbology);
        float ybind_top = yoffset - symbol->border_width;
        /* Following equivalent to yoffset + symbol->height + dot_overspill except for BARCODE_MAXICODE */
        float ybind_bot = barHeight - boffset;
        if (horz_outside) {
            ybind_top = 0;
            ybind_bot = barHeight - symbol->border_width;
        }

        /* Top */
        float x, y, height, width;
        x = 0.0f;
        y = ybind_top;
        width = barWidth;
        height = symbol->border_width;
        if (!(symbol->output_options & BARCODE_BOX) && is_codablockf) {
            /* CodaBlockF bind - does not extend over horizontal whitespace */
            x = xoffset;
            width -= xoffset + roffset;
        }
        drawRect(x, y, width, height, m_Paint, canvas);

        /* Bottom */
        x = 0.0f;
        y = ybind_bot;
        width = barWidth;
        height = symbol->border_width;
        if (!(symbol->output_options & BARCODE_BOX) && is_codablockf) {
            /* CodaBlockF bind - does not extend over horizontal whitespace */
            x = xoffset;
            width -= xoffset + roffset;
        }
        drawRect(x, y, width, height, m_Paint, canvas);

        if (symbol->output_options & BARCODE_BOX) {
            const float xbox_right = barWidth - symbol->border_width;
            float box_top = yoffset;
            /* Following equivalent to symbol->height except for BARCODE_MAXICODE */
            float box_height = barHeight - dot_overspill - yoffset - boffset;
            if (horz_outside) {
                box_top = symbol->border_width;
                box_height = barHeight - symbol->border_width * 2;
            }
            /* Left */
            drawRect(0.0f, box_top, symbol->border_width, box_height, m_Paint, canvas);
            /* Right */
            drawRect(xbox_right, box_top, symbol->border_width, box_height, m_Paint, canvas);
        }
    }

    m_barcode_dl_paint.setColor(flutter::DlColor::kRed());
    m_barcode_dl_paint.setDrawStyle(flutter::DlDrawStyle::kStroke);
    if(m_display_list_builder){
      m_display_list_builder->DrawRect(SkRect::MakeLTRB(0, 0, m_fBarcodeWidth, m_fBarcodeHeight), m_barcode_dl_paint);
    }
    //vector_reduce_rectangles(symbol);
    //vector_scale(symbol);
    m_BarcodePicture = picRecoder.finishRecordingAsPicture();

    return error_number;
}

void SkiaBarcode::drawCircle(float x, float y, float radius, const SkPaint& paint, SkCanvas* canvas) {
    if(m_display_list_builder){
      m_display_list_builder->DrawCircle(SkPoint::Make(flutter::SafeNarrow(x), flutter::SafeNarrow(y)),
                                       flutter::SafeNarrow(radius), m_barcode_dl_paint);
      return;
    }
    canvas->drawCircle(x * m_XDimensions, y * m_XDimensions, radius * m_XDimensions, paint);
}

void SkiaBarcode::drawPath(float x, float y, float diameter, const SkPaint& paint, SkCanvas* canvas) {
    float previous_diameter = diameter * m_XDimensions;
    float radius = 0.5 * previous_diameter;
    float half_radius = 0.25 * previous_diameter;
    float half_sqrt3_radius = 0.43301270189221932338 * previous_diameter;
    x *= m_XDimensions;
    y *= m_XDimensions;

    SkPath pt;
    pt.moveTo(x, y + radius);
    pt.lineTo(x + half_sqrt3_radius, y + half_radius);
    pt.lineTo(x + half_sqrt3_radius, y - half_radius);
    pt.lineTo(x, y - radius);
    pt.lineTo(x - half_sqrt3_radius, y - half_radius);
    pt.lineTo(x - half_sqrt3_radius, y + half_radius);
    pt.lineTo(x, y + radius);

    if(m_display_list_builder){
      m_display_list_builder->DrawPath(pt, m_barcode_dl_paint);
      return;
    }

    canvas->drawPath(pt, paint);
}

void SkiaBarcode::drawRect(float x, float y, float width, float height, const SkPaint& paint, SkCanvas* canvas) {
    float left = x * m_XDimensions;
    float top = y * m_XDimensions;
    float right = left + width * m_XDimensions;
    float bottom = top + height * m_XDimensions;

    if(m_display_list_builder){
      m_display_list_builder->DrawRect(SkRect::MakeLTRB(left, top, right, bottom), m_barcode_dl_paint);
      return;
    }

    canvas->drawRect(SkRect::MakeLTRB(left, top, right, bottom), paint);
}

std::unique_ptr<txt::Paragraph> createParagraph(const char* text, const txt::TextStyle& text_style){
  txt::ParagraphStyle paragraphStyle;
  paragraphStyle.font_size = text_style.font_size;
  if(!text_style.font_families.empty()){
    std::string first_family = text_style.font_families.front();
    paragraphStyle.font_family = first_family;
  }
  flutter::FontCollection& font_collection = flutter::UIDartState::Current()
                                        ->platform_configuration()
                                        ->client()
                                        ->GetFontCollection();
  auto impeller_enabled = flutter::UIDartState::Current()->IsImpellerEnabled();
  std::unique_ptr<txt::ParagraphBuilder> paragraph_builder = txt::ParagraphBuilder::CreateSkiaBuilder(
      paragraphStyle, font_collection.GetFontCollection(), impeller_enabled);

  paragraph_builder->PushStyle(text_style);

  // Convert the user input data from UTF-8 to char16_t (UTF-16)
  UErrorCode status = U_ZERO_ERROR;
  int32_t inputLen = 0;

  u_strFromUTF8(nullptr, 0, &inputLen, text, strlen(text), &status);
  if (status != U_BUFFER_OVERFLOW_ERROR) {
    return nullptr;
  }
  status = U_ZERO_ERROR;
  char16_t* fInput = static_cast<char16_t *>(uprv_malloc((inputLen+1) * sizeof(char16_t)));
  if (fInput == nullptr) {
    status = U_MEMORY_ALLOCATION_ERROR;
    return nullptr;
  }
  u_strFromUTF8(fInput, inputLen+1, nullptr, text, strlen(text), &status);
  std::u16string utf16Str = std::u16string(fInput, inputLen);
  paragraph_builder->AddText(utf16Str);
  std::unique_ptr<txt::Paragraph> txt_paragraph = paragraph_builder->Build();
  uprv_free(fInput);
  return txt_paragraph;
}

void SkiaBarcode::drawString(const char* text, float x, float y, float width, float scale, SkFont font, const SkPaint& paint, SkCanvas* canvas) {
    x *= m_XDimensions;
    y = y * m_XDimensions + m_iTextMargin;
    width *= m_XDimensions;

//    SkRect bounds;
//    font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds, &paint);
//    font.setSize(font.getSize() * scale);
//    x -= (bounds.width() / 2.0);//将文本居中
//    if (x < 0) {
//        x = 0;
//    }
//    y += bounds.height();       //根据文本实际高度，获取文本底部的值

    if(m_display_list_builder){

      std::unique_ptr<txt::Paragraph> paragraph = createParagraph(text,m_text_style);
      if(paragraph == nullptr){
        return;
      }
//      FML_LOG(ERROR) << "drawString: ";
//      FML_LOG(ERROR) << "m_fBarcodeWidth: " << m_fBarcodeWidth;
      paragraph->Layout(m_fBarcodeWidth);
//      FML_LOG(ERROR) << "Layout: ";
      skia::textlayout::LineMetrics line;
      const bool found = paragraph->GetLineMetricsAt(0, &line);
      if (!found) {
        return;
      }
      x -= ((line.fTextRight -line.fTextLeft) / 2.0);//将文本居中
      if (x < 0) {
        x = 0;
      }
      double baseline = line.fBaseline;
      double textTop = line.fTextTop;
      y += -(baseline + textTop);
//      y += (line.fTextBottom -line.fTextTop);       //根据文本实际高度，获取文本底部的值
      paragraph->Paint(m_display_list_builder,x,y);
//      FML_LOG(ERROR) << "Paint: ";
      return;
    }

    canvas->drawString(text, x, y, font, paint);
}



void SkiaBarcode::setCanvas(flutter::DisplayListBuilder* displayListBuilder){
  m_display_list_builder = displayListBuilder;
}

void SkiaBarcode::setBarcodeDlPaint(const flutter::DlPaint& paint){
  m_barcode_dl_paint = paint;
}

void SkiaBarcode::setTextStyle(const txt::TextStyle& textStyle){
  m_text_style = textStyle;
}

