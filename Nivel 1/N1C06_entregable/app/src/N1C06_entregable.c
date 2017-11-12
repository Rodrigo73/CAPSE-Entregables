/*============================================================================
* Autor: Rodrigo Furlani
 * Fecha: 06/09/2017
 *===========================================================================*/

/*==================[inlcusiones]============================================*/

//#include "program.h"   // <= su propio archivo de cabecera (opcional)
#include "sapi.h"        // <= Biblioteca sAPI

//#include "c_i18n_es.h" // <= para traducir el codigo C al espaï¿½ol (opcional)
//#include "c_i18n_es.h" // <= para traducir la sAPI al espaï¿½ol (opcional)

CONSOLE_PRINT_ENABLE //¿WTF?


// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma
   boardConfig();

   //CONSTANTES
   #define retardo 50   //para retardo de 50 ms
   #define recargaSegundo 20   //20 veces 50 ms para que pase un segundo 
   
   //VARIABLES
   uint8_t segundo = recargaSegundo;    //lleva el conteo de tiempo para 1 segundo
   uint8_t comando; //letra recibida de consola
   
   //config puerto serie USB
   uartConfig(UART_USB, 115200); //configura UART
   
   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
      //COMIENZO lectura de comando desde consola
      if (uartReadByte(UART_USB, &comando))  //si llego una letra por consola...
      {
         switch(comando)
          {
              case 'a':
                gpioToggle(LEDB);  //conmuta led azul
                break;
              case 'v':
                gpioToggle(LEDG); //conmuto led verde
                break;
              case 'r':
                 gpioToggle(LEDR);  //conmuto led rojo
                 break;
              case '1':
                 gpioToggle(LED1);  //conmuto led1
                 break;
              case '2':
                 gpioToggle(LED2);  //conmuto led2
                 break;
              case '3':
                 gpioToggle(LED3);  //conmuto led3
            }
      }
      //FIN lectura de comando desde consola
      
      
      //COMIENZO estado de leds y teclas
      segundo = segundo - 1;  //conteo de se segundo
      if (!segundo)  //si paso un segundo
      {
         segundo = recargaSegundo;  //recarga para conteo de proximo segundo
         uartWriteString(UART_USB, "#############################################\r\n");
         uartWriteString(UART_USB, "Led Azul: ");
         if (gpioRead(LEDB)) uartWriteString(UART_USB, "Encendido\r\n"); else uartWriteString(UART_USB, "Apagado\r\n");
         uartWriteString(UART_USB, "Led Verde: ");
         if (gpioRead(LEDG)) uartWriteString(UART_USB, "Encendido\r\n"); else uartWriteString(UART_USB, "Apagado\r\n");
         uartWriteString(UART_USB, "Led Rojo: ");
         if (gpioRead(LEDR)) uartWriteString(UART_USB, "Encendido\r\n"); else uartWriteString(UART_USB, "Apagado\r\n");
         uartWriteString(UART_USB, "Led 1: ");
         if (gpioRead(LED1)) uartWriteString(UART_USB, "Encendido\r\n"); else uartWriteString(UART_USB, "Apagado\r\n");
         uartWriteString(UART_USB, "Led 2: ");
         if (gpioRead(LED2)) uartWriteString(UART_USB, "Encendido\r\n"); else uartWriteString(UART_USB, "Apagado\r\n");
         uartWriteString(UART_USB, "Led 3: ");
         if (gpioRead(LED3)) uartWriteString(UART_USB, "Encendido\r\n"); else uartWriteString(UART_USB, "Apagado\r\n");
         uartWriteString(UART_USB, "Tecla 1: ");
         if (gpioRead(TEC1)) uartWriteString(UART_USB, "OFF\r\n"); else uartWriteString(UART_USB, "ON\r\n");
         uartWriteString(UART_USB, "Tecla 2: ");
         if (gpioRead(TEC2)) uartWriteString(UART_USB, "OFF\r\n"); else uartWriteString(UART_USB, "ON\r\n");
         uartWriteString(UART_USB, "Tecla 3: ");
         if (gpioRead(TEC3)) uartWriteString(UART_USB, "OFF\r\n"); else uartWriteString(UART_USB, "ON\r\n");
         uartWriteString(UART_USB, "Tecla 4: ");
         if (gpioRead(TEC4)) uartWriteString(UART_USB, "OFF\r\n"); else uartWriteString(UART_USB, "ON\r\n");
         uartWriteString(UART_USB, "#############################################\r\n");
         uartWriteString(UART_USB, "\r\n");
      }
      //FIN estado de leds y teclas
       
      
      /* Retardo bloqueante durante 50ms */
      delay( retardo );
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}


/*==================[fin del archivo]========================================*/