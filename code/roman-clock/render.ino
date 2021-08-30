/*
 * This file is part of COVID Calendar.
 *
 * COVID Calendar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * COVID Calendar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COVID Calendar. If not, see <https://www.gnu.org/licenses/>.
 */
 
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <Time.h>
#include <TimeLib.h>

#include "config_struct.h"

Adafruit_AlphaNum4 alpha0 = Adafruit_AlphaNum4();
Adafruit_AlphaNum4 alpha1 = Adafruit_AlphaNum4();
Adafruit_AlphaNum4 alpha2 = Adafruit_AlphaNum4();
Adafruit_AlphaNum4 alpha3 = Adafruit_AlphaNum4();

extern calConfig_t calendarConfig;

void render_setup()
{
  alpha0.begin(0x71);  // pass in the address
  alpha1.begin(0x72);  // pass in the address
  alpha2.begin(0x73);  // pass in the address
  alpha3.begin(0x74);  // pass in the address
}

static const char* romanStrings[] = {
  "",
  "I",
  "II",
  "III",
  "IV",
  "V",
  "VI",
  "VII",
  "VIII",
  "IX",
  "X",
  "XI",
  "XII",
  "XIII",
  "XIV",
  "XV",
  "XVI",
  "XVII",
  "XVIII",
  "XIX",
  "XX",
  "XXI",
  "XXII",
  "XXIII",
  "XXIV",
  "XXV",
  "XXVI",
  "XXVII",
  "XXVIII",
  "XXIX",
  "XXX",
  "XXXI",
  "XXXII",
  "XXXIII",
  "XXXIV",
  "XXXV",
  "XXXVI",
  "XXXVII",
  "XXXVIII",
  "XXXIX",
  "XL",
  "XLI",
  "XLII",
  "XLIII",
  "XLIV",
  "XLV",
  "XLVI",
  "XLVII",
  "XLVIII",
  "XLIX",
  "L",
  "LI",
  "LII",
  "LIII",
  "LIV",
  "LV",
  "LVI",
  "LVII",
  "LVIII",
  "LIX",
};

void render_time(tmElements_t now, uint8_t bright) {
  uint8_t hr, mn;
  char line1[9], line2[9];

  bright /= 16;

  if (bright == 0)
  {
    alpha0.clear();
    alpha1.clear();
    alpha2.clear();
    alpha3.clear();

    alpha0.writeDisplay();
    alpha1.writeDisplay();
    alpha2.writeDisplay();
    alpha3.writeDisplay();
    
    return;
  }

  alpha0.setBrightness(bright);
  alpha1.setBrightness(bright);
  alpha2.setBrightness(bright);
  alpha3.setBrightness(bright);

  memset(line1, 0, 9);
  memset(line2, 0, 9);

  /* This is probably unnecessary, but indexing romanStrings beyond the 60-string limit is bad, so here we are */
  hr = now.Hour % 24;
  mn = now.Minute % 60;

  strncpy(line1, romanStrings[hr], 9);
  strncpy(line2, romanStrings[mn], 9);
  
  alpha0.writeDigitAscii(0, line1[0], false);
  alpha0.writeDigitAscii(1, line1[1], false);
  alpha0.writeDigitAscii(2, line1[2], false);
  alpha0.writeDigitAscii(3, line1[3], false);
  alpha1.writeDigitAscii(0, line1[4], false);
  alpha1.writeDigitAscii(1, line1[5], false);
  alpha1.writeDigitAscii(2, line1[6], false);
  alpha1.writeDigitAscii(3, line1[7], false);

  alpha2.writeDigitAscii(0, line2[0], false);
  alpha2.writeDigitAscii(1, line2[1], false);
  alpha2.writeDigitAscii(2, line2[2], false);
  alpha2.writeDigitAscii(3, line2[3], false);
  alpha3.writeDigitAscii(0, line2[4], false);
  alpha3.writeDigitAscii(1, line2[5], false);
  alpha3.writeDigitAscii(2, line2[6], false);
  alpha3.writeDigitAscii(3, line2[7], false);

  alpha0.writeDisplay();
  alpha1.writeDisplay();
  alpha2.writeDisplay();
  alpha3.writeDisplay();
}
