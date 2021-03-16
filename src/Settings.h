#pragma once

#include <ArduinoNvs.h>

#define MAGICKEY 83634762ull

#define MAGICKEY_FLASHKEY "magickey"
#define SETTINGS_FLASHKEY "settings"

#define FORECASTNUMDAYS 5
#define FORECASTNUMDAYSSTR "5"

#define wifissiddefault "Andreas-PC_AP"
#define wifipassworddefault "12345678"
#define locationdefault "Karlsruhe"
#define updateintervaldefault "1"

//#define wifissiddefault "WLAN-66FB40_EXT"
//#define wifipassworddefault "3169335005153157"
//#define locationdefault "Eddersheim"

#define timezonedefault "Europe/Berlin"

struct ForecastDay
{
    char date[12];
    char condition[44];
    int conditioncode;
    float mintemp;
    float maxtemp;
    float windspeed;
    float humidity;
};
struct WeatherForecast
{
    char locationname[32] = locationdefault;
    char timezone[32] = timezonedefault;
    ForecastDay days[FORECASTNUMDAYS];
};
struct WiFiConfig
{
    char ssid[32] = wifissiddefault;
    char password[32] = wifipassworddefault;
};

struct Settings
{
    char location[32] = locationdefault;
    WiFiConfig wificonfig;
    WeatherForecast forecast;
    char updateinterval[4] = updateintervaldefault;
    bool lowbatteryprinted = false;
};

Settings settings;

bool SaveSettings()
{
    Serial.println("Saving settings.");

    bool res1 = NVS.setInt(MAGICKEY_FLASHKEY, MAGICKEY);
    bool res2 = NVS.setBlob(SETTINGS_FLASHKEY, (uint8_t*)&settings, sizeof(Settings));

    if (!res1 || !res2)
    {
        Serial.println("Couldn't save settings.");
        return false;
    }
    return true;
}

void PrintSettings(int level = 7)
{
    Serial.println(settings.location);
    Serial.println(settings.wificonfig.ssid);
    Serial.println(settings.wificonfig.password);
    Serial.printf("Update interval: %s\n", settings.updateinterval);

    if (level >= 1)
    {
        Serial.println(settings.forecast.locationname);
        Serial.println(settings.forecast.timezone);
    }

    for (int i = 0; i < FORECASTNUMDAYS && i <= level - 2; i++)
    {
        Serial.printf("Day: %d\n", i + 1);
        Serial.println(settings.forecast.days[i].date);
        Serial.println(settings.forecast.days[i].condition);
        Serial.println(settings.forecast.days[i].conditioncode);
        Serial.println(settings.forecast.days[i].maxtemp);
        Serial.println(settings.forecast.days[i].mintemp);
        Serial.println(settings.forecast.days[i].humidity);
        Serial.println(settings.forecast.days[i].windspeed);
    }
}

bool InitSettings()
{
    NVS.begin();

    uint64_t magickey = NVS.getInt(MAGICKEY_FLASHKEY);
    size_t blobsize = NVS.getBlobSize(SETTINGS_FLASHKEY);

    if (magickey != MAGICKEY || blobsize != sizeof(Settings))
    {
        NVS.eraseAll();
        Serial.println(blobsize);
        SaveSettings();
    }
    else
    {
        Serial.println("Loading settings.");

        bool res = NVS.getBlob(SETTINGS_FLASHKEY, (uint8_t*)&settings, sizeof(Settings));
        if (!res)
            Serial.println("Couldn't load settings.");
        PrintSettings(1);
    }

    return true;
}
