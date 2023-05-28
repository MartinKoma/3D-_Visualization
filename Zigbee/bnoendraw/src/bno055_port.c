/**************************************************************************//**
  \file bno055_port.c

  \author
    Gregor Napierski
    From  Sensor Driver file for BNO055 sensor: bno055.c

******************************************************************************/

#include <appTimer.h>
#include <zdo.h>
#include <sysTaskManager.h>
#include <usartManager.h>
#include <halTaskManager.h>
#include <i2cPacket.h>
#include <util/delay.h>
#include <bno055_port.h>

static HAL_I2cDescriptor_t p_packet;
static bool twibusy=false;
u8 page_id=0;
u8 chip_id=0;

/*!
 *	@brief
 *	This function is used for initialize
 *	read page id and chip id for proof of function
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 * 
 */
s8 bno055_init()
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1;
	u8 v_data_u8 = 0;
	u8 v_page_zero_u8 = 0;
	
	/* Write the default page as zero*/
	com_rslt = _bno055_i2c_bus_write(0X07, &v_page_zero_u8, 1);
	/* Read the chip id of the sensor from page
	zero 0x00 register*/
	com_rslt += _bno055_i2c_bus_read(0x00, &v_data_u8, 1);
	chip_id = v_data_u8;
	
	// page-id on start up after power on is always 0
	page_id = 0; 

	return com_rslt;
}

/*!	@brief This API used to write the power mode
 *	from register from 0x3E bit 0 to 1
 *
 *	@param v_power_mode_u8 : The value of power mode
 *
 * v_power_mode_u8 |      result        | comments
 * ----------|---------------------|-------------------------------------
 *  0x00     | POWER_MODE_NORMAL   | In the NORMAL mode the register
 *    -      |       -             | map and the internal peripherals
 *    -      |       -             | of the MCU are always
 *    -      |       -             | operative in this mode
 *  0x01     | POWER_MODE_LOWPOWER | This is first level of power saving mode
 *  0x02     | POWER_MODE_SUSPEND  | In suspend mode the system is
 *   -       |      -              | paused and all the sensors and
 *   -       |      -              | the micro controller are
 *   -       |      -              | put into sleep mode.
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 *	@note For detailed about LOWPOWER mode
 *	refer data sheet 3.4.2
 *
 */
s8 bno055_set_power_mode(u8 v_power_mode_u8)
{
s8 com_rslt = -1;//ERROR;
u8 v_data_u8r = 0;
u8 v_prev_opmode_u8 = 0X00; //OPERATION_MODE_CONFIG;
s8 v_stat_s8 = -1; //ERROR;

		/* The write operation effective only if the operation
		mode is in config mode, this part of code is checking the
		current operation mode and set the config mode */
		v_stat_s8 = bno055_get_operation_mode(&v_prev_opmode_u8);
		if (v_stat_s8 == 0) {
			if (v_prev_opmode_u8 != 0X00)
				v_stat_s8 += bno055_set_operation_mode(0X00);
			if (v_stat_s8 == 0) {
				/* Write the value of power mode */
				com_rslt = _bno055_i2c_bus_read	(0x3E, &v_data_u8r, 1);
				
				if (com_rslt == 0) {
					v_data_u8r = (v_data_u8r & ~0X03) | (v_power_mode_u8 & 0X03);					
					com_rslt +=	_bno055_i2c_bus_write(	0x3E, &v_data_u8r, 1);
				}
			} else {
			   com_rslt = -1;
			}
		} else {
		com_rslt = -1;
		}
	
	if (v_prev_opmode_u8 != 0X00)
		/* set the operation mode
		of previous operation mode*/
		com_rslt += bno055_set_operation_mode(v_prev_opmode_u8);
	return com_rslt;
}

/*!	@brief This API used to read the operation mode
 *	from register from 0x3D bit 0 to 3
 *
 *	@param v_operation_mode_u8 : The value of operation mode
 *
 * v_operation_mode_u8 |      result      | comments
 * ----------|----------------------------|----------------------------
 *  0x00     | OPERATION_MODE_CONFIG      | Configuration mode
 *  0x01     | OPERATION_MODE_ACCONLY     | Reads accel data alone
 *  0x02     | OPERATION_MODE_MAGONLY     | Reads mag data alone
 *  0x03     | OPERATION_MODE_GYRONLY     | Reads gyro data alone
 *  0x04     | OPERATION_MODE_ACCMAG      | Reads accel and mag data
 *  0x05     | OPERATION_MODE_ACCGYRO     | Reads accel and gyro data
 *  0x06     | OPERATION_MODE_MAGGYRO     | Reads accel and mag data
 *  0x07     | OPERATION_MODE_ANY_MOTION  | Reads accel mag and gyro data
 *  0x08     | OPERATION_MODE_IMUPLUS     | Inertial measurement unit
 *   -       |       -                    | Reads accel,gyro and fusion data
 *  0x09     | OPERATION_MODE_COMPASS     | Reads accel, mag data
 *   -       |       -                    | and fusion data
 *  0x0A     | OPERATION_MODE_M4G         | Reads accel, mag data
 *    -      |       -                    | and fusion data
 *  0x0B     | OPERATION_MODE_NDOF_FMC_OFF| Nine degrees of freedom with
 *   -       |       -                    | fast magnetic calibration
 *   -       |       -                    | Reads accel,mag, gyro
 *   -       |       -                    | and fusion data
 *  0x0C     | OPERATION_MODE_NDOF        | Nine degrees of freedom
 *   -       |       -                    | Reads accel,mag, gyro
 *   -       |       -                    | and fusion data
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 *	@note In the config mode, all sensor and fusion data
 *	becomes zero and it is mainly derived
 *	to configure the various settings of the BNO
 *
 */
s8 bno055_get_operation_mode(u8 *v_operation_mode_u8)
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1; //ERROR;
	u8 v_data_u8r = 0;
	s8 v_stat_s8 = -1; //ERROR;
	
		/*condition check for page, operation mode is
		available in the page zero*/
		if (page_id != 0)
			/* Write the page zero*/
			v_stat_s8 = bno055_write_page_id(0);
			
		if ((v_stat_s8 == 0) ||	(page_id == 0)) {
			/* Read the value of operation mode*/
			com_rslt = _bno055_i2c_bus_read	(0X3D, &v_data_u8r, 1);
			*v_operation_mode_u8 = v_data_u8r & 0x0F;
			
		} else {
			com_rslt = -1;
		}
	
	return com_rslt;
}

/*!	@brief This API used to write the operation mode
 *	from register from 0x3D bit 0 to 3
 *
 *	@param v_operation_mode_u8 : The value of operation mode
 *
 *  v_operation_mode_u8  |      result    | comments
 * ----------|----------------------------|----------------------------
 *  0x00     | OPERATION_MODE_CONFIG      | Configuration mode
 *  0x01     | OPERATION_MODE_ACCONLY     | Reads accel data alone
 *  0x02     | OPERATION_MODE_MAGONLY     | Reads mag data alone
 *  0x03     | OPERATION_MODE_GYRONLY     | Reads gyro data alone
 *  0x04     | OPERATION_MODE_ACCMAG      | Reads accel and mag data
 *  0x05     | OPERATION_MODE_ACCGYRO     | Reads accel and gyro data
 *  0x06     | OPERATION_MODE_MAGGYRO     | Reads accel and mag data
 *  0x07     | OPERATION_MODE_ANY_MOTION  | Reads accel mag and gyro data
 *  0x08     | OPERATION_MODE_IMUPLUS     | Inertial measurement unit
 *   -       |       -                    | Reads accel,gyro and fusion data
 *  0x09     | OPERATION_MODE_COMPASS     | Reads accel, mag data
 *   -       |       -                    | and fusion data
 *  0x0A     | OPERATION_MODE_M4G         | Reads accel, mag data
 *    -      |       -                    | and fusion data
 *  0x0B     | OPERATION_MODE_NDOF_FMC_OFF| Nine degrees of freedom with
 *   -       |       -                    | fast magnetic calibration
 *   -       |       -                    | Reads accel,mag, gyro
 *   -       |       -                    | and fusion data
 *  0x0C     | OPERATION_MODE_NDOF        | Nine degrees of freedom
 *   -       |       -                    | Reads accel,mag, gyro
 *   -       |       -                    | and fusion data
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 *	@note In the config mode, all sensor and fusion data
 *	becomes zero and it is mainly derived
 *	to configure the various settings of the BNO
 *
 */
s8 bno055_set_operation_mode(u8 v_operation_mode_u8)
{
s8 com_rslt = -1; //ERROR;
u8 v_data_u8r = 0; //
s8 v_stat_s8 = -1; //ERROR;
u8 v_prev_opmode_u8 = 0x00; //OPERATION_MODE_CONFIG
		/* The write operation effective only if the operation
		mode is in config mode, this part of code is checking the
		current operation mode and set the config mode */
		v_stat_s8 = bno055_get_operation_mode(&v_prev_opmode_u8);
		if (v_stat_s8 == 0) {
			/* If the previous operation mode is config it is
				directly write the operation mode */
			if (v_prev_opmode_u8 == 0x00) {
				com_rslt = _bno055_i2c_bus_read	(0x3D, &v_data_u8r, 1);
				if (com_rslt == 0) {		
								
					v_data_u8r = (v_data_u8r & ~0X0F) | ((v_operation_mode_u8 << 0) & 0X0F);
					com_rslt +=	_bno055_i2c_bus_write(0x3D,	&v_data_u8r, 1);
					/* Config mode to other
					operation mode switching
					required delay of 600ms*/
					//bno055_delay_msek(600);
				}
			} else {
				/* If the previous operation
				mode is not config it is
				 write the config mode */
				com_rslt = _bno055_i2c_bus_read	(0x3D, &v_data_u8r, 1);
				if (com_rslt == 0) {
					v_data_u8r = (v_data_u8r & ~0X0F) | (0x00 & 0X0F);
					com_rslt += _bno055_i2c_bus_write(0x3D,	&v_data_u8r, 1); 
					/* other mode to config mode switching
					required delay of 20ms*/
					//bno055_delay_msek(20);
				}
				/* Write the operation mode */
				if (v_operation_mode_u8 != 0x00) {
					com_rslt = _bno055_i2c_bus_read	(0x3D, &v_data_u8r, 1);					
					if (com_rslt == 0) {
						v_data_u8r = (v_data_u8r & ~0X0F) | ((v_operation_mode_u8 << 0) & 0X0F);						
						com_rslt +=	_bno055_i2c_bus_write(0x3D,	&v_data_u8r, 1);						
						/* Config mode to other
						operation mode switching
						required delay of 600ms*/
						//bno055_delay_msek(600);
					}
				}
			}
		} else {
		com_rslt = -1;
		
	}
	return com_rslt;
}

/*!
 *	@brief This API used to write the reset system
 *	from register from 0x3F bit 5
 *
 *	@param v_sys_rst_u8 : The value of reset system
 *
 *	 v_sys_rst_u8   | result
 *   -------------- |----------
 *     0x01         | ENABLED
 *     0x00         | DISABLED
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 *	@note It resets the whole system
 */
s8 bno055_set_sys_rst(u8 v_sys_rst_u8)
{
	s8 com_rslt = -1;
	u8 v_data_u8r = 0;
	s8 v_stat_s8 = -1;
	/* Check the struct p_bno055 is empty */

		/*condition check for page, reset system is
		available in the page zero*/
		if (page_id != 0)
			/* Write the page zero*/
			v_stat_s8 = bno055_write_page_id(0);
		if ((v_stat_s8 == 0) ||
		(page_id == 0)) {
			/* Write the value of reset system */
			com_rslt =_bno055_i2c_bus_read(	0x3F,&v_data_u8r, 1);			
			if (com_rslt == 0) {
				v_data_u8r = (v_data_u8r & ~0X20) | ((v_sys_rst_u8<<5) & 0X20);				
				com_rslt =	_bno055_i2c_bus_write(0x3F,	&v_data_u8r, 1);
				//bno055_delay_msek(600);	
			}
		} else {
		com_rslt = -1;
		}
	
	return com_rslt;
}
/*!
 *	@brief This API used to read
 *	accel calibration status from register from 0x35 bit 2 and 3
 *
 *	@param v_accel_calib_u8 : The value of accel calib status
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 */
s8 bno055_get_accel_calib_stat(
u8 *v_accel_calib_u8)
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1; //ERROR;
	u8 v_data_u8r = 0;
	s8 v_stat_s8 = -1;
	
		/*condition check for page, accel calib
		available in the page zero*/
		if (page_id != 0)
			/* Write the page zero*/
			v_stat_s8 = bno055_write_page_id(0);
		if ((v_stat_s8 == 0) ||	(page_id == 0)) {
			/* Read the accel calib v_stat_s8 */
			com_rslt =_bno055_i2c_bus_read(	0x35,&v_data_u8r, 1);
			*v_accel_calib_u8 = ((v_data_u8r & 0X0C) >> 2);
			
		} else {
		com_rslt = -1;
		}
	
	return com_rslt;
}
/*!
 *	@brief This API used to read
 *	gyro calibration status from register from 0x35 bit 4 and 5
 *
 *	@param v_gyro_calib_u8 : The value of gyro calib status
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 */
s8 bno055_get_gyro_calib_stat(
u8 *v_gyro_calib_u8)
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1; //ERROR;
	u8 v_data_u8r = 0;
	s8 v_stat_s8 = -1;
	
		/*condition check for page, gyro calib
		available in the page zero*/
		if (page_id != 0)
			/* Write the page zero*/
			v_stat_s8 = bno055_write_page_id(0);
			
		if ((v_stat_s8 == 0) ||	(page_id == 0)) {
			/* Read the gyro calib status */
				com_rslt =_bno055_i2c_bus_read(	0x35,&v_data_u8r, 1);
				*v_gyro_calib_u8 = ((v_data_u8r & 0X30) >> 4);			
			
		} else {
		com_rslt = -1;
		}
	
	return com_rslt;
}

/*!
 *	@brief This API used to read
 *	mag calibration status from register from 0x35 bit 0 and 1
 *
 *	@param v_mag_calib_u8 : The value of mag calib status
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 */
s8 bno055_get_mag_calib_stat(u8 *v_mag_calib_u8)
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1;
	u8 v_data_u8r = 0;
	s8 v_stat_s8 = -1;

		/*condition check for page, mag calib
		available in the page zero*/
		if (page_id != 0)
			/* Write the page zero*/
			v_stat_s8 = bno055_write_page_id(0);
		if ((v_stat_s8 == 0) ||
		(page_id == 0)) {
			/* Read the mag calib v_stat_s8 */			
			com_rslt =_bno055_i2c_bus_read(	0x35,&v_data_u8r, 1);
			*v_mag_calib_u8 = (v_data_u8r & 0X03);			
			
		} else {
		com_rslt = -1;
		}
	
	return com_rslt;
}

/*!
 *	@brief This API reads raw mag data array 
 *	from register 0x0E to 0x13 it is a six byte data
 *
 *
 *	@param mag : The mag xyz values
 *
 *	Parameter |    result
 *  --------- | -----------------
 *	 mag[0][1]| The mag x data
 *	 mag[2][3]| The mag y data
 *	 mag[4][5]| The mag z data
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 */
s8 bno055_read_mag_xyz_raw(u8 *mag)
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1;	
	s8 v_stat_s8 = -1;
		/*condition check for page, chip id is
		available in the page zero*/
		if (page_id != 0)
			/* Write the page zero*/
			v_stat_s8 = bno055_write_page_id(0);
		if ((v_stat_s8 == 0) ||
		(page_id == 0)) {
			/*Read the six byte value of mag xyz*/
			com_rslt = _bno055_i2c_bus_read	(0x0E, mag, 6);
			
		} else {
		com_rslt = -1;
		}
	
	return com_rslt;
}
/*!
 *	@brief This API reads Quaternion data array values
 *	from register 0x20 to 0x27 it is a six byte data
 *
 *
 *	@param quaternion : The value of quaternion wxyz data's array
 *
 *	Parameter			|    result
 *  ---------			| -----------------
 *	 quaternion[0][1]	| The quaternion w data
 *	 quaternion[2][3]	| The quaternion x data
 *	 quaternion[4][5]   | The quaternion y data
 *	 quaternion[6][7]   | The quaternion z data
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 */
s8 bno055_read_quaternion_wxyz_raw(u8 *quaternion)
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1;	
	s8 v_stat_s8 = -1;
	
		/*condition check for page, chip id is
		available in the page zero*/
		if (page_id != 0)
			/* Write the page zero*/
			v_stat_s8 = bno055_write_page_id(0);
		if ((v_stat_s8 == 0) ||	(page_id == 0)) {
			/* Read the eight byte value
			of quaternion wxyz data*/
			com_rslt = _bno055_i2c_bus_read	(0X20, quaternion, 8);
			
		} else {
		com_rslt = -1;
		}
	
	return com_rslt;
}
/*!
 *	@brief This API reads gyro data xyz values
 *	from register 0x14 to 0x19 it is a six byte data
 *
 *
 *	@param gyro : The value of gyro xyz data's
 *
 *	Parameter |    result
 *  --------- | -----------------
 *	 x        | The gyro x data
 *	 y        | The gyro y data
 *	 z        | The gyro z data
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 */
s8 bno055_read_gyro_xyz_raw(u8 *gyro)
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1; //;
	s8 v_stat_s8 = -1;
	
		/*condition check for page, chip id is
		available in the page zero*/
		if (page_id != 0)
			/* Write the page zero*/
			v_stat_s8 = bno055_write_page_id(0);
		if ((v_stat_s8 == 0) ||	(page_id == 0)) {
			/* Read the six bytes data of gyro xyz*/
			com_rslt = _bno055_i2c_bus_read	(0X14, gyro, 6);
			
		} else {
		com_rslt = -1;
		}
	
	return com_rslt;
}

/*!
 *	@brief This API reads acceleration data xyz values
 *	from register 0x08 to 0x0D it is a six byte data
 *
 *
 *	@param accel : The value of accel xyz data
 *
 *	Parameter |    result
 *  --------- | -----------------
 *	 x        | The accel x data
 *	 y        | The accel y data
 *	 z        | The accel z data
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 */
s8 bno055_read_accel_xyz_raw(u8 *accel)
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1;	
	s8 v_stat_s8 = -1;
	
	
		/*condition check for page, chip id is
		available in the page zero*/
		if (page_id != 0)
			/* Write the page zero*/
			v_stat_s8 = bno055_write_page_id(0);		
			if ((v_stat_s8 == 0) ||	(page_id == 0)) {
				/* Read the six bytes data of gyro xyz*/
				com_rslt = _bno055_i2c_bus_read	(0x08, accel, 6);				
			
		} else {
		com_rslt = -1;
		}
	
	return com_rslt;
}

/*!
 *	@brief This API reads Euler data hrp values
 *	from register 0x1A to 0x1F it is a six byte raw array data
 *
 *
 *	@param euler : The Euler hrp data's
 *
 *	Parameter |    result
 *  --------- | -----------------
 *	 euler[0]..[1]       | The Euler h data
 *	 euler[2]..[3]       | The Euler r data
 *	 euler[4]..[5]       | The Euler p data
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 */
s8 bno055_read_euler_raw(u8 *euler)
{
	
	s8 com_rslt = -1;	
	s8 v_stat_s8 = -1;
	
	if (page_id != 0X00)
		/* Write the page zero*/
		v_stat_s8 = bno055_write_page_id(0X00);
		
	if ((v_stat_s8 == 0) ||	(page_id == 0)) {
		/* Read the six byte of Euler hrp data*/
		com_rslt = _bno055_i2c_bus_read(0X1A, euler, 6);
			
	} else {
		com_rslt = -1;
	}
	
	return com_rslt;
}

/*!
 *	@brief This API used to write
 *	the page id register 0x07
 *
 *	@param v_page_id_u8 : The value of page id
 *
 *	PAGE_ZERO -> 0x00
 *	PAGE_ONE  -> 0x01
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 *
 */
s8 bno055_write_page_id(u8 v_page_id_u8)
{
	/* Variable used to return value of
	communication routine*/
	s8 com_rslt = -1;
	u8 v_data_u8r = 0;
	
	/* Read the current page*/
	com_rslt = _bno055_i2c_bus_read(0X07, &v_data_u8r, 1);
	/* Check condition for communication success*/
	if (com_rslt == 0) {
		v_data_u8r = v_page_id_u8;
		/* Write the page id*/
		com_rslt += _bno055_i2c_bus_write(0X07, &v_data_u8r, 1);
		if (com_rslt == 0)
			page_id = v_page_id_u8;
		else 
		com_rslt = -1;
	}
	
	return com_rslt;
}



/* Sensor data read, update current reading with new data */
// twibusy auf false setzen 
static void sensorReadData(bool result)
{
	result=result;
	twibusy=false;	
}


int bno055_initialize(void)
{	
	p_packet.tty = TWI_CHANNEL_0;
	p_packet.clockRate = I2C_CLOCK_RATE_62; 
	p_packet.f = sensorReadData; 
	p_packet.id = BNO055_I2C_ADDR1;	
	HAL_OpenI2cPacket(&p_packet);	
	bno055_init();
	
	// if chip.id is set, sensor is ready
	if (chip_id>0)
	return 0;
	return -1;
}


/*	\Brief: The function is used as I2C bus write. With BitCloud HAL structures and functions
 *	\Return : Status of the I2C write
 *	
 *	\param reg_addr : Address of the first register, will data is going to be written
 *	\param reg_data : It is a value hold in the array,
 *		will be used for write the value into the register
 *	\param cnt : The no of byte of data to be write
 */
s8 _bno055_i2c_bus_write(u8 reg_addr, u8 *reg_data, u8 cnt)
{
	int32_t ierror = 0;
   
    // if last state was I2C_TRANSAC_FAIL something went wrong
	if (p_packet.service.state == 7) //I2C_TRANSAC_FAIL
	return -1;
	
	// if we are still busy, 
	if (twibusy) return -1;
	
	p_packet.internalAddr=reg_addr;	
	p_packet.lengthAddr = HAL_ONE_BYTE_SIZE;
	p_packet.data = reg_data;
	p_packet.length = cnt;
	p_packet.service.state = 1; //I2C_IDLE
	
	twibusy=true;
	
	ierror=HAL_WriteI2cPacket (&p_packet); 	
	if (ierror==0)
	{
		
		while (twibusy) // wait, until I2C is ready and state is I2C_TRANSAC_SUCCESS
		{	 
			 if (p_packet.service.state == 6) //I2C_TRANSAC_SUCCESS -> Siehe i2cPacket.c
			 halSig2WireSerialHandler();  //calls callback : i2cPacket.c				
		}
	}
		
		
	if (p_packet.service.state == 7) 
	{		
		return -1;
	}
	
	return (int8_t)ierror;
}

 /*	\Brief: The function is used as I2C bus read
 *	\Return : Status of the I2C read 
 *	\param reg_addr : Address of the first register, will data is going to be read
 *	\param reg_data : This data read from the sensor, which is hold in an array
 *	\param cnt : The no of byte of data to be read
 */
s8 _bno055_i2c_bus_read(u8 reg_addr, u8 *reg_data, u8 cnt)
{
	int32_t ierror = 0;
	if (p_packet.service.state == 7) //I2C_TRANSAC_FAIL
	return -1;
	
	// if we are still busy, error ...
	if (twibusy) return -1;
	
	p_packet.internalAddr=reg_addr;
	p_packet.lengthAddr = HAL_ONE_BYTE_SIZE;
	p_packet.data = reg_data;
	p_packet.length = cnt;
	p_packet.service.state = 1; //I2C_IDLE

	twibusy=true;	
	ierror=HAL_ReadI2cPacket (&p_packet);

	if (ierror==0)
	while (twibusy) 
	{			
		if (p_packet.service.state >= 6) //I2C_TRANSAC_SUCCESS -> Siehe i2cPacket.c	
			 halSig2WireSerialHandler(); //calls callback : i2cPacket.c	
	}

	if (p_packet.service.state == 7) 
	{
		return -1;
	}
	
	return (int8_t)ierror;
}


/*	Brief : The delay routine
 *	\param : delay in ms
*/
// Wird von der Sensor-Lib aufgerufenb mit msek zwischen 10 und 600.
// Z,B. in bno055_set_operation_mode -> dort 600 ms delay nötig
// delays needed!!
void bno055_delay_msek(u32 msek)
{
	for (u32 i=0; i<msek; i++)
	{
      _delay_us(1000); // if we wait less then 1000 us sensor will not init!
	  // app task handler should not be delayed for more then 10 ms 
	  SYS_PostTask(APL_TASK_ID); 
	}
}
