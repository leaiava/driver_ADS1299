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
double calculaTemp ( int32_t dataCH);
int main( void )
{

	ads1299_func_t estructura_driver;
	ads1299_data_t ads1299_data;

	// ----- Setup -----------------------------------
	boardInit();
	uartInit( UART_USB, 115200 );
	/**
   	 * Cargamos todos los punteros a la estructura con la que se
   	 * inicializa el driver
   	 */
	estructura_driver.hardwareInit_fnc = initHardware_CIAA_port;	//!< Función para inicializar el hardware.
	estructura_driver.chip_select_ctrl = chipSelect_CIAA_port;		//!< Función para controlar el chip select.
	estructura_driver.spi_write_fnc = spiWrite_CIAA_port;			//!< Función para escribir en el puerto SPI un byte.
	estructura_driver.spi_read_fnc = spiRead_CIAA_port;				//!< Función para leer del puerto SPI un byte.
	estructura_driver.delay_us_fnc = delayus_CIAA_port;				//!< Función para generar retardos en microsegundos.
	estructura_driver.start_ctrl = hardwareStart_CIAA_port;			//!< Función para controlar el pin de START.
	estructura_driver.reset_ctrl = hardwareReset_CIAA_port;			//!< Función para controlar el pin de RESET
	estructura_driver.clksel_ctrl = hardwareClkSel_CIAA_port;		//!< Función para controlar el pin de CLKSEL

	/**
	 * Inicializamos el driver
	 */
	ads1299_InitDriver(estructura_driver);
	ads1299_initHardware();

	//lectura[0] = ads1299_readId();
	ads1299_setDataRate(ADS1299_CONFIG1_DATA_RATE_500);

	ads1299_CHallSET(ADS1299_CH_N_TEMP_SENSOR);

	ads1299_continuousConversionStart();

	uint16_t i=0;
	uint16_t j=0;

   // ----- Repeat for ever -------------------------
   while( true ) {

      __WFI();									//!< El ADS1299 genera una señal de interrupción para despertar al MCU
      if( ads1299_conversionReady() ){			//!< Se verifica que la conversión esté lista
    	  ads1299_readData( &ads1299_data );	//!< Se leen los datos de los 8 canales del ads1299

    	  i++;
		   if (i == 1000){
			   for (uint8_t indice = 1 ; indice < CANTIDAD_DE_CANALES+1 ; indice++)
				   printf("%f  ",calculaTemp(ads1299_data.data[indice]));
			   printf("\n");
			   i = 0;

			   ads1299_CHallSET(ADS1299_CH_N_TEMP_SENSOR);
			   j^=1;
			   ads1299_writeRegister( CH1SET +j , ADS1299_CH_N_POWER_DONE);
			   ads1299_writeRegister( CH3SET +j , ADS1299_CH_N_POWER_DONE);
			   ads1299_writeRegister( CH5SET +j , ADS1299_CH_N_POWER_DONE);
			   ads1299_writeRegister( CH7SET +j , ADS1299_CH_N_POWER_DONE);

			   ads1299_continuousConversionStart();

		   }
      }
   }

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
   // case of a PC program.
   return 0;
}
/**
 * @brief Pasa el valor del canal medido a Temp
 */
double calculaTemp ( int32_t dataCH){
	double ret;
	ret = ( ((0.536441803*dataCH) - 145300 )/490)+25;
	if (ret > 0)
		return ret;
	return 0;
}
