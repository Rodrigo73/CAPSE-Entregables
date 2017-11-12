/*============================================================================
 * Modificado por: Rodrigo Furlani
 * Fecha: 10/11/2017
 *===========================================================================*/

/*============================================================================
 Programa para implementar un teclado numérico USB con el teclado matricial.
 Se parte de un programa ya hecho, del cual no se entiende mucho, y se modifica
 para agregar la parte que lee el teclado matricial.
 En este módulo del proyecto (hid_keyboard.c) se modifica lo siguiente:
 - se #include la librería "tecladomatricial.h" para lectura de teclado matricial
 - en función "Keyboard_UpdateReport":
   ** se anula (comenta) el código original que lee los tactswitches.
   ** se agrega el código que lee el teclado matricial y envía por
      USB el código de tecla que corresponde a la tecla presionada.
 *===========================================================================*/
 
/*
 * @brief This file contains USB HID Keyboard example using USB ROM Drivers.
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "sapi.h"
#include <stdint.h>
#include <string.h>
#include "usbd_rom_api.h"
#include "hid_keyboard.h"

#include "tecladomatricial.h" //MODIFICADO RAF, libreria para lectura de teclado matricial

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/**
 * @brief Structure to hold Keyboard data
 */
typedef struct {
	USBD_HANDLE_T hUsb;	/*!< Handle to USB stack. */
	uint8_t report[KEYBOARD_REPORT_SIZE];	/*!< Last report data  */
	uint8_t tx_busy;	/*!< Flag indicating whether a report is pending in endpoint queue. */
} Keyboard_Ctrl_T;

/** Singleton instance of Keyboard control */
static Keyboard_Ctrl_T g_keyBoard;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

extern const uint8_t Keyboard_ReportDescriptor[];
extern const uint16_t Keyboard_ReportDescSize;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Routine to update keyboard state */
static void Keyboard_UpdateReport(void){
   
   uint8_t tecla;   //MODIFICADO RAF, variable con ASCII de tecla presionada
   
   HID_KEYBOARD_CLEAR_REPORT(&g_keyBoard.report[0]);

//**************** COMIENZO MODIFICACIONES RAF **********************
   tecla = tecladoMatricialRead();  //lee teclado matricial
   switch (tecla) {
      case '0':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x62); // 0x62 es 'keypad 0 and Insert'
         break;
      case '1':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x59); // 0x59 es 'keypad 1 and End'
         break;
      case '2':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x5A); // 0x5A es 'keypad 2 and Down Arrow'
         break;
      case '3':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x5B); // 0x5B es 'keypad 3 and PageDn'
         break;
      case '4':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x5C); // 0x5C es 'keypad 4 and Left Arrow'
         break;
      case '5':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x5D); // 0x5D es 'keypad 5'
         break;
      case '6':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x5E); // 0x5E es 'keypad 6 and Right Arrow'
         break;
      case '7':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x5F); // 0x5F es 'keypad 7 and Home'
         break;
      case '8':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x60); // 0x60 es 'keypad 8 and Up Arrow'
         break;
      case '9':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x61); // 0x61 es 'keypad 9 and PageUp'
         break;
      case '*':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x55); // 0x55 es 'keypad *'
         break;
      case '#':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x54); // 0x54 es 'keypad /'
         break;
      case 'A':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x04); // 0x04 es 'keyboard a and A'
         break;
      case 'B':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x05); // 0x05 es 'keyboard b and B'
         break;
      case 'C':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x06); // 0x06 es 'keyboard c and C'
         break;
      case 'D':
         HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x07); // 0x07 es 'keyboard d and D'
         break;
   }
//****************** FIN MODIFICACIONES RAF *************************

   
/*---------------------- CODIGO ORIGINAL, COMENTADO PARA ANULARLO ---------------
   if( !gpioRead( TEC1 ) ){
      HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x39); // 'CapsLock' 
      //HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x39); // 'CapsLock'
   }
   else if( !gpioRead( TEC2 ) ){
      HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x0C); // 'i' 
   }
   else if( !gpioRead( TEC3 ) ){
      HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x04); // 'a' 
   }
   else if( !gpioRead( TEC4 ) ){
      HID_KEYBOARD_REPORT_SET_KEY_PRESS(g_keyBoard.report, 0x28); // 'ENTER' 
   }
---------------------- CODIGO ORIGINAL, COMENTADO PARA ANULARLO ---------------*/

}

/* HID Get Report Request Callback. Called automatically on HID Get Report Request */
static ErrorCode_t Keyboard_GetReport( USBD_HANDLE_T hHid,
                                       USB_SETUP_PACKET *pSetup,
                                       uint8_t * *pBuffer,
                                       uint16_t *plength ){

	/* ReportID = SetupPacket.wValue.WB.L; */
	switch (pSetup->wValue.WB.H) {

      case HID_REPORT_INPUT:
         Keyboard_UpdateReport();
         memcpy(*pBuffer, &g_keyBoard.report[0], KEYBOARD_REPORT_SIZE);
         *plength = KEYBOARD_REPORT_SIZE;
      break;

      case HID_REPORT_OUTPUT:				/* Not Supported */
      case HID_REPORT_FEATURE:			/* Not Supported */

         return ERR_USBD_STALL;
	}

	return LPC_OK;
}

/* HID Set Report Request Callback. Called automatically on HID Set Report Request */
static ErrorCode_t Keyboard_SetReport( USBD_HANDLE_T hHid,
                                       USB_SETUP_PACKET *pSetup,
                                       uint8_t * *pBuffer,
                                       uint16_t length){

   /* we will reuse standard EP0Buf */
   if (length == 0) {
      return LPC_OK;
   }

   /* ReportID = SetupPacket.wValue.WB.L; */
   switch (pSetup->wValue.WB.H){

      case HID_REPORT_OUTPUT:
         /*  If the USB host tells us to turn on the NUM LOCK LED,
          *  then turn on LED#2.
          */
         if (**pBuffer & 0x01) {
            gpioWrite( LEDB, ON );
         }
         else {
            gpioWrite( LEDB, OFF);
         }
      break;

      case HID_REPORT_INPUT:				/* Not Supported */
      case HID_REPORT_FEATURE:			/* Not Supported */

         return ERR_USBD_STALL;
   }

   return LPC_OK;
}

/* HID interrupt IN endpoint handler */
static ErrorCode_t Keyboard_EpIN_Hdlr( USBD_HANDLE_T hUsb,
                                       void *data,
                                       uint32_t event ){
   switch (event) {
      case USB_EVT_IN:
         g_keyBoard.tx_busy = 0;
         break;
   }
   return LPC_OK;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* HID keyboard init routine */
ErrorCode_t Keyboard_init(USBD_HANDLE_T hUsb,
						  USB_INTERFACE_DESCRIPTOR *pIntfDesc,
						  uint32_t *mem_base,
						  uint32_t *mem_size){

   USBD_HID_INIT_PARAM_T hid_param;
   USB_HID_REPORT_T reports_data[1];
   ErrorCode_t ret = LPC_OK;

   /* Do a quick check of if the interface descriptor passed is the right one. */
   if( (pIntfDesc == 0) || (pIntfDesc->bInterfaceClass
       != USB_DEVICE_CLASS_HUMAN_INTERFACE)) {
      return ERR_FAILED;
   }

   /* Init HID params */
   memset((void *) &hid_param, 0, sizeof(USBD_HID_INIT_PARAM_T));
   hid_param.max_reports = 1;
   hid_param.mem_base = *mem_base;
   hid_param.mem_size = *mem_size;
   hid_param.intf_desc = (uint8_t *) pIntfDesc;

   /* user defined functions */
   hid_param.HID_GetReport = Keyboard_GetReport;
   hid_param.HID_SetReport = Keyboard_SetReport;
   hid_param.HID_EpIn_Hdlr  = Keyboard_EpIN_Hdlr;

   /* Init reports_data */
   reports_data[0].len = Keyboard_ReportDescSize;
   reports_data[0].idle_time = 0;
   reports_data[0].desc = (uint8_t *) &Keyboard_ReportDescriptor[0];
   hid_param.report_data  = reports_data;

   ret = USBD_API->hid->init(hUsb, &hid_param);

   /* update memory variables */
   *mem_base = hid_param.mem_base;
   *mem_size = hid_param.mem_size;

   /* store stack handle for later use. */
   g_keyBoard.hUsb = hUsb;
   g_keyBoard.tx_busy = 0;

   return ret;
}

/* Keyboard tasks */
void Keyboard_Tasks(void)
{
	/* check device is configured before sending report. */
	if ( USB_IsConfigured(g_keyBoard.hUsb)) {

		/* send report data */
		if (g_keyBoard.tx_busy == 0) {
			g_keyBoard.tx_busy = 1;

			/* update report based on board state */
			Keyboard_UpdateReport();
			USBD_API->hw->WriteEP( g_keyBoard.hUsb,
			                       HID_EP_IN,
			                       &g_keyBoard.report[0],
			                       KEYBOARD_REPORT_SIZE );
		}
	}
	else {
		/* reset busy flag if we get disconnected. */
		g_keyBoard.tx_busy = 0;
	}

}
