/*
 Name:		EnvironmentExplorer.ino
 Created:	06.02.2019 20:21:22
 Author:	Andreas Schaarschmidt
*/

#include "BME280Thermometer.h"
#include "BatteryMonitor.h"
// #include "BluetoothManager.h"
#include "EinkDisplayBW.h"
#include "RTC.h"
#include "WeatherForecast.h"
#include "WifiManager.h"
#include "driver/adc.h"
#include "driver/rtc_io.h"
#include "esp_sleep.h"
//#include "OledDisplay.h"

#define POWERPIN GPIO_NUM_33
#define TOUCHPIN1 T4
#define TOUCHPIN2 T2
#define TOUCHPIN3 T3

// the bigger the Threshhold the more sensitive
#define TOUCHTHRESHOLD1 55
#define TOUCHTHRESHOLD2 50
#define TOUCHTHRESHOLD3 40

#define LOWBATTERYTHRESHOLD 3.6

#define WAKEUPPERIOD 60 * 2

enum WakeupCause
{
    OtherWakeupCause,
    TimerWakeupCause,
    TouchPin1WakeupCause,
    TouchPin2WakeupCause,
    TouchPin3WakeupCause
};
WakeupCause wakeupcause;

volatile bool alarminterrupttriggered = false;
bool alarminterruptprocessed = false;
volatile bool touch1interrupttriggered = false;
bool touch1interruptprocessed = false;
volatile bool touch2interrupttriggered = false;
bool touch2interruptprocessed = false;
volatile bool touch3interrupttriggered = false;
bool touch3interruptprocessed = false;

void IRAM_ATTR HandleAlarmInterrupt()
{
    alarminterrupttriggered = true;
}

void IRAM_ATTR HandleTouch1Interrupt()
{
    touch1interrupttriggered = true;
}

void IRAM_ATTR HandleTouch2Interrupt()
{
    touch2interrupttriggered = true;
}

void IRAM_ATTR HandleTouch3Interrupt()
{
    touch3interrupttriggered = true;
}

void GetWakeupReason()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    wakeupcause = OtherWakeupCause;

    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
    {
        wakeupcause = TimerWakeupCause;
    }
    else if (wakeup_reason == ESP_SLEEP_WAKEUP_TOUCHPAD)
    {
        int touchpadnum = esp_sleep_get_touchpad_wakeup_status();

        if (touchpadnum == 4)
            wakeupcause = TouchPin1WakeupCause;
        else if (touchpadnum == 2)
            wakeupcause = TouchPin2WakeupCause;
        else if (touchpadnum == 3)
            wakeupcause = TouchPin3WakeupCause;
    }
}

void PrintWakeupReason()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("Wakeup caused by external signal using RTC_IO");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("Wakeup caused by external signal using RTC_CNTL");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Wakeup caused by timer");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            Serial.printf("Wakeup caused by touchpad %d\n", esp_sleep_get_touchpad_wakeup_status());
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            Serial.println("Wakeup caused by ULP program");
            break;
        default:
            Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
            break;
    }
}

void setup()
{
    pinMode(POWERPIN, OUTPUT);
    digitalWrite(POWERPIN, HIGH);

    Serial.begin(115200);

    GetWakeupReason();
    PrintWakeupReason();

    InitBatteryMonitor();
    InitSettings();
    InitEinkDisplay();
    // InitOledDisplay();
    InitThermometer();
    InitRTC();

    // attachInterrupt(digitalPinToInterrupt(ALARMINTERRUPTPIN), HandleAlarmInterrupt, FALLING);

    touchAttachInterrupt(TOUCHPIN1, HandleTouch1Interrupt, TOUCHTHRESHOLD1);
    touchAttachInterrupt(TOUCHPIN2, HandleTouch2Interrupt, TOUCHTHRESHOLD2);
    // touchAttachInterrupt(TOUCHPIN3, HandleTouch3Interrupt, TOUCHTHRESHOLD3);

    CheckBattery();

    ProcessWakeupCause();
}

void ProcessWakeupCause()
{
    UpdateTemperature();
    // RTC Alarm
    if (wakeupcause == TimerWakeupCause)
    {
        auto now = Rtc.GetDateTime();

        if (now.Minute() == 0 && now.Hour() % 4 == 0 && !(now.Hour() > 0 && now.Hour() < 8))
        {
            ConnectWifi(8);
            SyncRTCTime();
            GetWeatherData();
            EndWifi();
        }

        int cinterval = min(10, max(1, atoi(settings.updateinterval)));
        // Serial.printf("Parsed Interval: %d", cinterval);

        if (now.Minute() % cinterval == 0)
            EinkPrintAll();
    }
    // Other/normal Boot
    else if (wakeupcause == OtherWakeupCause)
    {
        // ConnectWifi(5);
        // SyncRTCTime();
        // GetWeatherData();
        // EndWifi();
        EinkPrintAll();
    }
    else if (wakeupcause == TouchPin1WakeupCause)
    {
        EinkPrintAll();
    }
    // Manual Update
    else if (wakeupcause == TouchPin2WakeupCause)
    {
        ConnectWifi(8);
        SyncRTCTime();
        GetWeatherData();
        EndWifi();

        EinkPrintAll();
    }
    // Wifi Config
    else if (wakeupcause == TouchPin3WakeupCause)
    {
        // GetSmartConfigs(30);

        // EnableBluetooth();

        sleep(30);
        // DisableBluetooth();

        esp_restart();
    }

    sleep();
}

void loop()
{
    UpdateTemperature();

    // DisplayInfos(alarminterrupttriggered, touch1interrupttriggered,
    //	touch1interrupttriggered, touch3interrupttriggered);
    ProcessInterrupts();

    delay(200);
}

void CheckBattery()
{
    double voltage = GetBatteryVoltage();

    Serial.println("Battery Voltage: " + String(voltage, 3));

    if (voltage > 1.6 && voltage < LOWBATTERYTHRESHOLD)
    {
        if (!settings.lowbatteryprinted)
        {
            EinkPrintBatteryLow();
            settings.lowbatteryprinted = true;
            SaveSettings();
        }
        sleep();
    }
    else if (settings.lowbatteryprinted)
    {
        settings.lowbatteryprinted = false;
        SaveSettings();
    }
}

void SyncRTCTime()
{
    if (!wifiConnected)
        return;

    Serial.printf("Syncing RCT time.");

    RtcDateTime now = GetNTPTime();
    if (now.Year() > 2000 && now.Year() < 2100)
        Rtc.SetDateTime(now);

    delay(200);
}

void sleep()
{
    EinkDisplaySleep();
    // oleddisplay.displayOff();
    EndWifi();

    long currentMillis = xTaskGetTickCount();
    Serial.printf("ON time: %d", currentMillis);

    auto now = Rtc.GetDateTime();

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    auto sleeptime = WAKEUPPERIOD - ((now.Minute() * 60 + now.Second()) % (WAKEUPPERIOD));
    esp_sleep_enable_timer_wakeup(sleeptime * 1000 * 1000);

    esp_sleep_enable_touchpad_wakeup();
    digitalWrite(POWERPIN, LOW);
    rtc_gpio_isolate(POWERPIN);
    // gpio_reset_pin(POWERPIN);
    gpio_reset_pin(GPIO_NUM_21);
    gpio_reset_pin(GPIO_NUM_22);
    gpio_reset_pin(GPIO_NUM_23);
    gpio_reset_pin(GPIO_NUM_5);
    gpio_reset_pin(GPIO_NUM_16);
    gpio_reset_pin(GPIO_NUM_17);
    gpio_reset_pin(GPIO_NUM_18);
    gpio_reset_pin(GPIO_NUM_19);
    gpio_reset_pin(GPIO_NUM_34);
    gpio_reset_pin(GPIO_NUM_36);
    gpio_reset_pin(GPIO_NUM_39);
    // gpio_reset_pin(GPIO_NUM_33);
    gpio_reset_pin(GPIO_NUM_32);
    gpio_reset_pin(GPIO_NUM_25);
    gpio_reset_pin(GPIO_NUM_26);
    gpio_reset_pin(GPIO_NUM_27);

    adc_power_off();
    delay(10);
    esp_deep_sleep_start();
}

void ProcessInterrupts()
{
    if (alarminterrupttriggered)
    {
        EinkPrintAll();

        alarminterrupttriggered = false;
    }

    if (touch1interrupttriggered)
    {
        sleep();
        touch1interrupttriggered = false;
    }

    if (touch2interrupttriggered)
    {
        ConnectWifi(8);

        SyncRTCTime();
        GetWeatherData();
        PrintSettings(7);
        touch2interrupttriggered = false;
    }

    if (touch3interrupttriggered)
    {
        // EnableBluetooth();

        // sleep(30);
        // DisableBluetooth();
        touch3interrupttriggered = false;
    }
}
