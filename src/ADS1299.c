/*=============================================================================
 * Author: Leandro Arrieta <leandroarrieta@gmail.com>
 * Date: 2021/09/14
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "ADS1299.h"


/**
 * Se define una estructura interna al driver que no es visible al usuario.
 * Se utiliza para cargar las funciones definidas especificamente como port para
 * el hardware especifico.
 */
static ads1299_func_t ads1299_control;

/**
 * Flag para marcar que hay un dato listo, la aplicación lo lee a través de la
 * API ads1299_conversionReady.
 * Se inicializa en ads1299_InitDriver
 */
static bool flagDataReady;

/**
 * El ptrFlagDataReady es un puntero al flag flagDataReady.
 * Sirve para que la rutina de interrupción pueda levantar el flag indicando que hay un dato nuevo
 * Se debe declarar en el arcihvo port.
 * En ads1299_InitDriver se inicializa con la dirección de flagDataReady
 *
 */
extern	bool* ptrFlagDataReady;

/**
 * @brief	Inicializacion del driver ADS1299
 *
 * @details	Se copian los punteros a funciones pasados por argumentos a la estructura interna
 *   		del driver, la cual no puede ser accedida por el resto del programa
 * @param	config	: Estructura de configuracion para el driver.
 * @return  false	: Si algún puntero a función apunta a Null
 * 			true	: Si pudo cargar todos los punteros a funcion
 */
bool ads1299_InitDriver(ads1299_func_t config){
	if ( config.chip_select_ctrl == 0			/// Chequeo que ningun puntero a función apunte a Null.
		|| config.spi_read_fnc == 0
		|| config.spi_write_fnc == 0
		|| config.hardwareInit_fnc == 0
		|| config.delay_us_fnc == 0)
		return false;

#ifdef USE_HARDWARE_START
	if(config.start_ctrl == 0)
		return false;
#endif

#ifdef USE_HARDWARE_RESET
	if(config.reset_ctrl == 0)
		return false;
#endif

#ifdef USE_HARDWARE_CLKSEL
	if(config.clksel_ctrl == 0)
		return false;
#endif

#ifdef USE_HARDWARE_PWDN
	if(config.pwdn_ctrl == 0)
		return false;
#endif

	ads1299_control.chip_select_ctrl = config.chip_select_ctrl;
	ads1299_control.spi_read_fnc = config.spi_read_fnc;
	ads1299_control.spi_write_fnc = config.spi_write_fnc;
	ads1299_control.hardwareInit_fnc = config.hardwareInit_fnc;
	ads1299_control.delay_us_fnc = config.delay_us_fnc;
	ads1299_control.clksel_ctrl = config.clksel_ctrl;
	ads1299_control.start_ctrl = config.start_ctrl;
	ads1299_control.reset_ctrl = config.reset_ctrl;
	ads1299_control.pwdn_ctrl = config.pwdn_ctrl;
	flagDataReady = false;
	ptrFlagDataReady = &flagDataReady;					// cargo en el puntero del port la dirección del flag

	return true;
}
/**
 * @brief	Inicializa el hardware a utilizar.
 * @details	hardwareInit_fnc	: Debe inicializar el puerto SPI y los GPIOs a utilizarse
 */
void ads1299_initHardware(void){
	ads1299_control.hardwareInit_fnc();
	ads1299_control.delay_us_fnc(2);
	ads1299_control.chip_select_ctrl(CS_DISABLE);

#ifdef USE_HARDWARE_START
	ads1299_control.start_ctrl(START_DISABLE);
#endif

#ifdef USE_HARDWARE_RESET
	ads1299_control.reset_ctrl(RESET_DISABLE);
#endif

#ifdef USE_HARDWARE_CLKSEL
	ads1299_control.clksel_ctrl(INTERNAL_CLOCK);
#endif

	ads1299_writeRegister( CONFIG3 , ADS1299_CONFIG3_INTERNAL_REF_BUF_EN);
}

/**
 * @brief	Lee el registro ID del dispositivo
 *
 * @return	Registro ID del dispositivo
 */
uint8_t ads1299_readId(void){

uint8_t id = ads1299_readRegister(ID);
return id;
}

/**
 * @brief	Setea en el registro config1 la frecuencia de muestreo
 */
void ads1299_setDataRate(uint8_t config1Data){

	config1Data &= DATA_RATE_MASK;	// aplico mascara para solo tocar los bits del DATA_RATE
	uint8_t config1Reg = ads1299_readRegister(CONFIG1);
	config1Reg &= ~DATA_RATE_MASK;	// pongo en cero los bits del DATA_RATE
	config1Reg |= config1Data;		// aplico la configuracion del DATA_RATE sin modificar el resto del registro
	ads1299_writeRegister(CONFIG1, config1Reg);
}

/**
 * @brief	Lee un regístro interno del ads1299
 * @param	registro	: Registro a leer del tipo registers_t.
 * @return	valor leido del registro
 */
uint8_t ads1299_readRegister(registers_t registro){
	uint8_t	readValue;
	ads1299_control.chip_select_ctrl(CS_ENABLE);			///< Necesita 6nS entre CS y que comience el SCLK,
	ads1299_control.spi_write_fnc( SDATAC_CMD);				///< Necesario si esta en RDATAC mode
	ads1299_control.spi_write_fnc( RREG_CMD | registro );	///< Hago la OR para tener el valor del comando compuesto
	ads1299_control.spi_write_fnc( NULL_CMD);				///< Indica que leo solo un registro
	readValue = ads1299_control.spi_read_fnc();
	ads1299_control.delay_us_fnc(2);						///< Se requiere antes de modificar el CS
	ads1299_control.chip_select_ctrl(CS_DISABLE);
	return readValue;
}
/**
 * @brief	Escribe en un regístro interno del ads1299
 * @param	registro	: Registro a escribir del tipo registers_t.
 *
 */
void ads1299_writeRegister(registers_t registro, uint8_t data){
	ads1299_control.chip_select_ctrl(CS_ENABLE);
	ads1299_control.spi_write_fnc( SDATAC_CMD);					///< Necesario si esta en RDATAC mode
	ads1299_control.spi_write_fnc( WREG_CMD | registro );		///< Hago la OR para tener el valor del comando compuesto
	ads1299_control.spi_write_fnc( NULL_CMD);					///< Indica que leo solo un registro
	ads1299_control.spi_write_fnc( data );
	ads1299_control.delay_us_fnc(2);							///< Se requiere antes de modificar el CS
	ads1299_control.chip_select_ctrl(CS_DISABLE);
}

/**
 * @brief	Envía un comando del tipo commands_t al ads1299
 *
 * @return	TRUE :	Si el comando recibido es valido devuelve true y envía el comando
 * 			FALSE:	Si el comando recibido no es válido devuelve FALSE y no envía nada
 */
static bool ads1299_sendCommand(commands_t comando){
	if (comando <= RDATA_CMD){
		ads1299_control.chip_select_ctrl(CS_ENABLE);	///< Necesita 6nS entre CS y que comience el SCLK,
														///  una instruccion son 4,9nS a 204MHz, hay mas de una instruccion acá.
		ads1299_control.spi_write_fnc( comando );
		ads1299_control.delay_us_fnc(2);				///< Se requiere antes de modificar el CS
		ads1299_control.chip_select_ctrl(CS_DISABLE);
		return true;
	}
	else
		return false;
}

/**
 * @brief	Lee el canal de status y los 8 canales.
 *
 * @param	ptrads1299_data	: Puntero a estructura donde se guardan los datos adquiridos.
 */
void ads1299_readData(ads1299_data_t* ptrads1299_data){
	uint32_t data;
	ads1299_control.chip_select_ctrl(CS_ENABLE);
	for (uint8_t i=0 ; i < (CANTIDAD_DE_CANALES + 1) ; i++){
		data = 0;
		data = (uint32_t) ads1299_control.spi_read_fnc();
		data <<= 8;
		data |= (uint32_t) ads1299_control.spi_read_fnc();
		data <<= 8;
		data |= (uint32_t) ads1299_control.spi_read_fnc();
		ptrads1299_data->data[i] = (int32_t) data;
	}
	ads1299_control.delay_us_fnc(2);
	ads1299_control.chip_select_ctrl(CS_DISABLE);
}

/**
 * @brief	Setea el modo de conversión continua e inicia la conversion
 */
void ads1299_continuousConversionStart(void){

	ads1299_sendCommand(SDATAC_CMD);
	ads1299_sendCommand(RDATAC_CMD);

#ifdef	USE_HARDWARE_START
	ads1299_control.start_ctrl(START_ENABLE);
	ads1299_control.delay_us_fnc(9);
	ads1299_control.start_ctrl(START_DISABLE);

#else
	ads1299_sendCommand(START_CMD);
#endif

}

/**
 * @brief	Énvía comando para detener la conversion
 */
void ads1299_conversionStop(void){
	ads1299_sendCommand(SDATAC_CMD);
}

/**
 * @brief	Verifica el flag de conversión lista.
 *
 * @return	true	: Si la conversión esta lista y borra el flag
 * 			false	: si no está lista la conversión
 */
bool ads1299_conversionReady(void){
	if (flagDataReady) {
		flagDataReady = false;
		return true;
	}

	return false;
}

/**
 * @brief	Setea los 8 canales con la misma palabra de configuracion
 * @param	CHnSETsettings	: OR entre los defines del tipo CHnSET settings.
 */
void ads1299_CHallSET(uint8_t CHnSETsettings){
	for ( uint8_t i = CH1SET ; i < (CH1SET+ CANTIDAD_DE_CANALES) ; i++){
		ads1299_writeRegister( i , CHnSETsettings);
	}
}

/**
 * @brief	Selecicona si se utilizará el clock interno o externo
 */
#ifdef USE_HARDWARE_CLKSEL
bool ads1299_clockSource( clkSel_t clksel){
	ads1299_control.clksel_ctrl (clksel);
	return true;
}
#endif

/**
 * @brief	Reset device
 * @detail	After a reset, 18tCLK cycles are required to complete initialization of the
 *	 		configuration registers to default states and start the conversion cycle.
 */

void ads1299_RESET(void){

#ifdef	USE_HARDWARE_RESET
	ads1299_control.reset_ctrl(RESET_ENABLE);
	ads1299_control.delay_us_fnc(2);
	ads1299_control.reset_ctrl(RESET_DISABLE);
#else
	ads1299_sendCommand(RESET_CMD);
#endif
	ads1299_control.delay_us_fnc(9); // delay de 18TCLk (Si se usa el clk interno es de 2.048MHz
}

#ifdef	USE_HARDWARE_START
/**
 * @brief	Sincroniza multiples ADS1299
 */
void ads1299_sync(void){

	ads1299_control.start_ctrl(START_ENABLE);
	ads1299_control.delay_us_fnc(2);
	ads1299_control.start_ctrl(START_DISABLE);

}
#endif

#ifdef	USE_HARDWARE_PWDN
/**
 * @brief	Enter in power-done mode to save power
 * @detail	When PWDN is ENABLE, all on-chip circuitry is powered down
 * 			the external clock is recommended to be shut down to save power.
 *
 * @param	estado : PWDN_ENABLE para deshabilitar el dispositivo
 * 					 PWDN_DISABLE para habilitar el dispositivo
 */
void ads1299_PWDN(pwdnState_t estado){
	ads1299_control.pwdn_ctrl(estado);
	ads1299_control.delay_us_fnc(9);
}
#endif



