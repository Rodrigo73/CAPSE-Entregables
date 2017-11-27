/*============================================================================
 * Autor: Rodrigo Furlani
 * Date: 12/10/2017
 *===========================================================================*/

/*============================================================================
Implementa la funci�n "tactswitchRead"
Esta funci�n devuelve el tactswitch presionado (TEC1..TEC4), o devuelve 0
en caso de que no hubiera ninguno presionado. El funcionamiento de la funci�n
no es bloqueante.
Para dar como v�lida la pulsaci�n de un tactswitch, debe ocurrir:
	- Que en una primer llamada a la funci�n, todos los tactswitch hayan
	  estado sueltos (sin presionar). En este caso se devuelve 0.
	- Que en alguna posterior llamada a la funci�n se vea un tactswitch
	  presionado. De ser as�, se comienza conteo antirrebote y se devuelve 0.
	- Que en otra posterior llamada a la funci�n haya finalizado el
	  conteo de antirrebote, y el tactswitch contin�e presionado. En este
	  caso se devuelve el tactswitch presionado (TEC1..TEC4).
Cumplido el proceso anterior, la funci�n retorna el tactswitch presionado
una sola vez. Si en la siguiente llamada a la funci�n el tactswitch
contin�a presionado, la funci�n retorna 0 (no hay tactswitch presionado). Dicho
de otra forma, para que la funci�n vuelva a dar como presionado a ese tactswitch,
tiene que volver a cumplirse la secuencia:
	- Ver todos tactswitches sueltos, luego ver uno presionado, contar
	  antirrebote, seguir viendo presionado.
Esto se implement� as� porque la idea es que si se mantiene largo
tiempo presionado un tactswitch, se de como v�lida una �nica
pulsaci�n, y se haga necesario soltar el tactswitch y volverlo a presionar
para que se de como v�lida una segunda pulsaci�n.
El tiempo de antirrebote se establece en el archivo header (.h) mediante
el valor "ANTIRREBOTE"
*===========================================================================*/

/*==================[inclusions]=============================================*/

#include "tactswitch.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

static bool_t inicializarUnicaVez = 1; //si es 1, por �nica vez se debe ejecutar
                                       //un codigo de inicializaci�n.
                                       //static => no es accesible fuera de este archivo
                                       
static bool_t estuvoSuelto = 1;	//Si es 1 indica que en una llamada anterior
                                 //a la funci�n, no hubo ning�n tactswitch presionado.
                                 //Se inicializa en 1 para el caso en que antes 
                                 //del arranque de la CIAA ya se est� presionando un
                                 //tactswitch (=> si fuera 0, al entrar por primera
                                 //vez a la funcion no habr�a llamada anterior que
                                 //haya visto ning�n tactswitch presionado => no
                                 //tomar�a v�lido el tactswitch presionado).
                                 //static => no es accesible fuera de este archivo
                                
static delay_t conteoAntirrebote;   //para llevar conteo de antirrebote
                                    //static => no es accesible fuera de este archivo

static uint8_t pulsador[4] = {TEC1, TEC2, TEC3, TEC4};	//static => no es accesible fuera de este archivo

/*==================[internal functions declaration]=========================*/

static uint8_t escanearTactswitches (void);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static uint8_t escanearTactswitches (void) {
   //Devuelve TEC1..TEC4 si hay algun tactswitch presionado,
   //o devuelve 0 si no hay tactswitch presionado.
   //static => no es accesible fuera de este archivo
   uint8_t presionado = 0;
   //
   for (uint8_t j=0; j<4; j++)
   {
      if (!gpioRead(pulsador[j])) //si hay tactswitch presionado...
      {
         presionado = pulsador[j];  //...lo identifico...
         break;                     //...y corto el for
      }
   }
   return presionado;
}

/*==================[external functions definition]==========================*/

uint8_t tactswitchRead(void) {
   uint8_t tsPresionado;	//guarda el tactswitch que est� presionado, si lo hubiera
   //
   if (inicializarUnicaVez)
   {
      inicializarUnicaVez = 0;
      delayConfig(&conteoAntirrebote, ANTIRREBOTE); //inicializo para conteo antirrebote, pero no arranco conteo
   }
   
   tsPresionado = escanearTactswitches();  //escaneo tactswitches
   
   if(tsPresionado == 0) //...si no hay tactswitch presionado
   {
      estuvoSuelto = 1;
      delayConfig(&conteoAntirrebote, ANTIRREBOTE); //inicializo para conteo antirrebote, pero no arranco conteo
      return 0;   //retorna indicando que no hay tactswitch presionado
   } else if (!estuvoSuelto)   //...si antes los tactswitch no estuvieron sueltos...
      {
         return 0;   //retorna indicando que no hay tactswitch presionado
      } else if (!delayRead(&conteoAntirrebote)) //...si no pas� tiempo de antirrebote...
         {
            return 0;   //retorna indicando que no hay tactswitch presionado
         } else   //...si hay tactswitch presionado, pas� antirrebote, y antes estuvieron sueltos...
            {
               estuvoSuelto = 0;  //indico que antes hubo pulsador presionado
               delayConfig(&conteoAntirrebote, ANTIRREBOTE); //inicializo para conteo antirrebote, pero no arranco conteo
               return tsPresionado;	//retorno TEC1..TEC4
            }
}

/*==================[end of file]============================================*/
