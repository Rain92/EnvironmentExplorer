#pragma once

#include <Wire.h>
#include "RtcDS3231.h"

RtcDS3231<TwoWire> Rtc(Wire);

void InitRTC()
{
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (!Rtc.IsDateTimeValid() || now < compiled)
    {
    	Serial.println("RTC is older than compile time!  (Updating DateTime)");
    	Rtc.SetDateTime(compiled);
    }

    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

    // RtcDateTime alarmTime = now + 5; // into the future
    // DS3231AlarmOne alarm1(
    // 	alarmTime.Day(),
    // 	alarmTime.Hour(),
    // 	alarmTime.Minute(),
    // 	alarmTime.Second(),
    // 	DS3231AlarmOneControl_HoursMinutesSecondsMatch);

    // DS3231AlarmTwo alarm2(
    // 	0,
    // 	0,
    // 	0,
    // 	DS3231AlarmTwoControl_OncePerMinute);

    // //Rtc.SetAlarmOne(alarm1);
    // Rtc.SetAlarmTwo(alarm2);

    // Rtc.LatchAlarmsTriggeredFlags();
}
