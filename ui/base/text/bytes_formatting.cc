// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/text/bytes_formatting.h"

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"
#include "ui/base/l10n/l10n_util.h"

namespace ui {

namespace {

// Byte suffix string constants. These both must match the DataUnits enum.
const int kByteStrings[] = {
  /*IDS_APP_BYTES*/0,
  /*IDS_APP_KIBIBYTES*/0,
  /*IDS_APP_MEBIBYTES*/0,
  /*IDS_APP_GIBIBYTES*/0,
  /*IDS_APP_TEBIBYTES*/0,
  /*IDS_APP_PEBIBYTES*/0
};

const int kSpeedStrings[] = {
  /*IDS_APP_BYTES_PER_SECOND*/0,
  /*IDS_APP_KIBIBYTES_PER_SECOND*/0,
  /*IDS_APP_MEBIBYTES_PER_SECOND*/0,
  /*IDS_APP_GIBIBYTES_PER_SECOND*/0,
  /*IDS_APP_TEBIBYTES_PER_SECOND*/0,
  /*IDS_APP_PEBIBYTES_PER_SECOND*/0
};

string16 FormatBytesInternal(int64 bytes,
                             DataUnits units,
                             bool show_units,
                             const int* const suffix) {
  DCHECK(units >= DATA_UNITS_BYTE && units <= DATA_UNITS_PEBIBYTE);
  if (bytes < 0) {
    NOTREACHED() << "Negative bytes value";
    return string16();
  }

  // Put the quantity in the right units.
  double unit_amount = static_cast<double>(bytes);
  for (int i = 0; i < units; ++i)
    unit_amount /= 1024.0;

  int fractional_digits = 0;
  if (bytes != 0 && units != DATA_UNITS_BYTE && unit_amount < 100)
    fractional_digits = 1;

  // todo
  string16 result; //base::FormatDouble(unit_amount, fractional_digits);

  if (show_units)
    result = l10n_util::GetStringFUTF16(suffix[units], result);

  return result;
}

}  // namespace

DataUnits GetByteDisplayUnits(int64 bytes) {
  // The byte thresholds at which we display amounts. A byte count is displayed
  // in unit U when kUnitThresholds[U] <= bytes < kUnitThresholds[U+1].
  // This must match the DataUnits enum.
  static const int64 kUnitThresholds[] = {
    0,                // DATA_UNITS_BYTE,
    3 * (1LL << 10),  // DATA_UNITS_KIBIBYTE,
    2 * (1LL << 20),  // DATA_UNITS_MEBIBYTE,
    1LL << 30,        // DATA_UNITS_GIBIBYTE,
    1LL << 40,        // DATA_UNITS_TEBIBYTE,
    1LL << 50         // DATA_UNITS_PEBIBYTE,
  };

  if (bytes < 0) {
    NOTREACHED() << "Negative bytes value";
    return DATA_UNITS_BYTE;
  }

  int unit_index = arraysize(kUnitThresholds);
  while (--unit_index > 0) {
    if (bytes >= kUnitThresholds[unit_index])
      break;
  }

  DCHECK(unit_index >= DATA_UNITS_BYTE && unit_index <= DATA_UNITS_PEBIBYTE);
  return DataUnits(unit_index);
}

string16 FormatBytesWithUnits(int64 bytes, DataUnits units, bool show_units) {
  return FormatBytesInternal(bytes, units, show_units, kByteStrings);
}

string16 FormatSpeedWithUnits(int64 bytes, DataUnits units, bool show_units) {
  return FormatBytesInternal(bytes, units, show_units, kSpeedStrings);
}

string16 FormatBytes(int64 bytes) {
  return FormatBytesWithUnits(bytes, GetByteDisplayUnits(bytes), true);
}

string16 FormatSpeed(int64 bytes) {
  return FormatSpeedWithUnits(bytes, GetByteDisplayUnits(bytes), true);
}

}  // namespace ui
