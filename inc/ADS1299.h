/*=============================================================================
 * Author: Leandro Arrieta <leandroarrieta@gmail.com>
 * Date: 2021/09/14
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef INC_ADS1299_H__
#define INC_ADS1299_H__

#define	USE_HARDWARE_START
#define USE_HARDWARE_RESET

typedef enum {
	CS_ENABLE=0,
	CS_DISABLE
}csState_t;

typedef enum {
	EXTERNAL_CLOCK=0,
	INTERNAL_CLOCK
}clkSel_t;

typedef enum {
	START_DISABLE=0,
	START_ENABLE
}startState_t;

typedef enum {
	RESET_ENABLE=0,
	RESET_DISABLE
}resetState_t;

typedef enum {
	PWDN_ENABLE=0,
	PWDN_DISABLE
}pwdnState_t;

typedef enum {
	//System Commands
	NULL_CMD	= 0x00,
	WAKEUP_CMD 	= 0x02,	/*!< Wake-up from standby mode */
	STANDBY_CMD = 0x04,	/*!< Enter standby mode */
	RESET_CMD	= 0x06,	/*!< Reset the device */
	START_CMD	= 0x08,	/*!< Start and restart (synchronize) conversions */
	STOP_CMD	= 0x0A,	/*!< Stop conversion */
	//Data Read Commands
	RDATAC_CMD	= 0x10,	/*!< Enable Read Data Continuous mode.
					 	 *  This mode is the default mode at power-up */

	SDATAC_CMD	= 0x11,	/*!< Stop Read Data Continuously mode */
	RDATA_CMD	= 0x12,	/*!< Read data by command; supports multiple read back */
	//Register Read Commands
	RREG_CMD	= 0x20,
	WREG_CMD	= 0x40,
}commands_t;

typedef enum {
	BY_CMD,
	BY_GPIO
}driveType_t;

typedef enum {
	ID,
	CONFIG1,
	CONFIG2,
	CONFIG3,
	LOFF,
	CH1SET,
	CH2SET,
	CH3SET,
	CH4SET,
	CH5SET,
	CH6SET,
	CH7SET,
	CH8SET,
	BIAS_SENSP,
	BIAS_SENSN,
	LOFF_SENSP,
	LOFF_SENSN,
	LOFF_FLIP,
	LOFF_STATP,
	LOFF_STATN,
	GPIO,
	MISC1,
	MISC2,
	CONFIG4,
}registers_t;

typedef struct {
	uint8_t id;
	uint8_t config1;
	uint8_t config2;
	uint8_t config3;
	uint8_t loff;
	uint8_t ch1set;
	uint8_t ch2set;
	uint8_t ch3set;
	uint8_t ch4set;
	uint8_t ch5set;
	uint8_t ch6set;
	uint8_t ch7set;
	uint8_t ch8set;
	uint8_t bias_sensp;
	uint8_t bias_sensn;
	uint8_t loff_sensp;
	uint8_t loff_sensn;
	uint8_t loff_flip;
	uint8_t loff_statp;
	uint8_t loff_statn;
	uint8_t gpio;
	uint8_t misc1;
	uint8_t misc2;
	uint8_t config4;
}ads1299_registers_t;
/**
 * typedefs para simplificar la lectura, puntero a funcion
 */
typedef void (*csFunction_t)(csState_t);
typedef void (*clkSelFunction_t)(clkSel_t);
typedef void (*startFunction_t)(startState_t);
typedef void (*resetFunction_t)(resetState_t);
typedef void (*pwdnFunction_t)(pwdnState_t);
typedef void (*commandSend_t)(commands_t);
typedef uint8_t (*spiRead_t)(void);
typedef void (*spiWrite_t)(uint8_t);
typedef void (*delay2us_t)(int32_t);


/**
 * Estructura simple con punteros a funciones para separar la capa mas baja del driver
 */

typedef struct {
	csFunction_t chip_select_ctrl;
	clkSelFunction_t clkSel_ctrl;
	startFunction_t start_ctrl;
	resetFunction_t reset_ctrl;
	pwdnFunction_t pwdn_ctrl;
	commandSend_t command_send_fnc;
	spiRead_t spi_read_fnc;
	spiWrite_t spi_write_fnc;
	delay2us_t delay_2us_fnc;
}ads1299_t;

/**
 * @ Status type definition
 */
typedef enum {ERROR = 0, SUCCESS = !ERROR} Status;

/**
 * @brief	Inicializa el driver con las funciones de la capa port.
 * @param	config	: estructura de puntero a funciones a cargar
 */
void ads1299_InitDriver(ads1299_t config);

/**
 * @brief	Send a system Command
 * @param	command	: Command to send
 *
 * @return	SUCCESS or ERROR
 */
Status ads1299_CommandSend(commands_t command);
/**
 * @brief	Send a register Read o write command
 * @param	regCommand	: Command for read or write
 * @param	reg			: number of register
 * @param	data		: pointer to the data to be read o write
 *
 * @return	SUCCESS or ERROR
 */
Status ads1299_RegCommandSend(regCommands_t regCommand, registers_t reg, uint8_t* data);

Status ads1299_ReadData();

void ads1299_wakeup(void);
void ads1299_enterStandbyMode(void);

//----------------------------------------------

void ads1299_clockSource( clkSel_t clksel);
void ads1299_START(void);
void ads1299_RESET(void);
void ads1299_PWDN(pwdnState_t state);
static void ads1299_delay_tSDECODE(void);
static void ads1299_delay_4tCLK(void);
uint8_t ads1299_readRegister(registers_t registro);
void ads1299_writeRegister(registers_t registro);

#endif /* INC_ADS1299_H__ */
