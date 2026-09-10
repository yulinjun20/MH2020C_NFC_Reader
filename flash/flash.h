#ifndef _FLASH_H_
#define _FLASH_H_

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned int
#endif


#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#define FLASH_IAP_PAGE_SIZE (4 * 1024)
#define FLASH_IAP_BASE_ADDRESS 0x01000000

#define FLASH_IAP_SECTOR_SIZE  (FLASH_IAP_PAGE_SIZE * 16)

typedef enum
{
	/** Specific No Error code */
	NO_ERROR = 0,
	/** Generic errors */
	COMMON_ERR_MIN = 0x7000,
	/** Error Code: No such device */
	COMMON_ERR_NO_DEV,
	/** Error Code: Value is not appropriate */
	COMMON_ERR_INVAL,
	/** Error Code: Pointer is null */
	COMMON_ERR_NULL_PTR,
	/** Error Code: Value is out of expected range */
	COMMON_ERR_OUT_OF_RANGE,
	/** Error Code: Module not initialized */
	COMMON_ERR_NOT_INITIALIZED,
	/** Error Code: Critical error */
	COMMON_ERR_FATAL_ERROR,
	/** Error Code: Still processing */
	COMMON_ERR_RUNNING,
	/** Error Code: Action not allowed in this state */
	COMMON_ERR_BAD_STATE,
	/** Error Code: Data does not match */
	COMMON_ERR_NO_MATCH,
	/** Error Code: Action already done */
	COMMON_ERR_ALREADY,
	/** Error Code: Action not finished yet, still in progress */
	COMMON_ERR_IN_PROGRESS,
	/** Error Code: Operation is not permitted */
	COMMON_ERR_NOT_PERMITTED,
	/** Error Code: Generic error for unknown behavior */
	COMMON_ERR_UNKNOWN,
	COMMON_ERR_MAX = COMMON_ERR_UNKNOWN,

} cobra_common_errors_t;


void WordToBytes(WORD *pdwIn, int iLen, BYTE *pbyOut);
void ByteToWord(BYTE *pbyIn, int iLen, WORD *pdwOut);
void ByteToDword(BYTE *pbyIn, uint32_t iLen, DWORD *pdwOut);
void DwordToBytes(DWORD *pdwIn, uint32_t iLen, BYTE *pbyOut);
unsigned char toupper(unsigned char c);
void vTwoOne(unsigned char *in, unsigned short in_len, unsigned char *out);
void vOneTwo(unsigned char *in, unsigned short lc, unsigned char *out);
int eraseFlashPage(int pageAddress);
int writeFlashLongWord(uint32_t addr, uint32_t value);
int eraseFixLength(uint32_t addr,uint32_t long length);
unsigned int readFlashLongWord(uint32_t addr);
void ReverseDword(uint32_t *pdwIn,uint32_t iLen, uint32_t *pbyOut);
int flash_read(uint8_t *data, uint32_t addr, uint32_t size );
int flash_write( uint32_t addr, uint8_t *data, uint32_t size);
int flash_erase( uint32_t page);
uint32_t flash_Is_Protect(uint32_t u32Addr);
void flash_not_protect(uint32_t u32Addr);

void flash_not_lock(void);

void TestFlash();

#endif
