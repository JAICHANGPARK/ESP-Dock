#include <SPI.h>
#include <MFRC522.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SS_PIN 12
#define RST_PIN 13

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

BLEServer *pServer = NULL;
BLEService *pService ; // Create the BLE Service
BLECharacteristic * pTxCharacteristic ; // Create a BLE Characteristic
BLECharacteristic * pRxCharacteristic ; // Create a BLE Characteristic
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

uint8_t nuidPICC[4] = {0x00,}; // Init array that will store new NUID
char inByte;
boolean sendFlag = false;


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();
        Serial.println("*********");
      }
    }
};




void setup() {

  Serial.begin(9600);

  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  BLEDevice::init("FITNESS_ADMIN");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());


  pService = pServer->createService(SERVICE_UUID); // Create the BLE Service
  pTxCharacteristic = pService -> createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY); // Create a BLE Characteristic
  pTxCharacteristic -> addDescriptor(new BLE2902());
  pRxCharacteristic = pService -> createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE); // Create a BLE Characteristic
  pRxCharacteristic -> setCallbacks(new MyCallbacks());


  pService->start();  // Start the service
  pServer->getAdvertising()->start();  // Start advertising

}

void loop() {

  if (deviceConnected) {
    pTxCharacteristic->setValue(&txValue, 1);
    pTxCharacteristic->notify();
    txValue++;
    delay(10); // bluetooth stack will go into congestion, if too many packets are sent
  }

  if (!deviceConnected && oldDeviceConnected) { // disconnecting
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) { // connecting
    oldDeviceConnected = deviceConnected;  // do stuff here on connecting
  }

  while (Serial.available()) {
    // get incoming byte:
    inByte = Serial.read();
    // read first analog input, divide by 4 to make the range 0-255:
    Serial.write(inByte);
    if (inByte == 'r') {
      //      printHex(rfid.uid.uidByte, rfid.uid.size); // 1차 시행 : 초기에 태그가 찍히지 않으면 리턴 값 없음
      std::string  msg = "";
      for (byte i = 0; i < 4; i++) {
        Serial.print(nuidPICC[i], HEX);
        msg += nuidPICC[i];
      }
      Serial.println("");
      //      pRxCharacteristic->setValue("Hello World");
      pRxCharacteristic -> setValue(nuidPICC, 4);
    }
  }

  if (sendFlag) {
    sendFlag = false;
  }

  Serial.println('x');
  delay(1000);


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
  else {
    Serial.println(F("Card read previously."));
  }

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();



}


/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
   Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
