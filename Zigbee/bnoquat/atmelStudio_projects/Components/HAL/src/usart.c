/**************************************************************************//**
\file  usart.c

\brief USART implementation. Asynchronous mode.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/05/07 E. Ivanov - Created
    18/02/09 A. Luzhetsky - Corretced.
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#if defined(HAL_USE_USART)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <halDbg.h>
#include <usart.h>
#include <appTimer.h>
#include <halDiagnostic.h>
#include <sysAssert.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define HANDLERS_GET(A, I) memcpy_P(A, &halUsartHandlers[I], sizeof(HalUsartTask_t))
/** \brief Amount of reserved bytes in received buffer. Some clients (PC Windows for example)
  send few more bytes after CTS setting, so we need to reserve some space for them.
  Reserved space = Buffer Size / 2^BUFFER_RESERV. */
#define BUFFER_RESERV    1
#define USART_HW_CONTROLLER_TIMER_PERIOD 10
#if defined(_USE_USART_ERROR_EVENT_)
  #define HAL_BM_FRAME_ERROR     (1 << 4)
  #define HAL_BM_DATA_OVERRUN    (1 << 3)
  #define HAL_BM_PARITY_ERROR    (1 << 2)
#endif
#if NUM_USART_CHANNELS == 0
  #error 'USART channels is not alowed.'
#endif

#define HAL_USART_ALL_TASKS_ACCEPTED_MASK 0xFF

#if defined(HAL_USE_USART_CHANNEL_0)
  #define HAL_USART0_COMMON_TASKS_MASK  ((1u << HAL_USART_TASK_USART0_DRE) | \
                                         (1u << HAL_USART_TASK_USART0_TXC) | \
                                         (1u << HAL_USART_TASK_USART0_RXC))

  #if defined(_USE_USART_ERROR_EVENT_)
    #define HAL_USART0_ERROR_TASKS_MASK  (1u << HAL_USART_TASK_USART0_ERR)
  #else
    #define HAL_USART0_ERROR_TASKS_MASK  (0)
  #endif

  #define HAL_USART0_TASKS_MASK    (HAL_USART0_COMMON_TASKS_MASK | HAL_USART0_ERROR_TASKS_MASK)
#else
  #define HAL_USART0_TASKS_MASK    HAL_USART_ALL_TASKS_ACCEPTED_MASK
#endif /* HAL_USE_USART_CHANNEL_0 */

#if defined(HAL_USE_USART_CHANNEL_1)
  #define HAL_USART1_COMMON_TASKS_MASK  ((1u << HAL_USART_TASK_USART1_DRE) | \
                                         (1u << HAL_USART_TASK_USART1_TXC) | \
                                         (1u << HAL_USART_TASK_USART1_RXC))

  #if defined(_USE_USART_ERROR_EVENT_)
    #define HAL_USART1_ERROR_TASKS_MASK  (1u << HAL_USART_TASK_USART1_ERR)
  #else
    #define HAL_USART1_ERROR_TASKS_MASK  (0)
  #endif

  #define HAL_USART1_TASKS_MASK    (HAL_USART1_COMMON_TASKS_MASK | HAL_USART1_ERROR_TASKS_MASK)
#else
  #define HAL_USART1_TASKS_MASK    HAL_USART_ALL_TASKS_ACCEPTED_MASK
#endif /* HAL_USE_USART_CHANNEL_1 */

/******************************************************************************
                   Types definition section
******************************************************************************/
/**************************************************************************//**
  \brief HAL USART tasks bit mask.
******************************************************************************/
typedef volatile uint8_t HalUsartTaskBitMask_t;

/**************************************************************************//**
  \brief HAL USART task type declaration.
******************************************************************************/
typedef void (* HalUsartTask_t)(void);

/******************************************************************************
                   Global functions prototypes section
******************************************************************************/
void halSigUsartHandler(void);
void halSetUsartConfig(HAL_UsartDescriptor_t *usartmode);
void halPostUsartTask(HalUsartTaskId_t taskId);
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
  void hwControlPinsPollCallback(void);
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

/******************************************************************************
                   Static function prototypes section
******************************************************************************/
#if defined(HAL_USE_USART_CHANNEL_0)
  static void halUsartTaskUsart0Dre(void);
  static void halUsartTaskUsart0Txc(void);
  static void halUsartTaskUsart0Rxc(void);
  #if defined(_USE_USART_ERROR_EVENT_)
    static void halUsartTaskUsart0Err(void);
  #endif
#endif

#if defined(HAL_USE_USART_CHANNEL_1)
  static void halUsartTaskUsart1Dre(void);
  static void halUsartTaskUsart1Txc(void);
  static void halUsartTaskUsart1Rxc(void);
  #if defined(_USE_USART_ERROR_EVENT_)
    static void halUsartTaskUsart1Err(void);
  #endif
#endif

static void halUsartHwController(UsartChannel_t tty);
static void halSigUsartReceptionComplete(UsartChannel_t tty);
static void halSetUsartClockPinDirection(HAL_UsartDescriptor_t *descriptor);

/******************************************************************************
                   Static variables section
******************************************************************************/
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
  static HAL_AppTimer_t halUsartAppTimer;
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT
HAL_UsartDescriptor_t *halPointDescrip[NUM_USART_CHANNELS] =
{
  #if defined(HAL_USE_USART_CHANNEL_0)
    NULL,
  #endif
  #if defined(HAL_USE_USART_CHANNEL_1)
    NULL
  #endif
};
static volatile HalUsartTaskBitMask_t halUsartTaskBitMask = 0; // HAL USART tasks' bit mask.
static HalUsartTaskBitMask_t halUsartAcceptedTasks = HAL_USART_ALL_TASKS_ACCEPTED_MASK;
static PROGMEM_DECLARE(HalUsartTask_t halUsartHandlers[HAL_USART_TASKS_NUMBER]) =
{
  #if defined(HAL_USE_USART_CHANNEL_0)
    halUsartTaskUsart0Dre,
    halUsartTaskUsart0Txc,
    halUsartTaskUsart0Rxc,
    #if defined(_USE_USART_ERROR_EVENT_)
      halUsartTaskUsart0Err,
    #endif
  #endif

  #if defined(HAL_USE_USART_CHANNEL_1)
    halUsartTaskUsart1Dre,
    halUsartTaskUsart1Txc,
    halUsartTaskUsart1Rxc,
    #if defined(_USE_USART_ERROR_EVENT_)
      halUsartTaskUsart1Err,
    #endif
  #endif
}; // List Of possible HAL USART tasks.

/******************************************************************************
  DTR service
******************************************************************************/
volatile bool halEnableDtrWakeUp = false;
void (* dtrWakeUpCallback)(void) = NULL;

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief  HAL USART task. Exact action depends on USART internal task.
******************************************************************************/
void halSigUsartHandler(void)
{
  HalUsartTask_t         handler;
  HalUsartTaskBitMask_t  mask = 1;
  HalUsartTaskId_t       index = 0;
  bool                   action = false;

  for ( ; index < HAL_USART_TASKS_NUMBER; index++, mask <<= 1)
  {
    ATOMIC_SECTION_ENTER
    BEGIN_MEASURE
    if (halUsartTaskBitMask & mask & halUsartAcceptedTasks)
      action = true;
    END_MEASURE(HALATOM_USART_CHECK_TASK_MEASURE_CODE)
    ATOMIC_SECTION_LEAVE

    if (action)
    {
      action = false;
      ATOMIC_SECTION_ENTER
      BEGIN_MEASURE
      halUsartTaskBitMask ^= mask;
      END_MEASURE(HALATOM_USART_CLEAR_TASK_MEASURE_CODE)
      ATOMIC_SECTION_LEAVE
      HANDLERS_GET(&handler, index);
      handler();
    }
  }
}

/**************************************************************************//**
\brief Posts specific USART task.

\param[in]
  taskId - unique identifier of the task to be posted.
******************************************************************************/
void halPostUsartTask(HalUsartTaskId_t taskId)
{
  halUsartTaskBitMask |= (HalUsartTaskBitMask_t)1 << taskId;
  halPostTask(HAL_TASK_USART);
}

/**************************************************************************//**
\brief Holds execution of all tasks except related to reqiured channel.

\param[in] tty - channel to accept tasks for
******************************************************************************/
void HAL_HoldOnOthersUsartTasks(UsartChannel_t tty)
{
  HalUsartTaskBitMask_t tmpMask = 0U;

  if (USART_CHANNEL_0 == tty)
    tmpMask = HAL_USART0_TASKS_MASK;
  else if (USART_CHANNEL_1 == tty)
    tmpMask = HAL_USART1_TASKS_MASK;

  halUsartAcceptedTasks &= tmpMask;
}

/**************************************************************************//**
\brief Accepts execution of previously holded tasks.
******************************************************************************/
void HAL_ReleaseAllHeldUsartTasks(void)
{
  halUsartAcceptedTasks = HAL_USART_ALL_TASKS_ACCEPTED_MASK;
  if (halUsartTaskBitMask)
    halPostTask(HAL_TASK_USART);
}

/**************************************************************************//**
\brief Puts the byte received to the cyclic buffer.

\param[in]
  tty - channel number.
\param[in]
  data - data to put.
******************************************************************************/
void halUsartRxBufferFiller(UsartChannel_t tty, uint8_t data)
{
  uint16_t           old;
  uint8_t            i;
  HalUsartService_t *halUsartControl;

  i = HAL_GET_INDEX_BY_CHANNEL(tty);
  if (NULL == halPointDescrip[i])
  {// abnormal
    halDisableUsartRxcInterrupt(tty); // disable usart
    return;
  }

  if (halPointDescrip[i]->flowControl & USART_SPI_WRITE_MODE)
    return;

  if (halPointDescrip[i]->flowControl & USART_SPI_READ_MODE)
  { // For spi mode.
    *(uint8_t*)(halPointDescrip[i]->rxBuffer) = data;
    halPointDescrip[i]->rxBuffer++;
    return;
  } // For spi mode.

  halUsartControl = &halPointDescrip[i]->service;
  if (NULL != halPointDescrip[i]->rxBuffer)
  {
    old = halUsartControl->rxPointOfWrite;

    if (++halUsartControl->rxPointOfWrite == halPointDescrip[i]->rxBufferLength)
      halUsartControl->rxPointOfWrite = 0;

    if (halUsartControl->rxPointOfWrite == halUsartControl->rxPointOfRead)
    { // Buffer full.
      halUsartControl->rxPointOfWrite = old;
      return;
    } // Buffer full.

    halPointDescrip[i]->rxBuffer[old] = data;
    halUsartControl->rxBytesInBuffer++;

#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
    if ((halPointDescrip[i]->flowControl & USART_FLOW_CONTROL_HARDWARE) && (HW_CONTROL_PINS_PORT_ASSIGNMENT == halPointDescrip[i]->tty))
    {
      if (halUsartControl->rxBytesInBuffer > (halPointDescrip[i]->rxBufferLength >> BUFFER_RESERV))
        GPIO_USART_CTS_set();// CTS_ON
    }
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT
  }
}

#if defined(_USE_USART_ERROR_EVENT_)
/**************************************************************************//**
\brief Save status register for analyzing of the error reason.

\param[in]
  tty - channel number.
\param[in]
  status - usart status register.
******************************************************************************/
void halUsartSaveErrorReason(UsartChannel_t tty, uint8_t status)
{
  HalUsartService_t *halUsartControl;
  uint8_t            i;

  i = HAL_GET_INDEX_BY_CHANNEL(tty);
  if (NULL == halPointDescrip[i])
  {// abnormal
    halDisableUsartRxcInterrupt(tty); // disable usart
    return;
  }

  halUsartControl = &halPointDescrip[i]->service;
  halUsartControl->errorReason = status;
}
#endif

/**************************************************************************//**
\brief Registers uasrt's event handlers. Performs configuration
of usart registers. Performs configuration of RTS, CTS and DTR pins.

\param[in]
  descriptor - pointer to HAL_UsartDescriptor_t structure

\return
  Returns positive usart descriptor on success or -1 in cases: \n
    - bad usart channel. \n
    - unsupported parameters. \n
    - the channel was already opened. \n
    - there are not enough resources. \n
******************************************************************************/
int HAL_OpenUsart(HAL_UsartDescriptor_t *descriptor)
{
  uint8_t i; // Descriptor index

  if (NULL == descriptor)
    return -1;
  if (false == halIsUsartChannelCorrect(descriptor->tty))
    return -1;

#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
  if ((descriptor->flowControl & USART_FLOW_CONTROL_HARDWARE) &&
      (HW_CONTROL_PINS_PORT_ASSIGNMENT != descriptor->tty))
    return -1; // Hardware control cannot be used for this channel.
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

  i = HAL_GET_INDEX_BY_CHANNEL(descriptor->tty);
  if (NULL != halPointDescrip[i])
    return -1; // Channel is already opened.

  halPointDescrip[i] = descriptor;
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
  if (HW_CONTROL_PINS_PORT_ASSIGNMENT == descriptor->tty)
  {
    if (descriptor->flowControl & USART_DTR_CONTROL)
      GPIO_USART_DTR_make_in();
    if (descriptor->flowControl & USART_FLOW_CONTROL_HARDWARE)
    {
      GPIO_USART_CTS_make_out();
      GPIO_USART_RTS_make_in();
      if (NULL == descriptor->rxBuffer)
        GPIO_USART_CTS_set(); // CTS_ON
      else
        GPIO_USART_CTS_clr(); // CTS_OFF
    }
  }
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

  if (USART_MODE_SYNC == descriptor->mode)
    halSetUsartClockPinDirection(descriptor);

  descriptor->service.txPointOfRead = 0;
  descriptor->service.txPointOfWrite = 0;
  if (NULL == descriptor->rxBuffer)
    descriptor->rxBufferLength = 0;
  if (NULL == descriptor->txBuffer)
    descriptor->txBufferLength = 0;
  descriptor->service.rxPointOfRead = 0;
  descriptor->service.rxPointOfWrite = 0;
  descriptor->service.usartShiftRegisterEmpty = 1;

  halSetUsartConfig(descriptor);

  return descriptor->tty;
}

/**************************************************************************//**
\brief Frees the usart channel and pins, if hardware flow control was used.

\param[in]
  descriptor - the usart descriptor.
\return
   0 on success, \n
  -1 if bad descriptor or channel is already closed.
******************************************************************************/
int HAL_CloseUsart(HAL_UsartDescriptor_t *descriptor)
{
  uint8_t i;

  if (NULL == descriptor)
    return -1;
  if (false == halIsUsartChannelCorrect(descriptor->tty))
    return -1;
  i = HAL_GET_INDEX_BY_CHANNEL(descriptor->tty);
  if (NULL == halPointDescrip[i])
    return -1; // Channel is already closed.

  /* halCloseUsart inline function returns from here is channel is wrong */
  if (0 != halCloseUsart(halPointDescrip[i]->tty))
    return -1;

#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
  if (halPointDescrip[i]->flowControl & USART_FLOW_CONTROL_HARDWARE)
    GPIO_USART_CTS_make_in();
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

  if (USART_MODE_SYNC == halPointDescrip[i]->mode)
  {
    halPointDescrip[i]->syncMode = USART_CLK_MODE_SLAVE;
    halSetUsartClockPinDirection(halPointDescrip[i]);
  }
  halPointDescrip[i] = NULL;

  return 0;
}

/**************************************************************************//**
\brief Controls RTS and DTR the pins and makes decision if the usart can transmit
 byte.

\param[in]
  tty - channel number.
******************************************************************************/
static void halUsartHwController(UsartChannel_t tty)
{
  uint8_t            i;
  HalUsartService_t *halUsartControl;

  i = HAL_GET_INDEX_BY_CHANNEL(tty);
  if (NULL == halPointDescrip[i])
    return; // Port closed.

  halUsartControl = &halPointDescrip[i]->service;
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
  if (HW_CONTROL_PINS_PORT_ASSIGNMENT == tty)
  {
    uint8_t hw1 = 0;
    uint8_t hw2 = 0;

    if (halPointDescrip[i]->flowControl & USART_DTR_CONTROL)
      hw1 = GPIO_USART_DTR_read();

    if (halPointDescrip[i]->flowControl & USART_FLOW_CONTROL_HARDWARE)
      hw2 = GPIO_USART_RTS_read();

    if (hw1 || hw2)
    {
      halUsartAppTimer.interval = USART_HW_CONTROLLER_TIMER_PERIOD;
      halUsartAppTimer.mode = TIMER_ONE_SHOT_MODE;
      halUsartAppTimer.callback = hwControlPinsPollCallback;
      HAL_StartAppTimer(&halUsartAppTimer);
      return;
    }
  }
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

  uint16_t poW;
  uint16_t poR;

  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    poW = halUsartControl->txPointOfWrite;
    poR = halUsartControl->txPointOfRead;
  END_MEASURE(HAL_USART_HW_CONTROLLER_LIMIT)
  ATOMIC_SECTION_LEAVE

  if (poW != poR)
  {
    halSendUsartByte(tty, halPointDescrip[i]->txBuffer[poR++]);
    if (poR == halPointDescrip[i]->txBufferLength)
      poR = 0;
    halEnableUsartDremInterrupt(tty);

    ATOMIC_SECTION_ENTER
    BEGIN_MEASURE
      halUsartControl->txPointOfRead = poR;
    END_MEASURE(HAL_USART_HW_CONTROLLER_LIMIT)
    ATOMIC_SECTION_LEAVE

  }
  else
  {
    // data register empty interrupt was disabled
    halEnableUsartTxcInterrupt(tty);// TX Complete interrupt enable
  }
}

/**************************************************************************//**
\brief Writes a number of bytes to a usart channel.
txCallback function will be used to notify when the transmission is finished.
If hardware flow control is used for transmitting then RTS and DTR pins will
be tested during transmission.

\param[in]
  descriptor - pointer to HAL_UsartDescriptor_t structure;

\param[in]
  buffer - pointer to the application data buffer;

\param[in]
  length - number of bytes to transfer;

\return
  -1 - bad descriptor; \n
   Number of bytes placed to the buffer - success.
******************************************************************************/
int HAL_WriteUsart(HAL_UsartDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
  uint8_t            i;
  uint16_t           poW;
  uint16_t           poR;
  uint16_t           old;
  uint16_t           wasWrote = 0;
  bool               needStartTrmt = false;
  HalUsartService_t *halUsartControl;

  if (NULL == descriptor)
    return -1;
  if (false == halIsUsartChannelCorrect(descriptor->tty))
    return -1;
  if (!buffer || !length)
    return -1;
  i = HAL_GET_INDEX_BY_CHANNEL(descriptor->tty);
  if (descriptor != halPointDescrip[i])
    return -1; // Channel is not opened.

  halUsartControl = &descriptor->service;
  if (0 == descriptor->txBufferLength)
  { // Callback mode
    if (halUsartControl->txPointOfWrite != halUsartControl->txPointOfRead)
      return -1; // there is unsent data
    descriptor->txBuffer = buffer;
    halUsartControl->txPointOfWrite = length;
    halUsartControl->txPointOfRead = 0;
    needStartTrmt = true;
    wasWrote = length;
  } // Callback mode.
  else
  { // Polling mode.
    ATOMIC_SECTION_ENTER
    BEGIN_MEASURE
      poW = halUsartControl->txPointOfWrite;
      poR = halUsartControl->txPointOfRead;
    END_MEASURE(HALATOM_WRITE_USART_0_MEASURE_CODE)
    ATOMIC_SECTION_LEAVE

    if (poW == poR)
      needStartTrmt = true; // Buffer empty.

    while (wasWrote < length)
    {
      old = poW;

      if (++poW == descriptor->txBufferLength)
        poW = 0;

      if (poW == poR)
      { // Buffer full.
        poW = old;
        break;
      } // Buffer full.

      descriptor->txBuffer[old] = buffer[wasWrote++];
    }

    ATOMIC_SECTION_ENTER
    BEGIN_MEASURE
      halUsartControl->txPointOfWrite = poW;
    END_MEASURE(HALATOM_WRITE_USART_1_MEASURE_CODE)
    ATOMIC_SECTION_LEAVE
  } // Polling mode

  if (needStartTrmt)
  {
    halUsartControl->usartShiftRegisterEmpty = 0; // Buffer and shift register is full
    // Enable interrupt. Transaction will be launched in the callback.
    halEnableUsartDremInterrupt(descriptor->tty);
  }

  return wasWrote;
}

/*************************************************************************//**
\brief Reads length bytes from usart and places ones to buffer.

\param[in]
  descriptor - usart descriptor;
\param[out]
  buffer - pointer to a application buffer;
\param[in]
  length - the number of bytes which should be placed to buffer

\return
  -1 - bad descriptor, bad number to read or number of bytes that \n
  were placed to buffer.
*****************************************************************************/
int HAL_ReadUsart(HAL_UsartDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
  uint8_t            i = 0;
  uint16_t           wasRead = 0;
  uint16_t           poW;
  uint16_t           poR;
  HalUsartService_t *halUsartControl;
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
  uint16_t           number;
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

  if (NULL == descriptor)
    return -1;
  if (false == halIsUsartChannelCorrect(descriptor->tty))
    return -1;
  if (!buffer || !length)
    return -1;
  i = HAL_GET_INDEX_BY_CHANNEL(descriptor->tty);
  if (descriptor != halPointDescrip[i])
    return -1; // Channel is not opened.

  halUsartControl = &halPointDescrip[i]->service;
  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    poW = halUsartControl->rxPointOfWrite;
    poR = halUsartControl->rxPointOfRead;
  END_MEASURE(HALATOM_READ_USART_0_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

  while ((poR != poW) && (wasRead < length))
  {
    buffer[wasRead] = descriptor->rxBuffer[poR];
    if (++poR == descriptor->rxBufferLength)
      poR = 0;
    wasRead++;
  }

  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    halUsartControl->rxPointOfRead = poR;
    halUsartControl->rxBytesInBuffer -= wasRead;
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
    number = halUsartControl->rxBytesInBuffer;
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT
  END_MEASURE(HALATOM_READ_USART_1_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
  if ((HW_CONTROL_PINS_PORT_ASSIGNMENT == descriptor->tty) && (descriptor->flowControl & USART_FLOW_CONTROL_HARDWARE))
    if (number <= (descriptor->rxBufferLength >> BUFFER_RESERV))
      GPIO_USART_CTS_clr();
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

  return wasRead;
}

/**************************************************************************//**
\brief Forbids to the host data transmiting. Only HW_CONTROL_PINS_PORT_ASSIGNMENT
         port can be used for hardware flow control.

\param[in]
  descriptor - usart descriptor.

\return
  -1 - bad descriptor, bad usart, unsupported mode;
   0 - on success.
******************************************************************************/
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
int HAL_OnUsartCts(HAL_UsartDescriptor_t *descriptor)
{
  uint8_t i;

  if (NULL == descriptor)
    return -1;

  if (false == halIsUsartChannelCorrect(descriptor->tty))
    return -1;

  i = HAL_GET_INDEX_BY_CHANNEL(descriptor->tty);
  if (descriptor != halPointDescrip[i])
    return -1; // Channel is not opened.

  if (HW_CONTROL_PINS_PORT_ASSIGNMENT != descriptor->tty)
    return -1;

  GPIO_USART_CTS_set();// CTS_ON

  return 0;
}
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

/**************************************************************************//**
\brief Allows to transfer a host data. Only HW_CONTROL_PINS_PORT_ASSIGNMENT
can be used for hardware flow control.

\param[in]
  descriptor - usart descriptor.

\return
  -1 - bad descriptor, bad usart, unsupported mode;
   0 - on success.
******************************************************************************/
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
int HAL_OffUsartCts(HAL_UsartDescriptor_t *descriptor)
{
  uint8_t i;

  if (NULL == descriptor)
    return -1;

  if (false == halIsUsartChannelCorrect(descriptor->tty))
    return -1;

  i = HAL_GET_INDEX_BY_CHANNEL(descriptor->tty);
  if (descriptor != halPointDescrip[i])
    return -1; // Channel is not opened.

  if (HW_CONTROL_PINS_PORT_ASSIGNMENT != descriptor->tty)
    return -1;

  GPIO_USART_CTS_clr(); // CTS_OFF

  return 0;
}
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

/**************************************************************************//**
\brief Fills UsartHardwareControl_t variable by potential of RTS pin. Only
  HW_CONTROL_PINS_PORT_ASSIGNMENT can be used for hardware flow control.

\param[in]
  descriptor - usart descriptor.
\return
  -1 - bad descriptor, bad usart, unsupported mode;
   0 - on success.
******************************************************************************/
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
int HAL_ReadUsartRts(HAL_UsartDescriptor_t *descriptor)
{
  uint8_t i;

  if (NULL == descriptor)
    return -1;

  if (false == halIsUsartChannelCorrect(descriptor->tty))
    return -1;

  i = HAL_GET_INDEX_BY_CHANNEL(descriptor->tty);
  if (descriptor != halPointDescrip[i])
    return -1; // Channel is not opened.

  if (HW_CONTROL_PINS_PORT_ASSIGNMENT != descriptor->tty)
    return -1;

  return GPIO_USART_RTS_read();
}
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

/**************************************************************************//**
\brief Fills UsartHardwareControl_t variable by potential of DTR pin. Only
  HW_CONTROL_PINS_PORT_ASSIGNMENT can be used for hardware flow control.

\param[in]
  descriptor - usart descriptor.
\return
  -1 - bad descriptor, bad usart, unsupported mode;
   0 - on success.
******************************************************************************/
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
int HAL_ReadUsartDtr(HAL_UsartDescriptor_t *descriptor)
{
  uint8_t i;

  if (NULL == descriptor)
    return -1;

  if (false == halIsUsartChannelCorrect(descriptor->tty))
    return -1;

  i = HAL_GET_INDEX_BY_CHANNEL(descriptor->tty);
  if (descriptor != halPointDescrip[i])
    return -1; // Channel is not opened.

  if (HW_CONTROL_PINS_PORT_ASSIGNMENT != descriptor->tty)
    return -1;

  return GPIO_USART_DTR_read();
}
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

// Interrupt handlers
/**************************************************************************//**
\brief Hardware Control pins polling timer callback.
******************************************************************************/
#ifdef HW_CONTROL_PINS_PORT_ASSIGNMENT
void hwControlPinsPollCallback(void)
{
  halUsartHwController(HW_CONTROL_PINS_PORT_ASSIGNMENT);
}
#endif // HW_CONTROL_PINS_PORT_ASSIGNMENT

/**************************************************************************//**
\brief Transmission complete interrupt handler.

\param[in]
  tty - USART channel identifier.
******************************************************************************/
void halSigUsartTransmissionComplete(UsartChannel_t tty)
{
  uint8_t            i;
  HalUsartService_t *halUsartControl;
  uint16_t           poW;
  uint16_t           poR;

  i = HAL_GET_INDEX_BY_CHANNEL(tty);
  if (NULL == halPointDescrip[i])
  {
    SYS_E_ASSERT_FATAL(false, USARTC_HALSIGUSARTTRANSMISSIONCOMPLETE_0);
    return; // Descriptor with "tty" channel is not found.
  }

  halUsartControl = &halPointDescrip[i]->service;

  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    poW = halUsartControl->txPointOfWrite;
    poR = halUsartControl->txPointOfRead;
  END_MEASURE(HAL_USART_TRANS_COMPLETE_LIMIT)
  ATOMIC_SECTION_LEAVE

  if (poW == poR)
    halUsartControl->usartShiftRegisterEmpty = 1; // Buffer is empty, shift register is empty too.

  if (0 == halPointDescrip[i]->txBufferLength)
    halPointDescrip[i]->txBuffer = NULL; // nulling pointer for callback mode

  if (NULL != halPointDescrip[i]->txCallback)
    halPointDescrip[i]->txCallback();
}

/**************************************************************************//**
\brief Reception complete interrupt handler.

\param[in]
  tty - USART channel identifier.
******************************************************************************/
static void halSigUsartReceptionComplete(UsartChannel_t tty)
{
  uint8_t            i;
  HalUsartService_t *halUsartControl;
  uint16_t           number;

  i = HAL_GET_INDEX_BY_CHANNEL(tty);
  if (NULL == halPointDescrip[i])
  {
    SYS_E_ASSERT_FATAL(false, USARTC_HALSIGUSARTRECEPTIONCOMPLETE_0);
    return; // Descriptor with "tty" channel is not found.
  }

  if (halPointDescrip[i]->flowControl & (USART_SPI_READ_MODE | USART_SPI_WRITE_MODE))
    return; // for spi mode

  halUsartControl = &halPointDescrip[i]->service;
  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    number = halUsartControl->rxBytesInBuffer;
  END_MEASURE(HALATOM_USART_RX_COMPLETE_MEASURE_CODE)
  ATOMIC_SECTION_LEAVE

  if (number)
    if (NULL != halPointDescrip[i]->rxCallback)
      halPointDescrip[i]->rxCallback(number);
}

#if defined(_USE_USART_ERROR_EVENT_)
/**************************************************************************//**
\brief Error occurred action handler.

\param[in]
  tty - USART channel identifier.
******************************************************************************/
static void halSigUsartErrorOccurred(UsartChannel_t tty)
{
  uint8_t            i;
  HalUsartService_t *halUsartControl;
  UsartErrorReason_t errReason = FRAME_ERROR;

  i = HAL_GET_INDEX_BY_CHANNEL(tty);
  if (NULL == halPointDescrip[i])
  {
    SYS_E_ASSERT_FATAL(false, USARTC_HALSIGUSARTERROROCCURED_0);
    return; // Descriptor with "tty" channel is not found.
  }

  halUsartControl = &halPointDescrip[i]->service;
  if (halUsartControl->errorReason & HAL_BM_FRAME_ERROR)
    errReason = FRAME_ERROR;
  else if (halUsartControl->errorReason & HAL_BM_DATA_OVERRUN)
    errReason = DATA_OVERRUN;
  else if (halUsartControl->errorReason & HAL_BM_PARITY_ERROR)
    errReason = PARITY_ERROR;
  else
  {
    SYS_E_ASSERT_FATAL(false, USARTC_HALUNKNOWNERRORREASON_0);
  }

  if (NULL != halPointDescrip[i]->errCallback)
    halPointDescrip[i]->errCallback(errReason);
}
#endif

/**************************************************************************//**
\brief Enables DTR wake up.

\param[in] callback - callback method pointer.
******************************************************************************/
void HAL_EnableDtrWakeUp(void (* callback)(void))
{
  dtrWakeUpCallback = callback;
  halEnableDtrWakeUp = true;
}

/**************************************************************************//**
\brief Disables DTR wake up.
******************************************************************************/
void HAL_DisableDtrWakeUp(void)
{
  halEnableDtrWakeUp = false;
}

/**************************************************************************//**
\brief Checks the status of tx buffer.

\param[in]
  descriptor - pointer to HAL_UsartDescriptor_t structure;

\return
  -1 - bad descriptor, no tx buffer; \n
   1 - tx buffer is empty; \n
   0 - tx buffer is not empty;
******************************************************************************/
int HAL_IsTxEmpty(HAL_UsartDescriptor_t *descriptor)
{
  uint8_t            i;
  HalUsartService_t *halUsartControl;
  uint16_t           poW;
  uint16_t           poR;

  if (NULL == descriptor)
     return -1;
  if (false == halIsUsartChannelCorrect(descriptor->tty))
     return -1;
  i = HAL_GET_INDEX_BY_CHANNEL(descriptor->tty);
  if (descriptor != halPointDescrip[i])
     return -1; // Channel is not opened.

  halUsartControl = &halPointDescrip[i]->service;
  ATOMIC_SECTION_ENTER
  BEGIN_MEASURE
    poW = halUsartControl->txPointOfWrite;
    poR = halUsartControl->txPointOfRead;
  END_MEASURE(HAL_USART_TX_EMPTY_LIMIT)
  ATOMIC_SECTION_LEAVE
  if (poW == poR)
    return halUsartControl->usartShiftRegisterEmpty;
  return 0;
}

/**************************************************************************//**
\brief Checks the channel number.

\param[in]
  channel - channel to be verified.

\return
  true if channel is possible, \n
  false otherwise.
******************************************************************************/
bool halIsUsartChannelCorrect(UsartChannel_t channel)
{
  switch (channel)
  {
#ifdef USART_CHANNEL_0
    case USART_CHANNEL_0:
#endif // USART_CHANNEL_0
#ifdef USART_CHANNEL_1
    case USART_CHANNEL_1:
#endif // USART_CHANNEL_0
#if defined(USART_CHANNEL_0) || defined(USART_CHANNEL_1)
      return true;
#endif
    default:
      return false;
  }
}

/**************************************************************************//**
\brief Set clock pin direction for synchronous mode.

\param[in]
  descriptor - pointer to usart channel descriptor.
******************************************************************************/
static void halSetUsartClockPinDirection(HAL_UsartDescriptor_t *descriptor)
{
  if (USART_CLK_MODE_MASTER == descriptor->syncMode)
  {
    switch (descriptor->tty)
    {
#ifdef USART_CHANNEL_0
      case USART_CHANNEL_0:
        GPIO_USART0_EXTCLK_make_out();
        break;
#endif // USART_CHANNEL_0
#ifdef USART_CHANNEL_1
      case USART_CHANNEL_1:
        GPIO_USART1_EXTCLK_make_out();
        break;
#endif // USART_CHANNEL_1
      default:
        break;
    }
  }
  else
  {
    switch (descriptor->tty)
    {
#ifdef USART_CHANNEL_0
      case USART_CHANNEL_0:
        GPIO_USART0_EXTCLK_make_in();
        GPIO_USART0_EXTCLK_make_pullup();
        break;
#endif // USART_CHANNEL_0
#ifdef USART_CHANNEL_1
      case USART_CHANNEL_1:
        GPIO_USART1_EXTCLK_make_in();
        GPIO_USART1_EXTCLK_make_pullup();
        break;
#endif // USART_CHANNEL_1
      default:
      break;
    }
  }
}

#if defined(HAL_USE_USART_CHANNEL_0)
/**************************************************************************//**
\brief Wrapper for data empty handler for usart channel 0
******************************************************************************/
static void halUsartTaskUsart0Dre(void)
{
  halUsartHwController(USART_CHANNEL_0);
}

/**************************************************************************//**
\brief Wrapper for transmit complete handler for usart channel 0
******************************************************************************/
static void halUsartTaskUsart0Txc(void)
{
  halSigUsartTransmissionComplete(USART_CHANNEL_0);
}

/**************************************************************************//**
\brief Wrapper for receive complete handler for usart channel 0
******************************************************************************/
static void halUsartTaskUsart0Rxc(void)
{
  halSigUsartReceptionComplete(USART_CHANNEL_0);
}

#if defined(_USE_USART_ERROR_EVENT_)
/**************************************************************************//**
\brief Wrapper for error occurred handler for usart channel 0
******************************************************************************/
static void halUsartTaskUsart0Err(void)
{
  halSigUsartErrorOccurred(USART_CHANNEL_0);
}
#endif // defined(_USE_USART_ERROR_EVENT_)
#endif // defined(HAL_USE_USART_CHANNEL_0)

#if defined(HAL_USE_USART_CHANNEL_1)
/**************************************************************************//**
\brief Wrapper for data empty handler for usart channel 1
******************************************************************************/
static void halUsartTaskUsart1Dre(void)
{
  halUsartHwController(USART_CHANNEL_1);
}

/**************************************************************************//**
\brief Wrapper for transmit complete handler for usart channel 1
******************************************************************************/
static void halUsartTaskUsart1Txc(void)
{
  halSigUsartTransmissionComplete(USART_CHANNEL_1);
}

/**************************************************************************//**
\brief Wrapper for receive complete handler for usart channel 0
******************************************************************************/
static void halUsartTaskUsart1Rxc(void)
{
  halSigUsartReceptionComplete(USART_CHANNEL_1);
}

#if defined(_USE_USART_ERROR_EVENT_)
/**************************************************************************//**
\brief Wrapper for error occurred handler for usart channel 1
******************************************************************************/
static void halUsartTaskUsart1Err(void)
{
  halSigUsartErrorOccurred(USART_CHANNEL_1);
}
#endif // defined(_USE_USART_ERROR_EVENT_)
#endif // defined(HAL_USE_USART_CHANNEL_1)
#endif // defined(HAL_USE_USART)

//eof usart.c
