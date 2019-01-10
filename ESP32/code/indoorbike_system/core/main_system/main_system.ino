#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define HEART_RATE_SERVICE_UUID             "0000180d-0000-1000-8000-00805f9b34fb" // Heart Rate service UUID
#define CHARACTERISTIC_HEART_RATE           "00002a37-0000-1000-8000-00805f9b34fb"

#define FITNESS_MACHINE_SERVICE_UUID        "00001826-0000-1000-8000-00805f9b34fb" // Fitness Machine service UUID
#define CHARACTERISTIC_INDOOR_BIKE          "00002ad2-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_TREADMILL            "00002acd-0000-1000-8000-00805f9b34fb"

#define DATETIME_SERVICE_UUID               "0000aaa0-0000-1000-8000-00805f9b34fb" // DateTime service UUID
#define CHARACTERISTIC_DATE_TIME_SYNC       "00000001-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_DATE_TIME            "00000002-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_RESULT_CHAR          "0000fff0-0000-1000-8000-00805f9b34fb"

#define AUTH_SERVICE_UUID                   "0000eee0-0000-1000-8000-00805f9b34fb" // Device Auth service UUID
#define CHARACTERISTIC_AUTH                 "0000eee1-0000-1000-8000-00805f9b34fb"

#define DATA_SYNC_SERVICE_UUID              "0000fff0-0000-1000-8000-00805f9b34fb" // DataSync service UUID
#define CHARACTERISTIC_CONTROL              "0000fff1-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_SYNC                 "0000fff2-0000-1000-8000-00805f9b34fb"

//#define CHARACTERISTIC_UUID_REALTIME        "0000ffe3-0000-1000-8000-00805f9b34fb"

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
