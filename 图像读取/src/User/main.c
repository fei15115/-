/****************************************************************************************************
【平    台】龙邱K60VG核心板
【编    写】CHIUSIR
【E-mail  】chiusir@163.com
【软件版本】V1.0
【最后更新】2016年09月01日
【相关信息参考下列地址】
【网    站】http://www.lqist.cn
【淘宝店铺】http://shop36265907.taobao.com
------------------------------------------------
【dev.env.】IAR7.3
【busclock】100.000MHz
【pllclock】200.000MHz
=============================================================
接口定义：
PWM口定义：
FTM2_CH0--PTB18
FTM2_CH1--PTB19

按键定义：
KEY0--PTB20
KEY1--PTB21
KEY2--PTB22

OLED口定义：
LCD_DC --PTC19
LCD_RST--PTC18
LCD_SDA--PTC17
LCD_SCL--PTC16
=============================================================
实验现象：
按下K0，占空比恢复初值
按下K1，占空比增加
按下K2，占空比减小
并在0.96寸OLED屏上显示数值
=============================================================
******************************************************************************************************/
#include "include.h"
u8 PIT0_f=0;
extern uint8_t Threshold;                  //OSTU大津法计算的图像阈值
extern volatile u8  Field_Over_Flag;
extern volatile u8  Frame_Over_Flag;
extern u8 Image_Data[IMAGEH][IMAGEW];      //图像原始数据存放
extern int OFFSET0;      //最远处，赛道中心值综合偏移量
extern int OFFSET1;      //第二格
extern int OFFSET2;      //最近，第三格
int OFFSET0_last;
int OFFSET0_last1;
int OFFSET1_last;
int OFFSET1_last1;
u8 F_kp=0;
u8 F_ki=10;
u8 F_kd=0;
void control(void);
void time_delay_ms(u32 ms)
{
  LPTMR_delay_ms(ms);
}

s16 a[8];

//主函数
void main(void)
{
   DisableInterrupts;        //关闭总中断
   PLL_Init(PLL200);         //初始化PLL为200M，总线为100MHZ  
   LCD_Init();
   LCD_CLS();
   UART_Init(UART4,115200);     //串口4初始化
  GPIO_Init (PORTA, 17, GPO,0);     //图像采集运行指示灯
  GPIO_Init (PORTA, 25, GPO,0);    //测试帧率引脚，示波器 
  GPIO_Init (PORTC, 2, GPO,0); 
   GPIO_Init (PORTC, 3, GPO,0);
   //频率：100M/(2^4)/62500=100HZ,每个脉冲高电平持续时间为1.5ms 
    FTM_PWM_Init(FTM0,FTM_CH0,390,39*20/10);//PTC1
    FTM_PWM_Init(FTM0,FTM_CH1,390,0);//PTC3
    FTM_PWM_Init(FTM0,FTM_CH2,390,0);//PTC3
     FTM_PWM_Init(FTM0,FTM_CH3,390,39*20/10);//PTC3
     FTM_PWM_Init(FTM2,FTM_CH1,62500,625*130/10);//PTC3
     PIT_Init(PIT0,20);
   EnableInterrupts;            //开启中断
   Draw_LQLogo();
      LQMT9V034_Init(); 
      TEST_LQV034();
        FTM_PWM_Duty(FTM0, FTM_CH0, 78*10/10);
        FTM_PWM_Duty(FTM0, FTM_CH3, 78*10/10);
   while(1)
   {
      if(Field_Over_Flag)
        {    
              GPIO_Reverse (PORTA, 17);   
              Get_Use_Image();  
              Get_01_Value();   
              Threshold = GetOSTU(Image_Data);   //OSTU大津法 获取全局阈值
              BinaryImage(Image_Data,Threshold);  //发送到上位机 
              Pixle_Filter();    //二值化图像数据           
              Seek_Road();
              Draw_Road();         //龙邱OLED模块显示动态图像
              Field_Over_Flag= 0;                   
        }    
     control();
   };
}

void control(void)
{
  int pian;
  int D;
  if(PIT0_f==1)
  {
    PIT0_f=0;
//    if((OFFSET0>=(-1300))&&(OFFSET0<=(1300)))
//    {
//      pian=(OFFSET0-OFFSET0_last)*F_kp/100+OFFSET0*F_ki/100+(OFFSET0-2*OFFSET0_last+OFFSET0_last1)*F_kd/100;
//      OFFSET0_last1=OFFSET0_last;
//      OFFSET0_last=OFFSET0;
//      OFFSET1_last1=0;
//      OFFSET1_last=0;
//    }else
//    {
      pian=(OFFSET1-OFFSET1_last)*F_kp/100+OFFSET1*F_ki/100+(OFFSET1-2*OFFSET1_last+OFFSET1_last1)*F_kd/100;
      //pian=pian*10/10;
      OFFSET1_last1=OFFSET1_last;
      OFFSET1_last=OFFSET1;
      OFFSET0_last1=0;
      OFFSET0_last=0;
//    }
    D=130-pian;
    if(D<=120)
    {D=120;}
    if(D>=143)
    {D=143;}
    FTM_PWM_Duty(FTM2, FTM_CH1, 625*D/10);
  }
}
   
   
   
   
   
   
   
   
   
  
//   
//   while(1)
//   {
//     for(int i=0;i<=3;i++)
//     {
//       a[i]=UART_Get_Char (UART4);
//     }
//     if(a[0]==0x0a)
//     {
//      FTM_PWM_Duty(FTM2, FTM_CH1, 625*a[1]/10);
//      FTM_PWM_Duty(FTM0, FTM_CH0, 39*a[2]/10);
//      FTM_PWM_Duty(FTM0, FTM_CH3, 39*a[3]/10);
//             for(int i=0;i<=3;i++)
//               {
//               UART_Put_Char(UART4,a[i]);
//               }
//     }
//   }
//}