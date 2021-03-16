#pragma once

#include "RTC.h"
#include "SSD1306Wire.h"

SSD1306Wire oleddisplay(0x3c, 21, 22);

void InitOledDisplay()
{
    if (!oleddisplay.init())
    {
        Serial.println("Couldn't initialize SHT31");
        return;
    }
    oleddisplay.flipScreenVertically();
}

void DisplayInfos(bool alarminterrupttriggered, bool touch1interrupttriggered, bool touch2interrupttriggered,
                  bool touch3interrupttriggered)
{
    RtcDateTime now = Rtc.GetDateTime();

    oleddisplay.clear();
    oleddisplay.drawString(10, 10, "Temp: " + String(thermometerdata.temperature, 1));
    oleddisplay.drawString(10, 20, "Hum:  " + String(thermometerdata.humidity, 1));
    oleddisplay.drawString(10, 30, "Time: " + String(now.Hour()) + ':' + now.Minute() + ':' + now.Second());

    if (alarminterrupttriggered)
        oleddisplay.drawString(10, 40, "Alarm! ");
    if (touch1interrupttriggered)
        oleddisplay.drawString(10, 50, "Touch1!");
    else if (touch2interrupttriggered)
        oleddisplay.drawString(50, 50, "Touch2!");
    else if (touch3interrupttriggered)
        oleddisplay.drawString(50, 50, "Touch3!");

    oleddisplay.display();
}
