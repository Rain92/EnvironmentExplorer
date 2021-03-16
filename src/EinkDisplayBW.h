
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "BME280Thermometer.h"
#include "Icons.h"
#include "RTC.h"
#include "Settings.h"

GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=5*/ SS, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

void InitEinkDisplay()
{
    display.init(0);
    u8g2Fonts.begin(display);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    display.setRotation(2);
    u8g2Fonts.setFontDirection(0);
}
void EinkDisplaySleep()
{
    display.hibernate();
}

void EinkDrawString(uint16_t x, uint16_t y, const char* format, ...)
{
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setCursor(x, y);

    char loc_buf[64];
    char* temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf))
    {
        temp = new char[len + 1];
        if (temp == NULL)
        {
            return;
        }
    }
    len = vsnprintf(temp, len + 1, format, arg);
    va_end(arg);

    u8g2Fonts.print(temp);

    if (len >= sizeof(loc_buf))
    {
        delete[] temp;
    }
}

void DisplayForecastQuadrant(int x, int y, char* date, char* date2, int mintemp, int maxtemp, const uint8_t* icon)
{
    u8g2Fonts.setFont(u8g2_font_profont17_tr);
    EinkDrawString(x + 5, y + 15, date);
    EinkDrawString(x + 5, y + 32, date2);

    u8g2Fonts.setFont(u8g2_font_courB24_tr);
    EinkDrawString(x + 25, y + 65, "%2d", maxtemp);
    EinkDrawString(x + 30, y + 87, "~");
    EinkDrawString(x + 25, y + 109, "%2d", mintemp);

    display.drawBitmap(x + 75, y - 1, icon, 128, 128, GxEPD_BLACK);
}

const uint8_t* GetWeatherBitmap(int conditioncode)
{
    // return BitmapSunRainCloud;
    switch (conditioncode)
    {
        case 800:  // Sunny
            return BitmapSun;

        case 801:  // few clouds: 11-25%
        case 802:  // scattered clouds: 25-50%
            return BitmapSunSmallCloud;

        case 803:  // broken clouds: 51-84%
        case 804:  // overcast clouds: 85-100%
            return BitmapCloud;

        case 701:  // Mist
        case 711:
        case 721:
        case 731:
        case 741:
        case 751:
        case 761:
        case 762:
        case 771:
        case 781:
            return BitmapSmallMist;

        case 500:
        case 501:
            return BitmapSunRainCloud;

        case 511:  //  	freezing rain
            return BitmapSleet;

        case 520:
        case 502:
        case 503:
        case 504:
            return BitmapRainCloud;

        case 521:
        case 522:
        case 531:
            return BitmapShower;

        case 200:  // thunderstorm with light rain
        case 201:
        case 202:
        case 210:
        case 211:
        case 212:
        case 221:
        case 230:
        case 231:
        case 232:
            return BitmapThunder;

        case 600:  // Snow
        case 601:
        case 602:
        case 611:
        case 612:
        case 613:
        case 615:
        case 616:
        case 620:
        case 621:
        case 622:
            return BitmapSnow;

        case 300:  // 	light intensity drizzle
        case 301:
        case 302:
        case 310:
        case 311:
        case 312:
        case 313:
        case 314:
        case 321:
            return BitmapRainCloud;

            // case 1237:  // Ice pellets
            //     return BitmapHail;

            // case 1273:  // Patchy light rain with thunder
            //     return BitmapShowerThunder;

        default:
            return BitmapSun;
    }
}

void EinkPrintTimePartial()
{
    auto now = Rtc.GetDateTime();

    display.setPartialWindow(5, 5, 180, 60);
    display.firstPage();
    do
    {
        u8g2Fonts.setFont(u8g2_font_fur49_tn);
        EinkDrawString(5, 60, "%02d:%02d", now.Hour(), now.Minute());
    } while (display.nextPage());
}

void EinkPrintTempPartial()
{
    display.setPartialWindow(70, 100, 65, 80);
    display.firstPage();
    do
    {
        u8g2Fonts.setFont(u8g2_font_helvB24_tf);
        EinkDrawString(70, 125, "%.1f", thermometerdata.temperature);
        EinkDrawString(70, 175, "%.1f", thermometerdata.humidity);
    } while (display.nextPage());
}

void EinkPrintAll()
{
    auto now = Rtc.GetDateTime();

    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);

        display.drawFastHLine(0, 70, 400, GxEPD_BLACK);
        display.drawFastVLine(200, 70, 115, GxEPD_BLACK);

        display.drawFastHLine(0, 185, 400, GxEPD_BLACK);
        display.drawFastVLine(200, 185, 115, GxEPD_BLACK);

        u8g2Fonts.setFont(u8g2_font_fur49_tn);
        EinkDrawString(5, 60, "%02d:%02d", now.Hour(), now.Minute());
        u8g2Fonts.setFont(u8g2_font_fur30_tr);
        EinkDrawString(230, 40, "%02d.%02d.%02d", now.Day(), now.Month(), now.Year() % 100);

        u8g2Fonts.setFont(u8g2_font_profont17_tr);
        EinkDrawString(340, 60, "%.1fv", GetBatteryVoltage());

        u8g2Fonts.setFont(u8g2_font_fur30_tr);

        display.drawBitmap(40, 98, BitmapThermometer, 16, 32, GxEPD_BLACK);
        display.drawBitmap(38, 141, BitmapWaterdrop, 24, 32, GxEPD_BLACK);

        u8g2Fonts.setFont(u8g2_font_profont17_tr);
        EinkDrawString(5, 85, "Current");

        u8g2Fonts.setFont(u8g2_font_helvB24_tf);
        EinkDrawString(70, 125, "%.1f C%c", thermometerdata.temperature, 176);
        EinkDrawString(70, 170, "%.1f %%", thermometerdata.humidity);

        DisplayForecastQuadrant(200, 70, "Today", "", settings.forecast.days[0].mintemp,
                                settings.forecast.days[0].maxtemp,
                                GetWeatherBitmap(settings.forecast.days[0].conditioncode));
        display.fillRect(250, 186, 150, 20, GxEPD_WHITE);
        DisplayForecastQuadrant(0, 185, "Tomorrow", "", settings.forecast.days[1].mintemp,
                                settings.forecast.days[1].maxtemp,
                                GetWeatherBitmap(settings.forecast.days[1].conditioncode));
        DisplayForecastQuadrant(200, 185, "Day after", "Tomorrow", settings.forecast.days[2].mintemp,
                                settings.forecast.days[2].maxtemp,
                                GetWeatherBitmap(settings.forecast.days[2].conditioncode));

    } while (display.nextPage());
}

void EinkPrintBatteryLow()
{
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(100, 90, BitmapBatteryLow, 200, 120, GxEPD_BLACK);

    } while (display.nextPage());
}
