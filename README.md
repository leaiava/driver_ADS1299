# driver_ADS1299
Driver para el dispositivo ADS1299 EEG front-end de Texas Instrument.

## Introduccion

El driver_ADS1299 esta realizado con una capa de abstraccion de hardware. Posee una estructura con punteros a funciones, por la cual logramos abstraernos del hardware y tener un driver portable. Las funciones para el hardware específico se ubican en un archivo fuente separado conocido como specific-port.
Junto con el driver se entrega un specifi-port basado en una placa EDU_CIAA.

## Requerimientos para el specific-Port

El specific port debe tener funciones para:
- Inicializar el hardware específico. 
- Controlar el GPIO de chip select.
- Escribir en el puerto SPI un byte.
- Leer del puerto SPI un byte.
- Generar retardos en microsegundos.
- Interrupción del GPIO que controlará la señal de data ready.

Opcionalmete:
- Controlar el GPIO de START.
- Controlar el GPIO de RESET.
- Controlar el GPIO de CLKSEL.
- Controlar el GPIO de PWDN.

Estos son opcionales porque se pueden controlar por hardware o por comandos del SPI. Dentro del archivo ads1299.h debemos colocar los siguientes defines en caso de querer utilizar estos controles por hardware
```
#define	USE_HARDWARE_START
#define USE_HARDWARE_RESET
#define	USE_HARDWARE_CLKSEL
#define USE_HARDWARE_PWDN
```
### Manejo de la interrupcion de GPIO

El specific-port debe manejar la interrupción del GPIO asignado para leer el pin Data Ready del ADS1299. 
El specific-port debe definir un puntero al flag de data ready
```
bool*	ptrFlagDataReady;
```
Y dentro de la interrupción se debe setear este flag para que el driver le pueda avisar al programa principal que tiene un nuevo dato.

Ejemplo de rutina de interrupción para la EDU_CIAA
```
void GPIO0_IRQHandler(void)
{
	//Verificamos que la interrupción es la esperada
	if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH0 )
	    {
		Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH0 ); //Borramos el flag de interrupción

		*ptrFlagDataReady = true;
		}
}

```

## Inicialización del driver

Para inicializar el driver hay que llamar a la función 
```
bool ads1299_InitDriver(ads1299_func_t config);
```
La cual carga en la estructura interna del driver los punteros a funcion del specific-port.
Para ello previamente debemos crear una variable de la estructura ads1299_func_t
```
ads1299_func_t estructura_driver;
```
Y cargar en la estructura los punteros a las funciones del specific-port. Por ejemplo:
```
	estructura_driver.hardwareInit_fnc = initHardware_CIAA_port;  //!< Función para inicializar el hardware.
	estructura_driver.chip_select_ctrl = chipSelect_CIAA_port;    //!< Función para controlar el chip select.
	estructura_driver.spi_write_fnc = spiWrite_CIAA_port;         //!< Función para escribir en el puerto SPI un byte.
	estructura_driver.spi_read_fnc = spiRead_CIAA_port;           //!< Función para leer del puerto SPI un byte.
	estructura_driver.delay_us_fnc = delayus_CIAA_port;           //!< Función para generar retardos en microsegundos.
	estructura_driver.start_ctrl = hardwareStart_CIAA_port;       //!< Función para controlar el pin de START.
	estructura_driver.reset_ctrl = hardwareReset_CIAA_port;       //!< Función para controlar el pin de RESET
	estructura_driver.clksel_ctrl = hardwareClkSel_CIAA_port;     //!< Función para controlar el pin de CLKSEL
	estructura_driver.pwdn_ctrl = hardwarePWDN_CIAA_port;         //!< Función para controlar el pin de PWDN
```
Luego hay que inicializar el hardware llamando a la función
```
	ads1299_initHardware();
```
## Ejemplo de lectura de datos
Luego de inicializar el driver y el hardware se debe iniciar la converison 
```
ads1299_continuousConversionStart();
```
Y dentro de un loop colocamos:
```
   while( true ) 
   {
      __WFI();                              //!< Queda a la espera de una interrupcion.
      if( ads1299_conversionReady() )       //!< Se verifica que la conversión esté lista
      {
    	  ads1299_readData( &ads1299_data );  //!< Se leen los datos de los 8 canales del ads1299
      }
   }
```
