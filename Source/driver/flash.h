#ifndef _FLASH_H_
#define _FLASH_H_


#include "system.h"


#pragma pack(1)
typedef struct 
{
	uint8_t temperature;	
	uint8_t music_type;
    uint8_t ring_duty;
    uint8_t temperature_unit;
	uint8_t reserved1;
	uint8_t reserved2;
	uint8_t reserved3;
	uint8_t reserved4;
	uint8_t reserved5;
    uint8_t reserved6;
    uint8_t reserved7;
	uint8_t version;
	uint32_t checksum;
}epprom_set_info_t;

#pragma pack()

#define CODE_BASE_ADDRESS     		((uint32_t)0x08000000)
#define RUN_INFO_ADDRESS     		((uint32_t)0x0801F000)
#define SET_INFO_ADDRESS     		((uint32_t)0x0801F800)
#define FLASH_PAGE_SIZE          	(1024)


#ifdef USING_EEPROM
extern void flash_read_epprom_set_info(set_info_t *sit);
extern void flash_read_epprom_run_info(run_info_t *rit);
#else
extern void flash_read_set_info(set_info_t *sit);
extern void flash_read_run_info(run_info_t *rit);
#endif
extern void flash_write_set_info(set_info_t *sit);
extern void flash_write_run_info(run_info_t *rit);
extern void flash_config(void);
extern void flash_epprom_read(uint16_t address, uint8_t *buffer, uint16_t length);
extern void flash_reset_epprom_run_info(run_info_t *rit);
extern void flash_reset_epprom_set_info(set_info_t *sit);
extern void trace_printf(const char *fmt, ...);
#endif
