/* flash.c
 * 作者：wlc(幽灵C)
 * 邮箱：85276902@qq.com/wlcwjy@163.com
 * 日期：2024年3月31日
 * 提供flash读写
 */
#include "system.h"
#include "flash.h"
#include "led.h"
#include "board.h"

#ifdef USING_EEPROM	
#define FLASH_READ						0x01
#define FLASH_WRITE					0x00

#define EEPROM_I2C						I2C1
#define EEPROM_WAIT					5000

#define FLASH_DMA						DMA1
#define FLASH_TX_DMA_CHANNEL      	DMA1_Channel6
#define FLASH_I2C_ADDRESS				0xA0
#define FLASH_CACHE_LENGHT      		128

static uint8_t flash_buffer[FLASH_CACHE_LENGHT];
 
 void flash_config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef  I2C_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1ENR_I2C1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBENR_DMA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFG, ENABLE);
	
	SYSCFG->CFGR|=(0x03<<21);
	
	
	VOICE_RESET_GPIO->BSRR=VOICE_RESET_PIN;

    I2C_DeInit(EEPROM_I2C);

    I2C_StructInit(&I2C_InitStruct);
    I2C_InitStruct.I2C_Mode       = I2C_Mode_MASTER;
    I2C_InitStruct.I2C_Speed      = I2C_Speed_STANDARD;
    I2C_InitStruct.I2C_ClockSpeed = 100000;
    I2C_Init(EEPROM_I2C, &I2C_InitStruct);

    I2C_Send7bitAddress(EEPROM_I2C, FLASH_I2C_ADDRESS);

    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_5);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_5);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    I2C_Cmd(EEPROM_I2C, ENABLE);
}

void fmc_erase_page(uint32_t address)
{
	__disable_irq();
	FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    FLASH_ErasePage(address);
	FLASH_ClearFlag(FLASH_FLAG_EOP);
	__enable_irq();
}
 /*!
 * program fmc word by word from address to address+nlen
*/
void fmc_program_words(uint32_t address, const void *buf, uint32_t nlen)
{
	uint32_t i,words;
	uint32_t *pdata=(uint32_t*)buf;

	if((address&0x07)!=0)
	{
		return;
	}
	
	if((nlen&0x03)!=0)
	{
		return;
	}
	__disable_irq();
	FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    FLASH_ErasePage(address);
	FLASH_ClearFlag(FLASH_FLAG_EOP);
	
	words= nlen>>2;
    /* program flash */
	for(i = 0; i < words; i++)
	{
		FLASH_ProgramWord(address, *pdata);
		FLASH_ClearFlag(FLASH_FLAG_EOP);
        address +=  sizeof(uint32_t);
        FLASH_ClearFlag(FLASH_FLAG_EOP);
		pdata++;
	}
  
    FLASH_Lock();
	__enable_irq();
}

/***********************************************************************************************************************
  * @brief
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void flash_tx_data(uint8_t *buffer, uint8_t length)
{
    uint8_t i = 0;
	uint16_t wait=EEPROM_WAIT;

    for (i = 0; i < length; i++)
    {
        I2C_SendData(I2C1, buffer[i]);

		wait=EEPROM_WAIT;
        while (RESET == I2C_GetFlagStatus(I2C1, I2C_STATUS_FLAG_TFE))
        {
			if((wait--)==0)
			{
				return;
			}
        }
    }
}

/***********************************************************************************************************************
  * @brief
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void flash_epprom_write(uint16_t address, uint8_t *buffer, uint8_t length)
{
	uint8_t addr[2];
	uint16_t wait=EEPROM_WAIT;
	
	flash_config();

    addr[0]=(address>>8);
	addr[1]=address;
    flash_tx_data(addr, 2);

    flash_tx_data((uint8_t *)buffer, length);

	wait=EEPROM_WAIT;
    while (RESET == I2C_GetFlagStatus(I2C1, I2C_STATUS_FLAG_TFE))
    {
		if((wait--)==0)
		{
			VOICE_RESET_GPIO->BRR=VOICE_RESET_PIN;
			return;
		}
    }

    I2C_GenerateSTOP(I2C1, ENABLE);

	wait=EEPROM_WAIT;
    while (RESET == I2C_GetITStatus(I2C1, I2C_IT_STOP_DET))
    {
		if((wait--)==0)
		{
			VOICE_RESET_GPIO->BRR=VOICE_RESET_PIN;
			return;
		}
    }
	VOICE_RESET_GPIO->BRR=VOICE_RESET_PIN;
}

/***********************************************************************************************************************
  * @brief
  * @note   none
  * @param  none
  * @retval none
  *********************************************************************************************************************/
void flash_rx_data(uint8_t *buffer, uint16_t length)
{
    uint16_t i = 0;
	uint16_t wait=EEPROM_WAIT;

    for (i = 0; i < length; i++)
    {
        I2C_ReadCmd(I2C1);
		
		wait=EEPROM_WAIT;
        while (RESET == I2C_GetFlagStatus(I2C1, I2C_STATUS_FLAG_RFNE))
        {
			if((wait--)==0)
			{
				return;
			}
        }

        buffer[i] = I2C_ReceiveData(I2C1);
    }
}

void flash_epprom_read(uint16_t address, uint8_t *buffer, uint16_t length)
{
	uint8_t addr[2];
	uint16_t wait=EEPROM_WAIT;
	
	flash_config();

	addr[0]=(address>>8);
	addr[1]=address;
    flash_tx_data((uint8_t *)&addr, 2);

    flash_rx_data((uint8_t *)buffer, length);

    I2C_GenerateSTOP(I2C1, ENABLE);

    while (!I2C_GetITStatus(I2C1, I2C_IT_STOP_DET))
    {
		if((wait--)==0)
		{
			VOICE_RESET_GPIO->BRR=VOICE_RESET_PIN;
			return;
		}
    }

	VOICE_RESET_GPIO->BRR=VOICE_RESET_PIN;
}

void flash_write_epprom_set_info(set_info_t *sit)
{
	flash_config();
	
	flash_epprom_write(0,&flash_buffer[0],FLASH_CACHE_LENGHT);
	
	VOICE_RESET_GPIO->BRR=VOICE_RESET_PIN;
}

void flash_read_epprom_run_info(run_info_t *rit)
{
	uint8_t i=0;
	uint16_t checksum=0;
	run_info_t epprom_rit;
	run_info_t *s = (run_info_t *)(&epprom_rit);
	uint32_t *s32= (uint32_t *)(&epprom_rit);

	flash_epprom_read(sizeof(epprom_set_info_t),(uint8_t *)&epprom_rit,sizeof(run_info_t));

	for(i=0;i<((sizeof(run_info_t)/4)-1);i++)
	{
		checksum+=*s32;
		s32++;
	}
	
	if((checksum==s->checksum)&&(s->version==SOFTWARE_DEBUG))
	{
		*rit=*s;
		return;
	}
	
	rit->run_time_s=0;	
	rit->start_key_count=0;	
	rit->keep_key_count=0;
	rit->remove_count=0;
	rit->boiled_count=0;
	rit->keep_count=0;
	rit->u01_count=0;
	rit->u02_count=0;
	rit->u03_count=0;
	rit->e01_count=0;
	rit->e02_001_count=0;
	rit->e02_002_count=0;
	rit->e03_count=0;
	rit->e04_count=0;
	rit->relay1_count=0;
	rit->relay1_time_s=0;
	rit->relay2_count=0;
	rit->relay2_time_s=0;
	rit->version=SOFTWARE_DEBUG;
	rit->checksum=0;
	
	s32= (uint32_t *)rit;
	for(i=0;i<((sizeof(run_info_t)/4)-1);i++)
	{
		rit->checksum+=*s32;
		s32++;
	}

	flash_config();
	
	flash_epprom_write(sizeof(epprom_set_info_t),(uint8_t *)rit,sizeof(run_info_t));
	VOICE_RESET_GPIO->BRR=VOICE_RESET_PIN;

}

void flash_read_epprom_set_info(set_info_t *sit)
{
	uint8_t i=0;
	uint16_t checksum=0;
	epprom_set_info_t epprom_sit;
	epprom_set_info_t *s = (epprom_set_info_t *)(&epprom_sit);
	uint8_t *s8= (uint8_t *)(&epprom_sit);
	
	flash_epprom_read(0,(uint8_t *)&epprom_sit,sizeof(epprom_set_info_t));
	
	for(i=0;i<(sizeof(epprom_set_info_t)-4);i++)
	{
		checksum+=*s8;
		s8++;
	}
	sit->led_mode=0;
	sit->pid_duty=0;
	sit->seg_duty=LED_MAX_DUTY;
	sit->indicator_duty=LED_MAX_DUTY;
	sit->seg_duty=LED_MAX_DUTY;

	if((checksum==s->checksum)&&(s->version==SOFTWARE_DEBUG))
	{
		sit->temperature=s->temperature;
		sit->temperature_unit=s->temperature_unit;
		sit->music_type=s->music_type;
		sit->ring_duty=s->ring_duty;
		sit->version=s->version;
		return;
	}
	
	sit->temperature=100;
	sit->temperature_unit=TEMP_UNIT_TYPE_CELSIYS;
	sit->music_type=1;
	sit->ring_duty=LED_MAX_DUTY;
	sit->version=SOFTWARE_DEBUG;
	sit->checksum=0;

	s->temperature=sit->temperature;
	s->temperature_unit=sit->temperature_unit;
	s->music_type=sit->music_type;
	s->ring_duty=sit->ring_duty;
	s->reserved1=0xFF;
	s->reserved2=0xFF;
	s->reserved3=0xFF;
	s->reserved4=0xFF;
	s->reserved5=0xFF;
	s->reserved6=0xFF;
	s->reserved7=0xFF;
	s->version=sit->version;

	s->checksum=0;
	s8= (uint8_t *)s;
	for(i=0;i<(sizeof(epprom_set_info_t)-4);i++)
	{
		s->checksum+=*s8;
		s8++;
	}

	flash_epprom_write(0,(uint8_t *)s,sizeof(epprom_set_info_t));
}

void flash_write_set_info(set_info_t *sit)
{
	uint8_t i=0;
	epprom_set_info_t epprom_sit;
	epprom_set_info_t *s = (epprom_set_info_t *)(&epprom_sit);
	uint8_t *s8= (uint8_t *)(&epprom_sit);
	
	s->temperature=sit->temperature;
	s->temperature_unit=sit->temperature_unit;
	s->music_type=sit->music_type;
	s->ring_duty=sit->ring_duty;
	s->reserved1=0xFF;
	s->reserved2=0xFF;
	s->reserved3=0xFF;
	s->reserved4=0xFF;
	s->reserved5=0xFF;
	s->reserved6=0xFF;
	s->reserved7=0xFF;
	s->version=sit->version;

	s->checksum=0;
	for(i=0;i<(sizeof(epprom_set_info_t)-4);i++)
	{
		s->checksum+=*s8;
		s8++;
	}
	
	flash_config();
	
	flash_epprom_write(0,(uint8_t *)(&epprom_sit),sizeof(epprom_set_info_t));
	VOICE_RESET_GPIO->BRR=VOICE_RESET_PIN;
}

void flash_write_run_info(run_info_t *rit)
{
	uint32_t *s32= (uint32_t *)(rit);
	uint8_t i=0;

	rit->checksum=0;
	for(i=0;i<((sizeof(run_info_t)/4)-1);i++)
	{
		rit->checksum+=*s32;
		s32++;
	}
	
	flash_config();

	flash_epprom_write(sizeof(epprom_set_info_t),(uint8_t *)rit,sizeof(run_info_t));
	VOICE_RESET_GPIO->BRR=VOICE_RESET_PIN;
	
}

void flash_reset_epprom_run_info(run_info_t *rit)
{
	rit->run_time_s=0;	
	rit->start_key_count=0;	
	rit->keep_key_count=0;
	rit->remove_count=0;
	rit->boiled_count=0;
	rit->keep_count=0;
	rit->u01_count=0;
	rit->u02_count=0;
	rit->u03_count=0;
	rit->e01_count=0;
	rit->e02_001_count=0;
	rit->e02_002_count=0;
	rit->e03_count=0;
	rit->e04_count=0;
	rit->relay1_count=0;
	rit->relay1_time_s=0;
	rit->relay2_count=0;
	rit->relay2_time_s=0;
	rit->checksum=0;
	flash_write_run_info(rit);
}

void flash_reset_epprom_set_info(set_info_t *sit)
{
	sit->temperature=100;
	sit->temperature_unit=0;
	sit->music_type=1;
	sit->ring_duty=LED_MAX_DUTY;
	sit->version=SOFTWARE_DEBUG;

	flash_write_set_info(sit);
}
#else

void flash_read_set_info(set_info_t *sit)
{
	set_info_t *s = (set_info_t *)(SET_INFO_ADDRESS);
	uint32_t *s32= (uint32_t *)(SET_INFO_ADDRESS);
	uint8_t i=0;
	uint16_t checksum=0;
	
	for(i=0;i<((sizeof(set_info_t)/4)-1);i++)
	{
		checksum+=*s32;
		s32++;
	}
	
	if(checksum==s->checksum)
	{
		*sit=*s;
		return;
	}
	
	s = (set_info_t *)(SET_INFO_ADDRESS+FLASH_PAGE_SIZE);
	s32= (uint32_t *)(SET_INFO_ADDRESS+FLASH_PAGE_SIZE);
	checksum=0;
	for(i=0;i<((sizeof(set_info_t)/4)-1);i++)
	{
		checksum+=*s32;
		s32++;
	}
	if(checksum==s->checksum)
	{
		*sit=*s;
		return;
	}

	sit->temperature=100;
	sit->temperature_unit=0;
	sit->music_type=1;
	sit->led_mode=0;
	sit->pid_duty=0;
	sit->ring_duty=LED_MAX_DUTY;
	sit->seg_duty=LED_MAX_DUTY;
	sit->indicator_duty=LED_MAX_DUTY;
	sit->checksum=0;
	s32= (uint32_t *)sit;
	for(i=0;i<((sizeof(set_info_t)/4)-1);i++)
	{
		sit->checksum+=*s32;
		s32++;
	}
	
	
	fmc_program_words(SET_INFO_ADDRESS,sit,sizeof(set_info_t));
	fmc_erase_page(SET_INFO_ADDRESS+FLASH_PAGE_SIZE);
}

void flash_write_set_info(set_info_t *sit)
{
	set_info_t *s = (set_info_t *)(SET_INFO_ADDRESS);
	uint32_t *s32= (uint32_t *)(SET_INFO_ADDRESS);
	
	uint8_t i=0;
	uint16_t checksum=0;
	
	for(i=0;i<((sizeof(set_info_t)/4)-1);i++)
	{
		checksum+=*s32;
		s32++;
	}
	sit->checksum=0;
	s32= (uint32_t *)(sit);
	for(i=0;i<((sizeof(set_info_t)/4)-1);i++)
	{
		sit->checksum+=*s32;
		s32++;
	}
	if(checksum!=s->checksum)
	{
		
		fmc_program_words(SET_INFO_ADDRESS,sit,sizeof(set_info_t));
		fmc_erase_page(SET_INFO_ADDRESS+FLASH_PAGE_SIZE);
		return;
	}

	fmc_program_words((SET_INFO_ADDRESS+FLASH_PAGE_SIZE),sit,sizeof(set_info_t));
	fmc_erase_page(SET_INFO_ADDRESS);
}



void flash_read_run_info(run_info_t *rit)
{
	run_info_t *s = (run_info_t *)(RUN_INFO_ADDRESS);
	uint32_t *s32= (uint32_t *)(RUN_INFO_ADDRESS);
	uint8_t i=0;
	uint16_t checksum=0;
	
	for(i=0;i<((sizeof(run_info_t)/4)-1);i++)
	{
		checksum+=*s32;
		s32++;
	}
	
	if(checksum==s->checksum)
	{
		*rit=*s;
		return;
	}
	
	s = (run_info_t *)(RUN_INFO_ADDRESS+FLASH_PAGE_SIZE);
	s32= (uint32_t *)(RUN_INFO_ADDRESS+FLASH_PAGE_SIZE);
	checksum=0;
	for(i=0;i<((sizeof(run_info_t)/4)-1);i++)
	{
		checksum+=*s32;
		s32++;
	}
	if(checksum==s->checksum)
	{
		*rit=*s;
		return;
	}
	
	rit->run_time_s=0;	
	rit->start_key_count=0;	
	rit->keep_key_count=0;
	rit->remove_count=0;
	rit->boiled_count=0;
	rit->keep_count=0;
	rit->u01_count=0;
	rit->u02_count=0;
	rit->u03_count=0;
	rit->e01_count=0;
	rit->e02_001_count=0;
	rit->e02_002_count=0;
	rit->e03_count=0;
	rit->e04_count=0;
	rit->relay1_count=0;
	rit->relay1_time_s=0;
	rit->relay2_count=0;
	rit->relay2_time_s=0;
	rit->checksum=0;
	
	s32= (uint32_t *)rit;
	for(i=0;i<((sizeof(run_info_t)/4)-1);i++)
	{
		rit->checksum+=*s32;
		s32++;
	}

	fmc_program_words(RUN_INFO_ADDRESS,rit,sizeof(run_info_t));
	fmc_erase_page(RUN_INFO_ADDRESS+FLASH_PAGE_SIZE);
}

void flash_write_run_info(run_info_t *rit)
{
	run_info_t *s = (run_info_t *)(RUN_INFO_ADDRESS);
	uint32_t *s32= (uint32_t *)(RUN_INFO_ADDRESS);
	
	uint8_t i=0;
	uint16_t checksum=0;
	
	for(i=0;i<((sizeof(run_info_t)/4)-1);i++)
	{
		checksum+=*s32;
		s32++;
	}
	rit->checksum=0;
	s32= (uint32_t *)(rit);
	for(i=0;i<((sizeof(run_info_t)/4)-1);i++)
	{
		rit->checksum+=*s32;
		s32++;
	}
	if(checksum!=s->checksum)
	{
		
		fmc_program_words(RUN_INFO_ADDRESS,rit,sizeof(run_info_t));
		fmc_erase_page(RUN_INFO_ADDRESS+FLASH_PAGE_SIZE);
		return;
	}

	fmc_program_words((RUN_INFO_ADDRESS+FLASH_PAGE_SIZE),rit,sizeof(run_info_t));
	fmc_erase_page(RUN_INFO_ADDRESS);
	
}
#endif





