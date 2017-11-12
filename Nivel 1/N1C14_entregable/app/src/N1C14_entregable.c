/*============================================================================
 * Autor: Rodrigo Furlani
 * Fecha: 04/10/2017
 *===========================================================================*/
 
 /*============================================================================
 El programa implementa un juego tipo simon.
 Las luces del simon son 4 leds de la CIAA, y los pulsadores del simon son
 los 4 switch de la CIAA.
 Por UART recibe una secuencia de hasta 10 caracteres, que se corresponden
 con las luces/teclas (a, 1, 2 y 3). Se da por cargada la secuencia cuando
 se hayan ingresado 10 caracteres, o cuando se ingresen menos caracteres y 
 el último que se reciba sea "#". Hay control de caracteres incorrectos
 y de que se ingrese "#" estando vacia la secuencia.
 Una vez cargada la secuencia, se hacen parpadear los 4 leds para indicar
 al jugador que comienza el juego. Luego se empieza a reproducir la secuencia,
 comenzano por un solo led y aumentando progresivamente de a un led hasta el 
 final de la secuencia. Antes de cada progresión a otro led, se va chequeando
 que el jugador presione las teclas en la secuencia correcta. Si el jugador
 se equivoca, se prende el RGB rojo por 3 segundos; si el jugador completa
 correctamente la secuencia , se prende el RGB verde por 3 segundos; en ambos
 casos luego se reinicia el juego para una próxima partida.
 *===========================================================================*/
  
/*==================[inclusiones]============================================*/

#include "sapi.h"        // <= Biblioteca sAPI

/*==================[definiciones y macros]==================================*/

#define FIN_SECUENCIA   '#'   //caracter de fin de secuencia

typedef enum{
   INICIALIZAR_JUEGO,
   RECIBIR_SECUENCIA,
   AVISAR_COMIENZO,
   JUGAR,
   REPRODUCIR_BIEN,
   REPRODUCIR_MAL
} estado_t;

/*======================[definiciones de datos]==============================*/

estado_t estadoJuego;   //para saber en que instancia del juego estoy (maquina estado)

struct secuenciaJuego{
   uint8_t sec[10];  //guarda la secuencia recibida por UART
   uint8_t indice;   //para saber de que longitud es la secuencia
   uint8_t fase;     //fase de la secuencia en que ese está jugando. Rango va de 0 a "indice"
   bool_t errorJuego;   //para indicar si jugador cometio un error (1) o no lo cometio (0)
} secuencia;

/*=======================[declaraciones de funciones]========================*/

void inicializarJuego(void);
void almacenarSecuencia(void);
bool_t chequearRecibido(uint8_t);
void blinkInicioJuego(void);
void mostrarSecuencia(uint8_t);
void jugarSimon(void);
uint8_t leerTecla(void);
void felicitar(void);
void defenestrar(void);


/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
   // ---------- CONFIGURACIONES ------------------------------
   boardConfig(); //Inicializar y configurar la plataforma
   uartConfig(UART_USB, 115200); //inicializa UART_USB
   
   estadoJuego = INICIALIZAR_JUEGO;

   
   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
      switch (estadoJuego)
      {
         case INICIALIZAR_JUEGO:
            inicializarJuego();
            break;
         
         case RECIBIR_SECUENCIA:
            almacenarSecuencia();
            break;
         
         case AVISAR_COMIENZO:
            blinkInicioJuego();
            break;
         
         case JUGAR:
            jugarSimon();
            break;
         
         case REPRODUCIR_BIEN:
            felicitar();
            break;
         
         case REPRODUCIR_MAL:
            defenestrar();
      }
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*=======================[definiciones de funciones]=========================*/

void inicializarJuego(void){
   //inicializa variables del juego
   estadoJuego = RECIBIR_SECUENCIA; //empiezo esperando la secuencia del simon
   secuencia.indice = 0;   //no se recibio secuencia desde UART
   secuencia.fase = 0;  //para arrancar la secuencia desde 0 al jugar
   secuencia.errorJuego = 0;  //inicializa que jugador no cometio error (aun, jeje)
}

void almacenarSecuencia(void){
   //Almacena la secuencia recibida por UART
   //Cada vez que llega un caracter por UART, chequea que sea valido,
   //si es invalido solicita que se reingrese
   //Da la secuencia como finalizada si llegan 10 caracteres o el simbolo #
   //Si el primer caracter es #, da error y pide que se reintente
   //Variable secuencia.indice termina valiendo la cantidad de elementos ingresados,
   //x ej, si se ingresaron elementos 0 a 5 en secuencia.sec => secuencia.indice vale 6
   uint8_t recibido;
   bool_t recibidoOk;   //1 si lo recibido es valido, 0 si error
   //
   if (uartReadByte(UART_USB, &recibido)) //si llego algo por UART...
   {
      recibidoOk = chequearRecibido(recibido);//
      if (recibidoOk)   //si lo recibido esta bien...(if else nº1)
      {
         if (recibido == FIN_SECUENCIA) //(if else nº2)
         {
            if (secuencia.indice == 0) //(if else nº3)
            {
               uartWriteString(UART_USB, "\r\n");
               uartWriteString(UART_USB, "La secuencia debe ser de al menos un caracter.\r\n");
               uartWriteString(UART_USB, "Intentelo nuevamente.\r\n");
               uartWriteString(UART_USB, "\r\n");//
            }
            else //(if else nº3)
            {
               estadoJuego = AVISAR_COMIENZO;
            } //(if else nº3)
         }
         else //(if else nº2)
         {
            secuencia.sec[secuencia.indice] = recibido;
            secuencia.indice++;
            if (secuencia.indice == 10)
            {
               estadoJuego = AVISAR_COMIENZO;
            }
         } //(if else nº2)
      }
      else //(if else nº1)
      {
         uartWriteString(UART_USB, "\r\n");
         uartWriteString(UART_USB, "Caracteres permitidos:\r\n");//
         uartWriteString(UART_USB, "a   1   2   3   #\r\n");
         uartWriteString(UART_USB, "Intentelo nuevamente:\r\n");
         uartWriteString(UART_USB, "\r\n");
      }  //(if else nº1)
   }
}

bool_t chequearRecibido(uint8_t recepcion){
   //chequea si lo recibido por UART es valido
   //devuelve 1 si es valido, 0 si no
   bool_t esValido = 0;
   uint8_t validos[5] = {'a', '1', '2', '3', FIN_SECUENCIA};
   //
   for (uint8_t j=0; j<5; j++)
   {
      if (recepcion == validos[j])
      {
         esValido = 1;
         break;   //ya coincidió, corto el for
      }
   }
   return esValido;
}

void blinkInicioJuego(void){
   //parpadea los 4 leds para indicar que comienza el juego
   delay_t retardo;
   //
   delayConfig(&retardo, 125);   //prepara para retardo no bloqueante de 125ms
   for (uint8_t j=0; j<4; j++)
   {
      gpioWrite(LEDB,1);
      gpioWrite(LED1,1);
      gpioWrite(LED2,1);
      gpioWrite(LED3,1);
      while(!delayRead(&retardo));  //espera a que pase el retardo
      gpioWrite(LEDB,0);
      gpioWrite(LED1,0);
      gpioWrite(LED2,0);
      gpioWrite(LED3,0);
      while(!delayRead(&retardo));  //espera a que pase el retardo
   }
   delayConfig(&retardo, 1000);  //prepara para retardo de 1seg
   while(!delayRead(&retardo));  //espera a que pase el retardo
   estadoJuego = JUGAR;
}

void felicitar(void){
   //enciende el RGB verde durante 3 segundos para felicitar al jugador
   delay_t retardo;
   //
   delayConfig(&retardo, 3000);  //prepara para retardo de 3 seg
   gpioWrite(LEDG,1);   //prende RGB verde
   while(!delayRead(&retardo));  //espera a que pase el retardo
   gpioWrite(LEDG,0);   //apaga RGB verde
   estadoJuego = INICIALIZAR_JUEGO;
}

void defenestrar(void){
   //enciende el RGB rojo durante 3 segundos para decirle al jugador que mejor se dedique al chin-chon
   delay_t retardo;
   //
   delayConfig(&retardo, 3000);  //prepara para retardo de 3 seg
   gpioWrite(LEDR,1);   //prende RGB rojo
   while(!delayRead(&retardo));  //espera a que pase el retardo
   gpioWrite(LEDR,0);   //apaga RGB rojo
   estadoJuego = INICIALIZAR_JUEGO;
}

void mostrarSecuencia(uint8_t ultimo){
   //Muestra al jugador la secuencia que va a tener que repetir
   //La muestra desde secuencia.sec[0] hasta secuencia.sec[ultimo]
   //o sea con "ultimo" defino hasta qué elemento de la secuencia completa voy a mostrar,
   //"ultimo" va en un rango de 0 a "secuencia.indice"
   //Al mostrar, prende cada led durante 600ms, apaga y espera 200ms para prender el otro
   uint8_t luces[4] = {LEDB, LED1, LED2, LED3};
   uint8_t led_a_mostrar;
   delay_t tiempoEncendido;
   delay_t tiempoApagado;
   //
   delayConfig(&tiempoEncendido, 600); //prepara para retardo de 600ms
   delayConfig(&tiempoApagado, 200);   //prepara para retardo de 200ms
   for (uint8_t j=0; j<(ultimo+1); j++)
   {
      while(!delayRead(&tiempoApagado));
      switch (secuencia.sec[j])
      {
         case 'a':
            led_a_mostrar = 0;
            break;
         case '1':
            led_a_mostrar = 1;
            break;
         case '2':
            led_a_mostrar = 2;
            break;
         case '3':
            led_a_mostrar = 3;
      }
      gpioWrite(luces[led_a_mostrar], 1);
      while(!delayRead(&tiempoEncendido));
      gpioWrite(luces[led_a_mostrar], 0);
   }
}

void jugarSimon(void){
   //
   uint8_t tecla; //tecla presionada
   while ((secuencia.fase < secuencia.indice) && (!secuencia.errorJuego))
   {
      mostrarSecuencia(secuencia.fase);  //muestro la parte de la secuencia a replicar por jugador (primero una luz, luego 2, etc)
      
      //ahora voy a ver que teclas presiona jugador, y si son las correctas
      for (uint8_t j=0; j<(secuencia.fase+1); j++)
      {
         tecla = leerTecla();
         if (tecla != secuencia.sec[j])   //comparo tecla presionada contra secuencia
         {
            secuencia.errorJuego = 1; //indico que hubo error
            break;   //corto el for
         }
      }
      secuencia.fase++;
      delay(1000);
   }
   
   if (secuencia.errorJuego) estadoJuego = REPRODUCIR_MAL; else estadoJuego = REPRODUCIR_BIEN;
}

uint8_t leerTecla(void){
   //Espera que se presione una tecla, y devuelve su equivalencia 'a', '1', '2' o '3'
   //Para independizarse del tiempo en que se mantenga una tecla presionada
   //(o sea, si la mantengo presionada, que no la interprete como varias pulsaciones sucesivas)
   //se hace lo siguiente:
   //- primero se espera a que no haya ninguna tecla presionada
   //- luego se espera un tiempo de antirrebote
   //- luego se vuelve a chequear que no haya ninguna tecla presionada
   //- luego queda a la espera de que se presione una tecla
   //- al presionar una tecla, enciende y apaga el led correspondiente como acuse pulsacion
   //RECORDAR que las teclas son logica negada (presionada = 0)
   uint8_t j;
   uint8_t teclaPresionada = 1;
   uint8_t teclado[4] = {TEC1, TEC2, TEC3, TEC4};
   uint8_t luces[4] = {LEDB, LED1, LED2, LED3};
   
   //aca espera que no haya ninguna tecla presionada, luego antirrebote, y luego vuelve
   //a chequear que no haya tecla presionada. Repite hasta que ocurra esta condicion
   while (teclaPresionada)
   {
      while (teclaPresionada)
      {
         teclaPresionada = 0;
         for (j=0; j<4; j++) {if (!gpioRead(teclado[j])) teclaPresionada = 1;}//
      }
      delay(100); //antirrebote
      for (j=0; j<4; j++) {if (!gpioRead(teclado[j])) teclaPresionada = 1;}
   }
   
   //ahora espera a que se presione una tecla
   teclaPresionada = 5;
   while (teclaPresionada == 5)
   {
      for (j=0; j<4; j++)
      {
         if (!gpioRead(teclado[j]))
         {
            teclaPresionada = j;
            break;   //corto el for porque se presiono tecla
         }
      }
   }
   
   gpioWrite(luces[teclaPresionada], 1);
   delay(300); //luz prendida para acuse
   gpioWrite(luces[teclaPresionada], 0);
   
   switch (teclaPresionada)
   {
      case 0:
         return 'a';
         break;
      case 1:
         return '1';
         break;
      case 2:
         return '2';
         break;
      default:
         return '3';
   }
}

/*==================[fin del archivo]========================================*/