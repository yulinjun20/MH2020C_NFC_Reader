 /**
 ****************************************************************
 * @file define.h
 *
 * @brief  define宏控制和部分变量定义
 *
 * @author 
 *
 * 
 ****************************************************************
 */ 
#ifndef DEFINE_H
#define DEFINE_H


/*
 * VESRION DESCRIPTION
 ****************************************************************
 */	

//#define VERSION "1.0.0"
// 1. 完成N12 SDK的编写
// 2. 更新软件，加入LPCD测试和COS指令测试功能
//#define VERSION "1.1.0"
// 1. 优化0x3B寄存器配置
//#define VERSION "1.2.0"
// 1. 优化35/3b上电初始化配置
//#define VERSION "1.3.0"
// 1. 优化SNR
//#define VERSION "1.4.0"
// 1. 优化注释
#define VERSION "1.5.0"
// 1. 优化高速率通信配置

#define MOTHERBOARD_V20	    1	//运行于新底板V2.0
#define SIKAI_V10		    0	//运行于与SIKAI兼容的小板

#define NFC_DEBUG		0	// printf打印控制开关

#define POWERON_POLLING 0 //上电即开始自动A/B polling
#define INT_USE_CHECK_REG   1	//中断检测使用查询07寄存器的irq bit的方式，而不是使用查询中断管脚

//PC机软件与下位机软件通信命令字
#define COM_PKT_CMD_READ_REG 		1
#define COM_PKT_CMD_WRITE_REG	2

#define COM_PKT_CMD_QUERY_MODE		      0x0D	//是否是手动模式，而不是自动寻卡模式
#define COM_PKT_CMD_CHIP_RESET            0x0E  //软复位数字芯片
#define COM_PKT_CMD_CARD_TYPE		      0x0F
#define COM_PKT_CMD_REQ_SELECT		      0x10
#define COM_PKT_CMD_HALT			      0x11  //发送Halt指令
#define COM_PKT_CMD_STATISTICS		      0x12  //统计信息
#define COM_PKT_CMD_LPCD			      0x13  //LPCD 功能
#define COM_PKT_CMD_LPCD_CONFIG_TEST      0x16  //测试LPCD功能
#define COM_PKT_CMD_LPCD_CONFIG_TEST_STOP 0x17


typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef unsigned char bool;
typedef unsigned long tick;

typedef unsigned long U32;
typedef unsigned short U16;
typedef unsigned char U8;
typedef unsigned char bit;
	
#define DATA		//data
#define IDATA		//idata
#define PDATA		//pdata
#define XDATA		//xdata
#define CODE		//	code
#define REENTRANT	//reentrant

//#define FALSE 	0
//#define TRUE	1
/*
#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

*/

#endif
