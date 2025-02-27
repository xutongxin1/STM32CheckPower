
#include <stdint.h>
#include "ssd1306.h"

#define IIC_SCK_PIN 5
#define IIC_SDA_PIN 6

#define OLED_COLUMN_NUMBER 128
#define OLED_LINE_NUMBER 64
#define OLED_PAGE_NUMBER (OLED_LINE_NUMBER/8)
#define OLED_COLUMN_OFFSET 0
#define OLED_LINE_OFFSET 0
#define OLED_CMD_NUMBER 25
/**********SPI引脚分配，连接oled屏，更具实际情况修改*********/

#define IIC_SCK_0  GPIOB->BRR=0X0040       // 设置sck接口到PB5   清零
#define IIC_SCK_1  GPIOB->BSRR=0X0040       //置位
#define IIC_SDA_0  GPIOB->BRR=0X0080       // 设置SDA接口到PB6
#define IIC_SDA_1  GPIOB->BSRR=0X0080
#define READ_SDA     (GPIOB->IDR & 0X0040)
const unsigned char *point;
unsigned char ACK = 0;
static SSD1306_t SSD1306;
// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];
const unsigned char OLED_init_cmd[OLED_CMD_NUMBER] =
    {

        0xAE,//关闭显示

        0xD5,//设置时钟分频因子,震荡频率
        0x80,  //[3:0],分频因子;[7:4],震荡频率

        0xA8,//设置驱动路数
        0X3F,//默认(1/64)

        0xD3,//设置显示偏移
        0X00,//默认为0

        0x40,//设置显示开始行 [5:0],行数.

        0x8D,//电荷泵设置
        0x10,//bit2，开启/关闭
        0x20,//设置内存地址模式
        0x02,//[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
        0xA1,//段重定义设置,bit0:0,0->0;1,0->127;  A1

        0xC8,//设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数 (C0 翻转显示) C8

        0xDA,//设置COM硬件引脚配置
        0x12,//[5:4]配置

        0x81,//对比度设置
        0xf0,//1~255;默认0X7F (亮度设置,越大越亮)

        0xD9,//设置预充电周期
        0x71,//[3:0],PHASE 1;[7:4],PHASE 2;

        0xDB,//设置VCOMH 电压倍率
        0x00,//[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

        0xA4,//全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)

        0xA6,//设置显示方式;bit0:1,反相显示;0,正常显示

        0xAF,//开启显示
    };

const unsigned char picture_tab[] = {
//图片显示，列行式，低位在前，阴码
    0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x20, 0x44, 0x08, 0x00, 0x18, 0x48, 0x28, 0xC8, 0x08, 0x28, 0x48, 0x18, 0x00,
    0x40, 0x40, 0xFC, 0x40, 0x40, 0xFC, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xFC, 0x00, 0x40, 0x40, 0xA0,
    0x90, 0x88, 0x84, 0x88, 0x90, 0x20, 0x40, 0x40, 0x00, 0x00, 0x40, 0x44, 0xD8, 0x20, 0xF0, 0xAC,
    0xA8, 0xE8, 0xB8, 0xA8, 0xE0, 0x00, 0x00, 0x00, 0xC0, 0x7C, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54,
    0x7C, 0x40, 0x40, 0x00, 0x00, 0xF0, 0x90, 0x90, 0x90, 0xFC, 0x90, 0x90, 0x90, 0xF0, 0x00, 0x00,
    0x00, 0x80, 0x88, 0x88, 0x88, 0x88, 0x88, 0xE8, 0xA8, 0x98, 0x8C, 0x88, 0x80, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x3E, 0x01, 0x10, 0x11, 0x09, 0x05, 0x3F, 0x05, 0x09, 0x11, 0x11, 0x00,
    0x08, 0x18, 0x0F, 0x24, 0x14, 0x0F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x3F, 0x00, 0x20, 0x22, 0x2A,
    0x32, 0x22, 0x3F, 0x22, 0x32, 0x2A, 0x22, 0x20, 0x00, 0x00, 0x20, 0x10, 0x0F, 0x10, 0x28, 0x24,
    0x23, 0x20, 0x2F, 0x28, 0x2A, 0x2C, 0x00, 0x30, 0x0F, 0x04, 0x3D, 0x25, 0x15, 0x15, 0x0D, 0x15,
    0x2D, 0x24, 0x24, 0x00, 0x00, 0x07, 0x04, 0x04, 0x04, 0x1F, 0x24, 0x24, 0x24, 0x27, 0x20, 0x38,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF,
    0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x81, 0x41, 0x21, 0x21, 0x61, 0x01, 0x01, 0x21, 0xE1, 0xE1, 0x01, 0xE1, 0xE1,
    0x21, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x21, 0xE1, 0x21, 0x21, 0x21, 0x61, 0x01, 0x01,
    0x21, 0x21, 0xE1, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xC1, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x21, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0F, 0x10, 0x20, 0x24, 0x1C, 0x04, 0x00, 0x20, 0x3F, 0x01, 0x3E, 0x01, 0x3F,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x3F, 0x22, 0x22, 0x27, 0x30, 0x00, 0x00,
    0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, 0x1E, 0x25, 0x25, 0x25, 0x16, 0x00, 0x00, 0x1E, 0x21,
    0x21, 0x21, 0x13, 0x00, 0x01, 0x01, 0x1F, 0x21, 0x21, 0x00, 0x00, 0x00, 0x21, 0x3F, 0x22, 0x21,
    0x01, 0x00, 0x00, 0x1E, 0x21, 0x21, 0x21, 0x1E, 0x00, 0x21, 0x3F, 0x22, 0x01, 0x01, 0x3E, 0x20,
    0x00, 0x21, 0x21, 0x3F, 0x20, 0x20, 0x00, 0x00, 0x1E, 0x21, 0x21, 0x21, 0x13, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xF0, 0x08, 0x04, 0x04, 0x04, 0x0C, 0x00, 0xF0, 0x08, 0x04, 0x04, 0x08, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFC, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0C, 0x04, 0xFC, 0x04, 0x0C, 0x00, 0x04, 0xFC, 0x04, 0x04, 0x08, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x81, 0x82, 0x84, 0x84, 0x84, 0x82, 0x80, 0x81, 0x82, 0x84, 0x84, 0x82, 0x81,
    0x80, 0x80, 0x86, 0x86, 0x80, 0x80, 0x80, 0x80, 0x80, 0x85, 0x83, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x84, 0x87, 0x84, 0x84,
    0x84, 0x86, 0x80, 0x80, 0x80, 0x84, 0x87, 0x84, 0x80, 0x80, 0x84, 0x87, 0x84, 0x84, 0x82, 0x81,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF

};

void delay_us(uint32_t us)
{
    uint32_t count = us * 9;  // 72M主频下大约9个指令周期是1微秒
    while (count--) {
        __asm("nop");  // 每个 NOP 指令大约消耗1个指令周期
    }
}

void delay_ms(unsigned int _ms_time)
{
    HAL_Delay(_ms_time);
}

/**************************IIC模块发送函数************************************************

 *************************************************************************/
//写入  最后将SDA拉高，以等待从设备产生应答
void IIC_write(unsigned char date)
{
    unsigned char i, temp;
    temp = date;

    for (i = 0; i < 8; i++) {
        IIC_SCK_0;

        if ((temp & 0x80) == 0)
            IIC_SDA_0;
        else
            IIC_SDA_1;
        temp = temp << 1;
        delay_us(1);
        IIC_SCK_1;
        delay_us(1);

    }
    IIC_SCK_0;
    delay_us(1);
    IIC_SDA_1;
    delay_us(1);
    IIC_SCK_1;
//								不需要应答
//	if (READ_SDA==0)
//		ACK = 1;
//	else ACK =0;
    delay_us(1);
    IIC_SCK_0;
    delay_us(1);


}

//启动信号
//SCL在高电平期间，SDA由高电平向低电平的变化定义为启动信号
void IIC_start()
{
    IIC_SDA_1;
    delay_us(1);
    IIC_SCK_1;
    delay_us(1);                   //所有操作结束释放SCL
    IIC_SDA_0;
    delay_us(3);
    IIC_SCK_0;

    IIC_write(0x78);

}

//停止信号
//SCL在高电平期间，SDA由低电平向高电平的变化定义为停止信号
void IIC_stop()
{
    IIC_SDA_0;
    delay_us(1);
    IIC_SCK_1;
    delay_us(3);
    IIC_SDA_1;

}

void OLED_send_cmd(unsigned char o_command)
{

    IIC_start();
    IIC_write(0x00);
    IIC_write(o_command);
    IIC_stop();

}

void OLED_send_data(unsigned char o_data)
{
    IIC_start();
    IIC_write(0x40);
    IIC_write(o_data);
    IIC_stop();
}

void Column_set(unsigned char column)
{
    column = column + OLED_COLUMN_OFFSET;
    OLED_send_cmd(0x10 | (column >> 4));    //设置列地址高位
    OLED_send_cmd(0x00 | (column & 0x0f));   //设置列地址低位

}

void Page_set(unsigned char page)
{
    page = page + (OLED_LINE_OFFSET / 8);
    OLED_send_cmd(0xb0 + page);
}

void OLED_clear(void)
{
    unsigned char page, column;
    for (page = 0; page < OLED_PAGE_NUMBER; page++)             //page loop
    {
        Page_set(page);
        Column_set(0);
        for (column = 0; column < OLED_COLUMN_NUMBER; column++)    //column loop
        {
            OLED_send_data(0x00);
        }
    }
}

void OLED_full(void)
{
    unsigned char page, column;
    for (page = 0; page < OLED_PAGE_NUMBER; page++)             //page loop
    {
        Page_set(page);
        Column_set(0);
        for (column = 0; column < OLED_COLUMN_NUMBER; column++)    //column loop
        {
            OLED_send_data(0x00);
        }
    }
}

void OLED_init(void)
{
    unsigned char i;
    for (i = 0; i < OLED_CMD_NUMBER; i++) {
        OLED_send_cmd(OLED_init_cmd[i]);
    }
}

__attribute__((unused)) void Picture_display(const unsigned char *ptr_pic)
{
    unsigned char page, column;
    for (page = 0; page < OLED_PAGE_NUMBER; page++)        //page loop
    {
        Page_set(page);
        Column_set(0);
        for (column = 0; column < OLED_COLUMN_NUMBER; column++)    //column loop
        {
            OLED_send_data(*ptr_pic++);
        }
    }
}

__attribute__((unused)) void Picture_ReverseDisplay(const unsigned char *ptr_pic)
{
    unsigned char page, column, data;
    for (page = 0; page < OLED_PAGE_NUMBER; page++)        //page loop
    {
        Page_set(page);
        Column_set(0);
        for (column = 0; column < OLED_COLUMN_NUMBER; column++)    //column loop
        {
            data = *ptr_pic++;
            data = ~data;
            OLED_send_data(data);
        }
    }
}
char ssd1306_WriteString(char* str, SSD1306_Font_t Font, SSD1306_COLOR color) {
    while (*str) {
        if (ssd1306_WriteChar(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }
        str++;
    }

    // Everything ok
    return *str;
}
/*
 * Draw 1 char to the screen buffer
 * ch       => char om weg te schrijven
 * Font     => Font waarmee we gaan schrijven
 * color    => Black or White
 */
char ssd1306_WriteChar(char ch, SSD1306_Font_t Font, SSD1306_COLOR color) {
    uint32_t i, b, j;

    // Check if character is valid
    if (ch < 32 || ch > 126)
        return 0;

    // Check remaining space on current line
    if (SSD1306_WIDTH < (SSD1306.CurrentX + Font.width) ||
        SSD1306_HEIGHT < (SSD1306.CurrentY + Font.height))
    {
        // Not enough space on current line
        return 0;
    }

    // Use the font to write
    for(i = 0; i < Font.height; i++) {
        b = Font.data[(ch - 32) * Font.height + i];
        for(j = 0; j < Font.width; j++) {
            if((b << j) & 0x8000)  {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            } else {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    SSD1306.CurrentX += Font.char_width ? Font.char_width[ch - 32] : Font.width;

    // Return written char for validation
    return ch;
}
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
    if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        // Don't write outside the buffer
        return;
    }

    // Draw in the right color
    if(color == White) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

void ssd1306_SetCursor(uint8_t x, uint8_t y) {
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

void ssd1306_UpdateScreen(void)
{
    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    //
    //  * 32px   ==  4 pages
    //  * 64px   ==  8 pages
    //  * 128px  ==  16 pages
    for(uint8_t i = 0; i < SSD1306_HEIGHT/8; i++) {
        OLED_send_cmd(0xB0 + i); // Set the current RAM page address.
        OLED_send_cmd(0x00 + SSD1306_X_OFFSET_LOWER);
        OLED_send_cmd(0x10 + SSD1306_X_OFFSET_UPPER);
        for (int j = 0; j < SSD1306_WIDTH; ++j) {
            OLED_send_data(SSD1306_Buffer[SSD1306_WIDTH*i+j]);

        }
    }
}

/* Draw circle by Bresenhem's algorithm */
void ssd1306_DrawCircle(uint8_t par_x,uint8_t par_y,uint8_t par_r,SSD1306_COLOR par_color) {
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t err = 2 - 2 * par_r;
    int32_t e2;

    if (par_x >= SSD1306_WIDTH || par_y >= SSD1306_HEIGHT) {
        return;
    }

    do {
        ssd1306_DrawPixel(par_x - x, par_y + y, par_color);
        ssd1306_DrawPixel(par_x + x, par_y + y, par_color);
        ssd1306_DrawPixel(par_x + x, par_y - y, par_color);
        ssd1306_DrawPixel(par_x - x, par_y - y, par_color);
        e2 = err;

        if (e2 <= y) {
            y++;
            err = err + (y * 2 + 1);
            if(-x == y && e2 <= x) {
                e2 = 0;
            }
        }

        if (e2 > x) {
            x++;
            err = err + (x * 2 + 1);
        }
    } while (x <= 0);

    return;
}
/* Draw filled circle. Pixel positions calculated using Bresenham's algorithm */
void ssd1306_FillCircle(uint8_t par_x,uint8_t par_y,uint8_t par_r,SSD1306_COLOR par_color) {
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t err = 2 - 2 * par_r;
    int32_t e2;

    if (par_x >= SSD1306_WIDTH || par_y >= SSD1306_HEIGHT) {
        return;
    }

    do {
        for (uint8_t _y = (par_y + y); _y >= (par_y - y); _y--) {
            for (uint8_t _x = (par_x - x); _x >= (par_x + x); _x--) {
                ssd1306_DrawPixel(_x, _y, par_color);
            }
        }

        e2 = err;
        if (e2 <= y) {
            y++;
            err = err + (y * 2 + 1);
            if (-x == y && e2 <= x) {
                e2 = 0;
            }
        }

        if (e2 > x) {
            x++;
            err = err + (x * 2 + 1);
        }
    } while (x <= 0);

    return;
}
//初始化显示所有中文
void oled_ShownChinese(char x,char y,char *p){
    //电压
    OLED_send_cmd(0x20); // Set the current RAM page address.
    OLED_send_cmd(0x00);
    OLED_send_cmd(0x21);
    OLED_send_cmd(x);
    OLED_send_cmd(x+0x0f);
    OLED_send_cmd(0x22);
    OLED_send_cmd(y);
    OLED_send_cmd(y+0x01);
    for (int i = 0; i < 31; ++i) {
        OLED_send_data(p[i]);
    }
}

void oled_ShownEnglish(char x,char y,char *p){
    //电压
    OLED_send_cmd(0x20); // Set the current RAM page address.
    OLED_send_cmd(0x00);
    OLED_send_cmd(0x21);
    OLED_send_cmd(x);
    OLED_send_cmd(x+0x07);
    OLED_send_cmd(0x22);
    OLED_send_cmd(y);
    OLED_send_cmd(y+0x01);
    for (int i = 0; i < 15; ++i) {
        OLED_send_data(p[i]);
    }
}
void oled_write_string(char x, char y, char *p, uint8_t len)
{

    uint8_t str_x = x;
    for(uint8_t i = 0; i < len; i++)
    {

        uint8_t place = p[i];
        oled_write_str(str_x, y, place);
        str_x = str_x + 0x08;

    }

}
void oled_write_str(char x, char y, uint8_t chr)         //
{

    chr = chr - ' ';
    OLED_send_cmd(0x20);
    OLED_send_cmd(0x00);
    OLED_send_cmd(0x21);
    OLED_send_cmd(x);
    OLED_send_cmd(x+0x07);
    OLED_send_cmd(0x22);
    OLED_send_cmd(y);
    OLED_send_cmd(y+0x01);

    for(int i = 0; i < 16; i++)
    {

        OLED_send_data(asc2_1608[chr][i]);

    }

}
void oled_write_str_Over(char x, char y, uint8_t chr)         //
{

    chr = chr - ' ';
    OLED_send_cmd(0x20);
    OLED_send_cmd(0x00);
    OLED_send_cmd(0x21);
    OLED_send_cmd(x);
    OLED_send_cmd(x+0x07);
    OLED_send_cmd(0x22);
    OLED_send_cmd(y);
    OLED_send_cmd(y+0x01);

    for(int i = 0; i < 16; i++)
    {

        OLED_send_data(0xff-asc2_1608[chr][i]);

    }

}
void oled_write_string_Over(char x, char y, char *p, uint8_t len)
{

    uint8_t str_x = x;
    for(uint8_t i = 0; i < len; i++)
    {

        uint8_t place = p[i];
        oled_write_str_Over(str_x, y, place);
        str_x = str_x + 0x08;

    }

}