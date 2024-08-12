/*
 * Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIB_UI_BARCODE_BARCODE_STYLE_H_
#define LIB_UI_BARCODE_BARCODE_STYLE_H_

#include <optional>
#include <string>
#include <vector>

#include "flutter/display_list/dl_paint.h"
#include "flutter/lib/ui/skiabarcode/BarcodeCommon.h"


namespace sojet {
namespace barcode {

class BarcodeStyle {
 public:

  std::optional<flutter::DlPaint> background;
  std::optional<flutter::DlPaint> foreground;
  float x_dimension = 9.0;
  float bar_height = 227.0/9;
  int quiet_zone_width = 0;
  int quiet_zone_height = 0;
  int human_readable_location = 0;
  int text_margin = 3;
  BearerBarType bearer_bar_type = BearerBarType::BEARER_BAR_NONE;
  int thickness = 0;

  BarcodeStyle();

  bool equals(const BarcodeStyle& other) const;
};

}  // namespace barcode
}  // namespace sojet

#endif  // LIB_TXT_SRC_TEXT_STYLE_H_
