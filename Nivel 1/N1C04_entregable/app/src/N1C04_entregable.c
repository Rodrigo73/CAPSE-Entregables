/*============================================================================
 * Autor: Rodrigo Furlani
 * Fecha: 27/08/2017
 *===========================================================================*/

/*==================[inlcusiones]============================================*/

//#include "program.h"   // <= su propio archivo de cabecera (opcional)
#include "sapi.h"        // <= Biblioteca sAPI


CONSOLE_PRINT_ENABLE //¿WTF?

/*
Tecla 1 -> Led RGB
Tecla 2 -> Led 1 (amarillo)
Tecla 3 -> Led 2 (rojo)
Tecla 4 -> Led 3 (verde)
*/

// RESUMEN
/*
Para cada conjunto pulsador - led, el programa hace lo siguiente:
- hay una variable booleana (mostrandoLedx) que indica si se está mostrando en el led el tiempo capturado en el pulsador
- hay una variable uint16 (contadorLedx) que se usa para:
    * contar el tiempo de captura en que el pulsador está presionado, o
    * contar (descontar en realidad) el tiempo en que se está mostrando el tiempo de captura en el led
El programa se fija si el contador es 0, de ser así apaga el led e indica en (mostrandoLedx) que el no se está mostrando tiempo de captura en led
Si se presiona el pulsador, y NO se está mostrando el tiempo de captura, se procede a ir contando el tiempo de captura.
Si el pulsador estuviera sin presionar y el tiempo capturado es distinto de 0, entonces se debe encender el led para mostrar el tiempo capturado; esto se indica seteando (mostrandoLedx)
Finalmente, si se indicó que se debe mostrar el tiempo capturado, se prende el led y se va contando (decrementando) el tiempo en que permanece encendido el led

El tiempo de captura se mide en múltiplos de 10 ms, generado por el delay que hay al final del ciclo

La lógia del programa está hecha de forma que si se presiona el pulsador mientras se está mostrando un tiempo de captura previo, se ignora al pulsador presionado
*/


// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{

   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma
   boardConfig();

   // Inicializar UART_USB como salida de consola
   consolePrintConfigUart( UART_USB, 115200 );

    //Variables
    bool_t mostrandoLedB = 0;
    uint16_t contadorLedB = 0;
    bool_t mostrandoLed1 = 0;
    uint16_t contadorLed1 = 0;
    bool_t mostrandoLed2 = 0;
    uint16_t contadorLed2 = 0;
    bool_t mostrandoLed3 = 0;
    uint16_t contadorLed3 = 0;

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
       
       //COMIENZO TRATAMIENTO TECLA 1 - LED RGB AZUL
       if (!contadorLedB)   //si contador LedB es 0...
       {
           gpioWrite(LEDB,0);   //apago LedB
           mostrandoLedB = 0;   //indico que no estoy mostrando tiempo en LedB
       }
       
       if ((!gpioRead(TEC1)) && (!mostrandoLedB))  //si Tecla1 presionada y además no estoy mostrando LedB...
       {
           contadorLedB = contadorLedB + 1; //incremento conteo de Tecla1 pulsada
       }
       
       if ((gpioRead(TEC1)) && (contadorLedB))   //si Tecla1 no esta presionada y además conteo no es cero...
       {
           mostrandoLedB = 1;   //indico que se debe mostrar tiempo en LedB
       }
      
       if (mostrandoLedB)   //si se debe mostrar tiempo en LedB...
       {
           gpioWrite(LEDB,1);   //prendo LedB
           contadorLedB = contadorLedB - 1; //decremento tiempo mostrado en LedB
       }
       //FIN TRATAMIENTO TECLA 1 - LED RGB AZUL
       
       //************************************************************************************
       
       //COMIENZO TRATAMIENTO TECLA 2 - LED 1
       if (!contadorLed1)   //si contador Led1 es 0...
       {
           gpioWrite(LED1,0);   //apago Led1
           mostrandoLed1 = 0;   //indico que no estoy mostrando tiempo en Led1
       }
       
       if ((!gpioRead(TEC2)) && (!mostrandoLed1))  //si Tecla2 presionada y además no estoy mostrando Led1...
       {
           contadorLed1 = contadorLed1 + 1; //incremento conteo de Tecla2 pulsada
       }
       
       if ((gpioRead(TEC2)) && (contadorLed1))   //si Tecla2 no esta presionada y además conteo no es cero...
       {
           mostrandoLed1 = 1;   //indico que se debe mostrar tiempo en Led1
       }
      
       if (mostrandoLed1)   //si se debe mostrar tiempo en Led1...
       {
           gpioWrite(LED1,1);   //prendo Led1
           contadorLed1 = contadorLed1 - 1; //decremento tiempo mostrado en Led1
       }
       //FIN TRATAMIENTO TECLA 2 - LED 1
       
       //************************************************************************************
       
       //COMIENZO TRATAMIENTO TECLA 3 - LED 2
       if (!contadorLed2)   //si contador Led2 es 0...
       {
           gpioWrite(LED2,0);   //apago Led2
           mostrandoLed2 = 0;   //indico que no estoy mostrando tiempo en Led2
       }
       
       if ((!gpioRead(TEC3)) && (!mostrandoLed2))  //si Tecla3 presionada y además no estoy mostrando Led2...
       {
           contadorLed2 = contadorLed2 + 1; //incremento conteo de Tecla3 pulsada
       }
       
       if ((gpioRead(TEC3)) && (contadorLed2))   //si Tecla3 no esta presionada y además conteo no es cero...
       {
           mostrandoLed2 = 1;   //indico que se debe mostrar tiempo en Led2
       }
      
       if (mostrandoLed2)   //si se debe mostrar tiempo en Led2...
       {
           gpioWrite(LED2,1);   //prendo Led2
           contadorLed2 = contadorLed2 - 1; //decremento tiempo mostrado en Led2
       }
       //FIN TRATAMIENTO TECLA 3 - LED 2
       
       //************************************************************************************
       
       //COMIENZO TRATAMIENTO TECLA 4 - LED 3
       if (!contadorLed3)   //si contador Led3 es 0...
       {
           gpioWrite(LED3,0);   //apago Led3
           mostrandoLed3 = 0;   //indico que no estoy mostrando tiempo en Led3
       }
       
       if ((!gpioRead(TEC4)) && (!mostrandoLed3))  //si Tecla4 presionada y además no estoy mostrando Led3...
       {
           contadorLed3 = contadorLed3 + 1; //incremento conteo de Tecla4 pulsada
       }
       
       if ((gpioRead(TEC4)) && (contadorLed3))   //si Tecla4 no esta presionada y además conteo no es cero...
       {
           mostrandoLed3 = 1;   //indico que se debe mostrar tiempo en Led3
       }
      
       if (mostrandoLed3)   //si se debe mostrar tiempo en Led3...
       {
           gpioWrite(LED3,1);   //prendo Led3
           contadorLed3 = contadorLed3 - 1; //decremento tiempo mostrado en Led3
       }
       //FIN TRATAMIENTO TECLA 4 - LED 3
       
       //************************************************************************************
       
       delay (10);
             
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}


/*==================[fin del archivo]========================================*/