/*****************************************************************************//**
  \file usartManager.h

  \brief

  \author
    Markus Krauﬂe
    Support email: wsn@fb2.fh-frankfurt.de


  \internal
******************************************************************************/
#ifndef _USARTMANAGER_H
#define _USARTMANAGER_H

#include <usart.h>


#define USART_MANAGER_CHANNEL USART_CHANNEL_1
#define USART_MANAGER_BAUDRATE USART_BAUDRATE_38400

#define USART_MANAGER_PACKET_SIZE 50
#define USART_MANAGER_BUFFER_SIZE 250

/******************************************************************************
                    Defines section
******************************************************************************/
HAL_UsartDescriptor_t usartDescriptor; // USART description

void appInitUsartManager(void);								//Initialisiert Usart Schnittstelle.
void appWriteDataToUsart(uint8_t* aData, uint8_t aLength);	//Fordert an aData an der Usart-Schnittstelle auszugeben.
bool appIsUsartEmpty(void);									//Testet, ob der Usart-Buffer abgearbeitet wurde.
void appCloseUsartManager(void);							//Schliesst die Usart-Schnittstelle wieder.

//Funktionen um Zahlen zu Strings oder Hexstrings umzuwandeln.
void uint8_to_hexstr(uint8_t * where, uint8_t size, uint8_t val, uint8_t position);
void uint16_to_hexstr(uint8_t * where, uint8_t size, uint16_t val, uint8_t position);
void uint32_to_hexstr(uint8_t * where, uint8_t size, uint32_t val, uint8_t position);
void uint64_to_hexstr(uint8_t * where, uint8_t size, uint64_t val, uint8_t position);
void uint32_to_str(uint8_t * where, uint8_t size, uint32_t val, uint8_t position,uint8_t digits);
void int32_to_str(uint8_t *where,  uint8_t size, int32_t val, uint8_t position, uint8_t digits);


#endif // _USARTMANAGER_H


