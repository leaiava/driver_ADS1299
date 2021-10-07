/*
 * ADS1299_CIAA_port.h
 *
 *  Created on: Sep 15, 2021
 *      Author: lea
 */

#ifndef ADS1299_CIAA_PORT_H_
#define ADS1299_CIAA_PORT_H_

#include "ADS1299.h"
#include "chip.h"
#include "sapi.h"


#define	CS_PIN		GPIO0
#define	CLKSEL_PIN	GPIO1
#define	START_PIN	GPIO2
#define RESET_PIN	GPIO3
#define	DRDY_PIN	GPIO4
#define	SPI_CIAA	LPC_SSP1
#define BITRATE		1000000

void initHardware_CIAA_port(void);
void chipSelect_CIAA_port(csState_t estado);
void spiWrite_CIAA_port(commands_t dato);
uint8_t spiRead_CIAA_port(void);
void delayus_CIAA_port(uint8_t microSeg);
void hardwareStart_CIAA_port (startState_t estado);
void hardwareReset_CIAA_port (resetState_t estado);
void hardwareClkSel_CIAA_port(clkSel_t estado);

#endif /* ADS1299_CIAA_PORT_H_ */
