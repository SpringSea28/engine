// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
part of dart.ui;

enum BearerBarType {
  /* No bearer bar. */
  BEARER_BAR_NONE,

/* Bearer Bar is drawn on the upper side of the bar code. */
  BEARER_BAR_TOP,

/* Bearer Bar is drawn on the bottom side of the bar code. */
  BEARER_BAR_BOTTOM,

/* Bearer Bar is drawn on top and bottom of the bar code. */
  BEARER_BAR_BIND,

/* Bearer Bar is drawn on all sides of the bar code. */
  BEARER_BAR_RECT,
}

Int32List _encodeBarcodeStyle(int? quietZoneWidth,
    int? quietZoneHeight,
    int? humanReadableLocation,
    int? textMargin,
    BearerBarType? bearerBarType,
    int? thickness,
    double? xDimension,
    double? barHeight,
    Paint? background,
    Paint? foreground,) {
  final Int32List result = Int32List(8);
  // The 0th bit of result[0] is reserved for leadingDistribution.

  if (quietZoneWidth != null) {
    result[0] |= 1 << 1;
    result[1] = quietZoneWidth;
  }
  if (quietZoneHeight != null) {
    result[0] |= 1 << 2;
    result[2] = quietZoneHeight;
  }
  if (humanReadableLocation != null) {
    result[0] |= 1 << 3;
    result[3] = humanReadableLocation;
  }
  if (textMargin != null) {
    result[0] |= 1 << 4;
    result[4] = textMargin;
  }
  if (bearerBarType != null) {
    result[0] |= 1 << 5;
    result[5] = bearerBarType.index;
  }
  if (thickness != null) {
    result[0] |= 1 << 6;
    result[6] = thickness;
  }


  if (xDimension != null) {
    result[0] |= 1 << 7;
  }
  if (barHeight != null) {
    result[0] |= 1 << 8;
  }
  if (background != null) {
    result[0] |= 1 << 9;
    // Passed separately to native.
  }
  if (foreground != null) {
    result[0] |= 1 << 10;
    // Passed separately to native.
  }

  return result;
}


class BarcodeStyle {
  BarcodeStyle({
    int? quietZoneWidth,
    int? quietZoneHeight,
    int? humanReadableLocation,
    int? textMargin,
    BearerBarType? bearerBarType,
    int? thickness,
    double? xDimension,
    double? barHeight,
    Paint? background,
    Paint? foreground,
  })
      :
        _encoded = _encodeBarcodeStyle(
          quietZoneWidth,
          quietZoneHeight,
          humanReadableLocation,
          textMargin,
          bearerBarType,
          thickness,
          xDimension,
          barHeight,
          background,
          foreground,
        ),
        _xDimension = xDimension,
        _barHeight = barHeight,
        _background = background,
        _foreground = foreground;


  final Int32List _encoded;
  final double? _xDimension;
  final double? _barHeight;
  final Paint? _background;
  final Paint? _foreground;

  @override
  bool operator ==(Object other) {
    if (identical(this, other)) {
      return true;
    }
    return other is BarcodeStyle
        && other._xDimension == _xDimension
        && other._barHeight == _barHeight
        && other._background == _background
        && other._foreground == _foreground
        && _listEquals<int>(other._encoded, _encoded);
  }

  @override
  int get hashCode {
    return Object.hash(
      Object.hashAll(_encoded),
      _xDimension,
      _barHeight,
      _background,
      _foreground,
    );
  }

  @override
  String toString() {
    return 'BarcodeStyle('
        'quietZoneWidth: ${ _encoded[0] & 0x00002 == 0x00002 ? _encoded[1] : "unspecified"}, '
        'quietZoneHeight: ${ _encoded[0] & 0x00004 == 0x00004 ? _encoded[2] : "unspecified"}, '
        'humanReadableLocation: ${ _encoded[0] & 0x00008 == 0x00008 ? _encoded[3] : "unspecified"}, '
        'textMargin: ${ _encoded[0] & 0x00010 == 0x00010 ? _encoded[4] : "unspecified"}, '
        'bearerBarType: ${ _encoded[0] & 0x00020 == 0x00020 ? BearerBarType.values[_encoded[5]] : "unspecified"}, '
        'thickness: ${ _encoded[0] & 0x00040 == 0x00040 ? _encoded[6] : "unspecified"}, '
        'xDimension: ${ _encoded[0] & 0x00080 == 0x00080 ? _xDimension : "unspecified"}, '
        'barHeight: ${ _encoded[0] & 0x00100 == 0x00100 ? _barHeight : "unspecified"}, '
        'background: ${ _encoded[0] & 0x08000 == 0x08000 ? _background : "unspecified"}, '
        'foreground: ${ _encoded[0] & 0x10000 == 0x10000 ? _foreground : "unspecified"}, '
        ')';
  }
}

// add by sojet start >>>
base class Barcode extends NativeFieldWrapperClass1 {

  Barcode(BarcodeStyle barcodeStyle) {
    final Int32List encoded = barcodeStyle._encoded;
    create(encoded,
      barcodeStyle._xDimension ?? 0,
      barcodeStyle._barHeight ?? 0,
      barcodeStyle._background?._objects,
      barcodeStyle._background?._data,
      barcodeStyle._foreground?._objects,
      barcodeStyle._foreground?._data,);
  }


  @Native<Void Function(Handle, Handle, Double, Double,
      Handle, Handle, Handle, Handle)>(
      symbol: 'sojet::barcode::Barcode::Create')
  external void create(Int32List encoded, double xDimension, double barHeight,
      List<Object?>? backgroundObjects,
      ByteData? backgroundData,
      List<Object?>? foregroundObjects,
      ByteData? foregroundData,);

  @Native<Int32 Function(Pointer<Void>, Handle, Int32)>(
      symbol: 'sojet::barcode::Barcode::encode')
  external int encode(String text, int length);

  @Native<Handle Function(Pointer<Void>)>(
      symbol: 'sojet::barcode::Barcode::getErrorInfo')
  external String? getErrorInfo();

  void setCanvas(Canvas canvas, Offset offset) {
    final _NativeCanvas _canvas = canvas as _NativeCanvas;
    _setCanvas(_canvas, offset.dx, offset.dy);
  }

  @Native<Void Function(Pointer<Void>, Pointer<Void>, Double, Double)>(
      symbol: 'sojet::barcode::Barcode::setCanvas')
  external void _setCanvas(_NativeCanvas canvas, double x, double y);

  // void setBarcodePaint(Paint paint) {
  //   _setBarcodePaint(paint._objects, paint._data);
  // }
  //
  // @Native<Void Function(Pointer<Void>, Handle, Handle)>(
  //     symbol: 'sojet::barcode::Barcode::setBarcodePaint')
  // external void _setBarcodePaint(List<Object?>? paintObjects,
  //     ByteData paintData);

  void setTextStyle(TextStyle style) {
    final List<String> fullFontFamilies = <String>[];
    fullFontFamilies.add(style._fontFamily);
    final List<String>? fontFamilyFallback = style._fontFamilyFallback;
    if (fontFamilyFallback != null) {
      fullFontFamilies.addAll(fontFamilyFallback);
    }

    final Int32List encoded = style._encoded;
    final TextLeadingDistribution finalLeadingDistribution = style
        ._leadingDistribution ?? TextLeadingDistribution.proportional;
    // ensure the enum can be represented using 1 bit.
    assert(TextLeadingDistribution.values.length <= 2);

    // Use the leading distribution from the paragraph's style if it's not
    // explicitly set in `style`.
    encoded[0] |= finalLeadingDistribution.index << 0;

    ByteData? encodedFontFeatures;
    final List<FontFeature>? fontFeatures = style._fontFeatures;
    if (fontFeatures != null) {
      encodedFontFeatures =
          ByteData(fontFeatures.length * FontFeature._kEncodedSize);
      int byteOffset = 0;
      for (final FontFeature feature in fontFeatures) {
        feature._encode(ByteData.view(
            encodedFontFeatures.buffer, byteOffset, FontFeature._kEncodedSize));
        byteOffset += FontFeature._kEncodedSize;
      }
    }

    ByteData? encodedFontVariations;
    final List<FontVariation>? fontVariations = style._fontVariations;
    if (fontVariations != null) {
      encodedFontVariations =
          ByteData(fontVariations.length * FontVariation._kEncodedSize);
      int byteOffset = 0;
      for (final FontVariation variation in fontVariations) {
        variation._encode(ByteData.view(
            encodedFontVariations.buffer, byteOffset,
            FontVariation._kEncodedSize));
        byteOffset += FontVariation._kEncodedSize;
      }
    }

    _setTextStyle(
      encoded,
      fullFontFamilies,
      style._fontSize ?? 0,
      style._letterSpacing ?? 0,
      style._wordSpacing ?? 0,
      style._height ?? 0,
      style._decorationThickness ?? 0,
      _encodeLocale(style._locale),
      style._background?._objects,
      style._background?._data,
      style._foreground?._objects,
      style._foreground?._data,
      Shadow._encodeShadows(style._shadows),
      encodedFontFeatures,
      encodedFontVariations,
    );
  }

  @Native<
      Void Function(
          Pointer<Void>,
          Handle,
          Handle,
          Double,
          Double,
          Double,
          Double,
          Double,
          Handle,
          Handle,
          Handle,
          Handle,
          Handle,
          Handle,
          Handle,
          Handle)>(symbol: 'sojet::barcode::Barcode::setTextStyle')
  external void _setTextStyle(Int32List encoded,
      List<Object?> fontFamilies,
      double fontSize,
      double letterSpacing,
      double wordSpacing,
      double height,
      double decorationThickness,
      String locale,
      List<Object?>? backgroundObjects,
      ByteData? backgroundData,
      List<Object?>? foregroundObjects,
      ByteData? foregroundData,
      ByteData shadowsData,
      ByteData? fontFeaturesData,
      ByteData? fontVariationsData,);

  static String _encodeLocale(Locale? locale) => locale?.toString() ?? '';

  //
  // @Native<Void Function(Pointer<Void>, Pointer<Void>)>(symbol: 'Barcode::setBarcodeParagraphBuilder')
  // external void setBarcodeParagraphBuilder(NativeBarcodeParagraphBuilder nativeBarcodeParagraphBuilder);

  @Native<Void Function(Pointer<Void>)>(
      symbol: 'sojet::barcode::Barcode::dispose')
  external void dispose();

  @override
  String toString() => 'Barcode';
}


// base class NativeBarcodeParagraphBuilder extends NativeFieldWrapperClass1 implements ParagraphBuilder {
//   NativeBarcodeParagraphBuilder(ParagraphStyle style)
//       : _defaultLeadingDistribution = style._leadingDistribution {
//     List<String>? strutFontFamilies;
//     final StrutStyle? strutStyle = style._strutStyle;
//     final ByteData? encodedStrutStyle;
//     if (strutStyle != null && strutStyle._enabled) {
//       final String? fontFamily = strutStyle._fontFamily;
//       strutFontFamilies = <String>[
//         if (fontFamily != null) fontFamily,
//         ...?strutStyle._fontFamilyFallback,
//       ];
//
//       assert(TextLeadingDistribution.values.length <= 2);
//       final TextLeadingDistribution leadingDistribution = strutStyle._leadingDistribution
//           ?? style._leadingDistribution;
//       encodedStrutStyle = strutStyle._encoded;
//       int bitmask = encodedStrutStyle.getInt8(0);
//       bitmask |= (leadingDistribution.index) << 3;
//       encodedStrutStyle.setInt8(0, bitmask);
//     } else {
//       encodedStrutStyle = null;
//     }
//     _constructor(
//       style._encoded,
//       encodedStrutStyle,
//       style._fontFamily ?? '',
//       strutFontFamilies,
//       style._fontSize ?? 0,
//       style._height ?? 0,
//       style._ellipsis ?? '',
//       _encodeLocale(style._locale),
//     );
//   }
//
//   @Native<Void Function(Handle, Handle, Handle, Handle, Handle, Double, Double, Handle, Handle)>(symbol: 'BarcodeParagraphBuilder::Create')
//   external void _constructor(
//       Int32List encoded,
//       ByteData? strutData,
//       String fontFamily,
//       List<Object?>? strutFontFamily,
//       double fontSize,
//       double height,
//       String ellipsis,
//       String locale,
//       );
//
//   @override
//   int get placeholderCount => _placeholderCount;
//   int _placeholderCount = 0;
//
//   @override
//   List<double> get placeholderScales => _placeholderScales;
//   final List<double> _placeholderScales = <double>[];
//
//   final TextLeadingDistribution _defaultLeadingDistribution;
//
//   @override
//   void pushStyle(TextStyle style) {
//     final List<String> fullFontFamilies = <String>[];
//     fullFontFamilies.add(style._fontFamily);
//     final List<String>? fontFamilyFallback = style._fontFamilyFallback;
//     if (fontFamilyFallback != null) {
//       fullFontFamilies.addAll(fontFamilyFallback);
//     }
//
//     final Int32List encoded = style._encoded;
//     final TextLeadingDistribution finalLeadingDistribution = style._leadingDistribution ?? _defaultLeadingDistribution;
//     // ensure the enum can be represented using 1 bit.
//     assert(TextLeadingDistribution.values.length <= 2);
//
//     // Use the leading distribution from the paragraph's style if it's not
//     // explicitly set in `style`.
//     encoded[0] |= finalLeadingDistribution.index << 0;
//
//     ByteData? encodedFontFeatures;
//     final List<FontFeature>? fontFeatures = style._fontFeatures;
//     if (fontFeatures != null) {
//       encodedFontFeatures = ByteData(fontFeatures.length * FontFeature._kEncodedSize);
//       int byteOffset = 0;
//       for (final FontFeature feature in fontFeatures) {
//         feature._encode(ByteData.view(encodedFontFeatures.buffer, byteOffset, FontFeature._kEncodedSize));
//         byteOffset += FontFeature._kEncodedSize;
//       }
//     }
//
//     ByteData? encodedFontVariations;
//     final List<FontVariation>? fontVariations = style._fontVariations;
//     if (fontVariations != null) {
//       encodedFontVariations = ByteData(fontVariations.length * FontVariation._kEncodedSize);
//       int byteOffset = 0;
//       for (final FontVariation variation in fontVariations) {
//         variation._encode(ByteData.view(encodedFontVariations.buffer, byteOffset, FontVariation._kEncodedSize));
//         byteOffset += FontVariation._kEncodedSize;
//       }
//     }
//
//     _pushStyle(
//       encoded,
//       fullFontFamilies,
//       style._fontSize ?? 0,
//       style._letterSpacing ?? 0,
//       style._wordSpacing ?? 0,
//       style._height ?? 0,
//       style._decorationThickness ?? 0,
//       _encodeLocale(style._locale),
//       style._background?._objects,
//       style._background?._data,
//       style._foreground?._objects,
//       style._foreground?._data,
//       Shadow._encodeShadows(style._shadows),
//       encodedFontFeatures,
//       encodedFontVariations,
//     );
//   }
//
//   @Native<
//       Void Function(
//           Pointer<Void>,
//           Handle,
//           Handle,
//           Double,
//           Double,
//           Double,
//           Double,
//           Double,
//           Handle,
//           Handle,
//           Handle,
//           Handle,
//           Handle,
//           Handle,
//           Handle,
//           Handle)>(symbol: 'BarcodeParagraphBuilder::pushStyle')
//   external void _pushStyle(
//       Int32List encoded,
//       List<Object?> fontFamilies,
//       double fontSize,
//       double letterSpacing,
//       double wordSpacing,
//       double height,
//       double decorationThickness,
//       String locale,
//       List<Object?>? backgroundObjects,
//       ByteData? backgroundData,
//       List<Object?>? foregroundObjects,
//       ByteData? foregroundData,
//       ByteData shadowsData,
//       ByteData? fontFeaturesData,
//       ByteData? fontVariationsData,
//       );
//
//   static String _encodeLocale(Locale? locale) => locale?.toString() ?? '';
//
//   @override
//   @Native<Void Function(Pointer<Void>)>(symbol: 'BarcodeParagraphBuilder::pop', isLeaf: true)
//   external void pop();
//
//   @override
//   void addText(String text) {
//     final String? error = _addText(text);
//     if (error != null) {
//       throw ArgumentError(error);
//     }
//   }
//
//   @Native<Handle Function(Pointer<Void>, Handle)>(symbol: 'BarcodeParagraphBuilder::addText')
//   external String? _addText(String text);
//
//   @override
//   void addPlaceholder(double width, double height, PlaceholderAlignment alignment, {
//     double scale = 1.0,
//     double? baselineOffset,
//     TextBaseline? baseline,
//   }) {
//     // Require a baseline to be specified if using a baseline-based alignment.
//     assert(!(alignment == PlaceholderAlignment.aboveBaseline ||
//         alignment == PlaceholderAlignment.belowBaseline ||
//         alignment == PlaceholderAlignment.baseline) || baseline != null);
//     // Default the baselineOffset to height if null. This will place the placeholder
//     // fully above the baseline, similar to [PlaceholderAlignment.aboveBaseline].
//     baselineOffset = baselineOffset ?? height;
//     _addPlaceholder(width * scale, height * scale, alignment.index, baselineOffset * scale, (baseline ?? TextBaseline.alphabetic).index);
//     _placeholderCount++;
//     _placeholderScales.add(scale);
//   }
//
//   @Native<Void Function(Pointer<Void>, Double, Double, Uint32, Double, Uint32)>(symbol: 'BarcodeParagraphBuilder::addPlaceholder')
//   external void _addPlaceholder(double width, double height, int alignment, double baselineOffset, int baseline);
//
//   @override
//   Paragraph build() {
//     final _NativeParagraph paragraph = _NativeParagraph._();
//
//     return paragraph;
//   }
//
//
//
//   @override
//   String toString() => 'NativeBarcodeParagraphBuilder';
// }
// <<< add by sojet end

