/*============================================================================
 * Autor: Rodrigo Furlani
 * Date: 12/10/2017
 *===========================================================================*/
#ifndef _TACTSWITCH_H_
#define _TACTSWITCH_H_
/*==================[inclusions]=============================================*/

#include "sapi.h"

/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
extern "C" {
#endif
/*==================[macros]=================================================*/

#ifndef ANTIRREBOTE
#define ANTIRREBOTE 30	//30ms de antirrebote
#endif

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

uint8_t tactswitchRead (void);

/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
}
#endif
/*==================[end of file]============================================*/
#endif /* #ifndef _TACTSWITCH_H_ */