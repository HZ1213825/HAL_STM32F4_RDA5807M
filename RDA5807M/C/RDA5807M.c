#include "RDA5807M.h"

uint16_t RDA5807M_RadioStadion_Freq[RDA5807M_N] = {0}; //查找到的电台
#ifdef RDA5807_Software_I2C
/**
 * @brief 一段延迟
 * @param 无
 * @return 无
 * @author HZ12138
 * @date 2022-07-27 08:53:30
 */
void I2C_Delay()
{
    int z = 0xff;
    while (z--)
        ;
}
/**
 * @brief 产生I2C起始信号
 * @param 无
 * @return 无
 * @author HZ12138
 * @date 2022-07-27 08:54:48
 */
void I2C_Start(void)
{
    I2C_Write_SDA(GPIO_PIN_SET);   //需在SCL之前设定
    I2C_Write_SCL(GPIO_PIN_SET);   // SCL->高
    I2C_Delay();                   //延时
    I2C_Write_SDA(GPIO_PIN_RESET); // SDA由1->0,产生开始信号
    I2C_Delay();                   //延时
    I2C_Write_SCL(GPIO_PIN_RESET); // SCL->低
}
/**
 * @brief 产生I2C结束信号
 * @param 无
 * @return 无
 * @author HZ12138
 * @date 2022-07-27 08:57:03
 */
void I2C_End(void)
{
    I2C_Write_SDA(GPIO_PIN_RESET); //在SCL之前拉低
    I2C_Write_SCL(GPIO_PIN_SET);   // SCL->高
    I2C_Delay();                   //延时
    I2C_Write_SDA(GPIO_PIN_SET);   // SDA由0->1,产生结束信号
    I2C_Delay();                   //延时
}
/**
 * @brief 发送应答码
 * @param ack:0 应答 1 不应达
 * @return 无
 * @author HZ12138
 * @date 2022-07-27 09:03:38
 */
void IIC_Send_ACK(uint8_t ack)
{
    if (ack == 1)
        I2C_Write_SDA(GPIO_PIN_SET); //产生应答电平
    else
        I2C_Write_SDA(GPIO_PIN_RESET);
    I2C_Delay();
    I2C_Write_SCL(GPIO_PIN_SET);   //发送应答信号
    I2C_Delay();                   //延时至少4us
    I2C_Write_SCL(GPIO_PIN_RESET); //整个期间保持应答信号
}
/**
 * @brief 接受应答码
 * @param 无
 * @return 应答码 0 应答 1 不应达
 * @author HZ12138
 * @date 2022-07-27 09:04:28
 */
uint8_t IIC_Get_ACK(void)
{
    uint8_t ret;                 //用来接收返回值
    I2C_Write_SDA(GPIO_PIN_SET); //电阻上拉,进入读
    I2C_Delay();
    I2C_Write_SCL(GPIO_PIN_SET); //进入应答检测
    I2C_Delay();                 //至少延时4us
    ret = I2C_Read_SDA();        //保存应答信号
    I2C_Write_SCL(GPIO_PIN_RESET);
    return ret;
}
/**
 * @brief I2C写1Byte
 * @param dat:1Byte数据
 * @return 应答结果 0 应答 1 不应达
 * @author HZ12138
 * @date 2022-07-27 09:05:14
 */
uint8_t I2C_SendByte(uint8_t dat)
{
    uint8_t ack;
    for (int i = 0; i < 8; i++)
    {
        // 高在前低在后
        if (dat & 0x80)
            I2C_Write_SDA(GPIO_PIN_SET);
        else
            I2C_Write_SDA(GPIO_PIN_RESET);
        I2C_Delay();
        I2C_Write_SCL(GPIO_PIN_SET);
        I2C_Delay(); //延时至少4us
        I2C_Write_SCL(GPIO_PIN_RESET);
        dat <<= 1; //低位向高位移动
    }

    ack = IIC_Get_ACK();

    return ack;
}
/**
 * @brief I2C读取1Byte数据
 * @param ack:应答 0 应答 1 不应达
 * @return 接受到的数据
 * @author HZ12138
 * @date 2022-07-27 09:06:13
 */
uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t ret = 0;
    // OLED_Read_SDA() 设置输入方向
    I2C_Write_SDA(GPIO_PIN_SET);
    for (int i = 0; i < 8; i++)
    {
        ret <<= 1;
        I2C_Write_SCL(GPIO_PIN_SET);
        I2C_Delay();
        // 高在前低在后
        if (I2C_Read_SDA())
        {
            ret++;
        }
        I2C_Write_SCL(GPIO_PIN_RESET);
        I2C_Delay();
    }

    IIC_Send_ACK(ack);

    return ret;
}
#endif
/**
 * @brief 写寄存器
 * @param Address:寄存器地址
 * @param Data:数据
 * @return 无
 * @author HZ12138
 * @date 2022-07-21 21:57:40
 */
void RDA5807M_Write_Reg(uint8_t Address, uint16_t Data)
{
    uint8_t Buf[2] = {0};
    Buf[0] = (Data & 0xff00) >> 8; //高位
    Buf[1] = Data & 0x00ff;        //低位
#ifdef RDA5807_Hardware_I2C
    HAL_I2C_Mem_Write(&RDA6807M_I2C_Handle, 0x22, Address, I2C_MEMADD_SIZE_8uint8_t, Buf, 2, 0xffff);
#endif
#ifdef RDA5807_Software_I2C
    I2C_Start();
    I2C_SendByte(0x22);
    I2C_SendByte(Address);
    I2C_SendByte(Buf[0]);
    I2C_SendByte(Buf[1]);
    I2C_End();
#endif
}
/**
 * @brief 读寄存器
 * @param Address:寄存器地址
 * @return 读取的数据
 * @author HZ12138
 * @date 2022-07-21 21:58:33
 */
uint16_t RDA5807M_Read_Reg(uint8_t Address)
{
    uint8_t Buf[2] = {0};
#ifdef RDA5807_Hardware_I2C
    HAL_I2C_Mem_Read(&RDA6807M_I2C_Handle, 0x22, Address, I2C_MEMADD_SIZE_8uint8_t, Buf, 2, 0xffff);
#endif
#ifdef RDA5807_Software_I2C
    I2C_Start();
    I2C_SendByte(0x22);
    I2C_SendByte(Address);
    I2C_Start();
    I2C_SendByte(0x23);
    Buf[0] = I2C_ReadByte(0);
    Buf[1] = I2C_ReadByte(1);
    I2C_End();
#endif
    return ((Buf[0] << 8) | Buf[1]);
}
/**
 * @brief 初始化
 * @param 无
 * @return 无
 * @author HZ12138
 * @date 2022-07-21 22:00:48
 */
void RDA5807M_init(void)
{
    RDA5807M_Write_Reg(0x02, 0x0002); //复位
    HAL_Delay(50);
    RDA5807M_Write_Reg(0x02, 0xc001);
    HAL_Delay(600);
    RDA5807M_Write_Reg(0x03, 0x0018);
    RDA5807M_Write_Reg(0x04, 0x0400);
    RDA5807M_Write_Reg(0x05, 0x86ad);
    RDA5807M_Write_Reg(0x06, 0x8000);
    RDA5807M_Write_Reg(0x07, 0x5F1A);
}
/**
 * @brief 将频率转为信道值
 * @param Freq:频率(以MHz为单位*100)(如108MHz=>10800)
 * @return 转换为的信道值
 * @author HZ12138
 * @date 2022-07-21 22:01:08
 */
uint16_t RDA5807M_FreqToChan(uint16_t Freq)
{
    uint16_t Start = 0; //频率开始
    uint16_t End = 0;   //频率结束
    uint16_t Space = 0; //频率间隔
    uint16_t zj = 0;
    zj = (RDA5807M_Read_Reg(0x03) & 0x000C) >> 2; // 0x03的3，2位（波段）

    if (zj == 0 /*0b00*/)
    {
        Start = 8700;
        End = 10800;
    }
    else if (zj == 1 /*0b01*/)
    {
        Start = 7600;
        End = 9100;
    }
    else if (zj == 2 /*0b10*/)
    {
        Start = 7600;
        End = 10800;
    }
    else if (zj == 3 /*0b11*/)
    {
        if ((RDA5807M_Read_Reg(0x07) >> 9) & 0x01)
        {
            Start = 6500;
            End = 7600;
        }
        else
        {
            Start = 5000;
            End = 7600;
        }
    }
    else
        return 0;

    zj = (RDA5807M_Read_Reg(0x03) & 0x0003);

    if (zj == 0 /*0b00*/)
        Space = 10;
    else if (zj == 1 /*0b01*/)
        Space = 5;
    else if (zj == 2 /*0b10*/)
        Space = 20;
    else if (zj == 3 /*0b11*/)
        Space = 40;
    else
        return 0;

    if (Freq < Start)
        return 0;
    if (Freq > End)
        return 0;

    return ((Freq - Start) / Space);
}
/**
 * @brief 将信道值转为频率
 * @param Chan:信道值
 * @return 频率(以MHz为单位*100)(如108MHz=>10800)
 * @author HZ12138
 * @date 2022-07-21 22:03:01
 */
uint16_t RDA5807M_ChanToFreq(uint16_t Chan)
{
    uint16_t Start = 0; //频率开始
    uint16_t End = 0;   //频率结束
    uint16_t Space = 0; //频率间隔
    uint16_t zj = 0;
    zj = (RDA5807M_Read_Reg(0x03) & 0x000C) >> 2; // 0x03的3，2位（波段）

    if (zj == 0 /*0b00*/)
    {
        Start = 8700;
        End = 10800;
    }
    else if (zj == 1 /*0b01*/)
    {
        Start = 7600;
        End = 9100;
    }
    else if (zj == 2 /*0b10*/)
    {
        Start = 7600;
        End = 10800;
    }
    else if (zj == 3 /*0b11*/)
    {
        if ((RDA5807M_Read_Reg(0x07) >> 9) & 0x01)
        {
            Start = 6500;
            End = 7600;
        }
        else
        {
            Start = 5000;
            End = 7600;
        }
    }
    else
        return 0;

    zj = (RDA5807M_Read_Reg(0x03) & 0x0003);

    if (zj == 0 /*0b00*/)
        Space = 10;
    else if (zj == 1 /*0b01*/)
        Space = 5;
    else if (zj == 2 /*0b10*/)
        Space = 20;
    else if (zj == 3 /*0b11*/)
        Space = 80;
    else
        return 0;
    zj = Start + Chan * Space;
    if (zj > End)
        return 0;
    if (zj < Start)
        return 0;
    return zj;
}
/**
 * @brief 读取当前频率
 * @param 无
 * @return 频率(以MHz为单位*100)(如108MHz=>10800)
 * @author HZ12138
 * @date 2022-07-21 22:05:43
 */
uint16_t RDA5807M_Read_Freq(void)
{
    uint16_t Chan = 0;
    Chan = RDA5807M_Read_Reg(0x0A) & 0x03FF;
    return RDA5807M_ChanToFreq(Chan);
}
/**
 * @brief 设置频率值
 * @param Freq:频率(以MHz为单位*100)(如108MHz=>10800)
 * @return 无
 * @author HZ12138
 * @date 2022-07-21 22:06:22
 */
void RDA5807M_Set_Freq(uint16_t Freq)
{
    uint16_t Chan = RDA5807M_FreqToChan(Freq); //先转化为信道值
    uint16_t zj = RDA5807M_Read_Reg(0x03);
    zj &= 0x003F;               //清空信道值
    zj |= (Chan & 0x03FF) << 6; //写入信道值
    zj |= (1) << 4;             //调频启用
    RDA5807M_Write_Reg(0x03, zj);
    RDA5807M_Write_Reg(0x03, zj); //需要写入两次，咱也不知道为啥
}
/**
 * @brief 向上搜索下一个电台（搜索完成后会设置当前频率为搜到的频率）
 * @param 无
 * @return 无
 * @author HZ12138
 * @date 2022-07-21 22:11:22
 */
void RDA5807M_Search_Freq_TurnUp(void)
{
    uint16_t zj;
    zj = RDA5807M_Read_Reg(0x03);
    zj &= ~(1 << 4); //禁用调谐
    RDA5807M_Write_Reg(0x03, zj);

    zj = RDA5807M_Read_Reg(0x02);
    zj |= 1 << 9; //向上搜索
    zj |= 1 << 8; //开启搜索
    zj |= 1 << 7; //到达最高频率停止
    RDA5807M_Write_Reg(0x02, zj);
    while ((RDA5807M_Read_Reg(0x0A) & 0x4000) == 0) //等待搜索完成
    {
        HAL_Delay(1);
    }
    RDA5807M_Set_Freq(RDA5807M_Read_Freq()); //将搜索到频率设置为播放频率
}
/**
 * @brief 判断当前频率是否为电台
 * @param 无
 * @return 返回1则是电台，0则不是电台
 * @author HZ12138
 * @date 2022-07-21 22:22:30
 */
uint8_t RDA5807M_Radio_Instructions(void)
{
    uint16_t zj;
    zj = RDA5807M_Read_Reg(0x0B);
    zj >>= 8;
    zj &= 1;
    return zj;
}
/**
 * @brief 搜索所有电台
 * @param 无
 * @return 搜到的电台数量
 * @author HZ12138
 * @date 2022-07-21 22:12:33
 */
uint16_t RDA5807M_Search_ALL_Freq(void)
{
    uint16_t i = 0;
    uint16_t zj = 0;
    uint16_t Start, End;
    zj = (RDA5807M_Read_Reg(0x03) & 0x000C) >> 2; // 0x03的3，2位（波段）

    if (zj == 0 /*0b00*/)
    {
        Start = 8700;
        End = 10800;
    }
    else if (zj == 1 /*0b01*/)
    {
        Start = 7600;
        End = 9100;
    }
    else if (zj == 2 /*0b10*/)
    {
        Start = 7600;
        End = 10800;
    }
    else if (zj == 3 /*0b11*/)
    {
        if ((RDA5807M_Read_Reg(0x07) >> 9) & 0x01)
        {
            Start = 6500;
            End = 7600;
        }
        else
        {
            Start = 5000;
            End = 7600;
        }
    }
    else
        return 0;

    RDA5807M_Set_Freq(Start);
    HAL_Delay(100);
    while (RDA5807M_Read_Freq() != End)
    {
        RDA5807M_Search_Freq_TurnUp();
        HAL_Delay(10);

        RDA5807M_RadioStadion_Freq[i] = RDA5807M_Read_Freq();
        i++;
    }
    HAL_Delay(100);
    if (!RDA5807M_Radio_Instructions())
        RDA5807M_RadioStadion_Freq[--i] = 0;
    return i;
}
/**
 * @brief 设置音量
 * @param Val:音量值(0-15)
 * @return 无
 * @author HZ12138
 * @date 2022-07-21 22:20:20
 */
void RDA5807M_Set_Volume(uint8_t Val)
{
    uint16_t zj;
    zj = RDA5807M_Read_Reg(0x05);
    zj &= 0xFFF0;
    zj |= (Val & 0x0F);
    RDA5807M_Write_Reg(0x05, zj);
}
/**
 * @brief 设置静音
 * @param Bool：1是静音，0是解除静音
 * @return 无
 * @author HZ12138
 * @date 2022-07-21 23:13:30
 */
void RDA5807M_SetMutea(uint8_t Bool)
{
    uint16_t zj;
    zj = RDA5807M_Read_Reg(0x02);
    if (Bool)
    {
        zj &= ~(1 << 14);
    }
    else
    {
        zj |= 1 << 14;
    }
    RDA5807M_Write_Reg(0x02, zj);
}
/**
 * @brief 将输出设为空闲状态（喇叭高阻）
 * @param Bool：1是空闲，0是解除空闲
 * @return 无
 * @author HZ12138
 * @date 2022-07-21 23:39:07
 */
void RDA5807M_Set_Output_Idle(uint8_t Bool)
{
    uint16_t zj;
    zj = RDA5807M_Read_Reg(0x02);
    if (Bool)
    {
        zj &= ~(1 << 15);
    }
    else
    {
        zj |= 1 << 15;
    }
    RDA5807M_Write_Reg(0x02, zj);
}
/**
 * @brief 获取当前频率的信号强度
 * @param 无
 * @return 信号强度(0-127)
 * @author HZ12138
 * @date 2022-07-21 23:47:17
 */
uint8_t RDA5807M_Read_Signal_Intensity(void)
{
    uint16_t zj;
    zj = RDA5807M_Read_Reg(0x0B);
    zj >>= 9;
    return (uint8_t)zj;
}
/**
 * @brief  读取芯片ID
 * @param 无
 * @return 芯片ID
 * @author HZ12138
 * @date 2022-07-21 23:53:58
 */
uint16_t RDA5807M_Read_ID(void)
{
    return RDA5807M_Read_Reg(0x00);
}
/**
 * @brief 设置频率段
 * @param Range：频率段，来自频率段选择组的宏定义，如RDA6807M_Freq_Range_76_108
 * @return 无
 * @author HZ12138
 * @date 2022-07-23 11:16:42
 */
void RDA5807M_Set_FreqRange(uint8_t Range)
{
    uint16_t zj;
    zj = RDA5807M_Read_Reg(0x03);
    if (Range == RDA6807M_Freq_Range_87_108)
    { /*0x03[3:2]=00 0x07[9]=x*/
        zj &= ~(1 << 3);
        zj &= ~(1 << 2);
        RDA5807M_Write_Reg(0x02, zj);
    }
    else if (Range == RDA6807M_Freq_Range_76_91)
    { /*0x03[3:2]=01 0x07[9]=x*/
        zj &= ~(1 << 3);
        zj |= 1 << 2;
        RDA5807M_Write_Reg(0x02, zj);
    }
    else if (Range == RDA6807M_Freq_Range_76_108)
    { /*0x03[3:2]=10 0x07[9]=x*/
        zj |= 1 << 3;
        zj &= ~(1 << 2);
        RDA5807M_Write_Reg(0x02, zj);
    }
    else if (Range == RDA6807M_Freq_Range_65_76)
    { /*0x03[3:2]=11 0x07[9]=1*/
        zj |= 1 << 2;
        zj |= 1 << 3;
        RDA5807M_Write_Reg(0x02, zj);
        zj = RDA5807M_Read_Reg(0x07);
        zj |= 1 << 9;
        RDA5807M_Write_Reg(0x07, zj);
    }
    else if (Range == RDA6807M_Freq_Range_50_76)
    { /*0x03[3:2]=11 0x07[9]=0*/
        zj |= 1 << 2;
        zj |= 1 << 3;
        RDA5807M_Write_Reg(0x02, zj);
        zj = RDA5807M_Read_Reg(0x07);
        zj &= ~(1 << 9);
        RDA5807M_Write_Reg(0x07, zj);
    }
}
/**
 * @brief 设置频率间隔
 * @param SPACE：间隔，从频率间隔选择组宏定义里选取，如RDA6807M_Freq_Space_100kHz
 * @return 无
 * @author HZ12138
 * @date 2022-07-23 16:01:05
 */
void RDA5807M_Set_FreqSpace(uint8_t SPACE)
{
    uint16_t zj;
    zj = RDA5807M_Read_Reg(0x03);
    if (SPACE == RDA6807M_Freq_Space_100kHz)
    { /*0x03[1:0]=00*/
        zj &= ~(1 << 1);
        zj &= ~(1 << 0);
    }
    else if (SPACE == RDA6807M_Freq_Space_200kHz)
    { /*0x03[1:0]=01*/
        zj &= ~(1 << 1);
        zj |= 1 << 0;
    }
    else if (SPACE == RDA6807M_Freq_Space_50KHz)
    { /*0x03[1:0]=10*/
        zj |= 1 << 1;
        zj &= ~(1 << 0);
    }
    else if (SPACE == RDA6807M_Freq_Space_25KHz)
    { /*0x03[1:0]=11*/
        zj |= 1 << 1;
        zj |= 1 << 0;
    }
    RDA5807M_Write_Reg(0x03, zj);
}
/**
 * @brief 复位
 * @param 无
 * @return 无
 * @author HZ12138
 * @date 2022-07-24 00:11:54
 */
void RDA5807M_Reast(void)
{
    RDA5807M_Write_Reg(0x02, 0x0002); //复位
    HAL_Delay(50);
}
