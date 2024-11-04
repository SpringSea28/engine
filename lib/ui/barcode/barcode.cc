#include "barcode.h"
#include "SkPicture.h"
#include "barcode_style.h"
#include "flutter/display_list/dl_op_flags.h"
#include "flutter/fml/logging.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

#include "flutter/lib/ui/skiabarcode/Codabar.h"
#include "flutter/lib/ui/skiabarcode/Code25.h"
#include "flutter/lib/ui/skiabarcode/Code39.h"
#include "flutter/lib/ui/skiabarcode/Code93.h"
#include "flutter/lib/ui/skiabarcode/Code128.h"
#include "flutter/lib/ui/skiabarcode/DataMatrix.h"
#include "flutter/lib/ui/skiabarcode/Ean.h"
#include "flutter/lib/ui/skiabarcode/GridMatrix.h"
#include "flutter/lib/ui/skiabarcode/MaxiCode.h"
#include "flutter/lib/ui/skiabarcode/Pdf417.h"
#include "flutter/lib/ui/skiabarcode/QRCode.h"
#include "flutter/lib/ui/skiabarcode/UPC.h"
#include "flutter/lib/ui/skiabarcode/ITF14.h"
#include "flutter/lib/ui/skiabarcode/HanXin.h"

#include <codecvt>
#include <string>

namespace sojet {
namespace barcode {

namespace {

// BarcodeStyle
const int kTSQuietZoneWidth = 1;
const int kTSQuietZoneHeight = 2;
const int kTSHumanReadableLocation = 3;
const int kTSTextMargin = 4;
const int kTSBearerBarType = 5;
const int kTSThickness = 6;
const int kTSXDimension = 7;
const int kTSBarHeight = 8;
const int kTSBarcodeBackgroundIndex = 9;
const int kTSBarcodeForegroundIndex = 10;

const int kTSQuietZoneWidthMask = 1 << kTSQuietZoneWidth;
const int kTSQuietZoneHeightMask = 1 << kTSQuietZoneHeight;
const int kTSHumanReadableLocationMask = 1 << kTSHumanReadableLocation;
const int kTSTextMarginMask = 1 << kTSTextMargin;
const int kTSBearerBarTypeMask = 1 << kTSBearerBarType;
const int kTSThicknessMask = 1 << kTSThickness;
const int kTSXDimensionMask = 1 << kTSXDimension;
const int kTSBarHeightMask = 1 << kTSBarHeight;
const int kTSBarcodeBackgroundIndexMask = 1 << kTSBarcodeBackgroundIndex;
const int kTSBarcodeForegroundIndexMask = 1 << kTSBarcodeForegroundIndex;

// TextStyle

const int kTSLeadingDistributionIndex = 0;
const int kTSColorIndex = 1;
const int kTSTextDecorationIndex = 2;
const int kTSTextDecorationColorIndex = 3;
const int kTSTextDecorationStyleIndex = 4;
const int kTSFontWeightIndex = 5;
const int kTSFontStyleIndex = 6;
const int kTSTextBaselineIndex = 7;
const int kTSTextDecorationThicknessIndex = 8;
const int kTSFontFamilyIndex = 9;
const int kTSFontSizeIndex = 10;
const int kTSLetterSpacingIndex = 11;
const int kTSWordSpacingIndex = 12;
const int kTSHeightIndex = 13;
const int kTSLocaleIndex = 14;
const int kTSBackgroundIndex = 15;
const int kTSForegroundIndex = 16;
const int kTSTextShadowsIndex = 17;
const int kTSFontFeaturesIndex = 18;
const int kTSFontVariationsIndex = 19;

const int kTSLeadingDistributionMask = 1 << kTSLeadingDistributionIndex;
const int kTSColorMask = 1 << kTSColorIndex;
const int kTSTextDecorationMask = 1 << kTSTextDecorationIndex;
const int kTSTextDecorationColorMask = 1 << kTSTextDecorationColorIndex;
const int kTSTextDecorationStyleMask = 1 << kTSTextDecorationStyleIndex;
const int kTSTextDecorationThicknessMask = 1 << kTSTextDecorationThicknessIndex;
const int kTSFontWeightMask = 1 << kTSFontWeightIndex;
const int kTSFontStyleMask = 1 << kTSFontStyleIndex;
const int kTSTextBaselineMask = 1 << kTSTextBaselineIndex;
const int kTSFontFamilyMask = 1 << kTSFontFamilyIndex;
const int kTSFontSizeMask = 1 << kTSFontSizeIndex;
const int kTSLetterSpacingMask = 1 << kTSLetterSpacingIndex;
const int kTSWordSpacingMask = 1 << kTSWordSpacingIndex;
const int kTSHeightMask = 1 << kTSHeightIndex;
const int kTSLocaleMask = 1 << kTSLocaleIndex;
const int kTSBackgroundMask = 1 << kTSBackgroundIndex;
const int kTSForegroundMask = 1 << kTSForegroundIndex;
const int kTSTextShadowsMask = 1 << kTSTextShadowsIndex;
const int kTSFontFeaturesMask = 1 << kTSFontFeaturesIndex;
const int kTSFontVariationsMask = 1 << kTSFontVariationsIndex;

// TextShadows decoding

constexpr uint32_t kColorDefault = 0xFF000000;
constexpr uint32_t kBytesPerShadow = 16;
constexpr uint32_t kShadowPropertiesCount = 4;
constexpr uint32_t kColorOffset = 0;
constexpr uint32_t kXOffset = 1;
constexpr uint32_t kYOffset = 2;
constexpr uint32_t kBlurOffset = 3;

// FontFeature decoding
constexpr uint32_t kBytesPerFontFeature = 8;
constexpr uint32_t kFontFeatureTagLength = 4;

// FontVariation decoding
constexpr uint32_t kBytesPerFontVariation = 8;
constexpr uint32_t kFontVariationTagLength = 4;

}  // namespace

IMPLEMENT_WRAPPERTYPEINFO(ui, Barcode);
void Barcode::Create(Dart_Handle wrapper,
                     const tonic::Int32List& encoded,
                     double x_dimension,
                     double bar_height,
                     Dart_Handle background_objects,
                     Dart_Handle background_data,
                     Dart_Handle foreground_objects,
                     Dart_Handle foreground_data,
                     int barcode_type) {
  flutter::UIDartState::ThrowIfUIOperationsProhibited();
  auto res = fml::MakeRefCounted<Barcode>(encoded, x_dimension, bar_height,
                                          background_objects, background_data,
                                          foreground_objects, foreground_data,
                                          barcode_type);
  res->AssociateWithDartWrapper(wrapper);
}

Barcode::Barcode(const tonic::Int32List& encoded,
                 double x_dimension,
                 double bar_height,
                 Dart_Handle background_objects,
                 Dart_Handle background_data,
                 Dart_Handle foreground_objects,
                 Dart_Handle foreground_data,
                 int barcode_type) {
  FML_LOG(ERROR) << "Barcode::Barcode barcode_type: "
                << barcode_type;

  m_barcode_type = barcode_type;
  if(x_dimension != 0){
    bar_height = bar_height/x_dimension;
  }

  auto upc_barcode = static_cast<sojet::barcode::UPC*>(nullptr);
  switch (barcode_type) {
    case BarcodeType::kCodabar:
      m_skia_barcode =
          std::make_unique<sojet::barcode::Codabar>(bar_height, x_dimension);
      break;
    case BarcodeType::kCode25:
      m_skia_barcode =
          std::make_unique<sojet::barcode::Code25>(bar_height, x_dimension);
      break;
    case BarcodeType::kCode39:
      m_skia_barcode =
          std::make_unique<sojet::barcode::Code39>(bar_height, x_dimension);
      break;
    case BarcodeType::kCode93:
      m_skia_barcode =
          std::make_unique<sojet::barcode::Code93>(bar_height, x_dimension);
      break;
    case BarcodeType::kCode128:
      m_skia_barcode =
          std::make_unique<sojet::barcode::Code128>(bar_height, x_dimension);
      break;
    case BarcodeType::kDataMatrix:
      m_skia_barcode =
          std::make_unique<sojet::barcode::DataMatrix>(bar_height, x_dimension);
      break;
    case BarcodeType::kEan:
      m_skia_barcode =
          std::make_unique<sojet::barcode::Ean>(bar_height, x_dimension);
      break;
    case BarcodeType::kGridMatrix:
      m_skia_barcode =
          std::make_unique<sojet::barcode::GridMatrix>(bar_height, x_dimension);
      break;
    case BarcodeType::kMaxiCode:
      m_skia_barcode =
          std::make_unique<sojet::barcode::MaxiCode>(bar_height, x_dimension);
      break;
    case BarcodeType::kPdf417:
      m_skia_barcode =
          std::make_unique<sojet::barcode::Pdf417>(bar_height, x_dimension);
      break;
    case BarcodeType::kQRCode:
      m_skia_barcode =
          std::make_unique<sojet::barcode::QRCode>(bar_height, x_dimension);
      break;
    case BarcodeType::kUPCA:
      m_skia_barcode =
          std::make_unique<sojet::barcode::UPC>(bar_height, x_dimension);
      upc_barcode = static_cast<sojet::barcode::UPC*>(m_skia_barcode.get());
      upc_barcode->setMode(sojet::barcode::UPC::Mode::UPCA);
      break;
    case BarcodeType::kUPCE:
      m_skia_barcode =
          std::make_unique<sojet::barcode::UPC>(bar_height, x_dimension);
      upc_barcode = static_cast<sojet::barcode::UPC*>(m_skia_barcode.get());
      upc_barcode->setMode(sojet::barcode::UPC::Mode::UPCE);
      break;
    case BarcodeType::kITF14:
      m_skia_barcode =
          std::make_unique<sojet::barcode::ITF14>(bar_height, x_dimension);
      break;
    case BarcodeType::kHanXin:
      m_skia_barcode =
          std::make_unique<sojet::barcode::HanXin>(bar_height, x_dimension);
      break;


    default:
      m_skia_barcode =
          std::make_unique<sojet::barcode::SkiaBarcode>(bar_height, x_dimension);
  }


  setBarcodeStyle(encoded, x_dimension, bar_height, background_objects,
                  background_data, foreground_objects, foreground_data);
}

Barcode::~Barcode() {}

float Barcode::getXDimensions() {
  return m_skia_barcode->getXDimensions();
}

void Barcode::setXDimensions(float xdimensions) {
  m_skia_barcode->setXDimensions(xdimensions);
}

bool Barcode::isGS1NoCheck() {
  return m_skia_barcode->isGS1NoCheck();
}

int Barcode::getDataType() {
  return m_skia_barcode->getDataType();
}

void Barcode::setDataType(int dataType, bool bGS1NoCheck, bool bParseEscapes, bool bGS1Parens) {
//  FML_LOG(ERROR) << "Barcode::setDataType dataType: " << dataType;
//  FML_LOG(ERROR) << "Barcode::setDataType GS1NoCheck: " << bGS1NoCheck;
//  FML_LOG(ERROR) << "Barcode::setDataType escaped: " << bParseEscapes;
//    FML_LOG(ERROR) << "Barcode::setDataType bGS1Parens: " << bGS1Parens;
  m_skia_barcode->setDataType(dataType, bGS1NoCheck, bParseEscapes ,bGS1Parens);
}

float Barcode::getBarHeight() {
  return m_skia_barcode->getBarHeight();
}

void Barcode::setBarHeight(float barHeight) {
  m_skia_barcode->setBarHeight(barHeight);
}

int Barcode::getQuietZoneWidth() {
  return m_skia_barcode->getQuietZoneWidth();
}

void Barcode::setQuietZoneWidth(int iQuietZoneWidth) {
  m_skia_barcode->setQuietZoneWidth(iQuietZoneWidth);
}

int Barcode::getQuietZoneHeight() {
  return m_skia_barcode->getQuietZoneHeight();
}

void Barcode::setQuietZoneHeight(int iQuietZoneHeight) {
  m_skia_barcode->setQuietZoneHeight(iQuietZoneHeight);
}

double Barcode::getWidth() {
  return m_skia_barcode->getWidth();
}

double Barcode::getHeight() {
  return m_skia_barcode->getHeight();
}

int Barcode::getHumanReadableLocation() {
  return m_skia_barcode->getHumanReadableLocation();
}

void Barcode::setHumanReadableLocation(int humanReadableLocation) {
  m_skia_barcode->setHumanReadableLocation(humanReadableLocation);
}

int Barcode::getTextMargin() {
  return m_skia_barcode->getTextMargin();
}

void Barcode::setTextMargin(int margin) {
  m_skia_barcode->setTextMargin(margin);
}

int Barcode::getBearerBarType() {
  return m_skia_barcode->getBearerBarType();
}

void Barcode::setBearerBarType(int type) {
  m_skia_barcode->setBearerBarType(type);
}

int Barcode::getBearerBarThickness() {
  return m_skia_barcode->getBearerBarThickness();
}

void Barcode::setBearerBarThickness(int thickness) {
  m_skia_barcode->setBearerBarThickness(thickness);
}

void Barcode::setTextHeight(int iTextHeight) {
  m_skia_barcode->setTextHeight(iTextHeight);
}

Dart_Handle Barcode::getErrorInfo() {
  const char* text = m_skia_barcode->getErrorInfo();
  if (text == nullptr || strlen(text) == 0) {
    return Dart_Null();
  }
  FML_LOG(ERROR) << "errorInfo: " << text;
  Dart_Handle dartText = tonic::ToDart(text);
  //  delete[] text;
  return dartText;
}

int Barcode::encode(const std::u16string& text, int length) {
  // 使用 std::wstring_convert 进行转换
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
  std::string utf8Text = converter.to_bytes(text);

  // 获取 const char* 指向 utf8Text
  const char* cStyleString = utf8Text.c_str();
//  FML_LOG(ERROR) << "encode: " << cStyleString;
//  FML_LOG(ERROR) << "length: " << length;
  return m_skia_barcode->encode(cStyleString, length);
}

sk_sp<SkPicture> Barcode::getBarcodePicture() {
  return m_skia_barcode->getBarcodePicture();
}

void Barcode::setFont(const SkFont& font) {
  m_skia_barcode->setFont(font);
}

void Barcode::setPaint(const SkPaint& paint) {
  m_skia_barcode->setPaint(paint);
}

void Barcode::setVersion(int version) {
  if(m_barcode_type == BarcodeType::kDataMatrix){
    static_cast<DataMatrix*>(m_skia_barcode.get())->setVersion(version);
  } else if(m_barcode_type == BarcodeType::kGridMatrix){
    static_cast<GridMatrix*>(m_skia_barcode.get())->setVersion(version);
  }else if(m_barcode_type == BarcodeType::kPdf417){
    static_cast<Pdf417*>(m_skia_barcode.get())->setDataColumns(version);
  }else if(m_barcode_type == BarcodeType::kQRCode){
    static_cast<QRCode*>(m_skia_barcode.get())->setVersion(version);
  }else if(m_barcode_type == BarcodeType::kHanXin){
    static_cast<HanXin*>(m_skia_barcode.get())->setVersion(version);
  }
}

void Barcode::setECLevel(int level) {
  if(m_barcode_type == BarcodeType::kGridMatrix){
    static_cast<GridMatrix*>(m_skia_barcode.get())->setECLevel(level);
  }else if(m_barcode_type == BarcodeType::kPdf417){
    static_cast<Pdf417*>(m_skia_barcode.get())->setECLevel(level);
  }else if(m_barcode_type == BarcodeType::kQRCode){
    static_cast<QRCode*>(m_skia_barcode.get())->setECLevel(level);
  }else if(m_barcode_type == BarcodeType::kHanXin){
    static_cast<HanXin*>(m_skia_barcode.get())->setECLevel(level);
  }
}

void Barcode::setGSSeparator(bool gsSep) {
  if(m_barcode_type == BarcodeType::kDataMatrix){
    static_cast<DataMatrix*>(m_skia_barcode.get())->setGSSeparator(gsSep);
  }
}

void Barcode::setCode25Mode(int mode) {
  if(m_barcode_type == BarcodeType::kCode25){
    static_cast<Code25*>(m_skia_barcode.get())->setMode(mode);
  }
}

void Barcode::setCode39CheckDigit(bool en) {
  if(m_barcode_type == BarcodeType::kCode39){
    static_cast<Code39*>(m_skia_barcode.get())->setCheckDigit(en);
  }
}

void Barcode::setCanvas(flutter::Canvas* canvas, double x, double y) {
  if (!canvas) {
    // disposed.
    return;
  }

  flutter::DisplayListBuilder* builder = canvas->builder();
  if (builder) {
//    FML_LOG(ERROR) << "Canvas builder: ";
    m_skia_barcode->setCanvas(builder);
  }
}


void decodeTextShadows(
    Dart_Handle shadows_data,
    std::vector<txt::TextShadow>& decoded_shadows) {  // NOLINT
  decoded_shadows.clear();

  tonic::DartByteData byte_data(shadows_data);
  FML_CHECK(byte_data.length_in_bytes() % kBytesPerShadow == 0);

  const uint32_t* uint_data = static_cast<const uint32_t*>(byte_data.data());
  const float* float_data = static_cast<const float*>(byte_data.data());

  size_t shadow_count = byte_data.length_in_bytes() / kBytesPerShadow;
  size_t shadow_count_offset = 0;
  for (size_t shadow_index = 0; shadow_index < shadow_count; ++shadow_index) {
    shadow_count_offset = shadow_index * kShadowPropertiesCount;
    SkColor color =
        uint_data[shadow_count_offset + kColorOffset] ^ kColorDefault;
    decoded_shadows.emplace_back(
        color,
        SkPoint::Make(float_data[shadow_count_offset + kXOffset],
                      float_data[shadow_count_offset + kYOffset]),
        float_data[shadow_count_offset + kBlurOffset]);
  }
}

void decodeFontFeatures(Dart_Handle font_features_data,
                        txt::FontFeatures& font_features) {  // NOLINT
  tonic::DartByteData byte_data(font_features_data);
  FML_CHECK(byte_data.length_in_bytes() % kBytesPerFontFeature == 0);

  size_t feature_count = byte_data.length_in_bytes() / kBytesPerFontFeature;
  for (size_t feature_index = 0; feature_index < feature_count;
       ++feature_index) {
    size_t feature_offset = feature_index * kBytesPerFontFeature;
    const char* feature_bytes =
        static_cast<const char*>(byte_data.data()) + feature_offset;
    std::string tag(feature_bytes, kFontFeatureTagLength);
    int32_t value = *(reinterpret_cast<const int32_t*>(feature_bytes +
                                                       kFontFeatureTagLength));
    font_features.SetFeature(tag, value);
  }
}

void decodeFontVariations(Dart_Handle font_variations_data,
                          txt::FontVariations& font_variations) {  // NOLINT
  tonic::DartByteData byte_data(font_variations_data);
  FML_CHECK(byte_data.length_in_bytes() % kBytesPerFontVariation == 0);

  size_t variation_count = byte_data.length_in_bytes() / kBytesPerFontVariation;
  for (size_t variation_index = 0; variation_index < variation_count;
       ++variation_index) {
    size_t variation_offset = variation_index * kBytesPerFontVariation;
    const char* variation_bytes =
        static_cast<const char*>(byte_data.data()) + variation_offset;
    std::string tag(variation_bytes, kFontVariationTagLength);
    float value = *(reinterpret_cast<const float*>(variation_bytes +
                                                   kFontVariationTagLength));
    font_variations.SetAxisValue(tag, value);
  }
}

void Barcode::setTextStyle(const tonic::Int32List& encoded,
                           const std::vector<std::string>& fontFamilies,
                           double fontSize,
                           double letterSpacing,
                           double wordSpacing,
                           double height,
                           double decorationThickness,
                           const std::string& locale,
                           Dart_Handle background_objects,
                           Dart_Handle background_data,
                           Dart_Handle foreground_objects,
                           Dart_Handle foreground_data,
                           Dart_Handle shadows_data,
                           Dart_Handle font_features_data,
                           Dart_Handle font_variations_data) {
  FML_DCHECK(encoded.num_elements() == 9);

  int32_t mask = encoded[0];

  // Set to use the properties of the previous style if the property is not
  // explicitly given.
  txt::TextStyle style = txt::TextStyle();

  style.half_leading = mask & kTSLeadingDistributionMask;
  // Only change the style property from the previous value if a new explicitly
  // set value is available
  if (mask & kTSColorMask) {
    style.color = encoded[kTSColorIndex];
  }

  if (mask & kTSTextDecorationMask) {
    style.decoration =
        static_cast<txt::TextDecoration>(encoded[kTSTextDecorationIndex]);
  }

  if (mask & kTSTextDecorationColorMask) {
    style.decoration_color = encoded[kTSTextDecorationColorIndex];
  }

  if (mask & kTSTextDecorationStyleMask) {
    style.decoration_style = static_cast<txt::TextDecorationStyle>(
        encoded[kTSTextDecorationStyleIndex]);
  }

  if (mask & kTSTextDecorationThicknessMask) {
    style.decoration_thickness_multiplier = decorationThickness;
  }

  if (mask & kTSTextBaselineMask) {
    // TODO(abarth): Implement TextBaseline. The CSS version of this
    // property wasn't wired up either.
  }

  if (mask & (kTSFontWeightMask | kTSFontStyleMask | kTSFontSizeMask |
              kTSLetterSpacingMask | kTSWordSpacingMask)) {
    if (mask & kTSFontWeightMask) {
      style.font_weight =
          static_cast<txt::FontWeight>(encoded[kTSFontWeightIndex]);
    }

    if (mask & kTSFontStyleMask) {
      style.font_style =
          static_cast<txt::FontStyle>(encoded[kTSFontStyleIndex]);
    }

    if (mask & kTSFontSizeMask) {
      style.font_size = fontSize;
    }

    if (mask & kTSLetterSpacingMask) {
      style.letter_spacing = letterSpacing;
    }

    if (mask & kTSWordSpacingMask) {
      style.word_spacing = wordSpacing;
    }
  }

  if (mask & kTSHeightMask) {
    style.height = height;
    style.has_height_override = true;
  }

  if (mask & kTSLocaleMask) {
    style.locale = locale;
  }

  if (mask & kTSBackgroundMask) {
    flutter::Paint background(background_objects, background_data);
    if (background.isNotNull()) {
      flutter::DlPaint dl_paint;
      background.toDlPaint(dl_paint);
      style.background = dl_paint;
    }
  }

  if (mask & kTSForegroundMask) {
    flutter::Paint foreground(foreground_objects, foreground_data);
    if (foreground.isNotNull()) {
      flutter::DlPaint dl_paint;
      foreground.toDlPaint(dl_paint);
      style.foreground = dl_paint;
    }
  }

  if (mask & kTSTextShadowsMask) {
    decodeTextShadows(shadows_data, style.text_shadows);
  }

  if (mask & kTSFontFamilyMask) {
    // The child style's font families override the parent's font families.
    // If the child's fonts are not available, then the font collection will
    // use the system fallback fonts (not the parent's fonts).
    style.font_families = fontFamilies;
  }

  if (mask & kTSFontFeaturesMask) {
    decodeFontFeatures(font_features_data, style.font_features);
  }

  if (mask & kTSFontVariationsMask) {
    decodeFontVariations(font_variations_data, style.font_variations);
  }

//  FML_LOG(ERROR) << "Barcode::setTextStyle color: " << style.color;
//  FML_LOG(ERROR) << "Barcode::setTextStyle fontSize: " << style.font_size;

  this->m_text_style = style;
  m_skia_barcode->setTextStyle(m_text_style);
}

void Barcode::setBarcodeStyle(const tonic::Int32List& encoded,
                              double x_dimension,
                              double bar_height,
                              Dart_Handle background_objects,
                              Dart_Handle background_data,
                              Dart_Handle foreground_objects,
                              Dart_Handle foreground_data) {
  int32_t mask = encoded[0];

  sojet::barcode::BarcodeStyle style = sojet::barcode::BarcodeStyle();

  // Only change the style property from the previous value if a new explicitly
  // set value is available
  if (mask & kTSQuietZoneWidthMask) {
    style.quiet_zone_width = encoded[kTSQuietZoneWidth];
  }

  if (mask & kTSQuietZoneHeightMask) {
    style.quiet_zone_height = encoded[kTSQuietZoneHeight];
  }

  if (mask & kTSHumanReadableLocationMask) {
    style.human_readable_location = encoded[kTSHumanReadableLocation];
  }

  if (mask & kTSTextMarginMask) {
    style.text_margin = encoded[kTSTextMargin];
  }

  if (mask & kTSBearerBarTypeMask) {
    style.bearer_bar_type =
        static_cast<sojet::barcode::BearerBarType>(encoded[kTSBearerBarType]);
  }

  if (mask & kTSThicknessMask) {
    style.thickness = encoded[kTSThickness];
  }

  if (mask & kTSXDimensionMask) {
    style.x_dimension = x_dimension;
  }

  if (mask & kTSBarHeightMask) {
    style.bar_height = bar_height;
  }

  if (mask & kTSBarcodeBackgroundIndexMask) {
    flutter::Paint background(background_objects, background_data);
    if (background.isNotNull()) {
      flutter::DlPaint dl_paint;
      background.toDlPaint(dl_paint);
      style.background = dl_paint;
    }
  }

  if (mask & kTSBarcodeForegroundIndexMask) {
    flutter::Paint foreground(foreground_objects, foreground_data);
    if (foreground.isNotNull()) {
      flutter::DlPaint dl_paint;
      foreground.toDlPaint(dl_paint);
      style.foreground = dl_paint;
    }
  }

//  FML_LOG(ERROR) << "Barcode::setBarcodeStyle bar_height: "
//                 << style.bar_height;
//  FML_LOG(ERROR) << "Barcode::setBarcodeStyle x_dimension: "
//                 << style.x_dimension;
//  FML_LOG(ERROR) << "Barcode::setBarcodeStyle quiet_zone_width: "
//                 << style.quiet_zone_width;
//  FML_LOG(ERROR) << "Barcode::setBarcodeStyle quiet_zone_height: "
//                 << style.quiet_zone_height;
//  FML_LOG(ERROR) << "Barcode::setBarcodeStyle human_readable_location: "
//                 << style.human_readable_location;
//  FML_LOG(ERROR) << "Barcode::setBarcodeStyle text_margin: "
//                 << style.text_margin;
//  FML_LOG(ERROR) << "Barcode::setBarcodeStyle bearer_bar_type: "
//                 << style.bearer_bar_type;
//  FML_LOG(ERROR) << "Barcode::setBarcodeStyle thickness: "
//                 << style.thickness;
  m_barcode_style = style;
  setXDimensions(m_barcode_style.x_dimension);
  setBarHeight(m_barcode_style.bar_height);
  setQuietZoneWidth(m_barcode_style.quiet_zone_width);
  setQuietZoneHeight(m_barcode_style.quiet_zone_height);
  setHumanReadableLocation(m_barcode_style.human_readable_location);
  setTextMargin(m_barcode_style.text_margin);
  setBearerBarType(m_barcode_style.bearer_bar_type);
  setBearerBarThickness(m_barcode_style.thickness);
  m_skia_barcode->setBarcodeDlPaint(m_barcode_style.foreground.value_or(flutter::DlPaint()));
}

void Barcode::dispose() {
  ClearDartWrapper();
}

}  // namespace barcode
}  // namespace sojet
