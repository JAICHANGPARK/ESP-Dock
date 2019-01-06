
#include <SPI.h>
#include <MFRC522.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SS_PIN 12
#define RST_PIN 13

#define SERVICE_UUID                        "0000ffe0-0000-1000-8000-00805f9b34fb" // UART service UUID
#define CHARACTERISTIC_UUID_RX              "0000ffe1-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID_TX              "0000ffe2-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID_REALTIME        "0000ffe3-0000-1000-8000-00805f9b34fb"


MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
BLECharacteristic *pRealTimeCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;
bool readRfidFlag = false; // 테그 정보 요청이 들어왔을 때 처리하는 플래그
bool deleteRfidFlag = false;
uint8_t txValue = 0;

// Init array that will store new NUID
byte nuidPICC[4];

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      digitalWrite(5, false);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      digitalWrite(5, true);

    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      uint8_t tmp[rxValue.length()];
      Serial.print("콜백으로 들어온 데이터 길이 : ");  Serial.println(rxValue.length());
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
          tmp[i] = rxValue[i];
        }
        Serial.println();
        Serial.println("*********");

        if (rxValue[0] == 0x02 && rxValue[1]  == 0x07 && rxValue[2] == 0x70 && rxValue[3] == 0x03) { // 실시간 전송 첫번째 인증과정
          readRfidFlag = true;
        }
        else if (rxValue[0] == 0x02 && rxValue[1]  == 0x70 && rxValue[2] == 0x07 && rxValue[3] == 0x03) deleteRfidFlag = true;
      }
    }
};
void settingBleProfile() {
  BLEDevice::init("KNU_RFID");
  pServer = BLEDevice::createServer();  // Create the BLE Server
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID); // Create the BLE Service
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);  // Create a BLE Characteristic
  pTxCharacteristic->addDescriptor(new BLE2902());
  pRealTimeCharacteristic = pService -> createCharacteristic(CHARACTERISTIC_UUID_REALTIME, BLECharacteristic::PROPERTY_NOTIFY);
  pRealTimeCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();// Start the service
  pServer->getAdvertising()->start();// Start advertising
  Serial.println("Waiting a client connection to notify...");

}

void setup() {


  settingBleProfile();

  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

}

void loop() {

  if (deviceConnected) { // BLE 연결이 되어 있을때
    if (readRfidFlag) { // 테그 정보 전송하기
      uint8_t firstBuffer[4] = {0x00, 0x00, 0x00, 0x00};

      for (byte i = 0; i < 4; i++) {
        firstBuffer[i] = (uint8_t)nuidPICC[i];
      }
      
      pRealTimeCharacteristic->setValue(firstBuffer, 4);
      pRealTimeCharacteristic->notify();
      readRfidFlag = false;
      
    }

    if (deleteRfidFlag) { // 테그 정보 삭제
//      uint8_t firstBuffer[3] = {0x02, 0x10, 0x03};
//      pRealTimeCharacteristic->setValue(firstBuffer, 3);
//      pRealTimeCharacteristic->notify();

      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = 0x00;
      }
      
      deleteRfidFlag = false;
    }
  }
  
  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
      rfid.uid.uidByte[1] != nuidPICC[1] ||
      rfid.uid.uidByte[2] != nuidPICC[2] ||
      rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }


}


/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
   Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
