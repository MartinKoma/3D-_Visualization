/**
 * \file
 *
 * \brief BNO055 extension board example for SAME70.
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#ifndef __BNO055_PORTING_I2C_H__
#define __BNO055_PORTING_I2C_H__

/* Operation mode settings*/
#define OPERATION_MODE_CONFIG			0X00
#define OPERATION_MODE_ACCONLY			0X01
#define OPERATION_MODE_MAGONLY			0X02
#define OPERATION_MODE_GYRONLY			0X03
#define OPERATION_MODE_ACCMAG			0X04
#define OPERATION_MODE_ACCGYRO			0X05
#define OPERATION_MODE_MAGGYRO			0X06
#define OPERATION_MODE_AMG				0X07
#define OPERATION_MODE_IMUPLUS			0X08
#define OPERATION_MODE_COMPASS			0X09
#define OPERATION_MODE_M4G				0X0A
#define OPERATION_MODE_NDOF_FMC_OFF		0X0B
#define OPERATION_MODE_NDOF				0X0C

/* Power mode*/
#define POWER_MODE_NORMAL				0X00
#define POWER_MODE_LOWPOWER				0X01
#define POWER_MODE_SUSPEND				0X02

/* bno055 I2C Address */
#define BNO055_I2C_ADDR1                0x28
#define BNO055_I2C_ADDR2                0x29



#include "stdint.h"
#include <i2cPacket.h>
typedef	uint8_t u8;
typedef	uint32_t u32;/**< used for unsigned 32bit */
typedef	uint16_t u16;/**< used for unsigned 32bit */

typedef int8_t s8;/**< used for signed 8bit */
typedef	int16_t s16;/**< used for signed 16bit */
typedef	int32_t s32;/**< used for signed 32bit */

s8 bno055_set_power_mode(u8 v_power_mode_u8);
s8 bno055_get_operation_mode(u8 *v_operation_mode_u8);
s8 bno055_set_sys_rst(u8 v_sys_rst_u8);
s8 bno055_write_page_id(u8 v_page_id_u8);
s8 bno055_read_euler_raw(u8 *euler);
s8 bno055_read_mag_xyz_raw(u8 *mag);
s8 bno055_read_quaternion_wxyz_raw(u8 *quaternion);
s8 bno055_read_accel_xyz_raw(u8 *accel);
s8 bno055_read_gyro_xyz_raw(u8 *gyro);
s8 bno055_get_gyro_calib_stat(u8 *v_gyro_calib_u8);
s8 bno055_get_accel_calib_stat(u8 *v_accel_calib_u8);
s8 bno055_get_mag_calib_stat(u8 *v_mag_calib_u8);
s8 bno055_set_operation_mode(u8 v_operation_mode_u8);

int bno055_initialize(void);
s8 _bno055_i2c_bus_write(u8 reg_addr, u8 *reg_data, u8 cnt);
s8 _bno055_i2c_bus_read(u8 reg_addr, u8 *reg_data, u8 cnt);
void bno055_delay_msek(u32 msek);

#endif  /* __BNO055_PORTING_I2C_H__ */