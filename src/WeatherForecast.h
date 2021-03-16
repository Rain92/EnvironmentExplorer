#pragma once

#include <ArduinoJson.h>
#include "Settings.h"
#include "WifiManager.h"
#include "ezTime.h"

#define FORECASTNUMDAYS 5
#define FORECASTNUMDAYSSTR "5"
// #define FORECASTKEY "72f3c04de34a4f568fe232257190702"
// #define FORECASTAPIURL "http://api.apixu.com/v1/forecast.json?key=" FORECASTKEY "&days=" FORECASTNUMDAYSSTR "&q="

#define FORECASTKEY "01f7a96eba9dd52d84bfe8cc8e9929dc"
#define FORECASTAPIURL                                                                                                \
    "https://api.openweathermap.org/data/2.5/onecall?units=metric&exclude=current,minutely,hourly&appid=" FORECASTKEY \
    "&lat=49.0069&lon=8.4037"

void GetWeatherData()
{
    if (!wifiConnected)
        return;

    String url(FORECASTAPIURL);
    // url += settings.location;

    Serial.println(url);

    httpclient.begin(url);  // Specify the URL
    String jsonstring;
    auto start = millis();
    int httpCode = httpclient.GET();
    if (httpCode > 0)
    {  // Check for the returning code

        jsonstring = httpclient.getString();
        // Serial.println(httpCode);
        // Serial.println(jsonstring);
    }
    else
    {
        Serial.println("Error on HTTP request");
        return;
    }

    DynamicJsonDocument root(10000);
    DeserializationError error = deserializeJson(root, jsonstring);
    if (error)
    {
        Serial.println("Error parsing JSON object.");
        Serial.println(error.c_str());

        return;
    }

    // strcpy(settings.forecast.locationname, root["location"]["name"].as<char*>());
    strcpy(settings.forecast.timezone, root["timezone"].as<char*>());

    for (int i = 0; i < FORECASTNUMDAYS; i++)
    {
        int utc = root["daily"][i]["sunrise"].as<int>();

        sprintf(settings.forecast.days[i].date, "%02d.%02d.%02d", day(utc), month(utc), year(utc)%100);
        strcpy(settings.forecast.days[i].condition, root["daily"][i]["weather"][0]["main"].as<char*>());

        settings.forecast.days[i].conditioncode = root["daily"][i]["weather"][0]["id"].as<int>();
        settings.forecast.days[i].maxtemp = root["daily"][i]["temp"]["max"].as<float>();
        settings.forecast.days[i].mintemp = root["daily"][i]["temp"]["min"].as<float>();
        settings.forecast.days[i].humidity = root["daily"][i]["humidity"].as<float>();
        settings.forecast.days[i].windspeed = root["daily"][i]["wind_speed"].as<float>();
    }

    auto timetaken = millis() - start;
    Serial.printf("APIcall time taken: %lums\n", timetaken);

    SaveSettings();
}