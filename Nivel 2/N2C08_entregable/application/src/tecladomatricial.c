/*============================================================================
 * Autor: Rodrigo Furlani
 * Date: 10/11/2017
 *===========================================================================*/

/*============================================================================
Implementa la funci�n "tecladoMatricialRead"
Esta funci�n devuelve el ASCII de la tecla presionada, o devuelve 0
en caso de que no hubiera ninguna tecla presionada. El funcionamiento
de la funci�n no es bloqueante.
Para dar como v�lida la pulsaci�n de una tecla, debe ocurrir:
	- Que en una primer llamada a la funci�n, todas las teclas hayan
	  estado sueltas (sin presionar). En este caso se devuelve 0.
	- Que en alguna posterior llamada a la funci�n se vea una tecla
	  presionada. De ser as�, se comienza conteo antirrebote y se devuelve 0.
	- Que en otra posterior llamada a la funci�n haya finalizado el
	  conteo de antirrebote, y la tecla contin�e presionada. En este
	  caso se devuelve el ASCII de la tecla presionada.
Cumplido el proceso anterior, la funci�n retorna el ASCII de la tecla presionada
una sola vez. Si en la siguiente llamada a la funci�n la tecla
contin�a presionada, la funci�n retorna 0 (no hay tecla presionada). Dicho
de otra forma, para que la funci�n vuelva a dar como presionada a esa tecla,
tiene que volver a cumplirse la secuencia:
	- Ver todas las teclas sueltas, luego ver una presionado, contar
	  antirrebote, seguir viendo presionada.
Esto se implement� as� porque la idea es que si se mantiene largo
tiempo presionada una tecla, se de como v�lida una �nica
pulsaci�n, y se haga necesario soltar la tecla y volverla a presionar
para que se de como v�lida una segunda pulsaci�n.
El tiempo de antirrebote se establece en el archivo header (.h) mediante
el valor "ANTIRREBOTE_MATRICIAL"
*===========================================================================*/

/*==================[inclusions]=============================================*/

#include "tecladomatricial.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

// GPIO conectadas a las filas del teclado (modo GPIO_OUTPUT)
// static => no accesible fuera de este archivo
static uint8_t tecladoFilas[4] =  
{
   TECfila0,
   TECfila1,
   TECfila2,
   TECfila3
};

// GPIO conectadas a las columnas del teclado (modo GPIO_INPUT_PULLDOWN)
// static => no accesible fuera de este archivo
static uint8_t tecladoColumnas[4] =  
{
   TECcol0,
   TECcol1,
   TECcol2,
   TECcol3
};

// Vector para determinar ASCII de tecla presionada
// static => no accesible fuera de este archivo
static uint8_t TECascii[16] = 
{
   '1', '2', '3', 'A',
   '4', '5', '6', 'B',
   '7', '8', '9', 'C',
   '*', '0', '#', 'D'
};

static bool_t inicializarUnicaVez = 1; //si es 1, por �nica vez se debe ejecutar
                                       //un codigo de inicializaci�n.
                                       //static => no es accesible fuera de este archivo

static bool_t estuvoSuelto = 1;	//Si es 1 indica que en una llamada anterior
                                 //a la funci�n, no hubo ninguna tecla presionada.
                                 //Se inicializa en 1 para el caso en que antes 
                                 //del arranque de la CIAA ya se est� presionando una
                                 //tecla (=> si fuera 0, al entrar por primera
                                 //vez a la funcion no habr�a llamada anterior que
                                 //haya visto ninguna tecla presionada => no
                                 //tomar�a v�lida la tecla presionada).
                                 //static => no es accesible fuera de este archivo

static delay_t conteoAntirrebote;   //para llevar conteo de antirrebote
                                    //static => no es accesible fuera de este archivo

/*==================[internal functions declaration]=========================*/

static void tecladoMatricialConfig (void);
static uint8_t tecladoMatricialScan (void);
static uint8_t buscarTecla (void);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void tecladoMatricialConfig (void) {
   //Configura como entradas y salidas las GPIO que van conectadas al teclado
   //Las GPIO de entrada se configuran con resistencia PULLDOWN activadas
   //Tambien inicializa para conteo antirrebote (pero no arranca conteo)
   //
   uint8_t i;  //auxiliar para for
   //
   inicializarUnicaVez = 0;   //para indicar que ya se ejecut� por unica vez esta funci�n
   delayConfig(&conteoAntirrebote, ANTIRREBOTE_MATRICIAL); //inicializo para conteo antirrebote, pero no arranco conteo
   
   for (i=0; i<4; i++) gpioConfig(tecladoFilas[i], GPIO_OUTPUT);  //configura como salida los GPIO para las filas
   for (i=0; i<4; i++) gpioConfig(tecladoColumnas[i], GPIO_INPUT_PULLDOWN);  //configura como entrada con PULLDOWN los GPIO para las columnas
}

static uint8_t tecladoMatricialScan (void) {
   //Devuelve el ASCII de la tecla presionada,
   //o devuelve 0 si no hay tecla presionada.
   //static => no es accesible fuera de este archivo
   uint8_t tmMatriz; //posicion de tecla presionada (0..15) o 16 si no hay tecla presionada
   //
   tmMatriz = buscarTecla();
   if (tmMatriz == 16) return 0;    //si no hay tecla presionada devuelve 0....
      else return TECascii[tmMatriz];  //...sino, devuelve ASCII de tecla presionada
}

static uint8_t buscarTecla (void) {
   //Si hay una tecla presionada, devuelve su posici�n en la matriz (0..15)
   //Si no hay tecla presionada, devuelve 16.
   //La matriz es:
   //    C0 C1 C2 C3
   // F0  0  1  2  3
   // F1  4  5  6  7
   // F2  8  9 10 11
   // F3 12 13 14 15
   //
   uint8_t fil, col, i; //para for escaneo teclado
   uint8_t teclaPres = 16; //tecla encontrada -> 0..15 valida, 16 no encontro tecla
   //
   for (fil=0; fil<4; fil++)  //para cada fila...
   {
      for (i=0; i<4; i++) gpioWrite(tecladoFilas[i], 0); //pone en 0 las 4 filas del teclado
      gpioWrite(tecladoFilas[fil], 1); //pone en 1 la fila "fil" del teclado
      for (col= 0; col<4; col++) //para cada columna...
      {
         if (gpioRead(tecladoColumnas[col])) //si la columna esta en 1 (encontro tecla)....
         {
            teclaPres = fil*4 + col;
         }
      }
   }
   return teclaPres;
}

/*==================[external functions definition]==========================*/

uint8_t tecladoMatricialRead (void) {
   uint8_t tmPresionada;   //guarda el ASCII de la tecla que est� presionada, si la hubiera (si no => guarda 0)
   //
   if (inicializarUnicaVez) tecladoMatricialConfig(); //configuraci�n por �nica vez
   
   tmPresionada = tecladoMatricialScan(); //escaneo teclado
   
   if (tmPresionada == 0)  //...si no hay tecla presionada
   {
      estuvoSuelto = 1;
      delayConfig(&conteoAntirrebote, ANTIRREBOTE_MATRICIAL); //inicializo para conteo antirrebote, pero no arranco conteo
      return 0;   //retorna indicando que no hay tecla presionada
   } else if (!estuvoSuelto)   //...si antes hubo tecla presionada...
      {
         return 0;   //retorna indicando que no hay tecla presionada
      } else if (!delayRead(&conteoAntirrebote))   //...si no pas� el tiempo de antirrebote...
         {
            return 0;   //retorna indicando que no hay tecla presionada
         } else   //...si hay tecla presionada, pas� antirrebote, y antes estuvieron sueltas...
            {
               estuvoSuelto = 0;  //indico que antes hubo tecla presionada
               delayConfig(&conteoAntirrebote, ANTIRREBOTE_MATRICIAL); //inicializo para conteo antirrebote, pero no arranco conteo
               return tmPresionada;	//retorno ASCII de tecla presionada
            }
}
/*==================[end of file]============================================*/
