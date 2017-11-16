/**
   @mainpage xmcfly microcontroller project

   @section Usage

   This project is intended to be used along with Eclipse or from the CMD-Line
   using a Makefile. The latter provides the following commands:

   @verbatim
   make                        # build the project
   make debug                  # build the project and start the debugger
   make flash                  # flash the application
   make doc                    # generate the documentation
   make clean                  # delete intermediate files
   @endverbatim

   @section Folders

   The code is organized in various folders - checkout the comments below:

   @verbatim
   Makefile          # top level Makefile used for the command-line variant
   doxygen           # doxygen configuration file
   inc/              # the place for your header files
   src/              # the place for your source files
   bin/              # output folder - here you will find listings and binaries
   Debug/            # this fiolder is only used by Eclipse
   doc/              # here the generated documentation is found
   system/           # CMSIS library, Infineon header files, linker script, etc.
   xmclib/           # low-level XMC library
   @endverbatim

   @file main.c
   @date 15 November 2017
   @version 0.0.1

   @brief XMC4500 Relax Interface Template

	ADD DESCRIPTION

   History <br>
   
   Version 0.0.1	LV	Initial	<br>
 */

#include <debug_lib.h>
#include <xmc_gpio.h>
#include <xmc_i2c.h>
#include <GPIO.h>

//ADR1: AD0 level low
//ADR2: AD0 level high - current configuration
#define MPU6050_ADR1 0xD0
#define MPU6050_ADR2 0xD2
#define MPU6050_ADR MPU6050_ADR2

#define MPU6050_READ MPU6050_ADR & (0x00)
#define MPU6050_WRITE MPU6050_ADR & (0x01)

//Config for Gyroscope Fullscale +/-2000°/s
#define GYRO_CONFIG_ADR 	0x1B
#define GYRO_CONFIG_DATA 	0x18
//#define GYRO_CONFIG_DATA 	0x00

//Gyroscope Data Registers
#define GYRO_DATA_X_HIGH 	0x43
#define GYRO_DATA_X_LOW 	0x44
#define GYRO_DATA_Y_HIGH 	0x45
#define GYRO_DATA_Y_LOW 	0x46
#define GYRO_DATA_Z_HIGH 	0x47
#define GYRO_DATA_Z_LOW 	0x48

//Config for Accelerometer Fullscale +/-16g
#define ACCEL_CONFIG_ADR 	0x1C
#define ACCEL_CONFIG_DATA 	0x18
//#define ACCEL_CONFIG_DATA 	0x00

//Accelerometer Data Registers
#define ACCEL_DATA_X_HIGH 	0x3B
#define ACCEL_DATA_X_LOW 	0x3C
#define ACCEL_DATA_Y_HIGH 	0x3D
#define ACCEL_DATA_Y_LOW 	0x3E
#define ACCEL_DATA_Z_HIGH 	0x3F
#define ACCEL_DATA_Z_LOW 	0x40


//Enable Data Ready Interrupt
#define INT_ENABLE_ADR 	0x38
#define INT_ENABLE_DATA 0x01
#define INT_STATUS_ADR 	0x3A

#define USER_CTRL_ADR 	0x6A
#define USER_CTRL_DATA 	0x45

#define FIFO_EN_ADR 	0x23
#define FIFO_EN_DATA	0x78

#define I2C_MST_CTRL_ADR	0x24
#define I2C_MST_CTRL_DATA	0x0D

#define PWR_MGMT_1_ADR 	0x6B
#define PWR_MGMT_1_DATA	0x00

#define SDA_PIN P1_5
#define SCL_PIN P1_1




struct MPU6050_DATA
{
	uint8_t gyro_x_h;
	uint8_t gyro_x_l;
	uint8_t gyro_y_h;
	uint8_t gyro_y_l;
	uint8_t gyro_z_h;
	uint8_t gyro_z_l;

	uint8_t accel_x_h;
	uint8_t accel_x_l;
	uint8_t accel_y_h;
	uint8_t accel_y_l;
	uint8_t accel_z_h;
	uint8_t accel_z_l;
};

XMC_GPIO_CONFIG_t i2c_sda =
{
  .mode = XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN_ALT2,
  .output_strength = XMC_GPIO_OUTPUT_STRENGTH_MEDIUM
};

XMC_GPIO_CONFIG_t i2c_scl =
{
  .mode = XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN_ALT2,
  .output_strength = XMC_GPIO_OUTPUT_STRENGTH_MEDIUM
};

XMC_GPIO_CONFIG_t p1_2_conf =
{
  .mode = XMC_GPIO_MODE_INPUT_TRISTATE,
  .output_strength = XMC_GPIO_OUTPUT_STRENGTH_MEDIUM
};

XMC_I2C_CH_CONFIG_t i2c_cfg =
{
  .baudrate = 100000U,
};

void delay(uint32_t counter);

uint8_t i2c_start(XMC_USIC_CH_t *const channel, uint8_t adress, XMC_I2C_CH_CMD_t command);
uint8_t i2c_repstart(XMC_USIC_CH_t *const channel, uint8_t adress, XMC_I2C_CH_CMD_t command);
uint8_t i2c_sendbyte(XMC_USIC_CH_t *const channel, uint8_t byte);
uint8_t i2c_sendACK(XMC_USIC_CH_t *const channel);
uint8_t i2c_sendNACK(XMC_USIC_CH_t *const channel);
uint8_t i2c_readbyte(XMC_USIC_CH_t *const channel);
uint8_t i2c_stop(XMC_USIC_CH_t *const channel);

uint8_t i2c_start(XMC_USIC_CH_t *const channel, uint8_t adress, XMC_I2C_CH_CMD_t command)
{
	XMC_I2C_CH_MasterStart(channel, adress, command);
	while((XMC_I2C_CH_GetStatusFlag(channel) & XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED) == 0U)
	{
	/* wait for ACK */
	}
	XMC_I2C_CH_ClearStatusFlag(channel, XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED);

	return 0;
}

uint8_t i2c_repstart(XMC_USIC_CH_t *const channel, uint8_t adress, XMC_I2C_CH_CMD_t command)
{
	XMC_I2C_CH_MasterRepeatedStart(channel, adress, command);
	while((XMC_I2C_CH_GetStatusFlag(channel) & XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED) == 0U)
	{
	  /* wait for ACK */
	}
	XMC_I2C_CH_ClearStatusFlag(channel, XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED);

	return 0;
}

uint8_t i2c_sendbyte(XMC_USIC_CH_t *const channel, uint8_t byte)
{
	XMC_I2C_CH_MasterTransmit(channel, byte);
	while((XMC_I2C_CH_GetStatusFlag(channel) & XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED) == 0U)
	{
		/* wait for ACK */
	}
	XMC_I2C_CH_ClearStatusFlag(channel, XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED);

	return 0;
}

uint8_t i2c_sendACK(XMC_USIC_CH_t *const channel)
{
	XMC_I2C_CH_MasterReceiveAck(channel);
	return 0;
}

uint8_t i2c_sendNACK(XMC_USIC_CH_t *const channel)
{
	XMC_I2C_CH_MasterReceiveNack(channel);
	return 0;
}

uint8_t i2c_readbyte(XMC_USIC_CH_t *const channel)
{
	//XMC_I2C_CH_MasterTransmit(channel, 0xFF);
	return XMC_I2C_CH_GetReceivedData(channel);
}

uint8_t i2c_stop(XMC_USIC_CH_t *const channel)
{
	XMC_I2C_CH_MasterStop(channel);
	return 0;
}

int main(void)
{
	uint8_t counter = 0;
	uint8_t recv_data = 0;
	struct MPU6050_DATA Data1;

	initRetargetSwo();
	printf("Hello World\n");
	

	XMC_I2C_CH_Init(XMC_I2C0_CH0, &i2c_cfg);
	XMC_I2C_CH_SetInputSource(XMC_I2C0_CH0, XMC_I2C_CH_INPUT_SDA, USIC0_C0_DX0_P1_5);
	XMC_I2C_CH_SetInputSource(XMC_I2C0_CH0, XMC_I2C_CH_INPUT_SCL, USIC0_C0_DX1_P1_1);
	XMC_I2C_CH_Start(XMC_I2C0_CH0);

	XMC_GPIO_Init(SCL_PIN, &i2c_scl);
	XMC_GPIO_Init(SDA_PIN, &i2c_sda);
	XMC_GPIO_Init(P1_2, &p1_2_conf);

	printf("[I2C] Init done\n");

	i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_WRITE);

	i2c_sendbyte(XMC_I2C0_CH0, USER_CTRL_ADR);
	i2c_sendbyte(XMC_I2C0_CH0, USER_CTRL_ADR);

	i2c_stop(XMC_I2C0_CH0);

	i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_WRITE);

	i2c_sendbyte(XMC_I2C0_CH0, FIFO_EN_ADR);
	i2c_sendbyte(XMC_I2C0_CH0, FIFO_EN_DATA);

	i2c_stop(XMC_I2C0_CH0);

	printf("[I2C] FIFO cleared and enabled\n");

	i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_WRITE);

	i2c_sendbyte(XMC_I2C0_CH0, GYRO_CONFIG_ADR);
	i2c_sendbyte(XMC_I2C0_CH0, GYRO_CONFIG_DATA);

	i2c_stop(XMC_I2C0_CH0);

	printf("[I2C] Gyro Config done\n");

	i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_WRITE);

	i2c_sendbyte(XMC_I2C0_CH0, ACCEL_CONFIG_ADR);
	i2c_sendbyte(XMC_I2C0_CH0, ACCEL_CONFIG_DATA);

	i2c_stop(XMC_I2C0_CH0);

	printf("[I2C] Accel Config done\n");

	i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_WRITE);

	i2c_sendbyte(XMC_I2C0_CH0, INT_ENABLE_ADR);
	i2c_sendbyte(XMC_I2C0_CH0, INT_ENABLE_DATA);

	i2c_stop(XMC_I2C0_CH0);

	printf("[I2C] Interrupt Config done\n");

	i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_WRITE);

	i2c_sendbyte(XMC_I2C0_CH0, PWR_MGMT_1_ADR);
	i2c_sendbyte(XMC_I2C0_CH0, PWR_MGMT_1_DATA);

	i2c_stop(XMC_I2C0_CH0);

	printf("[I2C] Sleep mode disabled\n");

	while(1)
	{
		i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_WRITE);
		i2c_sendbyte(XMC_I2C0_CH0, INT_STATUS_ADR);
		i2c_stop(XMC_I2C0_CH0);

		i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_READ);
		recv_data=i2c_readbyte(XMC_I2C0_CH0);
		i2c_sendNACK(XMC_I2C0_CH0);
		i2c_stop(XMC_I2C0_CH0);

		if((recv_data & 0x01)==0x01)
		{
			i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_WRITE);
			i2c_sendbyte(XMC_I2C0_CH0, GYRO_DATA_X_HIGH);
			i2c_stop(XMC_I2C0_CH0);

			i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_READ);

			Data1.gyro_x_h=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendACK(XMC_I2C0_CH0);
			Data1.gyro_x_l=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendACK(XMC_I2C0_CH0);
			Data1.gyro_y_h=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendACK(XMC_I2C0_CH0);
			Data1.gyro_y_l=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendACK(XMC_I2C0_CH0);
			Data1.gyro_z_h=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendACK(XMC_I2C0_CH0);
			Data1.gyro_z_l=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendNACK(XMC_I2C0_CH0);

			i2c_stop(XMC_I2C0_CH0);

			printf("[I2C] Gyro data read\n");

			printf("%x\n", Data1.gyro_x_h);
			printf("%x\n", Data1.gyro_x_l);
			printf("%x\n", Data1.gyro_y_h);
			printf("%x\n", Data1.gyro_y_l);
			printf("%x\n", Data1.gyro_z_h);
			printf("%x\n\n", Data1.gyro_z_l);


			i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_WRITE);
			i2c_sendbyte(XMC_I2C0_CH0, ACCEL_DATA_X_HIGH);
			i2c_stop(XMC_I2C0_CH0);

			i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_READ);

			Data1.accel_x_h=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendACK(XMC_I2C0_CH0);
			Data1.accel_x_l=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendNACK(XMC_I2C0_CH0);

			i2c_stop(XMC_I2C0_CH0);

			i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_READ);

			Data1.accel_y_h=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendACK(XMC_I2C0_CH0);
			Data1.accel_y_l=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendNACK(XMC_I2C0_CH0);

			i2c_stop(XMC_I2C0_CH0);

			i2c_start(XMC_I2C0_CH0, MPU6050_ADR, XMC_I2C_CH_CMD_READ);

			Data1.accel_z_h=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendACK(XMC_I2C0_CH0);
			Data1.accel_z_l=i2c_readbyte(XMC_I2C0_CH0);
			i2c_sendNACK(XMC_I2C0_CH0);

			i2c_stop(XMC_I2C0_CH0);

			printf("[I2C] Accel data read\n");

			printf("%x\n", Data1.accel_x_h);
			printf("%x\n", Data1.accel_x_l);
			printf("%x\n", Data1.accel_y_h);
			printf("%x\n", Data1.accel_y_l);
			printf("%x\n", Data1.accel_z_h);
			printf("%x\n\n", Data1.accel_z_l);
		}
		delay(3000);
	}
}


void delay(uint32_t counter)
{
	volatile uint32_t cnt = counter;
	while(--cnt);
}

/* EOF */
