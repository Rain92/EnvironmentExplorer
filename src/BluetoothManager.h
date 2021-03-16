#pragma once

#include <BLEDevice.h>
#include "Settings.h"

BLEServer* pServer = NULL;
BLEService* pService = NULL;
BLECharacteristic* pCharacteristic1 = NULL;
BLECharacteristic* pCharacteristic2 = NULL;
BLECharacteristic* pCharacteristic3 = NULL;
BLECharacteristic* pCharacteristic4 = NULL;

bool bluetoothon = false;
bool deviceConnected = false;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID1 "beb5483e-36e1-4688-b7f5-ea07361b26a6"
#define CHARACTERISTIC_UUID2 "beb5483e-36e1-4688-b7f5-ea07361b26a7"
#define CHARACTERISTIC_UUID3 "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID4 "beb5483e-36e1-4688-b7f5-ea07361b26a9"

class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer)
    {
        deviceConnected = true;
        BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) { deviceConnected = false; }
};

class CharacteristicCallbacks : public BLECharacteristicCallbacks
{
    char* buffer;

    void onWrite(BLECharacteristic* pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0)
        {
            Serial.println("*********");
            Serial.print("Received Value: ");
            int i;
            for (i = 0; i < rxValue.length(); i++)
            {
                Serial.print(rxValue[i]);
                buffer[i] = rxValue[i];
            }
            buffer[i + 1] = 0;
            Serial.println();
            Serial.println("*********");
        }

        SaveSettings();
    }

public:
    CharacteristicCallbacks(char* buffer) : BLECharacteristicCallbacks(), buffer(buffer) {}
};

void EnableBluetooth()
{
    if (!bluetoothon)
    {
        BLEDevice::init("EnvironmentExplorer");

        // Create the BLE Server
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new ServerCallbacks());

        // Create the BLE Service
        pService = pServer->createService(SERVICE_UUID);

        // Create a BLE Characteristic
        pCharacteristic1 = pService->createCharacteristic(
            CHARACTERISTIC_UUID1, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
        pCharacteristic2 = pService->createCharacteristic(
            CHARACTERISTIC_UUID2, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
        pCharacteristic3 = pService->createCharacteristic(
            CHARACTERISTIC_UUID3, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
        pCharacteristic4 = pService->createCharacteristic(
            CHARACTERISTIC_UUID4, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

        pCharacteristic1->setCallbacks(new CharacteristicCallbacks(settings.wificonfig.ssid));
        pCharacteristic2->setCallbacks(new CharacteristicCallbacks(settings.wificonfig.password));
        pCharacteristic3->setCallbacks(new CharacteristicCallbacks(settings.location));
        pCharacteristic4->setCallbacks(new CharacteristicCallbacks(settings.updateinterval));

        // Start the service
        pService->start();

        // Start advertising
        BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(false);
        pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
        BLEDevice::startAdvertising();
        Serial.println("Waiting a client connection to notify...");

        pCharacteristic1->setValue((uint8_t*)settings.wificonfig.ssid, strlen(settings.wificonfig.ssid));
        pCharacteristic2->setValue((uint8_t*)settings.wificonfig.password, strlen(settings.wificonfig.password));
        pCharacteristic3->setValue((uint8_t*)settings.location, strlen(settings.location));
        pCharacteristic4->setValue((uint8_t*)settings.location, strlen(settings.updateinterval));

        bluetoothon = true;
    }
}

void DisableBluetooth()
{
    if (bluetoothon)
    {
        pService->stop();
        pServer->removeService(pService);
        BLEDevice::deinit(true);
        bluetoothon = false;
    }
}