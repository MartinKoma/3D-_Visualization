/**************************************************************************//**
  \file  halRfCtrl.c

  \brief mac pin interface implementation.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      28/05/07 ALuzhetsky - Created.
      06/08/07 A. Mandychev, E. Ivanov - Modified.
    Last change:
     $Id$
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
/******************************************************************************
                        Includes section.
******************************************************************************/
#include <halRfCtrl.h>
#include <halIrq.h>

/******************************************************************************
                   Prototypes section
******************************************************************************/
#ifdef _HAL_RF_RX_TX_INDICATOR_
/**************************************************************************//**
\brief  Turn on pin 1 (DIG3) and pin 2 (DIG4) to indicate the transmit state of
the radio transceiver.
******************************************************************************/
void phyRxTxSwitcherOn(void);

#endif //_HAL_RF_RX_TX_INDICATOR_

#ifdef _HAL_ANT_DIVERSITY_
/**************************************************************************//**
\brief  Enable antenna diversity feature.
******************************************************************************/
void phyAntennaDiversityInit(void);

/**************************************************************************//**
\brief  Enable antenna diversity in the receive state of the radio transceiver.
******************************************************************************/
void phyRxAntennaDiversity(void);

/**************************************************************************//**
\brief  Enable antenna diversity in the transmit state of the radio transceiver.
******************************************************************************/
void phyTxAntennaDiversity(void);

/**************************************************************************//**
\brief  read the antenna used for last transmission and switch to other antenna 
******************************************************************************/
void phySwitchLastTxAntenna(void);

#endif //_HAL_ANT_DIVERSITY_

/******************************************************************************
                    Implementations section
******************************************************************************/
/**************************************************************************//**
  \brief Enables RX TX indicator for radio if that is supported.
******************************************************************************/
void HAL_EnableRxTxSwitcher(void)
{
  #ifdef _HAL_RF_RX_TX_INDICATOR_
    phyRxTxSwitcherOn();
  #endif //_HAL_RF_RX_TX_INDICATOR_
}

/**************************************************************************//**
  \brief Enables Antenna diversity option for radio if that is supported.
******************************************************************************/
void HAL_InitAntennaDiversity(void)
{
  #ifdef _HAL_ANT_DIVERSITY_
    phyAntennaDiversityInit();
  #endif //_HAL_ANT_DIVERSITY_
}

/**************************************************************************//**
  \brief Enables Antenna diversity in RX mode for radio if that is supported.
******************************************************************************/
void HAL_EnableRxAntennaDiversity(void)
{
  #ifdef _HAL_ANT_DIVERSITY_
    phyRxAntennaDiversity();
  #endif //_HAL_ANT_DIVERSITY_
}

/**************************************************************************//**
  \brief Enables Antenna diversity in TX mode for radio if that is supported.
******************************************************************************/
void HAL_EnableTxAntennaDiversity(void)
{
  #ifdef _HAL_ANT_DIVERSITY_
    phyTxAntennaDiversity();
  #endif //_HAL_ANT_DIVERSITY_
}

/**************************************************************************//**
  \brief To switch from the antenna used for last transmission to the other
******************************************************************************/
void HAL_SwitchLastTxAntenna(void)
{
  #ifdef _HAL_ANT_DIVERSITY_
    phySwitchLastTxAntenna();
  #endif //_HAL_ANT_DIVERSITY_
}
//eof halRfCtrl.c
