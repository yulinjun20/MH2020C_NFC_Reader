#ifndef _USER_H_
#define _USER_H_
#include "stdint.h"
enum cpu_type
{
    MH2020C_QFN48,
    MH2020C_QFN32,
};

enum rfid_type
{
     RF_MH1608,
};

extern  unsigned int g_CPU_Type;

#define PCI_COM   0

//#define PICC_STANDARD_RETURN_VALUE_35XX

/*
主要是在检卡成功后获取ATQA
*/
//#define PICC_CHECK_RETURN_OTHERS_INFO

//==================================================
/*
北京慧能达新疆项目出货需要打开次宏定义
*/
//#define NEED_REMOVE_ID_AFTER_READ_SUCCESS
//==================================================

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned int
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef ushort
#define ushort unsigned short
#endif

#ifndef ulong
#define ulong  unsigned long
#endif

#ifndef uint
#define uint unsigned int
#endif

#ifndef NULL
#define NULL 0x0
#endif


/*
cpu flash存储分配图
boot: [4KB~60KB)----------------56KB
APP Flag:[60KB~64KB)-------------4KB
APP:[64KB~384KB)---------------320KB
Reserved:[384KB~480KB)----------96KB
boot version:[480KB~484KB)-------4KB
TypeB Card ModGsp:[484KB~488KB)-4KB
Reserved:[488KB~512KB)----------24KB
*/

#define   ABS(a) ((a)>=0?(a):(-(a)))
#define   LOW_BYTE(x)   ((x)   &   0xff)
#define   HI_BYTE(x)   (((x)   >>   8)   &   0xff)
#define   LOW_WORD(x)   ((x)   &   0xffff)
#define   HI_WORD(x)   ((x)   >>   16)
#define   LOW_DWORD(x)   ((x)   &   0xffffffff)   
#define   HI_DWORD(x)   ((x)   >>   32)

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))


#define BOOTVERSION_ADDR    (480*1024)
#define TYPEB_PARA_CONFIG_ADDR (484*1024)

#define TYPEB_PARA_CONFIG_FLAG  0xAA55AA55

typedef struct
{
	unsigned char   Command[4]; // CLA INS  P1 P2
	unsigned short  Lc;         // P3
	unsigned char   DataIn[512];//512
	unsigned short  Le;
} APDU_SEND;

typedef struct
{
	unsigned short  LenOut;     // length of dataout 
	unsigned char   DataOut[512];
	unsigned char   SWA;
	unsigned char   SWB;
} APDU_RESP;


#endif

