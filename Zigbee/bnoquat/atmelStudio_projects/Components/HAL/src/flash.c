/**************************************************************************//**
  \file  flash.c

  \brief Implementation of flash access hardware-independent module.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2015, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      27/06/12 D. Kolmakov - Created
  Last change:
    $Id: flash.c 27584 2015-01-09 14:45:42Z unithra.c $
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#if defined(HAL_USE_FLASH_ACCESS)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <sysTypes.h>
#include <atomic.h>
#include <halSpDriver.h>
#include <halFlash.h>
#include <flash.h>
#include <halDbg.h>
#include <sysUtils.h>

/******************************************************************************
                   Definitions section
******************************************************************************/
#define EEPROM_BUSY_WAIT()              do{} while (EECR & (1 << EEPE));
#define FLASH_BUSY_WAIT(hndlr)          while (SPMCSR & (1 << SPMEN)) {if (hndlr) hndlr();}
#define PAGE_NUMBER_SHIFT               8
#define PAGE_POSITION_OFFSET_MASK       0xFF
#define FORBIDEN_ACCESS_POINT_VALUE     ((HAL_BootSectionAccessPoint_t)(~0u))

#if defined(__ICCAVR__)
/* Since standard c doesn’t have this pointer type, so the coverity tool picked up this as error
   Normal IAR Compiler behavior for accessing the memory of more than 64Kbytes */

#define READ_ACCESS_POINT(to, from) memcpy_P(&to, (__farflash void *)from, sizeof(HAL_BootSectionAccessPoint_t)); \
                                      if (to == FORBIDEN_ACCESS_POINT_VALUE) to = NULL;
#elif defined(__GNUC__)
  #define READ_ACCESS_POINT(to, from) to = (void *)pgm_read_word_far(from); \
                                      if (to == FORBIDEN_ACCESS_POINT_VALUE) to = NULL;
#endif

/******************************************************************************
                   Types section
******************************************************************************/
typedef struct _HalPageWriteContext_t
{
  /* Word which keeps pending byte value */
  uint16_t  word;
  /* Address to write pending byte */
  uint32_t address;
  /* Pending byte presence indicator */
  bool     pendingByte;
} HalPageWriteContext_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
static void fillPageBufferWithPendingWord(void);

/******************************************************************************
                   Local variables section
******************************************************************************/
static HAL_FlashAccessReq_t bootFlashReq;
static HalPageWriteContext_t writeContext =
{
  .pendingByte = false
};

// Queue of registered while write flash poll handlers
static DECLARE_QUEUE(pollDuringFlashWritingHandlers);

/******************************************************************************
                   Prototypes section
******************************************************************************/
BOOT_SECTION_DECLARE void HAL_FlashAccessRequestHandler(void);
BOOT_SECTION_DECLARE static void halFillFlashPageBuffer(HAL_FillPageBufferReq_t *req);
BOOT_SECTION_DECLARE static void halProgramFlashPage(HAL_ProgramPageReq_t *req);
BOOT_SECTION_DECLARE static void halEraseFlashPage(HAL_ErasePageReq_t *req);
BOOT_SECTION_DECLARE static void halWriteFlashPage(HAL_WritePageReq_t *req);
BOOT_SECTION_DECLARE NO_RETURN static void halSwapImages(HAL_SwapImagesReq_t *req);

NRWW_SECTION_DECLARE void HAL_PollDuringFlashWritingAccessPoint(void);

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
  \brief Fills Flash page buffer with word w
******************************************************************************/
static void fillPageBufferWithPendingWord(void)
{
  /* Fill buffer with pending word */
  bootFlashReq.id = HAL_FLASH_ACCESS_FILL_PAGE_BUFFER;

  bootFlashReq.payload.fillPageBuffer.data = (uint8_t*)&writeContext.word;
  bootFlashReq.payload.fillPageBuffer.offset = writeContext.address;
  bootFlashReq.payload.fillPageBuffer.length = sizeof(writeContext.word);
  halFlashAccessReq(&bootFlashReq);

  writeContext.pendingByte = false;
}

/**************************************************************************//**
  \brief Writes data to the page buffer.
  \param[in] startOffset - start position for writing within the page
  \param[in] length - data to write length in bytes
  \param[in] data - pointer to data to write
******************************************************************************/
void HAL_FillFlashPageBuffer(uint32_t startOffset, uint16_t length, uint8_t *data)
{
  bootFlashReq.id = HAL_FLASH_ACCESS_FILL_PAGE_BUFFER;

  /* Check, if byte is pending */
  if (writeContext.pendingByte)
  {
    /* If flash access is next byte from pending one */
    if (writeContext.address + 1 == startOffset)
    {
      /* Extend pending word with first byte of current data buffer */
      writeContext.word &= 0x00FF | ((uint16_t)*data << 8);

      fillPageBufferWithPendingWord();

      startOffset++;
      length--;
      data++;
    }
    /* Otherwise - just put pending byte */
    else
      fillPageBufferWithPendingWord();

  }

  /* Check, if first byte of current data is unaligned */
  if (startOffset % sizeof(writeContext.word))
  {
    /* Fill it as a MSB of a word */
    writeContext.word = 0x00FF | ((uint16_t)*data << 8);
    writeContext.address = startOffset - 1;

    fillPageBufferWithPendingWord();

    startOffset++;
    length--;
    data++;
  }

  /* Check if last byte occurs to be unaligned and should be pended */
  if (length % sizeof(writeContext.word))
  {
    writeContext.word = 0xFF00 | data[length - 1];
    writeContext.address = startOffset + length - 1;
    writeContext.pendingByte = true;

    length--;
  }

  /* Put the rest of data to page buffer */
  if (length)
  {
    bootFlashReq.payload.fillPageBuffer.data = data;
    bootFlashReq.payload.fillPageBuffer.offset = startOffset;
    bootFlashReq.payload.fillPageBuffer.length = length;
    halFlashAccessReq(&bootFlashReq);
  }
}

/**************************************************************************//**
  \brief Writes data to the page buffer.
  \param[in] pageStartAddress - address within the target page
******************************************************************************/
void HAL_ProgramFlashPage(uint32_t pageStartAddress)
{
  /* Fill page buffer, if byte is pending */
  if (writeContext.pendingByte)
    fillPageBufferWithPendingWord();

  bootFlashReq.id = HAL_FLASH_ACCESS_PROGRAM_PAGE;
  bootFlashReq.payload.programPage.pageStartAddr = pageStartAddress;
  halFlashAccessReq(&bootFlashReq);
}

/**************************************************************************//**
  \brief Erases specified flash page.
  \param[in] pageStartAddress - address within the target page
******************************************************************************/
void HAL_EraseFlashPage(uint32_t pageStartAddress)
{
  bootFlashReq.id = HAL_FLASH_ACCESS_ERASE_PAGE;
  bootFlashReq.payload.erasePage.pageStartAddr = pageStartAddress;
  halFlashAccessReq(&bootFlashReq);
}

/**************************************************************************//**
  \brief Writes flash page buffer without erasing.
  \param[in] pageStartAddress - address within the target page
******************************************************************************/
void HAL_WriteFlashPage(uint32_t pageStartAddress)
{
  /* Fill page buffer, if byte is pending */
  if (writeContext.pendingByte)
    fillPageBufferWithPendingWord();

  bootFlashReq.id = HAL_FLASH_ACCESS_WRITE_PAGE;
  bootFlashReq.payload.writePage.pageStartAddr = pageStartAddress;
  halFlashAccessReq(&bootFlashReq);
}

/**************************************************************************//**
\brief Adds new entity to the queue of poll during flash writing handlers.

\param[in] handler - structure which shall consist a pointer to a function
                     to be called during flash writing. This function shall
                     be placed in NRWW section (use the NRWW_SECTION_DECLARE
                     macro to put the function here).
******************************************************************************/
void HAL_RegisterPollDuringFlashWritingHandler(HAL_PollDuringFlashWritingHandler_t *handler)
{
  putQueueElem(&pollDuringFlashWritingHandlers, &handler->next);
}

/**************************************************************************//**
\brief Deletes an entity from the queue of poll during flash writing handlers.
******************************************************************************/
void HAL_UnregisterPollDuringFlashWritingHandler(HAL_PollDuringFlashWritingHandler_t *handler)
{
  deleteQueueElem(&pollDuringFlashWritingHandlers, &handler->next);
}

/**************************************************************************//**
\brief Poll handler which is called periodicly during flash writing process.
******************************************************************************/
void HAL_PollDuringFlashWritingAccessPoint(void)
{
  for (QueueElement_t *queueElem = getQueueElem(&pollDuringFlashWritingHandlers); queueElem; queueElem = getNextQueueElem(queueElem))
  {
    const HAL_PollDuringFlashWritingHandler_t *hnd = GET_STRUCT_BY_FIELD_POINTER(HAL_PollDuringFlashWritingHandler_t,
                                                                                 next,
                                                                                 queueElem);
    if (hnd->poll)
      hnd->poll();
  }
}

/**************************************************************************//**
\brief Handler which is called after flash writing process.
******************************************************************************/
void HAL_FlashAccessFinishedHandler(void)
{
  for (QueueElement_t *queueElem = getQueueElem(&pollDuringFlashWritingHandlers); queueElem; queueElem = getNextQueueElem(queueElem))
  {
    const HAL_PollDuringFlashWritingHandler_t *hnd = GET_STRUCT_BY_FIELD_POINTER(HAL_PollDuringFlashWritingHandler_t,
                                                                                 next,
                                                                                 queueElem);
    if (hnd->finished)
      hnd->finished();
  }
}

/**************************************************************************//**
\brief Entry point to flash access module.
******************************************************************************/
void HAL_FlashAccessRequestHandler(void)
{
  HAL_FlashAccessReq_t *request = (HAL_FlashAccessReq_t *)(((uint16_t)GPIOR1 << 8) | GPIOR0);

  switch (request->id)
  {
    case HAL_FLASH_ACCESS_FILL_PAGE_BUFFER:
      halFillFlashPageBuffer(&request->payload.fillPageBuffer);
      break;
    case HAL_FLASH_ACCESS_PROGRAM_PAGE:
      halProgramFlashPage(&request->payload.programPage);
      break;
    case HAL_FLASH_ACCESS_SWAP_IMAGES:
      halSwapImages(&request->payload.swapImages);
      break;
    case HAL_FLASH_ACCESS_ERASE_PAGE:
      halEraseFlashPage(&request->payload.erasePage);
      break;
    case HAL_FLASH_ACCESS_WRITE_PAGE:
      halWriteFlashPage(&request->payload.writePage);
      break;
    default:
      break;
  }
}

/**************************************************************************//**
\brief Fills flash page buffer.
\param[in] req - pointer to fill page buffer request payload.
******************************************************************************/
static void halFillFlashPageBuffer(HAL_FillPageBufferReq_t *req)
{
  HAL_BootSectionAccessPoint_t pollAccessPoint = NULL;
  uint16_t addr;

  EEPROM_BUSY_WAIT();

  for (addr = (uint16_t)req->offset; addr < req->length + (uint16_t)req->offset; addr += sizeof(uint16_t), req->data += sizeof(uint16_t))
  {
    ATOMIC_SECTION_ENTER
    // Perform word load.
    halSpLoadFlashWord(addr, *(uint16_t *)req->data);
    // Wait for SPM to finish.
    FLASH_BUSY_WAIT(pollAccessPoint);
    ATOMIC_SECTION_LEAVE
  }
}

/**************************************************************************//**
\brief Erases flash page and writes flash page buffer there
\param[in] req - pointer to program page request payload.
******************************************************************************/
static void halProgramFlashPage(HAL_ProgramPageReq_t *req)
{
  HAL_BootSectionAccessPoint_t pollAccessPoint = NULL;
  // Since standard c doesn’t have this pointer type, so the coverity tool picked up this as error
  // Normal IAR Compiler behavior for accessing the memory of more than 64Kbytes 
  // POLL_DURING_FLASH_WRITING_ACCESS_POINT intern typecast by (__farflash void *)
  
  READ_ACCESS_POINT(pollAccessPoint, POLL_DURING_FLASH_WRITING_ACCESS_POINT);

  ATOMIC_SECTION_ENTER

  EEPROM_BUSY_WAIT();
  // Perform page erase.
  halSpEraseApplicationPage(req->pageStartAddr);
  // Wait for SPM to finish.
  FLASH_BUSY_WAIT(pollAccessPoint);
  // Perform page write.
  halSpWriteApplicationPage(req->pageStartAddr);
  // Wait for SPM to finish.
  FLASH_BUSY_WAIT(pollAccessPoint);
  // enable rww section
  halSpRWWSectionEnable();
  // Wait for SPM to finish.
  FLASH_BUSY_WAIT(pollAccessPoint);

  ATOMIC_SECTION_LEAVE
}

/**************************************************************************//**
\brief Starts flash images swapping from additional area to general area.
\param[in] req - pointer to swap images request payload.
******************************************************************************/
NO_RETURN static void halSwapImages(HAL_SwapImagesReq_t *req)
{
  uint16_t currentPage = req->imageStartAddr >> PAGE_NUMBER_SHIFT;
  uint16_t dataPage = currentPage;
  HAL_FlashAccessReqPayload_t flashAccess;
  uint32_t pageStartAddr;
  uint16_t savingPageNumber = 0;
  uint8_t  dataWord[sizeof(uint16_t)];

  cli();  // Disable global interrupts

  EEPROM_BUSY_WAIT();

  while (req->imageSize)
  {
    dataPage = req->imageStartAddr >> PAGE_NUMBER_SHIFT;

    if (dataPage != currentPage)
    {
      /* Store data from flash buffer to flash page */
      flashAccess.programPage.pageStartAddr = (uint32_t)savingPageNumber << PAGE_NUMBER_SHIFT;
      halProgramFlashPage(&flashAccess.programPage);
      currentPage = dataPage;
      savingPageNumber++;
    }

    pageStartAddr = req->imageStartAddr & PAGE_POSITION_OFFSET_MASK;
    dataWord[0] = halSpReadInternalFlash(req->imageStartAddr);
    dataWord[1] = halSpReadInternalFlash(req->imageStartAddr + 1);
    /* Fill temporary page buffer */
    flashAccess.fillPageBuffer.offset = pageStartAddr;
    flashAccess.fillPageBuffer.length = sizeof(dataWord);
    flashAccess.fillPageBuffer.data = dataWord;
    halFillFlashPageBuffer(&flashAccess.fillPageBuffer);

    req->imageStartAddr += sizeof(dataWord);
    req->imageSize -= sizeof(dataWord);
  }

  /* flush last page into flash */
  flashAccess.programPage.pageStartAddr = (uint32_t)savingPageNumber << PAGE_NUMBER_SHIFT;
  halProgramFlashPage(&flashAccess.programPage);

  /* Start wdt on 16 ms and reset mcu */
  wdt_reset();
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  WDTCSR = (1 << WDE);
  while(1);
}

/**************************************************************************//**
\brief Erases specified flash page.
\param[in] req - pointer to erase flash page request payload.
******************************************************************************/
static void halEraseFlashPage(HAL_ErasePageReq_t *req)
{
  HAL_BootSectionAccessPoint_t pollAccessPoint = NULL;
  
  // Since standard c doesn’t have this pointer type, so the coverity tool picked up this as error
  // Normal IAR Compiler behavior for accessing the memory of more than 64Kbytes 
  // POLL_DURING_FLASH_WRITING_ACCESS_POINT intern typecast by (__farflash void *)
  READ_ACCESS_POINT(pollAccessPoint, POLL_DURING_FLASH_WRITING_ACCESS_POINT);

  ATOMIC_SECTION_ENTER

  EEPROM_BUSY_WAIT();

  halSpEraseApplicationPage(req->pageStartAddr);
  FLASH_BUSY_WAIT(pollAccessPoint);  // Wait until the memory is erased.

  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.
  halSpRWWSectionEnable();
  FLASH_BUSY_WAIT(pollAccessPoint);

  ATOMIC_SECTION_LEAVE
}

/**************************************************************************//**
\brief Writes page buffer to flash.
\param[in] req - pointer to write flash page request payload.
******************************************************************************/
static void halWriteFlashPage(HAL_WritePageReq_t *req)
{
  HAL_BootSectionAccessPoint_t pollAccessPoint = NULL;
  // Since standard c doesn’t have this pointer type, so the coverity tool picked up this as error
  // Normal IAR Compiler behavior for accessing the memory of more than 64Kbytes 
  // POLL_DURING_FLASH_WRITING_ACCESS_POINT intern typecast by (__farflash void *)

#if defined(__ICCAVR__)
  READ_ACCESS_POINT(pollAccessPoint, POLL_DURING_FLASH_WRITING_ACCESS_POINT);
#endif  

  ATOMIC_SECTION_ENTER

  EEPROM_BUSY_WAIT();

  halSpWriteApplicationPage(req->pageStartAddr);  // Store buffer in flash page.
  FLASH_BUSY_WAIT(pollAccessPoint);  // Wait until the memory is written.

  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.
  halSpRWWSectionEnable();
  FLASH_BUSY_WAIT(pollAccessPoint);

  ATOMIC_SECTION_LEAVE
}

#endif // defined(HAL_USE_FLASH_ACCESS)
// eof flash.c

