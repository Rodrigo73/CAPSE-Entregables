/*============================================================================
 * Autor: Rodrigo Furlani
 * Date: 10/11/2017
 *===========================================================================*/
#ifndef _TECLADO_MATRICIAL_
#define _TECLADO_MATRICIAL_
/*==================[inclusions]=============================================*/

#include "sapi.h"

/*==================[definiciones y macros]==================================*/

// Alias para GPIO destinadas a teclado. De usar otras GPIO, se cambian acá
#define  TECfila0 RS232_TXD   //salida RS232_TXD para fila 0 teclado
#define  TECfila1 CAN_RD      //salida CAN_RD para fila 1 teclado
#define  TECfila2 CAN_TD      //salida CAN_TD para fila 2 teclado
#define  TECfila3 T_COL1      //salida T_COL1 para fila 3 teclado
#define  TECcol0  T_FIL0      //entrada T_FIL0 para columna 0 teclado (en modo GPIO_INPUT_PULLDOWN)
#define  TECcol1  T_FIL3      //entrada T_FIL3 para columna 1 teclado (en modo GPIO_INPUT_PULLDOWN)
#define  TECcol2  T_FIL2      //entrada T_FIL2 para columna 2 teclado (en modo GPIO_INPUT_PULLDOWN)
#define  TECcol3  T_COL0      //entrada T_COL0 para columna 3 teclado (en modo GPIO_INPUT_PULLDOWN)

#define  ANTIRREBOTE_MATRICIAL   40 //tiempo en ms, para antirrebote de teclado

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

uint8_t tecladoMatricialRead (void);

#endif /* #ifndef _TECLADO_MATRICIAL_ */