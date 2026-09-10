#include <string.h>
#include <stdio.h>
#include "mhscpu.h"
#include "uart.h"
#include "spi.h"

#include "user.h"

/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

unsigned int g_CPU_Type=1;
unsigned int g_debug_enable = 0;

unsigned int g_RF_Module = RF_MH1608;

int s_SystemInit(void)
{
   // s_TimerInit();
   //init_timer0();
    s_PiccInit();
    return 0;
}

void Debug_Printf_Enable(void)
{
    g_debug_enable = 1;
}

void Debug_Printf_Disable(void)
{
    g_debug_enable = 0;
}


int main(void)
{
    int i;
    unsigned char SN[30];
    SYSCTRL_ClocksTypeDef clocks;

    QSPI_Init(NULL); 
    SYSCTRL_SYSCLKSourceSelect(SELECT_INC12M);// SELECT_EXT12M
    SYSCTRL_PLLConfig(SYSCTRL_PLL_144MHz);
    SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div_None);
    SYSCTRL_PCLKConfig(SYSCTRL_PCLK_Div2);

    //修改系统频率后需设置
    QSPI_SetLatency(0);
	
    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);
    SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);	
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_UART0 | SYSCTRL_APBPeriph_UART1 |SYSCTRL_APBPeriph_SPI0 | SYSCTRL_APBPeriph_TIMM0 | SYSCTRL_APBPeriph_GPIO, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART0 | SYSCTRL_APBPeriph_UART1 |SYSCTRL_APBPeriph_SPI0 | SYSCTRL_APBPeriph_TIMM0, ENABLE);

    //	UART_Configuration();
    //	printf("MegaHunt SCPU GPIO Demo V1.0.\r\n");

    GetUSN(SN);

    if(strstr(SN, "2020CQ32"))
    {
        g_CPU_Type = MH2020C_QFN32;
    }
    else
    {
        g_CPU_Type = MH2020C_QFN48;
    }

    PortOpen(PCI_COM, 115200);
    Debug_Printf_Enable();
    //Debug_Printf_Disable();
    s_SystemInit();

    /* get clock frequence */
    SYSCTRL_GetClocksFreq(&clocks);
		
   // printf(" sysclock=%d,%d,%d\r\n",clocks.HCLK_Frequency, clocks.PCLK_Frequency, clocks.PLL_Frequency);
	
//    printf("MegaHunt SCPU UART Polling Demo V1.0.\n");

     //printf("g_CPU_Type=%d\r\n", g_CPU_Type);  
//Test();
   //test_ID();

     //Debug_Printf_Disable();
   //test_ID();
	
	/* while(1)
	 {
			printf("txz_test: start test picc\r\n");
            //TestPicc();
			NFC_Tag2_Test();
	 }
	 */
	
        
   command_handler();   
	 
	while (1)
	{
		
		
	}
}


//Retarget Printf
int SER_PutChar (int ch) 
{
    if(g_debug_enable)
    {
   //     while(!UART_IsTXEmpty(UART0));
    //    UART_SendData(UART0, (uint8_t) ch);
    }

    return ch;
}

int fputc(int c, FILE *f)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	if (c == '\n')
	{
		SER_PutChar('\r');
	}
	return (SER_PutChar(c));
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
