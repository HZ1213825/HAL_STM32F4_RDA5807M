#include "RDA5807M.h"
uint16_t RDA5807M_RadioStadion_Freq[RDA5807M_N] = {0}; //查找到的电台
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
    HAL_I2C_Mem_Write(&RDA6807M_I2C_Handle, 0x22, Address, I2C_MEMADD_SIZE_8BIT, Buf, 2, 0xffff);
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
    HAL_I2C_Mem_Read(&RDA6807M_I2C_Handle, 0x22, Address, I2C_MEMADD_SIZE_8BIT, Buf, 2, 0xffff);
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
