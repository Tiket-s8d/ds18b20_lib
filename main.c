#include "stm32f1xx_hal.h"

#include "ds18.h"

#define MAX_SENSORS 2

Sensor sensors[MAX_SENSORS];										// array of structures to store sensors data

void Init_Sensors () {													// reset all values in sensors
	for (uint8_t i = 0; i < MAX_SENSORS; i++){
		sensors[i].raw_temp = 0x0;
		sensors[i].temp = 0.0;
		sensors[i].crc8_rom = 0x0;
		sensors[i].crc8_data = 0x0;
		sensors[i].crc8_rom_error = 0x0;
		sensors[i].crc8_data_error = 0x0;
		for (uint8_t j = 0; j < 8; j++){
			sensors[i].ROM_code[j] = 0x00;
		}
		for (uint8_t j = 0; j < 9; j++){
			sensors[i].scratchpad_data[j] = 0x00;
		}
	}
}

int main(void)
{			
	uint8_t devFound = 1, devCount = 0, i = 0;
	SysTick_Config(SystemCoreClock / 1000000);   																																						// SysTick 1 mcsec interrupts
															 																																								
	ds18_PortInit();																																																			//	Init PortB.11 for data transfering
	while (ds18_Reset()) ;																																																// 	If ds18 exists then continue
						
	devCount = Search_ROM(0xF0,&sensors);	 																																										// 	Init Search ROM routine
			   																																																										
	while (1) {
		for (i = 0; i < devCount; i++) {
			if (!sensors[i].crc8_data_error) {
				ds18_ConvertTemp(1, sensors[i].ROM_code);																																// 	Measure Temperature (Send Convert Temperature Command)
			}
			DelayMicro(750000); 																																														// 	Delay for conversion (max 750 ms for 12 bit)
			ds18_ReadStratchpad(1, sensors[i].scratchpad_data, sensors[i].ROM_code); 																		// 	Read measured temperature
			sensors[i].crc8_data = Compute_CRC8(sensors[i].scratchpad_data,8);																							// 	Compute CRC for data
			sensors[i].crc8_data_error = Compute_CRC8(sensors[i].scratchpad_data,9) == 0 ? 0 : 1;														// 	Get CRC Error Signal
			if (!sensors[i].crc8_data_error) {																																							// 	Check if data correct
				sensors[i].raw_temp = ((uint16_t)sensors[i].scratchpad_data[1] << 8) | sensors[i].scratchpad_data[0]; 			// 	Get 16 bit temperature with sign
				sensors[i].temp = sensors[i].raw_temp * 0.0625;																															// 	Convert to float
			}
			DelayMicro(100000);
		}
	}
}
