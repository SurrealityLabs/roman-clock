/*
   This file is part of COVID Calendar.

   COVID Calendar is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   COVID Calendar is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with COVID Calendar. If not, see <https://www.gnu.org/licenses/>.
*/

#include <Wire.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <Commander.h>          // https://github.com/CreativeRobotics/Commander - MIT License
#include <Time.h>               // https://github.com/PaulStoffregen/Time - LGPL License
#include <TimeLib.h>            // https://github.com/PaulStoffregen/Time - LGPL License
#include <Adafruit_GFX.h>       // https://github.com/adafruit/Adafruit-GFX-Library - LGPL License
#include <Adafruit_LEDBackpack.h>
#include <Timezone.h>           // https://github.com/JChristensen/Timezone - GPL License
#include <RtcDS3231.h>          // https://github.com/Makuna/Rtc - GPL License
#include <NTPClient.h>          // https://github.com/arduino-libraries/NTPClient - MIT License. NOTE: Requires version newer than 3.2.0

RtcDS3231<TwoWire> Rtc(Wire);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

#include "config_struct.h"

calConfig_t calendarConfig;

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule myDST = {"EDT", Second, Sun, Mar, 2, -240};    // Daylight time = UTC - 4 hours
TimeChangeRule mySTD = {"EST", First, Sun, Nov, 2, -300};     // Standard time = UTC - 5 hours
Timezone myTZ(myDST, mySTD);

time_t getHwTime(void) {
  RtcDateTime tm = Rtc.GetDateTime();
  return tm.Epoch32Time();
}

void setHwTime(time_t epochTime) {
  RtcDateTime tm;
  tm.InitWithEpoch32Time(epochTime);
  Rtc.SetDateTime(tm);
}

void setup(void) {
  Serial.begin(115200);
  Serial.println(F("Starting"));

  SPIFFS.begin();

  loadConfig(&calendarConfig);

  if (strlen(calendarConfig.wifiSSID) > 0)
  {
    Serial.println(F("Connecting to WiFi..."));
    WiFi.mode(WIFI_STA);
    WiFi.begin(calendarConfig.wifiSSID, calendarConfig.wifiKey);

    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      i++;
      if (i > 20)
      {
        break;
      }
    }
  }

  render_setup();

  Wire.begin();
  tmElements_t tm;

  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!Rtc.IsDateTimeValid())
  {
    if (Rtc.LastError() != 0)
    {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }
    else
    {
      // Common Causes:
      //    1) first time you ran and the device wasn't running yet
      //    2) the battery on the device is low or even missing

      Serial.println("RTC lost confidence in the DateTime!");

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue

      Rtc.SetDateTime(compiled);
    }
  }
  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  setTime(getHwTime());
  setSyncProvider(getHwTime);
  setSyncInterval(600);

  Serial.print(F("The current time is:"));

  tmElements_t newTime;
  time_t tmp_t;
  tmp_t = now();
  tmp_t = myTZ.toLocal(tmp_t);
  breakTime(tmp_t, newTime);
  Serial.print(tmYearToCalendar(newTime.Year), DEC);
  Serial.print('/');
  Serial.print(newTime.Month, DEC);
  Serial.print('/');
  Serial.print(newTime.Day, DEC);
  Serial.print(' ');
  Serial.print(newTime.Hour, DEC);
  Serial.print(':');
  Serial.print(newTime.Minute, DEC);
  Serial.print(':');
  Serial.print(newTime.Second, DEC);
  Serial.println();

  timeClient.begin();
  timeClient.setTimeOffset(0);
  timeClient.setUpdateInterval(calendarConfig.ntpInterval);
  if (strlen(calendarConfig.ntpServer) > 0)
  {
    timeClient.setPoolServerName(calendarConfig.ntpServer);
  }

  randomSeed(now());
  command_setup();

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("posixclock");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop(void) {
  tmElements_t nowTime;
  time_t tmp_t, my_tmp_t;
  uint16_t brightTime;
  uint8_t bright;
  static tmElements_t bright_now;
  static uint32_t last_update_millis;

  command_loop();

  tmp_t = now();
  my_tmp_t = myTZ.toLocal(tmp_t);
  breakTime(my_tmp_t, nowTime);

  yield();
  if ((millis() - last_update_millis) > 100)
  {
    last_update_millis = millis();
    // Time rendering code

    breakTime(my_tmp_t, bright_now);

    brightTime = (bright_now.Hour * 100) + bright_now.Minute;

    if (calendarConfig.nightModeStart > calendarConfig.dayModeStart)
    {
      if ((brightTime >= calendarConfig.dayModeStart) && (brightTime < calendarConfig.nightModeStart))
      {
        bright = calendarConfig.dayModeBright;
      }
      else
      {
        bright = calendarConfig.nightModeBright;
      }
    }
    else
    {
      if ((brightTime >= calendarConfig.nightModeStart) && (brightTime < calendarConfig.dayModeStart))
      {
        bright = calendarConfig.nightModeBright;
      }
      else
      {
        bright = calendarConfig.dayModeBright;
      }
    }

    render_time(nowTime, bright);
  }
  yield();

  if (WiFi.status() == WL_CONNECTED)
  {
    ArduinoOTA.handle();
    if (strlen(calendarConfig.ntpServer) > 0)
    {
      if (calendarConfig.ntpInterval > 0)
      {
        time_t tmp_t;
        if (timeClient.update())
        {
          tmp_t = timeClient.getEpochTime();
          Serial.print(F("NTP update. Timestamp: "));
          Serial.println(tmp_t);
          setHwTime(tmp_t);
          setTime(tmp_t);
        }
      }
    }
  }
}
