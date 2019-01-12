HardwareSerial _mySerial(0);

const int wake_up = 14;
const int reset_pin = 3;

#define    CMD_SIZE                           512

/*
  frame format
*/
#define    FRAME_B                            0xA5
#define    FRAME_E0                           0xCC
#define    FRAME_E1                           0x33
#define    FRAME_E2                           0xC3
#define    FRAME_E3                           0x3C


/*
  color define
*/
#define    WHITE                              0x03
#define    GRAY                               0x02
#define    DARK_GRAY                          0x01
#define    BLACK                              0x00

/*
  command define
*/
#define    CMD_HANDSHAKE                      0x00                                                     //handshake
#define    CMD_SET_BAUD                       0x01                                                     //set baud
#define    CMD_READ_BAUD                      0x02                                                     //read baud
#define    CMD_MEMORYMODE                     0x07                                                     //set memory mode
#define    CMD_STOPMODE                       0x08                                                     //enter stop mode 
#define    CMD_UPDATE                         0x0A                                                     //update
#define    CMD_SCREEN_ROTATION                0x0D                                                     //set screen rotation
#define    CMD_LOAD_FONT                      0x0E                                                     //load font
#define    CMD_LOAD_PIC                       0x0F                                                     //load picture

#define    CMD_SET_COLOR                      0x10                                                     //set color
#define    CMD_SET_EN_FONT                    0x1E                                                     //set english font
#define    CMD_SET_CH_FONT                    0x1F                                                     //set chinese font

#define    CMD_DRAW_PIXEL                     0x20                                                     //set pixel
#define    CMD_DRAW_LINE                      0x22                                                     //draw line
#define    CMD_FILL_RECT                      0x24                                                     //fill rectangle
#define    CMD_DRAW_CIRCLE                    0x26                                                     //draw circle
#define    CMD_FILL_CIRCLE                    0x27                                                     //fill circle
#define    CMD_DRAW_TRIANGLE                  0x28                                                     //draw triangle
#define    CMD_FILL_TRIANGLE                  0x29                                                     //fill triangle
#define    CMD_CLEAR                          0x2E                                                     //clear screen use back color

#define    CMD_DRAW_STRING                    0x30                                                     //draw string

#define    CMD_DRAW_BITMAP                    0x70                                                     //draw bitmap


/*
  FONT
*/
#define    GBK32                              0x01
#define    GBK48                              0x02
#define    GBK64                              0x03

#define    ASCII32                            0x01
#define    ASCII48                            0x02
#define    ASCII64                            0x03


/*
  Memory Mode
*/
#define    MEM_NAND                           0
#define    MEM_TF                             1

/*
  set screen rotation
*/
#define    EPD_NORMAL                         0                                                        //screen normal
#define    EPD_INVERSION                      1

/*
  command define
*/
static const unsigned char _cmd_handshake[8] = {0xA5, 0x00, 0x09, CMD_HANDSHAKE, 0xCC, 0x33, 0xC3, 0x3C};        //CMD_HANDSHAKE
static const unsigned char _cmd_read_baud[8] = {0xA5, 0x00, 0x09, CMD_READ_BAUD, 0xCC, 0x33, 0xC3, 0x3C};       //CMD_READ_BAUD
static const unsigned char _cmd_stopmode[8] = {0xA5, 0x00, 0x09, CMD_STOPMODE, 0xCC, 0x33, 0xC3, 0x3C};         //CMD_STOPMODE
static const unsigned char _cmd_update[8] = {0xA5, 0x00, 0x09, CMD_UPDATE, 0xCC, 0x33, 0xC3, 0x3C};           //CMD_UPDATE
static const unsigned char _cmd_load_font[8] = {0xA5, 0x00, 0x09, CMD_LOAD_FONT, 0xCC, 0x33, 0xC3, 0x3C};       //CMD_LOAD_FONT
static const unsigned char _cmd_load_pic[8] = {0xA5, 0x00, 0x09, CMD_LOAD_PIC, 0xCC, 0x33, 0xC3, 0x3C};         //CMD_LOAD_PIC
static unsigned char _cmd_buff[CMD_SIZE];

static void _putchars(const unsigned char * ptr, int n)
{
  int i, x;

  for (i = 0; i < n; i++)
  {
    x = ptr[i];
    _mySerial.write(x);
  }
}

static unsigned char _verify(const void * ptr, int n)
{
  int i;
  unsigned char * p = (unsigned char *)ptr;
  unsigned char result;

  for (i = 0, result = 0; i < n; i++)
  {
    result ^= p[i];
  }

  return result;
}

void epd_init(void)
{

  _mySerial.begin(115200, SERIAL_8N1);
  pinMode(wake_up, HIGH);
  pinMode(reset_pin, HIGH);
}

void epd_reset(void)
{
  digitalWrite(reset_pin, LOW);
  delayMicroseconds(10);
  digitalWrite(reset_pin, HIGH);
  delayMicroseconds(500);
  digitalWrite(reset_pin, LOW);
  delay(3000);
}

void epd_wakeup(void)
{
  digitalWrite(wake_up, LOW);
  delayMicroseconds(10);
  digitalWrite(wake_up, HIGH);
  delayMicroseconds(500);
  digitalWrite(wake_up, LOW);
  delay(10);
}

void epd_handshake(void)
{
  memcpy(_cmd_buff, _cmd_handshake, 8);
  _cmd_buff[8] = _verify(_cmd_buff, 8);

  _putchars(_cmd_buff, 9);
}

void epd_set_baud(long baud)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x0D;

  _cmd_buff[3] = CMD_SET_BAUD;

  _cmd_buff[4] = (baud >> 24) & 0xFF;
  _cmd_buff[5] = (baud >> 16) & 0xFF;
  _cmd_buff[6] = (baud >> 8) & 0xFF;
  _cmd_buff[7] = baud & 0xFF;

  _cmd_buff[8] = FRAME_E0;
  _cmd_buff[9] = FRAME_E1;
  _cmd_buff[10] = FRAME_E2;
  _cmd_buff[11] = FRAME_E3;
  _cmd_buff[12] = _verify(_cmd_buff, 12);

  _putchars(_cmd_buff, 13);

  delay(10);
  _mySerial.begin(baud);
}
/*******************************************************************************
  Function Name  : void epd_read_baud(void)
  Description    : read uart baud
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_read_baud(void)
{
  memcpy(_cmd_buff, _cmd_read_baud, 8);
  _cmd_buff[8] = _verify(_cmd_buff, 8);

  _putchars(_cmd_buff, 9);
}

void epd_set_memory()
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x0A;

  _cmd_buff[3] = CMD_MEMORYMODE;

  _cmd_buff[4] = MEM_NAND;

  _cmd_buff[5] = FRAME_E0;
  _cmd_buff[6] = FRAME_E1;
  _cmd_buff[7] = FRAME_E2;
  _cmd_buff[8] = FRAME_E3;
  _cmd_buff[9] = _verify(_cmd_buff, 9);

  _putchars(_cmd_buff, 10);
}

void epd_enter_stopmode(void)
{
  memcpy(_cmd_buff, _cmd_stopmode, 8);
  _cmd_buff[8] = _verify(_cmd_buff, 8);

  _putchars(_cmd_buff, 9);
}

void epd_udpate(void)
{
  memcpy(_cmd_buff, _cmd_update, 8);
  _cmd_buff[8] = _verify(_cmd_buff, 8);

  _putchars(_cmd_buff, 9);
}

void epd_screen_rotation(unsigned char mode)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x0A;

  _cmd_buff[3] = CMD_SCREEN_ROTATION;

  _cmd_buff[4] = mode;

  _cmd_buff[5] = FRAME_E0;
  _cmd_buff[6] = FRAME_E1;
  _cmd_buff[7] = FRAME_E2;
  _cmd_buff[8] = FRAME_E3;
  _cmd_buff[9] = _verify(_cmd_buff, 9);

  _putchars(_cmd_buff, 10);
}

void epd_load_font(void)
{
  memcpy(_cmd_buff, _cmd_load_font, 8);
  _cmd_buff[8] = _verify(_cmd_buff, 8);

  _putchars(_cmd_buff, 9);
}

void epd_load_pic(void)
{
  memcpy(_cmd_buff, _cmd_load_pic, 8);
  _cmd_buff[8] = _verify(_cmd_buff, 8);

  _putchars(_cmd_buff, 9);
}
/*******************************************************************************
  Function Name  : void epd_set_color(unsigned char color, unsigned char bkcolor)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_set_color(unsigned char color, unsigned char bkcolor)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x0B;

  _cmd_buff[3] = CMD_SET_COLOR;

  _cmd_buff[4] = color;
  _cmd_buff[5] = bkcolor;

  _cmd_buff[6] = FRAME_E0;
  _cmd_buff[7] = FRAME_E1;
  _cmd_buff[8] = FRAME_E2;
  _cmd_buff[9] = FRAME_E3;
  _cmd_buff[10] = _verify(_cmd_buff, 10);

  _putchars(_cmd_buff, 11);
}
/*******************************************************************************
  Function Name  : void epd_set_en_font(unsigned char font)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_set_en_font(unsigned char font)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x0A;

  _cmd_buff[3] = CMD_SET_EN_FONT;

  _cmd_buff[4] = font;

  _cmd_buff[5] = FRAME_E0;
  _cmd_buff[6] = FRAME_E1;
  _cmd_buff[7] = FRAME_E2;
  _cmd_buff[8] = FRAME_E3;
  _cmd_buff[9] = _verify(_cmd_buff, 9);

  _putchars(_cmd_buff, 10);
}
/*******************************************************************************
  Function Name  : void epd_set_ch_font(unsigned char font)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_set_ch_font(unsigned char font)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x0A;

  _cmd_buff[3] = CMD_SET_CH_FONT;

  _cmd_buff[4] = font;

  _cmd_buff[5] = FRAME_E0;
  _cmd_buff[6] = FRAME_E1;
  _cmd_buff[7] = FRAME_E2;
  _cmd_buff[8] = FRAME_E3;
  _cmd_buff[9] = _verify(_cmd_buff, 9);

  _putchars(_cmd_buff, 10);
}
/*******************************************************************************
  Function Name  : void epd_draw_pixel(int x0, int y0)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_draw_pixel(int x0, int y0)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x0D;

  _cmd_buff[3] = CMD_DRAW_PIXEL;

  _cmd_buff[4] = (x0 >> 8) & 0xFF;
  _cmd_buff[5] = x0 & 0xFF;
  _cmd_buff[6] = (y0 >> 8) & 0xFF;
  _cmd_buff[7] = y0 & 0xFF;

  _cmd_buff[8] = FRAME_E0;
  _cmd_buff[9] = FRAME_E1;
  _cmd_buff[10] = FRAME_E2;
  _cmd_buff[11] = FRAME_E3;
  _cmd_buff[12] = _verify(_cmd_buff, 12);

  _putchars(_cmd_buff, 13);
}
/*******************************************************************************
  Function Name  : void epd_draw_line(int x0, int y0, int x1, int y1)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_draw_line(int x0, int y0, int x1, int y1)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x11;

  _cmd_buff[3] = CMD_DRAW_LINE;

  _cmd_buff[4] = (x0 >> 8) & 0xFF;
  _cmd_buff[5] = x0 & 0xFF;
  _cmd_buff[6] = (y0 >> 8) & 0xFF;
  _cmd_buff[7] = y0 & 0xFF;
  _cmd_buff[8] = (x1 >> 8) & 0xFF;
  _cmd_buff[9] = x1 & 0xFF;
  _cmd_buff[10] = (y1 >> 8) & 0xFF;
  _cmd_buff[11] = y1 & 0xFF;

  _cmd_buff[12] = FRAME_E0;
  _cmd_buff[13] = FRAME_E1;
  _cmd_buff[14] = FRAME_E2;
  _cmd_buff[15] = FRAME_E3;
  _cmd_buff[16] = _verify(_cmd_buff, 16);

  _putchars(_cmd_buff, 17);
}
/*******************************************************************************
  Function Name  : void epd_fill_rect(int x0, int y0, int x1, int y1)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_fill_rect(int x0, int y0, int x1, int y1)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x11;

  _cmd_buff[3] = CMD_FILL_RECT;

  _cmd_buff[4] = (x0 >> 8) & 0xFF;
  _cmd_buff[5] = x0 & 0xFF;
  _cmd_buff[6] = (y0 >> 8) & 0xFF;
  _cmd_buff[7] = y0 & 0xFF;
  _cmd_buff[8] = (x1 >> 8) & 0xFF;
  _cmd_buff[9] = x1 & 0xFF;
  _cmd_buff[10] = (y1 >> 8) & 0xFF;
  _cmd_buff[11] = y1 & 0xFF;

  _cmd_buff[12] = FRAME_E0;
  _cmd_buff[13] = FRAME_E1;
  _cmd_buff[14] = FRAME_E2;
  _cmd_buff[15] = FRAME_E3;
  _cmd_buff[16] = _verify(_cmd_buff, 16);

  _putchars(_cmd_buff, 17);
}
/*******************************************************************************
  Function Name  : void epd_draw_circle(int x0, int y0, int r)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_draw_circle(int x0, int y0, int r)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x0F;

  _cmd_buff[3] = CMD_DRAW_CIRCLE;

  _cmd_buff[4] = (x0 >> 8) & 0xFF;
  _cmd_buff[5] = x0 & 0xFF;
  _cmd_buff[6] = (y0 >> 8) & 0xFF;
  _cmd_buff[7] = y0 & 0xFF;
  _cmd_buff[8] = (r >> 8) & 0xFF;
  _cmd_buff[9] = r & 0xFF;


  _cmd_buff[10] = FRAME_E0;
  _cmd_buff[11] = FRAME_E1;
  _cmd_buff[12] = FRAME_E2;
  _cmd_buff[13] = FRAME_E3;
  _cmd_buff[14] = _verify(_cmd_buff, 14);

  _putchars(_cmd_buff, 15);
}
/*******************************************************************************
  Function Name  : void epd_fill_circle(int x0, int y0, int r)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_fill_circle(int x0, int y0, int r)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x0F;

  _cmd_buff[3] = CMD_FILL_CIRCLE;

  _cmd_buff[4] = (x0 >> 8) & 0xFF;
  _cmd_buff[5] = x0 & 0xFF;
  _cmd_buff[6] = (y0 >> 8) & 0xFF;
  _cmd_buff[7] = y0 & 0xFF;
  _cmd_buff[8] = (r >> 8) & 0xFF;
  _cmd_buff[9] = r & 0xFF;


  _cmd_buff[10] = FRAME_E0;
  _cmd_buff[11] = FRAME_E1;
  _cmd_buff[12] = FRAME_E2;
  _cmd_buff[13] = FRAME_E3;
  _cmd_buff[14] = _verify(_cmd_buff, 14);

  _putchars(_cmd_buff, 15);
}
/*******************************************************************************
  Function Name  : void epd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x15;

  _cmd_buff[3] = CMD_DRAW_TRIANGLE;

  _cmd_buff[4] = (x0 >> 8) & 0xFF;
  _cmd_buff[5] = x0 & 0xFF;
  _cmd_buff[6] = (y0 >> 8) & 0xFF;
  _cmd_buff[7] = y0 & 0xFF;
  _cmd_buff[8] = (x1 >> 8) & 0xFF;
  _cmd_buff[9] = x1 & 0xFF;
  _cmd_buff[10] = (y1 >> 8) & 0xFF;
  _cmd_buff[11] = y1 & 0xFF;
  _cmd_buff[12] = (x2 >> 8) & 0xFF;
  _cmd_buff[13] = x2 & 0xFF;
  _cmd_buff[14] = (y2 >> 8) & 0xFF;
  _cmd_buff[15] = y2 & 0xFF;

  _cmd_buff[16] = FRAME_E0;
  _cmd_buff[17] = FRAME_E1;
  _cmd_buff[18] = FRAME_E2;
  _cmd_buff[19] = FRAME_E3;
  _cmd_buff[20] = _verify(_cmd_buff, 20);

  _putchars(_cmd_buff, 21);
}
/*******************************************************************************
  Function Name  : void epd_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x15;

  _cmd_buff[3] = CMD_FILL_TRIANGLE;

  _cmd_buff[4] = (x0 >> 8) & 0xFF;
  _cmd_buff[5] = x0 & 0xFF;
  _cmd_buff[6] = (y0 >> 8) & 0xFF;
  _cmd_buff[7] = y0 & 0xFF;
  _cmd_buff[8] = (x1 >> 8) & 0xFF;
  _cmd_buff[9] = x1 & 0xFF;
  _cmd_buff[10] = (y1 >> 8) & 0xFF;
  _cmd_buff[11] = y1 & 0xFF;
  _cmd_buff[12] = (x2 >> 8) & 0xFF;
  _cmd_buff[13] = x2 & 0xFF;
  _cmd_buff[14] = (y2 >> 8) & 0xFF;
  _cmd_buff[15] = y2 & 0xFF;

  _cmd_buff[16] = FRAME_E0;
  _cmd_buff[17] = FRAME_E1;
  _cmd_buff[18] = FRAME_E2;
  _cmd_buff[19] = FRAME_E3;
  _cmd_buff[20] = _verify(_cmd_buff, 20);

  _putchars(_cmd_buff, 21);
}
/*******************************************************************************
  Function Name  : void epd_clear(void)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_clear(void)
{
  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = 0x00;
  _cmd_buff[2] = 0x09;

  _cmd_buff[3] = CMD_CLEAR;

  _cmd_buff[4] = FRAME_E0;
  _cmd_buff[5] = FRAME_E1;
  _cmd_buff[6] = FRAME_E2;
  _cmd_buff[7] = FRAME_E3;
  _cmd_buff[8] = _verify(_cmd_buff, 8);

  _putchars(_cmd_buff, 9);
}

/*******************************************************************************
  Function Name  : void epd_disp_char(unsigned char ch, int x0, int y0);
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_disp_char(unsigned char ch, int x0, int y0)
{
  unsigned char buff[2];

  buff[0] = ch;
  buff[1] = 0;

  epd_disp_string(buff, x0, y0);
}
/*******************************************************************************
  Function Name  : void epd_disp_string(const void * p, int x0, int y0)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_disp_string(const void * p, int x0, int y0)
{
  int string_size;
  unsigned char * ptr = (unsigned char *)p;


  string_size = strlen((const char *)ptr);
  string_size += 14;

  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = (string_size >> 8) & 0xFF;
  _cmd_buff[2] = string_size & 0xFF;

  _cmd_buff[3] = CMD_DRAW_STRING;

  _cmd_buff[4] = (x0 >> 8) & 0xFF;
  _cmd_buff[5] = x0 & 0xFF;
  _cmd_buff[6] = (y0 >> 8) & 0xFF;
  _cmd_buff[7] = y0 & 0xFF;

  strcpy((char *)(&_cmd_buff[8]), (const char *)ptr);

  string_size -= 5;

  _cmd_buff[string_size] = FRAME_E0;
  _cmd_buff[string_size + 1] = FRAME_E1;
  _cmd_buff[string_size + 2] = FRAME_E2;
  _cmd_buff[string_size + 3] = FRAME_E3;
  _cmd_buff[string_size + 4] = _verify(_cmd_buff, string_size + 4);

  _putchars(_cmd_buff, string_size + 5);
}
/*******************************************************************************
  Function Name  : void epd_disp_bitmap(const void * p, int x0, int y0)
  Description    :
  Input          :
  Output         : None
  Return         :
  Attention      : None
*******************************************************************************/
void epd_disp_bitmap(const void * p, int x0, int y0)
{
  int string_size;
  unsigned char * ptr = (unsigned char *)p;

  string_size = strlen((const char *)ptr);
  string_size += 14;

  _cmd_buff[0] = FRAME_B;

  _cmd_buff[1] = (string_size >> 8) & 0xFF;
  _cmd_buff[2] = string_size & 0xFF;

  _cmd_buff[3] = CMD_DRAW_BITMAP;

  _cmd_buff[4] = (x0 >> 8) & 0xFF;
  _cmd_buff[5] = x0 & 0xFF;
  _cmd_buff[6] = (y0 >> 8) & 0xFF;
  _cmd_buff[7] = y0 & 0xFF;

  strcpy((char *)(&_cmd_buff[8]), (const char *)ptr);

  string_size -= 5;

  _cmd_buff[string_size] = FRAME_E0;
  _cmd_buff[string_size + 1] = FRAME_E1;
  _cmd_buff[string_size + 2] = FRAME_E2;
  _cmd_buff[string_size + 3] = FRAME_E3;
  _cmd_buff[string_size + 4] = _verify(_cmd_buff, string_size + 4);

  _putchars(_cmd_buff, string_size + 5);
}
void draw_text_demo(void)
{
  char buff[] = {'G', 'B', 'K', '3', '2', ':', ' ', 0xc4, 0xe3, 0xba, 0xc3, 0xca, 0xc0, 0xbd, 0xe7, 0};
  epd_set_color(BLACK, WHITE);
  epd_clear();
  epd_set_ch_font(GBK32);
  epd_set_en_font(ASCII32);
  epd_disp_string(buff, 0, 50);
  epd_disp_string("ASCII32: Hello, World!", 0, 300);

  epd_set_ch_font(GBK48);
  epd_set_en_font(ASCII48);
  buff[3] = '4';
  buff[4] = '8';
  epd_disp_string(buff, 0, 100);
  epd_disp_string("ASCII48: Hello, World!", 0, 350);

  epd_set_ch_font(GBK64);
  epd_set_en_font(ASCII64);
  buff[3] = '6';
  buff[4] = '4';
  epd_disp_string(buff, 0, 160);
  epd_disp_string("ASCII64: Hello, World!", 0, 450);


  epd_udpate();
  delay(3000);
}
