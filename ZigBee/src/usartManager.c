/**************************************************************************//**
  \file usartManager.c

  \author
    Markus Krauﬂe
    Email: wsn@fb2.fh-frankfurt.de

******************************************************************************/

//#include <types.h>
#include <usartManager.h>
#include <appTimer.h>


/***********************************************************************************
  Local variables
 ***********************************************************************************/

static bool usartBusy = false; // USART busy flag

static struct {
	uint8_t data[USART_MANAGER_PACKET_SIZE];
	uint8_t size;
} usartPacket;

static struct {
	uint8_t data[USART_MANAGER_BUFFER_SIZE];
	uint8_t begin,end,size;
} usartBuffer;



/***********************************************************************************
  Local Functions Prototypes
 ***********************************************************************************/


//CB-Funktion, die aufgerufen wird, sobald ide Usart-Schnittstelle erfolgreich das letzte Packet ausgegeben hat.
static void usartWriteConf(void); 
//Kopiert Daten als usartPacket zum direkten senden an die Usart-Schnittstelle.
static void copyStrToUsartPacket(uint8_t* aData, uint8_t aLength);	
//Kopiert Daten in den usartBuffer				
static void copyStrToBuffer(uint8_t* aData, uint8_t  position, uint8_t aLength);
//Breitet das n‰chste usartPacket zum senden an die Usart-Schnittstelle vor
//und holt dazu die Daten aus dem Buffer.
static void copyStrFromBufferToUsartPacket(void);




/***********************************************************************************
  Global Functions
 ***********************************************************************************/


/********************************************************************
  Description: Init USART, register USART callbacks.
********************************************************************/
void appInitUsartManager(void){
	usartDescriptor.tty            = USART_MANAGER_CHANNEL;
	usartDescriptor.mode           = USART_MODE_ASYNC;        // USART synchronization mode
	usartDescriptor.baudrate       = USART_MANAGER_BAUDRATE;    // USART baud rate
	usartDescriptor.dataLength     = USART_DATA8;             // USART data length
	usartDescriptor.parity         = USART_PARITY_NONE;       // USART parity mode.
	usartDescriptor.stopbits       = USART_STOPBIT_1;         // USART stop bit
	usartDescriptor.flowControl    = USART_FLOW_CONTROL_NONE; // Flow control
	usartDescriptor.rxBuffer       = NULL;
	usartDescriptor.rxBufferLength = 0;
	usartDescriptor.txBuffer       = NULL;
	usartDescriptor.txBufferLength = 0;
	usartDescriptor.txCallback     = usartWriteConf;  // Callback function, confirming data writing

	HAL_OpenUsart(&usartDescriptor); // Open USART using specified parameters
}





/********************************************************************
  Description: Create and send message to the USART

  Parameters: aData - frame being sent to USART
              aLength - size of the frame in bytes
********************************************************************/
void appWriteDataToUsart(uint8_t* aData, uint8_t aLength){
	aData=aData;
	aLength=aLength;
	if (usartBusy){ 		// USART-Port ist belegt, alles in den Buffer
		copyStrToBuffer(aData,0,aLength);
	} else {			// USART-Port ist frei
		if(aLength<=USART_MANAGER_PACKET_SIZE){  //Die ganzen Daten passen in ein usartPacket
			copyStrToUsartPacket(aData, aLength);
		}else{	//Die ganzen Daten passen nicht in ein usartPacket und der Rest muss in den Buffer
			copyStrToUsartPacket(aData,USART_MANAGER_PACKET_SIZE);
			copyStrToBuffer(aData,USART_MANAGER_PACKET_SIZE,aLength-USART_MANAGER_PACKET_SIZE);
		}
		usartBusy = true; // Mark USART port as occupied
		HAL_WriteUsart(&usartDescriptor, (void*) usartPacket.data, usartPacket.size); // Transmit data to USART
	}
}

/********************************************************************
  Description: Gibt an, ob der UsartBuffer leer ist.
********************************************************************/
bool appIsUsartEmpty(void){
	if(usartPacket.size || usartBuffer.size)
		return false;
	return true;
}

/********************************************************************
  Description: Schliesst die Usart-Schnittstelle wieder.
********************************************************************/
void appCloseUsartManager(void){
	HAL_CloseUsart(&usartDescriptor); 
}

/********************************************************************
	Description: Konvertiert uint8 in ein uint8 array als 2 stelliger (Hexstring)
  
	Parameters: where - das array in dem der string gespeichert wird.
				size - Die Grˆﬂe des arrays where (zur Sicherheit).
				value - der zu konvertierende String.
				position - die zu schreibende Position im ¸bergebenen Array
 ***********************************************************************************/
void uint8_to_hexstr(uint8_t * where, uint8_t size, uint8_t val, uint8_t position){
	if(size>position+1)
		for(int digit=1;digit>=0;digit--){
			where[position+digit] = val%16 < 10 ? val%16 + '0' : val%16 + 'A'-10;
			val /= 16;
		}	
}

/********************************************************************
	Description: Konvertiert uint16 in ein uint8 array als 4 stelliger (Hexstring)
  
	Parameters: where - das array in dem der string gespeichert wird.
				size - Die Grˆﬂe des arrays where (zur Sicherheit).
				value - der zu konvertierende String.
				position - die zu schreibende Position im ¸bergebenen Array
 ***********************************************************************************/
void uint16_to_hexstr(uint8_t * where, uint8_t size, uint16_t val, uint8_t position){
	if(size>position+3)
		for(int digit=3;digit>=0;digit--){
			where[position+digit] = val%16 < 10 ? val%16 + '0' : val%16 + 'A'-10;
			val /= 16;
		}	
}

/********************************************************************
	Description: Konvertiert uint32 in ein uint8 array als 8 stelliger (Hexstring)
  
	Parameters: where - das array in dem der string gespeichert wird.
				size - Die Grˆﬂe des arrays where (zur Sicherheit).
				value - der zu konvertierende String.
				position - die zu schreibende Position im ¸bergebenen Array
 ***********************************************************************************/
void uint32_to_hexstr(uint8_t * where, uint8_t size, uint32_t val, uint8_t position){
	if(size>position+7)
		for(int digit=7;digit>=0;digit--){
			where[position+digit] = val%16 < 10 ? val%16 + '0' : val%16 + 'A'-10;
			val /= 16;
		}	
}


/********************************************************************
	Description: Konvertiert uint64 in ein uint8 array als 16 stelliger (Hexstring)
  
	Parameters: where - das array in dem der string gespeichert wird.
				size - Die Grˆﬂe des arrays where (zur Sicherheit).
				value - der zu konvertierende String.
				position - die zu schreibende Position im ¸bergebenen Array
 ***********************************************************************************/
void uint64_to_hexstr(uint8_t * where, uint8_t size, uint64_t val, uint8_t position){
	if(size>position+15)
		for(int digit=15;digit>=0;digit--){
			where[position+digit] = val%16 < 10 ? val%16 + '0' : val%16 + 'A'-10;
			val /= 16;
		}	
}


/********************************************************************
	Description: Konvertiert uint32 zu einem uint8 array (string)
  
	Parameters: where - das array in dem der string gespeichert wird.
				value - der zu konvertierende String.
				position - die zu schreibende Position im ¸bergebenen Array
				digits - die Anzahl der Stellen die der Hex-String haben soll (ggf. mit 0 aufgef¸llt).
 ***********************************************************************************/
void uint32_to_str(uint8_t *where,  uint8_t size, uint32_t val, uint8_t position, uint8_t digits){
	if(size>position+digits-1){
		for(int digit=digits-1;digit>=0;digit--){
			where[position+digit] = (val % 10) + '0';
			val /= 10;
		}
	}	
}

/********************************************************************
	Description: Konvertiert uint32 zu einem uint8 array (string)
  
	Parameters: where - das array in dem der string gespeichert wird.
				value - der zu konvertierende String.
				position - die zu schreibende Position im ¸bergebenen Array
				digits - die Anzahl der Stellen die der Hex-String haben soll (ggf. mit 0 aufgef¸llt).
 ***********************************************************************************/
void int32_to_str(uint8_t *where,  uint8_t size, int32_t val, uint8_t position, uint8_t digits){
	if (val>=0){
		where[position] = (uint8_t)'+';
	}else{
		val*=-1;
		where[position] = (uint8_t)'-';
	}	
	if(size>position+1+digits-1){
		for(int digit=digits-2;digit>=0;digit--){
			where[position+1+digit] = (val % 10) + '0';
			val /= 10;
		}
	}	
}

void int16_to_str(uint8_t *where,  uint8_t size, int16_t val, uint8_t position, uint8_t digits){
	if (val>=0){
		where[position] = (uint8_t)'+';
		}else{
		val*=-1;
		where[position] = (uint8_t)'-';
	}
	if(size>position+digits-1){
		for(int digit=digits-1;digit>=0;digit--){
			where[position+digit] = (val % 10) + '0';
			val /= 10;
		}
	}
}

void uint16_to_str(uint8_t *where,  uint8_t size, uint16_t val, uint8_t position, uint8_t digits){
	
	if(size>position+1+digits-1){
		for(int digit=digits-2;digit>=0;digit--){
			where[position+1+digit] = (val % 10) + '0';
			val /= 10;
		}
	}
}

void int8_to_str(uint8_t *where,  uint8_t size, int8_t val, uint8_t position, uint8_t digits){
	if (val>=0){
		where[position] = (uint8_t)'+';
		}else{
		val*=-1;
		where[position] = (uint8_t)'-';
	}
	if(size>position+1+digits-1){
		for(int digit=digits-2;digit>=0;digit--){
			where[position+1+digit] = (val % 10) + '0';
			val /= 10;
		}
	}
}
void uint8_to_str(uint8_t * where, uint8_t size, uint8_t val, uint8_t position){
	if(size>position+1)
		for(int digit=1;digit>=0;digit--){
			where[position+digit] = (val % 10)  + '0';
			val /= 10;
		}	
}

/********************************************************************
  Description: USART writing confirmation has been received. New message can be sent.
********************************************************************/

static void usartWriteConf(void){
	usartPacket.size=0;
	usartBusy = false; // USART port has been released
	
	if (usartBuffer.size) { // Es gibt noch Daten im usartBuffer
		copyStrFromBufferToUsartPacket();
		usartBusy = true; 
		HAL_WriteUsart(&usartDescriptor, (void*) usartPacket.data, usartPacket.size); 
	}	
}

/********************************************************************
  Description:	Kopiert Teile von aData als uartPacket, welches als 
				n‰chstes an die Usart geschickt wird.
    
  Parameters:	aData - uint8_t String Array
				aLength - Die Anzahl der Zeichen, die kopiert werden sollen.
********************************************************************/
static void copyStrToUsartPacket(uint8_t* aData, uint8_t aLength){
	for(int i=0; i<aLength; i++){
		usartPacket.data[i]=aData[i];
	}
	usartPacket.size=aLength;
}

/********************************************************************
  Description:	Kopiert Teile von aData in den uartBuffer
  
  Parameters:	aData - uint8_t String Array
				position - Die Stelle von der ab aus dem Array kopiert werden soll.
				aLength - Die Anzahl der Zeichen, die kopiert werden sollen.
********************************************************************/
static void copyStrToBuffer(uint8_t* aData, uint8_t position, uint8_t aLength){
	if (aLength<=(USART_MANAGER_BUFFER_SIZE-usartBuffer.size)){ //Ist noch genug Platz im Buffer?
		for(int i=0; i<aLength; i++){
			usartBuffer.data[usartBuffer.end]=aData[position+i];
			usartBuffer.end=(usartBuffer.end+1)%USART_MANAGER_BUFFER_SIZE;
		}
		
		usartBuffer.size+=aLength;
	}	
}



/********************************************************************
  Description:	Kopiert Teile aus dem usartBuffer ins uartPacket, 
				welches dann als n‰chstes an die Usart geschickt wird.
********************************************************************/
static void copyStrFromBufferToUsartPacket(void){
	if(usartBuffer.size<=USART_MANAGER_PACKET_SIZE){  //Der Buffer wird geleert, es geht alles in ein Packet
		for(int i=0; i<usartBuffer.size; i++){
			usartPacket.data[i]=usartBuffer.data[usartBuffer.begin];
			usartBuffer.begin=(usartBuffer.begin+1)%USART_MANAGER_BUFFER_SIZE;
		}
		usartPacket.size=usartBuffer.size;
		usartBuffer.size=0;
	}else{
		for(int i=0; i<USART_MANAGER_PACKET_SIZE; i++){
			usartPacket.data[i]=usartBuffer.data[usartBuffer.begin];
			usartBuffer.begin=(usartBuffer.begin+1)%USART_MANAGER_BUFFER_SIZE;
		}
		usartPacket.size=USART_MANAGER_PACKET_SIZE;
		usartBuffer.size-=USART_MANAGER_PACKET_SIZE;
	}
}

//eof usartManager.c
