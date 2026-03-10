#include <STC15F2K60S2.H>
#include <intrins.h>

unsigned char code smg_code[13] = {
    0xC0, //0
    0xF9, //1
    0xA4, //2
    0xB0, //3
    0x99, //4
    0x92, //5
    0x82, //6
    0xF8, //7
    0x80, //8
    0x90, //9
    0xFF, //10 空白
    0xBF, //11 负号
    0x7F  //12 小数点单独不用
};

unsigned char show_buf[8] = {10,10,10,10,10,10,10,10};
bit is_neg = 0;
unsigned int temperature = 0;

// ===== DS18B20 函数声明 =====
bit init_ds18b20(void);
void Write_DS18B20(unsigned char dat);
unsigned char Read_DS18B20(void);

//==================== 基础硬件 ====================

void System_Init(void)
{
    P2 = 0xA0; P0 = 0x00; P2 = 0x00;   // 关蜂鸣器/继电器
    P2 = 0x80; P0 = 0xFF; P2 = 0x00;   // 关LED
}

void Nixie_scan(void)
{
    static unsigned char pos = 0;
    unsigned char seg;

    // 1. 先关所有位选
    P0 = 0xFF;
    P2 = 0xC0;
    P2 = 0x00;

    // 2. 取段码
    if(show_buf[pos] >= 20)
        seg = smg_code[show_buf[pos] - 20] & 0x7F;   // 带小数点
    else
        seg = smg_code[show_buf[pos]];

    // 3. 送段码
    P0 = seg;
    P2 = 0xE0;
    P2 = 0x00;

    // 4. 开当前位
    P0 = 0x01 << pos;
    P2 = 0xC0;
    P2 = 0x00;

    pos++;
    if(pos >= 8) pos = 0;
}

void Timer0_Init(void)        // 1ms@11.0592MHz, 1T
{
    AUXR |= 0x80;
    TMOD &= 0xF0;
    TL0 = 0xCD;
    TH0 = 0xD4;
    TF0 = 0;
    TR0 = 1;
    ET0 = 1;
    EA = 1;
}

void Timer0_Isr(void) interrupt 1
{
    Nixie_scan();
}

//==================== 简单延时 ====================

void DelayMs(unsigned int ms)
{
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 1100; j++);
}

//==================== DS18B20 读取 ====================

unsigned int Read_DS18B20_temp(void)
{
    unsigned char LSB, MSB;
    signed int raw;
    unsigned int temp;
    unsigned int timeout = 0;

    // 1. 启动转换
    if(init_ds18b20()){
        is_neg = 0;
        return 0;
    }
    Write_DS18B20(0xCC);   // Skip ROM
    Write_DS18B20(0x44);   // Convert T

    // 2. 等待转换完成（外部供电可轮询DQ）
    while(!P1^4){
        timeout++;
        if(timeout > 60000){
            is_neg = 0;
            return 0;
        }
    }

    // 3. 读温度寄存器
    if(init_ds18b20()){
        is_neg = 0;
        return 0;
    }
    Write_DS18B20(0xCC);
    Write_DS18B20(0xBE);

    LSB = Read_DS18B20();
    MSB = Read_DS18B20();

    raw = (signed int)((MSB << 8) | LSB);

    if(raw < 0){
        is_neg = 1;
        raw = -raw;
    }else{
        is_neg = 0;
    }

    // 转成 0.01℃
    temp = (unsigned int)(raw * 625 / 100);

    return temp;
}

//==================== 显示温度 ====================

void Display_show_temp(void)
{
    unsigned char i;

    for(i = 0; i < 8; i++)
        show_buf[i] = 10;

    // 右4位显示，例如 23.45 /  2.89 / -5.12
    if(is_neg)
        show_buf[3] = 11;   // 负号
    else
        show_buf[3] = 10;   // 空白

    if(temperature >= 1000)
        show_buf[4] = temperature / 1000;
    else
        show_buf[4] = 10;

    show_buf[5] = temperature / 100 % 10 + 20;   // 带小数点
    show_buf[6] = temperature / 10 % 10;
    show_buf[7] = temperature % 10;
}

//==================== 主函数 ====================

void main(void)
{
    System_Init();
    Timer0_Init();

    while(1)
    {
        temperature = Read_DS18B20_temp();
        Display_show_temp();
        DelayMs(500);
    }
}