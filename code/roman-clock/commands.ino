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
 
#include <Commander.h>
#include "dateTimeValidator.h"
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <NTPClient.h>

#include "config_struct.h"

Commander cmd;
extern Timezone myTZ;

extern calConfig_t calendarConfig;
extern void setHwTime(time_t epochTime);

extern NTPClient timeClient;

const commandList_t masterCommands[] = {
  {"setDate",               setDateHandler,             "setDate [day] [month] [year]"},
  {"setTime",               setTimeHandler,             "setTime [hours] [minutes] [seconds]"},
  {"printTime",             printTimeHandler,           "printTime"},
  {"setBright",             setBrightHandler,           "setBright [night hour] [night minute] [night brightness] [day hour] [day minute] [day brightness]"},
  {"printBright",           printBrightHandler,         "printBright"},
  {"setWifi",               setWifiHandler,             "setWifi [ssid] [psk]"},
  {"printWifi",             printWifiHandler,           "printWifi"},
  {"printIP",               printIPHandler,             "printIP"},
  {"setNTP",                setNTPHandler,              "setNTP [server] [interval]"},
  {"printNTP",              printNTPHandler,            "printNTP"},
};

void command_setup()
{
  cmd.begin(&Serial, masterCommands, sizeof(masterCommands));
  cmd.commandPrompt(ON); //enable the command prompt
  cmd.echo(true);     //Echo incoming characters to theoutput port
  cmd.errorMessages(ON); //error messages are enabled - it will tell us if we issue any unrecognised commands
  cmd.autoChain(ON);
  cmd.delimiters("= ,\t\\|");
  cmd.setBuffer(384);
  cmd.printCommandPrompt();
}

void command_loop()
{
  cmd.update();
}

bool setDateHandler(Commander &Cmdr) {
  if(3 != Cmdr.countItems()) {
    Serial.println(F("Insufficient arguments!"));
    return 1;
  }

  uint8_t dayNum;
  uint8_t monthNum;
  uint16_t yearNum;
  Cmdr.getInt(dayNum);
  Cmdr.getInt(monthNum);
  Cmdr.getInt(yearNum);

  uint8_t tmp = validateDate(yearNum, monthNum, dayNum);

  if(tmp == 2) {
    Serial.println(F("Invalid year!"));
    return true;
  } else if(tmp == 3) {
    Serial.println(F("Invalid month!"));
    return true;
  } else if(tmp == 4) {
    Serial.println(F("Invalid day!"));
    return true;
  }
  
  tmElements_t newTime;
  time_t tmp_t;
  tmp_t = now();
  tmp_t = myTZ.toLocal(tmp_t);
  breakTime(tmp_t, newTime);
  newTime.Year = CalendarYrToTm(yearNum);
  newTime.Month = monthNum;
  newTime.Day = dayNum;
  
  tmp_t = makeTime(newTime);
  tmp_t = myTZ.toUTC(tmp_t);
  setHwTime(tmp_t);
  setTime(tmp_t);

  Serial.print(F("Setting date to "));
  Serial.print(dayNum);
  Serial.print('/');
  Serial.print(monthNum);
  Serial.print('/');
  Serial.println(yearNum);
  return 0;  
}

bool setTimeHandler(Commander &Cmdr) {
  if(3 != Cmdr.countItems()) {
    Serial.println(F("Insufficient arguments!"));
    return 1;
  }

  uint8_t hourNum;
  uint8_t minNum;
  uint16_t secNum;
  Cmdr.getInt(hourNum);
  Cmdr.getInt(minNum);
  Cmdr.getInt(secNum);

  uint8_t tmp = validateTime(hourNum, minNum, secNum);

  if(tmp == 2) {
    Serial.println(F("Invalid hours!"));
    return true;
  } else if(tmp == 3) {
    Serial.println(F("Invalid minutes!"));
    return true;
  } else if(tmp == 4) {
    Serial.println(F("Invalid seconds!"));
    return true;
  }

  tmElements_t newTime;
  time_t tmp_t;
  tmp_t = now();
  tmp_t = myTZ.toLocal(tmp_t);
  breakTime(tmp_t, newTime);
  newTime.Hour = hourNum;
  newTime.Minute = minNum;
  newTime.Second = secNum;

  tmp_t = makeTime(newTime);
  tmp_t = myTZ.toUTC(tmp_t);
  setHwTime(tmp_t);
  setTime(tmp_t);


  Serial.print(F("Setting time to "));
  Serial.print(hourNum);
  Serial.print(F(":"));
  Serial.print(minNum);
  Serial.print(F(":"));
  Serial.println(secNum);
  return 0;  
}

bool printTimeHandler(Commander &Cmdr) {
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
  return 0;
}

bool setBrightHandler(Commander &Cmdr) {
  if(6 != Cmdr.countItems()) {
    Serial.println(F("Insufficient arguments!"));
    return 1;
  }

  uint8_t hourNum;
  uint8_t minNum;
  uint8_t secNum = 0;

  Cmdr.getInt(hourNum);
  Cmdr.getInt(minNum);

  uint8_t tmp = validateTime(hourNum, minNum, secNum);

  if(tmp == 2) {
    Serial.println(F("Invalid hours!"));
    return true;
  } else if(tmp == 3) {
    Serial.println(F("Invalid minutes!"));
    return true;
  }

  calendarConfig.nightModeStart = (hourNum * 100) + minNum;

  Cmdr.getInt(tmp);

  calendarConfig.nightModeBright = tmp;

  Cmdr.getInt(hourNum);
  Cmdr.getInt(minNum);

  tmp = validateTime(hourNum, minNum, secNum);

  if(tmp == 2) {
    Serial.println(F("Invalid hours!"));
    return true;
  } else if(tmp == 3) {
    Serial.println(F("Invalid minutes!"));
    return true;
  }

  calendarConfig.dayModeStart = (hourNum * 100) + minNum;

  Cmdr.getInt(tmp);

  calendarConfig.dayModeBright = tmp;

  saveConfig(&calendarConfig);
}

bool printBrightHandler(Commander &Cmdr) {
  Serial.print(F("Night mode begins at "));
  Serial.print(calendarConfig.nightModeStart);
  Serial.print(F(" with a brightness of "));
  Serial.println(calendarConfig.nightModeBright);
  Serial.print(F("Day mode begins at "));
  Serial.print(calendarConfig.dayModeStart);
  Serial.print(F(" with a brightness of "));
  Serial.println(calendarConfig.dayModeBright);
}

bool setWifiHandler(Commander &Cmdr) {
  String tmp = "";
  int itms = Cmdr.countItems();
  
  if(1 > itms) {
    Serial.println(F("Insufficient arguments!"));
    return 1;
  }

  Cmdr.getString(tmp);
  itms--;
  strlcpy(calendarConfig.wifiSSID, tmp.c_str(), 33);
  if (itms >= 1)
  {
    Cmdr.getString(tmp);
    strlcpy(calendarConfig.wifiKey, tmp.c_str(), 33);
  }
  else
  {
    memset(calendarConfig.wifiKey, 0, 33);
  }

  saveConfig(&calendarConfig);

  if (strlen(calendarConfig.wifiSSID) > 0)
  {
    Serial.println(F("Connecting to WiFi..."));
    WiFi.mode(WIFI_STA);
    WiFi.begin(calendarConfig.wifiSSID, calendarConfig.wifiKey);
  }

  return 0;
}

bool printWifiHandler(Commander &Cmdr) {
  Serial.print(F("Saved SSID is \""));
  Serial.print(calendarConfig.wifiSSID);
  Serial.println(F("\""));
  Serial.print(F("Saved key is \""));
  Serial.print(calendarConfig.wifiKey);
  Serial.println(F("\""));
}

bool printIPHandler(Commander &Cmdr) {
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print(F("WiFi is connected. IP address is "));
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println(F("WiFi is not connected."));
  }
}

bool setNTPHandler(Commander &Cmdr) {
  String tmp = "";
  uint32_t tmpInt;
  int itms = Cmdr.countItems();
  
  if(1 > itms) {
    Serial.println(F("Insufficient arguments!"));
    return 1;
  }

  Cmdr.getString(tmp);
  itms--;
  strlcpy(calendarConfig.ntpServer, tmp.c_str(), 33);
  if (itms >= 1)
  {
    Cmdr.getInt(tmpInt);
    calendarConfig.ntpInterval = tmpInt;
  }
  else
  {
    calendarConfig.ntpInterval = 86400000ul;
  }

  timeClient.setUpdateInterval(calendarConfig.ntpInterval);
  if (strlen(calendarConfig.ntpServer) > 0)
  {
    timeClient.setPoolServerName(calendarConfig.ntpServer);
  }

  saveConfig(&calendarConfig);

  return 0;
}

bool printNTPHandler(Commander &Cmdr) {
  Serial.print(F("Saved server is \""));
  Serial.print(calendarConfig.ntpServer);
  Serial.println(F("\""));
  Serial.print(F("Saved interval is "));
  Serial.print(calendarConfig.ntpInterval);
  Serial.println(F(""));
}

