/*=============================================================================
 * Author: Leandro Arrieta <leandroarrieta@gmail.com>
 * Date: 2021/09/14
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "ADS1299.h"

#include "sapi.h"

/**
 * Se define una estructura interna al driver que no es visible al usuario.
 * Se utiliza para cargar las funciones definidas especificamente como port para
 * el hardware especifico.
 */
static ads1299_t ads1299_control;

void ads1299_InitDriver(ads1299_t config){
	ads1299_control.chip_select_ctrl = config.chip_select_ctrl;
	ads1299_control.command_send_fnc = config.command_send_fnc;

	ads1299_control.clkSel_ctrl = config.clkSel_ctrl;
}

void ads1299_clockSource( clkSel_t clksel){
		ads1299_control.clkSel_ctrl(clksel);
}
/**
 * @brief	Begin conversions.
 * @detail	if USE_HARDWARE_START is defined, use the START pin.
 * 			else use the start command (START pin must tied low)
 */
void ads1299_START(void){

	//verifico que la función no sea NULL
	if(ads1299_control.start_ctrl == NULL)
			while(1);

#ifdef	USE_HARDWARE_START
	ads1299_control.start_ctrl(START_ENABLE);
	agregar delay de 2TCLK
	ads1299_control.start_ctrl(START_DISABLE);

#else
	ads1299_control.start_ctrl(START_DISABLE);
	ads1299_CommandSend(START_CMD);
#endif
}

/**
 * @brief	Reset device
 * @detail	After a reset, 18tCLK cycles are required to complete initialization of the
 *	 		configuration registers to default states and start the conversion cycle.
 */
void ads1299_RESET(void){

	//verifico que la función no sea NULL
	if(ads1299_control.reset_ctrl == NULL)
		while(1);

#ifdef	USE_HARDWARE_RESET
	ads1299_control.reset_ctrl(RESET_ENABLE);
	ver cuanto tiempo hay que esperar
	ads1299_control.reset_ctrl(RESET_DISABLE);
#else
	ads1299_CommandSend(RESET_CMD);
	agregar delay de 18TCLk
#endif
}

/**
 * @brief	Enter in power-done mode to save power
 * @detail	When PWDN is ENABLE, all on-chip circuitry is powered down
 * 			the external clock is recommended to be shut down to save power.
 */
void ads1299_PWDN(pwdnState_t state){

	//verifico que la función no sea NULL
	if(ads1299_control.pwdn_ctrl == NULL)
		while(1);

	ads1299_control.pwdn_ctrl(state);
	ver cuanto tiempo hay que esperar cuando se habilita
}

/**
 * @brief	delay para cumplir con el tiempo tSDECODE que requiere el ads1299
 * @detail	Un comando demora 4tCLK para decodificarse, se requiere que pase
 * 			este tiempo antes de tener otro comando listo
 * 			Un byte se transfiere en 8tSCLK por lo tanto
 * 			8tSCLK > 4tCLK
 * 			2tSCLK > tCLK	Si se cumple esto no requiero delay
 * 			Si no se cumple hay que agregar un delay
 * 			2tSCLK + tDELAY > tCLK
 * 			tDELAY > tCLK - 2tSCLK
 */
static void ads1299_delay_tSDECODE(void){

}
/**
 * @brief	delay fijo de 4tCLK para cumplir con requisitos temporales.
 */
static void ads1299_delay_4tCLK(void){

}

/**
 * @brief	Lee un regístro interno del ads1299
 * @param	registro	: Registro a leer del tipo registers_t.
 * @return	valor leido del registro
 */
uint8_t ads1299_readRegister(registers_t registro){
	uint8_t	readValue;
	ads1299_control.chip_select_ctrl(CS_ENABLE);
	ads1299_delay_4tCLK();									///< Se requiere despues de modificar el CS
	ads1299_control.spi_write_fnc( SDATAC_CMD);				///< Necesario si esta en RDATAC mode
	ads1299_delay_tSDECODE();
	ads1299_control.spi_write_fnc( RREG_CMD | registro );	///< Hago la OR para tener el valor del comando compuesto
	ads1299_delay_tSDECODE();
	ads1299_control.spi_write_fnc( NULL_CMD);				///< Indica que leo solo un registro
	ads1299_delay_tSDECODE();
	readValue = ads1299_control.spi_read_fnc();
	ads1299_delay_4tCLK();									///< Se requiere antes de modificar el CS
	ads1299_control.chip_select_ctrl(CS_DISABLE);
	return readValue;
}

void ads1299_writeRegister(registers_t registro){
	ads1299_control.chip_select_ctrl(CS_ENABLE);
	ads1299_delay_4tCLK();									///< Se requiere despues de modificar el CS
	ads1299_control.spi_write_fnc( SDATAC_CMD);				///< Necesario si esta en RDATAC mode
	ads1299_delay_tSDECODE();
	ads1299_control.spi_write_fnc( WREG_CMD | registro );	///< Hago la OR para tener el valor del comando compuesto
	ads1299_delay_tSDECODE();
	ads1299_control.spi_write_fnc( NULL_CMD);				///< Indica que leo solo un registro
	ads1299_delay_tSDECODE();
	readValue = ads1299_control.spi_read_fnc();
	ads1299_delay_4tCLK();									///< Se requiere antes de modificar el CS
	ads1299_control.chip_select_ctrl(CS_DISABLE);
}
