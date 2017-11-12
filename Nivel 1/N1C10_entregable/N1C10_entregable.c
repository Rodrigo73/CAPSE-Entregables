/*============================================================================
 * Autor: Rodrigo Furlani
 * Fecha: 19/09/2017
 *===========================================================================*/
/*
El programa espera a recibir un caracter por UART, y según el caracter recibido
prende un determinado led y muestra un número en display de 7 segmentos.
A su vez, si se mantiene presionada una tecla, durante el tiempo en que dure
la pulsacion se muestra el nº correspondiente en display. Al soltar la tecla
se vuelve a mostrar el nº correspondiente al led que se encuentre encendido.
*/

/*==================[inclusiones]============================================*/

#include "sapi.h"        // <= Biblioteca sAPI

/*==================[definiciones y macros]==================================*/


/*===========================[variables]=====================================*/

uint8_t display7sPines[8]= //GPIO de CIAA conectados a display
{
   GPIO5,   //segmento "a" en GPIO5
   GPIO7,   //segmento "b" en GPIO7
   GPIO6,   //segmento "c" en GPIO6
   GPIO1,   //segmento "d" en GPIO1
   GPIO2,   //segmento "e" en GPIO2
   GPIO4,   //segmento "f" en GPIO4
   GPIO3,   //segmento "g" en GPIO3
   GPIO8    //segmento "punto" en GPIO8
};

uint8_t display7sMapaCaracteres[10]=   //mapa de caracteres del display
                                       //distribucion "pgfedcba"
{
   0b00111111,  //para dibujar "0"
   0b00000110,  //para dibujar "1"
   0b01011011,  //para dibujar "2"
   0b01001111,  //para dibujar "3"
   0b01100110,  //para dibujar "4"
   0b01101101,  //para dibujar "5"
   0b01111101,  //para dibujar "6"
   0b00000111,  //para dibujar "7"
   0b01111111,  //para dibujar "8"
   0b01101111,  //para dibujar "9"
};

uint8_t recibidoUART;   //para guardar valor recibido por UART

/*==================[declaraciones de funciones]=============================*/

void display7sConfigurarPines (void);
void display7sMostrarCaracter (uint8_t);
void apagarLeds (void);
void escribirLedyDisplay (uint8_t);
void escanearUART (void);
void escanearTeclas (void);


/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{

   // ---------- CONFIGURACIONES ------------------------------

   boardConfig(); //inicializar y configurar la plataforma
   display7sConfigurarPines();   //GPIO para display como salidas
   uartConfig(UART_USB, 115200); //configura UART
   
   recibidoUART = 'a';
   escribirLedyDisplay(recibidoUART);  //para que programa inicie con led azul encendido y "1" en display 7s
         
   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
      escanearUART();
      escanearTeclas();
      delay(50);  //retardo de 50ms
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones]==============================*/

void display7sConfigurarPines (void)
//configura como salida los GPIO utilizados para el display
{
   for (uint8_t i=0; i<8; i=i+1)   //...para cada una de las 8 salidas de CIAA para display....
   {
      gpioConfig (display7sPines[i], GPIO_OUTPUT); //...la configura como salida...
   }
}

void display7sMostrarCaracter (uint8_t caracter)
//prende o apaga los GPIO de "display7sPines[]" para dibujar en display el caracter "caracter" (lo rescata de "display7sMapaCaracteres[caracter]")
{
   uint8_t auxiliar; //variable auxiliar
   for (uint8_t i=0; i<8; i=i+1)
   {
      auxiliar = display7sMapaCaracteres[caracter];   //copio en "auxiliar" el mapa de caracteres a sacar por display
      auxiliar = auxiliar >> i;  //desplazo "i" bits a la izquierda
      auxiliar = auxiliar & 0b00000001; //AND bit a bit para dejar en 1 o 0 el LSB, y todos los demas en 0
      if (auxiliar) gpioWrite(display7sPines[i], 1); else gpioWrite(display7sPines[i], 0); //prendo o apago el segmento
   }
}

void apagarLeds (void)
//apaga todos los leds
{
   gpioWrite(LEDB, 0);
   gpioWrite(LEDG, 0);
   gpioWrite(LEDR, 0);
   gpioWrite(LED1, 0);
   gpioWrite(LED2, 0);
   gpioWrite(LED3, 0);
}

void escribirLedyDisplay (uint8_t comando)
//segun el valor de comando, prende LED correspondiente y escribe caracter en display 7 seg
{
   switch (comando)
   {
      case 'a':
         apagarLeds();  //apaga todos los leds   
         gpioWrite(LEDB, 1);  //prende led azul
         display7sMostrarCaracter(1);  //muestra "1" en display
         break;
      case 'v':
         apagarLeds();  //apaga todos los leds   
         gpioWrite(LEDG, 1);  //prende led verde
         display7sMostrarCaracter(2);  //muestra "2" en display
         break;
      case 'r':
         apagarLeds();  //apaga todos los leds   
         gpioWrite(LEDR, 1);  //prende led rojo
         display7sMostrarCaracter(3);  //muestra "3" en display
         break;
      case '1':
         apagarLeds();  //apaga todos los leds   
         gpioWrite(LED1, 1);  //prende led1
         display7sMostrarCaracter(4);  //muestra "4" en display
         break;
      case '2':
         apagarLeds();  //apaga todos los leds   
         gpioWrite(LED2, 1);  //prende led2
         display7sMostrarCaracter(5);  //muestra "5" en display
         break;
      case '3':
         apagarLeds();  //apaga todos los leds   
         gpioWrite(LED3, 1);  //prende led3
         display7sMostrarCaracter(6);  //muestra "6" en display
         break;
   }
}

void escanearUART (void)
//chequea si recibió un caracter por UART, y actúa en consecuencia
{
   if (uartReadByte(UART_USB, &recibidoUART)) escribirLedyDisplay(recibidoUART); //segun lo que recibe por UART, prendo led y escribo display 7 seg
}

void escanearTeclas (void)
//chequea si hay alguna tecla presionada, y actúa en consecuencia
{
   bool_t presionada = 0;  //para saber si se presiono una tecla (0 -> no se presionó)
   while(!gpioRead(TEC1))  //si se presiona tecla1....
   {
      if (!presionada)
      {
         display7sMostrarCaracter(7);  //muestra "7" en display 7s
         presionada = 1;
      }
   }
   while(!gpioRead(TEC2))  //si se presiona tecla2....
   {
      if (!presionada)
      {
         display7sMostrarCaracter(8);  //muestra "8" en display 7s
         presionada = 1;
      }
   }
   while(!gpioRead(TEC3))  //si se presiona tecla3....
   {
      if (!presionada)
      {
         display7sMostrarCaracter(9);  //muestra "9" en display 7s
         presionada = 1;
      }
   }
   while(!gpioRead(TEC4))  //si se presiona tecla1....
   {
      if (!presionada)
      {
         display7sMostrarCaracter(0);  //muestra "0" en display 7s
         presionada = 1;
      }
   }
   
   //si se presionó alguna tecla, hay que dejar de mostrar tecla y mostrar led
   if (presionada) display7sMostrarCaracter(recibidoUART);
}

/*==================[fin del archivo]========================================*/