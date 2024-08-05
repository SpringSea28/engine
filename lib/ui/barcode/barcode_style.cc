/*
 * Copyright 2017 Google, Inc.
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

#include "barcode_style.h"

namespace sojet {
namespace barcode {

BarcodeStyle::BarcodeStyle()  {}

bool BarcodeStyle::equals(const BarcodeStyle& other) const {
  if (foreground != other.foreground)
    return false;
  if (x_dimension != other.x_dimension)
    return false;
  if (bar_height != other.bar_height)
    return false;
  if (quiet_zone_width != other.quiet_zone_width)
    return false;
  if (quiet_zone_height != other.quiet_zone_height)
    return false;
  if (human_readable_location != other.human_readable_location)
    return false;
  if (text_margin != other.text_margin)
    return false;
  if (bearer_bar_type != other.bearer_bar_type)
    return false;
  if (thickness != other.thickness)
    return false;


  return true;
}

}  // namespace barcode
}  // namespace sojet
