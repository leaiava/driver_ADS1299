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


}



