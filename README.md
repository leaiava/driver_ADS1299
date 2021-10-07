# driver_ADS1299
Driver para el dispositivo ADS1299 EEG front-end de Texas Instrument.

## Introduccion

El driver_ADS1299 esta realizado con una capa de abstraccion de hardware. Posee una estructura con punteros a funciones, por la cual logramos abstraernos del hardware y tener un driver portable. Las funciones para el hardware específico se ubican en un archivo fuente separado conocido como specific-port.
Junto con el driver se entrega un specific-port basado en una placa EDU_CIAA.

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
## Uso del driver
Para utilizar el ADS1299 debemos seguir los siguientes pasos
1. Inicializar el driver
2. Inicializar el Hardware
3. Configurar el ADS1299
4. Iniciar la conversión continua
5. Esperar la interrupción de conversión lista
6. Verificar que la conversión termino y leer los datos.

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
## Inicializar el Hardware

Para inicializar el hardware llamar a la siguiente función que hace uso del specific-port
```
	ads1299_initHardware();
```
## Configurar el ADS1299
El driver incluye funciones para configurar los registros internos del ADS1299, algunas específicas como
```
void ads1299_setDataRate(uint8_t config1Data)
```
que setea solo los bits correspondientes a la frecuencia de muestreo.

También hay dos funciones genéricas para leer y escribir sobre cualquier registro
```
uint8_t ads1299_readRegister(registers_t registro)
void ads1299_writeRegister(registers_t registro, uint8_t data)
```
En el archivo ADS1299.h tenemos un enum con todos los registros y defines con las palabras de configuración para escribir los registros.

## Iniciar la conversión continua
El driver incluye una API para iniciar la conversión continua
```
ads1299_continuousConversionStart();
```
Esta función le envía los comandos necesarios al ADS1299 para que inicie la conversión. El ADS1299 indicará a través de un GPIO que tiene los datos listos.

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


## Ejemplo de lectura de datos

Dentro de un loop en el programa principal esperamos por una interrupcion, se verifica que la conversión este lista y se leen los datos del ADS1299.:
```
   while( true ) 
   {
      __WFI();                              //!< Queda a la espera de una interrupcion.
      if( ads1299_conversionReady() )       //!< Se verifica que la conversión esté lista
      {
    	  ads1299_readData( &ads1299_data );  //!< Se leen los datos de todos los canales del ads1299
      }
   }
```
nota: Al leer los datos del ADS1299, el dispositivo siempre nos entrega los datos de todos los canales, no importa que hayan canales en power done, en ese caso esos canales se leeran como cero.
