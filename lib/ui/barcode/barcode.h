#ifndef __BARCODE_H__
#define __BARCODE_H__

#include "flutter/lib/ui/skiabarcode/SkiaBarcode.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/painting/paint.h"
#include "flutter/lib/ui/painting/canvas.h"
#include "flutter/lib/ui/barcode/barcode_style.h"
#include "flutter/third_party/txt/src/txt/text_style.h"


namespace sojet {
namespace barcode {

enum BarcodeType { kCodabar, kCode25, kCode39, kCode93, kCode128, kDataMatrix,
                   kEan,kGridMatrix,kMaxiCode,kPdf417,kQRCode,kUPCA,kUPCE,kITF14,kHanXin };

class Barcode : public flutter::RefCountedDartWrappable<Barcode> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(Barcode);

 public:
  static void Create(Dart_Handle wrapper,
                     const tonic::Int32List& encoded,
                     double x_dimension,
                     double bar_height,
                     Dart_Handle background_objects,
                     Dart_Handle background_data,
                     Dart_Handle foreground_objects,
                     Dart_Handle foreground_data,
                     int barcode_type);

  /**
    * 构造函数
    *@param barHeight：条码高度,一个xdimension的高度(barHeight/xdimension)
    *@param xdimension：X维缩放比例
   */
  Barcode(const tonic::Int32List& encoded,
          double x_dimension,
          double bar_height,
          Dart_Handle background_objects,
          Dart_Handle background_data,
          Dart_Handle foreground_objects,
          Dart_Handle foreground_data,
          int barcode_type);
  virtual ~Barcode();

  /**
    * 获取X维度缩放比例
    *@return     缩放比例
   */
  float getXDimensions();

  /**
    * 设置X维度缩放比例
    *@param     X维缩放比例
   */
  void setXDimensions(float xdimensions);

  /**
    * Barcode height in X-dimensions (ignored for fixed-width barcodes)
    * 获取X-dimensions = 1的条码高度
    * @return  条码高度
   */
  float getBarHeight();

  /**
    * Barcode height in X-dimensions (ignored for fixed-width barcodes)
    * 设置X-dimensions = 1的条码高度
    * @param  条码高度
   */
  void setBarHeight(float barHeight);

  /**
    * 获取数据编码类型
    * @return  DATA_MODE(Binary):0  UNICODE_MODE(UTF-8):1  GS1_MODE(GS1):2
   */
  int getDataType();

  /**
    * GS1数据内容校验
    * @return  true:不校验       false:校验
   */
  bool isGS1NoCheck();

  /**
    * 设置数据编码类型(GS1 数据类型还可以设置是否对于数据内容进行校验)
    * @param dataType： DATA_MODE(Binary):0  UNICODE_MODE(UTF-8):1  GS1_MODE(GS1):2
    * @param bGS1NoCheck： true:不校验GS1数据格式 false:校验GS1数据格式
    * @param bParseEscapes true:解析转义字符 false:不解析转义字符
    * @param bGS1Parens  true:Ai标识符（）括号                  false:Ai标识符 [] 括号
   */
  void setDataType(int dataType,
                   bool bGS1NoCheck = false,
                   bool bParseEscapes = false,
                   bool bGS1Parens = false);

  /**
    * 获取条码两边的空白区域宽度
    * @return  空白区域宽度
   */
  int getQuietZoneWidth();

  /**
    * 设置条码两边的空白区域宽度
    * @param  空白区域宽度
   */
  void setQuietZoneWidth(int iQuietZoneWidth);

  /**
    * 获取条码上下的空白区域高度(注意：一维码上下的空白区域在文本外)
    * @return  空白区域高度
   */
  int getQuietZoneHeight();

  /**
    * 设置条码上下的空白区域高度(注意：一维码上下的空白区域在文本外)
    * @param  空白区域高度
   */
  void setQuietZoneHeight(int iQuietZoneHeight);

  /**
    * 获取条码的宽度(包括空白区域和边框大小)
    * @return  条码的宽度
   */
  double getWidth();

  /**
    * 获取条码的高度(包括空白区域和边框大小，不包括文本高度和文本与条码的边距)
    * @return  条码的高度
   */
  double getHeight();

  /**
    * 获取是否显示文本内容
    * @return   0:不显示       1:显示
   */
  int getHumanReadableLocation();

  /**
    * 设置是否显示文本内容
    * @param   0:不显示       1:显示
   */
  void setHumanReadableLocation(int humanReadableLocation);

  /**
    * 获取文本的外边距
    *@return   文本的外边距
   */
  int getTextMargin();

  /**
    * 设置文本的高度
    *@param   文本的高度
   */
  void setTextMargin(int margin);

  /**
    * 获取保护框类型
    *@return   保护框类型
   */
  int getBearerBarType();

  /**
    * 设置保护框类型
    *@param   保护框类型
   */
  void setBearerBarType(int type);

  /**
    * 获取条码边框的厚度
    *@return     条码边框的厚度
   */
  int getBearerBarThickness();

  /**
    * 设置条码边框的厚度
    *@param     条码边框的厚度
   */
  void setBearerBarThickness(int thickness);

  /**
    * 设置文本的高度
    *@param   文本的高度
   */
  void setTextHeight(int iTextHeight);

  /**
    * 获取错误信息
    *@return    错误信息
   */
  Dart_Handle getErrorInfo();

  /**
    * 编码条码
    *@param 文本信息
    *@param 文本长度
    *@return    0:成功      其他:失败
   */
  int encode(const std::u16string& text, int length);

  /**
    * 获取条码图片
    *@return    条码图片
   */
  sk_sp<SkPicture> getBarcodePicture();

  /**
    * 设置字体
    *@param 字体
   */
  void setFont(const SkFont& font);

  /**
    * 设置画笔
    *@param 画笔
   */
  void setPaint(const SkPaint& paint);

  void setVersion(int version);
  void setECLevel(int level);
  void setGSSeparator(bool gsSep);
  void setCode25Mode(int mode);
  void setCode39CheckDigit(bool en);

  void setCanvas(flutter::Canvas* canvas, double x, double y);

  void setTextStyle(const tonic::Int32List& encoded,
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
                    Dart_Handle font_variations_data);



  void dispose();

 private:
  void setBarcodeStyle(const tonic::Int32List& encoded,
                       double x_dimension,
                       double bar_height,
                       Dart_Handle background_objects,
                       Dart_Handle background_data,
                       Dart_Handle foreground_objects,
                       Dart_Handle foreground_data);

  BarcodeStyle m_barcode_style;
  txt::TextStyle m_text_style;
  std::unique_ptr<sojet::barcode::SkiaBarcode> m_skia_barcode;
  int m_barcode_type;
};

} // namespace barcode
} // namespace sojet

#endif //__SKIA_BARCODE_H__
