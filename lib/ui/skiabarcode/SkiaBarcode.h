#ifndef __SKIA_BARCODE_H__
#define __SKIA_BARCODE_H__

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkFont.h"
#include "SkPicture.h"

#include "flutter/display_list/dl_builder.h"
#include "flutter/lib/ui/text/paragraph.h"
#include "flutter/third_party/txt/src/txt/text_style.h"

struct zint_symbol;

namespace sojet {
namespace barcode {

class SkiaBarcode{
public:
    /**
     * The location of a bar code's human-readable text.
     */
    enum HumanReadableLocation {
        /** Do not display the human-readable text. */
        HUMAN_READABLE_NONE,

        /** Display the human-readable text below the bar code. */
        HUMAN_READABLE_BOTTOM,

        /** Display the human-readable text above the bar code. */
        HUMAN_READABLE_TOP
    };

    enum DataType {
        BINARY,
// add by sojet start >>>
//        UNICODE,
// <<< add by sojet end
        GS1,
        HIBC
    };

    /**
    * 构造函数
    *@param barHeight：条码高度,一个xdimension的高度(barHeight/xdimension)
    *@param xdimension：X维缩放比例
    */
    SkiaBarcode(float barHeight, float xdimension);
    virtual ~SkiaBarcode();

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
    * @param bGS1NoCheck： true:不校验GS1数据格式               false:校验GS1数据格式
    * @param bParseEscapes true:解析转义字符                  false:不解析转义字符
    */
    void setDataType(int dataType, bool bGS1NoCheck = false, bool bParseEscapes  = false);

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
    float getWidth();

    /**
    * 获取条码的高度(包括空白区域和边框大小，不包括文本高度和文本与条码的边距)
    * @return  条码的高度
    */
    float getHeight();

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
    const char* getErrorInfo();

    /**
    * 编码条码
    *@param 文本信息
    *@param 文本长度
    *@return    0:成功      其他:失败
    */
    int encode(const char* text, int length);

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


    void setCanvas(flutter::DisplayListBuilder* displayListBuilder);
    void setBarcodeDlPaint(const flutter::DlPaint& paint);
    void setTextStyle(const txt::TextStyle& textStyle);

private:
    int native_encode(const char* text, int length);
    /**
    * 将编码后的条码数据缓存到容器
    *@return 0:成功 1-4:告警 >4:错误
    */
    int bufferVector();
    /**
    * 编码条码,并将编码后的条码数据缓存到容器
    *@param 文本信息
    *@param 文本长度
    *@return 0:成功 1-4:告警 >4:错误
    */
    int encodeAndBufferVector(unsigned char *input, int length);
    int plot_vector();
    void clearBarcode();
    void drawCircle(float x, float y, float radius, const SkPaint& paint, SkCanvas* canvas);
    void drawPath(float x, float y, float diameter, const SkPaint& paint, SkCanvas* canvas);
    void drawRect(float x, float y, float width, float height, const SkPaint& paint, SkCanvas* canvas);
    void drawString(const char* text, float x, float y, float width, float scale, SkFont font, const SkPaint& paint, SkCanvas* canvas);

protected:
    virtual bool gs1Supported();

protected:
    zint_symbol* symbol;                //zint 符号结构体
    float m_fGuardPatternExtraHeight;   //ENA/UPC条码，防护图案额外高度

private:
    float m_fBarcodeWidth;      //条码宽度
    float m_fBarcodeHeight;     //条码高度
    int m_iTextMargin;          //文本外边距
    int m_iTextHeight;          //文本的高度
    float m_XDimensions;        //X-Dimensions 密度

    SkFont m_Font;
    SkPaint m_Paint;
    sk_sp<SkPicture> m_BarcodePicture;

    flutter::DisplayListBuilder* m_display_list_builder;
    flutter::DlPaint m_barcode_dl_paint;
    txt::TextStyle m_text_style;
};

}
}

#endif //__SKIA_BARCODE_H__
