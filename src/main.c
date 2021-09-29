/*=============================================================================
 * Author: Leandro Arrieta <leandroarrieta@gmail.com>
 * Date: 2021/09/14
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "ADS1299_CIAA_port.h"
#include "ADS1299.h"

#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/


uint8_t	registro;
int main( void )
{
	ads1299_func_t estructura_driver;
	ads1299_data_t ads1299_data;

	// ----- Setup -----------------------------------
	boardInit();
	/**
   	 * Cargamos todos los punteros a la estructura con la que se
   	 * inicializa el driver
   	 */
	estructura_driver.hardwareInit_fnc = initHardware_CIAA_port;
	estructura_driver.chip_select_ctrl = chipSelect_CIAA_port;
	estructura_driver.spi_write_fnc = spiWrite_CIAA_port;
	estructura_driver.spi_read_fnc = spiRead_CIAA_port;
	estructura_driver.delay_us_fnc = delayus_CIAA_port;
	estructura_driver.start_ctrl = hardwareStart_CIAA_port;
	estructura_driver.reset_ctrl = hardwareReset_CIAA_port;
	estructura_driver.clksel_ctrl = hardwareClkSel_CIAA_port;

	/**
	 * Inicializamos el driver
	 */
	ads1299_InitDriver(estructura_driver);
	ads1299_initHardware();
	//lectura[0] = ads1299_readId();
	ads1299_setDataRate(ADS1299_CONFIG1_DATA_RATE_1000);
	ads1299_writeRegister( CONFIG2 , ADS1299_CONFIG2_INTERNAL_TEST_SIGNAL);
	ads1299_CHallSET(ADS1299_CH_N_INPUT_SHORTED);
	ads1299_writeRegister( CH1SET , ADS1299_CH_N_POWER_DONE);
	ads1299_writeRegister( CH3SET , ADS1299_CH_N_POWER_DONE);
	ads1299_writeRegister( CH5SET , ADS1299_CH_N_POWER_DONE);
	ads1299_writeRegister( CH7SET , ADS1299_CH_N_POWER_DONE);

	ads1299_continuousConversionStart();

	uint16_t i=0;
   // ----- Repeat for ever -------------------------
   while( true ) {
	   i++;
	   if (i == 1000){
		   i = 0;
		   gpioToggle(LED);
	   }

      __WFI();
      if( ads1299_conversionReady() ){
    	  ads1299_readData( &ads1299_data );
      }


   }

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
   // case of a PC program.
   return 0;
}
