#pragma once

#include <HTTPClient.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <string.h>
//#include <esp_smartconfig.h>
#include <RtcDS3231.h>
#include <ezTime.h>
#include "Settings.h"

#define SPARATIONSYMBOL ((char)31)

bool smartConfigDone = false;
bool wifiConnected = false;
HTTPClient httpclient;

bool ConnectWifi(int timeoutSeconds = 5)
{
    if (WiFi.status() == WL_CONNECTED)
        return true;

    Serial.println("Starting WiFi..");
    WiFi.begin(settings.wificonfig.ssid, settings.wificonfig.password);

    int loopcounter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        if (++loopcounter > timeoutSeconds)
            return false;
        delay(1000);
        Serial.println("Establishing connection to WiFi..");
    }

    wifiConnected = true;

    Serial.println("Connected to network");
    Serial.println(WiFi.macAddress());
    Serial.println(WiFi.localIP());
    return true;
}

void EndWifi()
{
    WiFi.disconnect(true, true);
    wifiConnected = false;

    esp_wifi_stop();
}

RtcDateTime GetNTPTime()
{
    if (!wifiConnected)
        return RtcDateTime();

    // setDebug(INFO);
    Serial.println("Getting Time. Waiting for sync");

    waitForSync(5);

    Timezone timezone;
    timezone.setLocation(settings.forecast.timezone);

    Serial.println("Time:");
    Serial.println(timezone.dateTime());

    RtcDateTime rtcdatetime(timezone.year(), timezone.month(), timezone.day(), timezone.hour(), timezone.minute(),
                            timezone.second());

    return rtcdatetime;
}

//
// void smartConfigCallback(uint32_t st, void* result) {
//	smartconfig_status_t status = (smartconfig_status_t)st;
//	if (status == SC_STATUS_GETTING_SSID_PSWD)
//	{
//	}
//	else if (status == SC_STATUS_LINK)
//	{
//		wifi_sta_config_t* sta_conf = reinterpret_cast<wifi_sta_config_t*>(result);
//
//		auto ssid = (char*)(sta_conf->ssid);
//		auto passlocation = (char*)(sta_conf->password);
//
//		int i = 0;
//		int ii = 0;
//		for (i = 0; ssid[i] != 0; i++)
//		{
//			settings.wificonfig.ssid[i] = ssid[i];
//		}
//		settings.wificonfig.ssid[i] = 0;
//
//		for (i = 0; passlocation[i] != SPARATIONSYMBOL; i++)
//		{
//			settings.wificonfig.password[i] = passlocation[i];
//		}
//		settings.wificonfig.password[i] = 0;
//
//		passlocation[i++] = 0;
//		for (ii = 0; passlocation[i] != 0; i++, ii++)
//		{
//			settings.location[ii] = passlocation[i];
//		}
//		settings.location[ii] = 0;
//
//		SaveSettings();
//
//		sta_conf->bssid_set = 0;
//		//esp_wifi_set_config(WIFI_IF_STA, (wifi_config_t*)sta_conf);
//		//esp_wifi_connect();
//		smartConfigDone = true;
//	}
//	else if (status == SC_STATUS_LINK_OVER) {
//		WiFi.stopSmartConfig();
//	}
//}
//
//
// bool GetSmartConfigs(int timeoutSeconds = 10)
//{
//	EndWifi();
//	WiFi.stopSmartConfig();
//
//	delay(100);
//
//	Serial.printf("\n Waiting for SmartConfig\n\n");
//
//	WiFi.mode(WIFI_STA);
//
//	delay(100);
//
//	esp_wifi_disconnect();
//	delay(100);
//
//	esp_smartconfig_start(reinterpret_cast<sc_callback_t>(&smartConfigCallback), 1);
//
//
//	int loopCounter = 0;
//	while (!smartConfigDone)
//	{
//		if (loopCounter > timeoutSeconds * 10)
//		{
//			Serial.printf("Smartconfig Timeout.");
//			WiFi.stopSmartConfig();
//			return false;
//		}
//
//		delay(100);
//		++loopCounter;
//	}
//
//	PrintSettings(0);
//
//	//while (WiFi.status() != WL_CONNECTED)    	// check till connected
//	//{
//	//	delay(50);
//	//}
//
//
//	WiFi.stopSmartConfig();
//	loopCounter = 0;
//
//
//	return true;
//}
