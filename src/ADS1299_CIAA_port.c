/*
 * ADS1299_CIAA_port.c
 *
 *  Created on: Sep 15, 2021
 *      Author: lea
 */

#include "ADS1299_CIAA_port.h"

/**
 * Puntero al Flag de Data Ready para indicarle al programa principal
 * que hay un nuevo dato.
 */
bool*	ptrFlagDataReady;

/**
 * @brief	Interrupción INT0 donde se conecta el GPIO de la pata Data Ready del Ads1299
 * @details	Esta función verifica que efectivamente sea la interrupcón correcta y pone en
 * 			alto el flag de data ready inicializado en ads1299_InitDriver
 */
void GPIO0_IRQHandler(void){
	//Verificamos que la interrupción es la esperada
	if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH0 )
	    {
		Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH0 ); //Borramos el flag de interrupción

		*ptrFlagDataReady = true;
		}
}

/**
 * @brief	Configuracion general del hardware EDU-CIAA para el modulo ADS1299
 * @details	GPIO0 CS en bajo
 * 			GPIO1 CLKSEL en alto
 * 			GPIO2 START en bajo
 * 			GPIO3 RESET en alto
 * 			GPIO4 DRDY entrada con pullup
 *
 */
void initHardware_CIAA_port(void)  {
	  /**
	  * Configuracion de los GPIOs a utilizar
	  */
	 gpioInit ( CS_PIN	  , GPIO_OUTPUT );
	 gpioInit ( CLKSEL_PIN, GPIO_OUTPUT );
	 hardwareClkSel_CIAA_port(INTERNAL_CLOCK);
	 gpioInit ( START_PIN , GPIO_OUTPUT );
	 hardwareStart_CIAA_port(START_DISABLE);
	 gpioInit ( RESET_PIN , GPIO_OUTPUT );
	 hardwareReset_CIAA_port(RESET_DISABLE);
	 gpioInit ( DRDY_PIN  , GPIO_INPUT_PULLUP );

	 /**
	  * Configuracion de la interrupcion del DRDY_PIN
	  */
	 Chip_SCU_GPIOIntPinSel( 0 ,  5 , 16 );	// DRDY_PIN (GPIO4 tiene PortNum=5 y PinNum=16 internamente en el LPC4337) a INT0 (numero 0)
	 Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 0 ) );	// Borra el pending de la IRQ INT0
	 Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 0 ) );	//Selecciona activo por flanco
	 Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( 0 ) );	//Selecciona activo por flanco descendente

	 NVIC_EnableIRQ( PIN_INT0_IRQn );
	 // ----- Inicializo el modulo SPI -----
	 /**
	 * Configuracion e Inicializacion de puerto SPI
	 */
	 Chip_SCU_PinMuxSet( 0xF, 4, (SCU_MODE_PULLUP | SCU_MODE_FUNC0)); // SSP1_SCK
	 Chip_SCU_PinMuxSet( 0x1, 3, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC5)); // SSP1_MISO
	 Chip_SCU_PinMuxSet( 0x1, 4, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC5)); // SSP1_MOSI

	 Chip_SSP_Init(SPI_CIAA);

	 Chip_SSP_SetFormat(SPI_CIAA, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_CPHA1_CPOL0);
	 Chip_SSP_SetBitRate(SPI_CIAA, BITRATE);

	 Chip_SSP_Enable( SPI_CIAA );

}

void chipSelect_CIAA_port(csState_t estado)  {

	switch(estado)  {

	case CS_ENABLE:
		gpioWrite( 	CS_PIN	, OFF );
		break;

	case CS_DISABLE:
		gpioWrite( 	CS_PIN	, ON );
		break;

	default:
		;
	}
}

void spiWrite_CIAA_port(commands_t dato){

	Chip_SSP_DATA_SETUP_T xferConfig;

	xferConfig.tx_data = &dato;
	xferConfig.tx_cnt  = 0;
	xferConfig.rx_data = NULL;
	xferConfig.rx_cnt  = 0;
	xferConfig.length  = 1;

	Chip_SSP_RWFrames_Blocking( SPI_CIAA, &xferConfig );
}

uint8_t spiRead_CIAA_port(void){

	Chip_SSP_DATA_SETUP_T xferConfig;
	uint8_t buffer;
	uint8_t tx_data=0;
	bool_t res;

	xferConfig.tx_data = &tx_data;
	xferConfig.tx_cnt  = 0;
	xferConfig.rx_data = &buffer;
	xferConfig.rx_cnt  = 0;
	xferConfig.length  = 1;

	res = Chip_SSP_RWFrames_Blocking(SPI_CIAA,  &xferConfig);

	return buffer;
}
void delayus_CIAA_port(uint8_t microSeg){
	uint32_t x=0;
	while(x < 23*microSeg)
		x++;
}
void hardwareStart_CIAA_port (startState_t estado){

	switch(estado)  {

	case START_DISABLE:
		gpioWrite( 	START_PIN	, OFF );
		break;

	case START_ENABLE:
		gpioWrite( 	START_PIN	, ON );
		break;

	default:;
	}
}
void hardwareReset_CIAA_port (resetState_t estado){

	switch(estado)  {

	case RESET_ENABLE:
		gpioWrite( 	RESET_PIN	, OFF );
		break;

	case RESET_DISABLE:
		gpioWrite( 	RESET_PIN	, ON );
		break;

	default:;
	}
}

void hardwareClkSel_CIAA_port(clkSel_t estado){

	switch(estado)  {

	case EXTERNAL_CLOCK:
		gpioWrite( 	CLKSEL_PIN	, OFF );
		break;

	case INTERNAL_CLOCK:
		gpioWrite( 	CLKSEL_PIN	, ON );
		break;

	default:;
	}
}
