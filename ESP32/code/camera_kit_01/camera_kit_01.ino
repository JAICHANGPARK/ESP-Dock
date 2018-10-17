#define PIC_PKT_LEN    128        //data length of each read, dont set this too big because ram is limited
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define CAM_SERIAL     softSerial

#define PIC_FMT        PIC_FMT_VGA

const byte cameraAddr = (CAM_ADDR << 5);  // addr
const int buttonPin = A5;                 // the number of the pushbutton pin
unsigned long picTotalLen = 0;            // picture length
int picNameNum = 0;

void clearRxBuf()
{
    while (Serial.available())
    {
        Serial.read();
    }
}

/*********************************************************************/
void sendCmd(char cmd[], int cmd_len)
{
  for (char i = 0; i < cmd_len; i++) {
    Serial.print(cmd[i]);
  }
}

/*********************************************************************/
void kit_init()
{
  char cmd[] = {0xaa, 0x0d | cameraAddr, 0x00, 0x00, 0x00, 0x00} ;
  unsigned char resp[6];

  Serial.setTimeout(500);
  while (1)
  {
    //clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial.readBytes((char *)resp, 6) != 6)
    {
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0)
    {
      if (Serial.readBytes((char *)resp, 6) != 6) {
        continue;
      }
      if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0) {
        break;
      }
    }
  }
  cmd[1] = 0x0e | cameraAddr;
  cmd[2] = 0x0d;
  sendCmd(cmd, 6);
  Serial.println("\nCamera initialization done.");
}

/*********************************************************************/
void preCapture()
{
  char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };
  unsigned char resp[6];

  Serial.setTimeout(100);
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial.readBytes((char *)resp, 6) != 6) {
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) {
      break;
    }
  }
}

void Capture()
{
  char cmd[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN >> 8) & 0xff , 0};
  unsigned char resp[6];

  Serial.setTimeout(100);
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial.readBytes((char *)resp, 6) != 6) {
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0) {
      break;
    }
  }
  cmd[1] = 0x05 | cameraAddr;
  cmd[2] = 0;
  cmd[3] = 0;
  cmd[4] = 0;
  cmd[5] = 0;
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial.readBytes((char *)resp, 6) != 6) {
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0) {
      break;
    }
  }
  cmd[1] = 0x04 | cameraAddr;
  cmd[2] = 0x1;
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (Serial.readBytes((char *)resp, 6) != 6) {
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
    {
      Serial.setTimeout(1000);
      if (Serial.readBytes((char *)resp, 6) != 6)
      {
        continue;
      }
      if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
      {
        picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16);
        Serial.print("picTotalLen:");
        Serial.println(picTotalLen);
        break;
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:

  kit_init();
  preCapture();
  Capture();
  

}

void loop() {
  // put your main code here, to run repeatedly:

}
