#include "mhscpu.h"
#include "mhscpu_timer.h"
#include "timer.h"
#include "mhscpu_sysctrl.h"
#include "mhscpu_exti.h"

void delay_ms(unsigned int ms);

tick g_current_tick = 0;

#define  nop()      {int i; for (i = 0; i < 1; i++);}   /*¶¨Òå¿ÕÖ¸Áî*/

/*****************************************************************************
 * Name		: init_timer0
 * Function	: initial the timer 0, it will be token interrupt interval 1ms.
 * ---------------------------------------------------------------------------
 * Input Parameters:None
 * Output Parameters:None
 * Return Value:
 * ---------------------------------------------------------------------------
 * Description:
 * 		Must init the timer 0 before calling get_tick(), mdelay(), is_timeout() etc.
 *****************************************************************************/
void init_timer0(void)
{	
	TIM_InitTypeDef d;
	NVIC_InitTypeDef nvic;
	g_current_tick = 0;
	d.TIMx = TIM_1;
	d.TIM_Period = SYSCTRL->PCLK_1MS_VAL;
	TIM_DeInit(TIMM0);
	TIM_Init(TIMM0, &d);
	TIM_ITConfig(TIMM0, TIM_1, ENABLE);
	TIM_Cmd(TIMM0, TIM_1, ENABLE);		
	
	nvic.NVIC_IRQChannel = TIM0_1_IRQn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 1;
	nvic.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&nvic);
/*
	d.TIMx = TIM_2;
	d.TIM_Period = SYSCTRL->PCLK_1MS_VAL*50;
	TIM_Init(TIMM0, &d);
	TIM_ITConfig(TIMM0, TIM_2, ENABLE);

	nvic.NVIC_IRQChannel = TIM0_2_IRQn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 1;
	nvic.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&nvic);
    */
}
#define MAX_TIMER   10
volatile static unsigned long TimerCount[MAX_TIMER]={0};


void s_SetTimer(unsigned char TimerNo, unsigned long count)
{
    TimerCount[TimerNo] = count*100;
}
unsigned long s_CheckTimer(unsigned char TimerNo)
{
	return TimerCount[TimerNo];
}

void TIM0_1_IRQHandler()
{
	int i;

	for(i=0;i<MAX_TIMER;i++)
	{
		if(TimerCount[i]!=0) 
		{
			TimerCount[i]--;
		}
	}

	g_current_tick++;
	TIM_ClearITPendingBit(TIMM0, TIM_1);
}
static int (* timer_dev2_isr_handler)(void);

void TIM0_2_IRQHandler(void)
{
	//printf("TIM0_2_IRQHandler\n");
	TIM_ClearITPendingBit(TIMM0, TIM_2);
	TIM_Cmd(TIMM0, TIM_2, DISABLE);	
	timer_dev2_isr_handler();
}
/*
void keyTimer(void(*handler)(void),uint32_t Period)
{
	TIM_SetPeriod(TIMM0, TIM_2,SYSCTRL->PCLK_1MS_VAL*Period);

	if(timer_dev2_isr_handler == NULL){
		//lite_printf("%s %d\r\n",__FUNCTION__,__LINE__);
		timer_dev2_isr_handler = handler;
	}

	TIM_Cmd(TIMM0, TIM_2, ENABLE);	
}
*/
/*****************************************************************************
 * Name		: get_tick
 * Function	: Get current tick.
 * ---------------------------------------------------------------------------
 * Input Parameters:None
 * Output Parameters:None
 * Return Value:
 * ---------------------------------------------------------------------------
 * Description:
 * 
 *****************************************************************************/
tick get_tick(void)
{
	return g_current_tick;
}

/*****************************************************************************
 * Name		: mdelay
 * Function	: Millisecond delay.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *			msec		Milliseconds of delay
 * Output Parameters:None
 * Return Value:none
 * ---------------------------------------------------------------------------
 * Description:
 * 	
 *****************************************************************************/
void Lib_DelayMs(unsigned int ms)
{
	//mdelay(ms);
	delay_ms(ms);
}


void mdelay(tick msec)
{
	delay_ms(msec);

	/*tick old_tick;

	old_tick = get_tick();
	while (get_diff_tick(get_tick(), old_tick) < msec)
	{
	}*/
}
void Lib_DelayUs(unsigned int us)
{
	udelay(us);
}

void delay_ms(unsigned int ms)
{
	udelay(ms*1000);
}

void delay_us(unsigned int us)
{
	udelay(us);
}



//void udelay(tick usec)
void udelay(unsigned int usec)
{
	/*volatile int i,j;
	for (i = usec; i > 0; i--)
	{
		for (j = 0; j < 4; j++);
	}*/
/*	unsigned long int currentCount=0;	
	
	TIMM0->TIM[0].ControlReg = 0x04;	
	TIMM0->TIM[0].LoadCount = 0xFFFFFFFF;	
	TIMM0->TIM[0].ControlReg = 0x05;	
	while((0xFFFFFFFF-TIMM0->TIM[0].CurrentValue)<(36*usec));	
	TIMM0->TIM[0].ControlReg = 0x04;	
	TIMM0->TIM[0].ControlReg = 0x05;	
	TIMM0->TIM[0].ControlReg = 0x04;*/

     volatile int i;
      for(i = 0; i < usec; i++)
      {
      
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
				
						__nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
				
						__nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();
            __nop();__nop();__nop();__nop();__nop();


            __nop();__nop();__nop();__nop();__nop();            
           
      }
}

#if 0
void delay_unit(void)
{
/*	TIMM0->TIM[0].ControlReg = 0x04;	
	TIMM0->TIM[0].LoadCount = 0xFFFFFFFF;	
	TIMM0->TIM[0].ControlReg = 0x05;	
	while((0xFFFFFFFF-TIMM0->TIM[0].CurrentValue)<153);	
	TIMM0->TIM[0].ControlReg = 0x04;	*/

       SysCtlDelay(96);
//udelay(2);
}

void delay_unit1(void)
{
/*	TIMM0->TIM[0].ControlReg = 0x04;	
	TIMM0->TIM[0].LoadCount = 0xFFFFFFFF;	
	TIMM0->TIM[0].ControlReg = 0x05;	
	while((0xFFFFFFFF-TIMM0->TIM[0].CurrentValue)<129);	
	TIMM0->TIM[0].ControlReg = 0x04;*/

//udelay(1);
    SysCtlDelay(48);

}
#endif

/*****************************************************************************
 * Name		: is_timeout
 * Function	: Determine whether the timeout that millisecond.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *			start_time	start time
 *			interval		time interval
 * Output Parameters:None
 * Return Value:
 *			TRUE		timeout
 *			FALSE		It is not timeout
 * ---------------------------------------------------------------------------
 * Description:
 * 	
 *****************************************************************************/
int is_timeout(tick start_time, tick interval)
{
	return (get_diff_tick(get_tick(), start_time) >= interval);
}

/*****************************************************************************
 * Name		: get_diff_tick
 * Function	: Get the time interval (unit is 1ms) for two tick.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *			cur_tick		lastest tick
 *			prior_tick		prior tick
 * Output Parameters:None
 * Return Value:
 *			Return the ticks of two tick difference.
 * ---------------------------------------------------------------------------
 * Description:
 * 
 *****************************************************************************/
tick get_diff_tick(tick cur_tick, tick prior_tick)
{
	if (cur_tick < prior_tick){
		return (cur_tick + (~prior_tick));
	}
	else{
		return (cur_tick - prior_tick);
	}
}
