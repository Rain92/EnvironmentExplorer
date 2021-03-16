#pragma once

#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r
#include <U8g2_for_Adafruit_GFX.h>

#include "Settings.h"
#include "BME280Thermometer.h"
#include "RTC.h"


GxIO_Class einkdisplayio(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class einkdisplay(einkdisplayio, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;


void InitEinkDisplay()
{
	einkdisplay.init(115200);
	einkdisplay.setRotation(1);
	u8g2_for_adafruit_gfx.begin(einkdisplay);
}

void EinkDisplaySleep()
{
	einkdisplay.powerDown();
}

void EinkDrawStringColored(uint16_t x, uint16_t y, const char* format, ...)
{
	u8g2_for_adafruit_gfx.setFontMode(1);
	u8g2_for_adafruit_gfx.setCursor(x, y);

	char loc_buf[64];
	char* temp = loc_buf;
	va_list arg;
	va_list copy;
	va_start(arg, format);
	va_copy(copy, arg);
	size_t len = vsnprintf(NULL, 0, format, arg);
	va_end(copy);
	if (len >= sizeof(loc_buf)) {
		temp = new char[len + 1];
		if (temp == NULL) {
			return;
		}
	}
	len = vsnprintf(temp, len + 1, format, arg);
	va_end(arg);

	for (uint16_t i = 0; i < len; i++)
	{
		if (temp[i] == '\r')
			u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_RED);
		else if (temp[i] == '\b')
			u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_BLACK);
		else
			u8g2_for_adafruit_gfx.write(temp[i]);
	}

	if (len >= sizeof(loc_buf)) {
		delete[] temp;
	}
}


void EinkDisplayInfos()
{
	auto now = Rtc.GetDateTime();
	einkdisplay.fillScreen(GxEPD_WHITE);
	u8g2_for_adafruit_gfx.setFontDirection(0);
	u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_BLACK);

	u8g2_for_adafruit_gfx.setFont(u8g2_font_profont17_mf);
	EinkDrawStringColored(0, 20, "%02d.%02d.%02d \r%02d:%02d\b", now.Day(), now.Month(),
		now.Year()%100, now.Hour(), now.Minute());
	EinkDrawStringColored(0, 60, "Temp: \r%.1f\b C%c", thermometerdata.temperature, 176);
	EinkDrawStringColored(0, 90, "Hum:  \r%.1f\b %%", thermometerdata.humidity);

	einkdisplay.drawFastVLine(136, 0, 128, GxEPD_BLACK);

	EinkDrawStringColored(140, 20, "Today:");
	EinkDrawStringColored(140, 36, "\r%s\b", settings.forecast.days[0].condition);
	EinkDrawStringColored(140, 52, "Max: \r%.0f\b Min: \r%.0f\b", settings.forecast.days[0].maxtemp, settings.forecast.days[0].mintemp);
	EinkDrawStringColored(140, 80, "Tomorrow:");
	EinkDrawStringColored(140, 96, "\r%s\b", settings.forecast.days[1].condition);
	EinkDrawStringColored(140, 112, "Max: \r%.0f\b Min: \r%.0f\b", settings.forecast.days[1].maxtemp, settings.forecast.days[1].mintemp);

	einkdisplay.update();
}