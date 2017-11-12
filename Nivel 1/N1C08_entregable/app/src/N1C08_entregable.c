/*============================================================================
 * Autor: Rodrigo Furlani
 * Fecha: 13/09/2017
 *===========================================================================*/
 
 //***************************************************************************
 //DESCRIPCION
 /*
 Se comanda la posicion del servomotor mediante tres fuentes:
 - potenciometro
 - UART
 - pulsadores
 Inicialmente el control lo tiene el potenciometro
 Segun el caracter recibido por UART se cambia el control a potenciometro, UART, o pulsadores
 Si el control lo tiene potenciometro, el servo se posiciona en un rango continuo de 0° a 180°
 Si el control lo tiene UART o pulsadores. el servo se posiciona en 4 valores fijos: 2°, 60°, 120°, o 178°
 */
 //***************************************************************************

/*==================[inlcusiones]============================================*/

//#include "program.h"   // <= su propio archivo de cabecera (opcional)
#include "sapi.h"        // <= Biblioteca sAPI

//#include "c_i18n_es.h" // <= para traducir el codigo C al espaï¿½ol (opcional)
//#include "c_i18n_es.h" // <= para traducir la sAPI al espaï¿½ol (opcional)

/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

CONSOLE_PRINT_ENABLE

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void ){

   //CONSTANTES
   #define pote '1'     //caracter recibido por UART para control por potenciometro
   #define uart '2'     //caracter recibido por UART para control por UART
   #define pulsador '3' //caracter recibido por UART para control por pulsador
   
   
   //VARIABLES
   unsigned char control = pote;  //indica quien tiene el control del servo
   unsigned char recibido; //para lectura de caracter recibido por UART
   uint16_t potePosition;  //contiene lectura ADC del potenciometro
   uint32_t servoPosition; //contiene angulo a posicionar por el servo
   bool_t boolPWM;   //variable auxiliar para trabajar con PWM
   
   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma
   boardConfig();
   
   // UART
   uartConfig( UART_USB, 115200 );
   
   // ADC
   adcConfig( ADC_ENABLE);
   
   // SERVO PWM
   boolPWM = servoConfig( SERVO0, SERVO_ENABLE );
   boolPWM = servoConfig( SERVO0, SERVO_ENABLE_OUTPUT);

   // Inicializar UART_USB como salida de consola
   //consolePrintConfigUart( UART_USB, 115200 );
  
   
   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
      //TRATAMIENTO CONTROL CON POTENCIOMETRO
      //******************************************************************
      while( control == pote )   //mientras el control lo tenga el pote...
      {
         potePosition = adcRead( CH2 );   //lee potenciometro en CH2 ADC
         servoPosition = potePosition * 180;
         servoPosition = servoPosition / 1023;  //servoPosition = potePosition * 180 / 1023 => servoPosition queda en rango 0..180°
         boolPWM = servoWrite( SERVO0, ((uint8_t)servoPosition) ); //setea posicion del servo. Uso casting porque la funcion espera una variable de 8 bits unsigned
         delay( 100 );  //espero 100ms
         
         //chequeo si llego algo por UART, y de ser asi si debo pasar el control a pulsador o a UART
         if( uartReadByte( UART_USB, &recibido)) //si llego un caracter por UART
         {
            switch( recibido )
            {
               case uart:
                  control = uart;   //si caracter recibido es "uart", paso control a UART
                  break;
               case pulsador:
                  control = pulsador;  //si caracter recibido es "pulsador", paso control a pulsador
                  break;
               default:
                  control = pote;   //si caracter recibido no es "uart" o "pulsador", control sigue siendo potenciometro
            }
         }
      }
      //******************************************************************
      
      
      //TRATAMIENTO CONTROL CON UART
      //******************************************************************
      while( control == uart )   //mientras el control lo tenga la UART...
      {
         if( uartReadByte( UART_USB, &recibido) ) //si llego un caracter por UART
         {
            switch( recibido )
            {
               case 'a':
                  boolPWM = servoWrite( SERVO0, 2); //si se recibe 'a' => servo a 2°
                  break;
               case 'b':
                  boolPWM = servoWrite( SERVO0, 60); //si se recibe 'b' => servo a 60°
                  break;
               case 'c':
                  boolPWM = servoWrite( SERVO0, 120); //si se recibe 'c' => servo a 120°
                  break;
               case 'd':
                  boolPWM = servoWrite( SERVO0, 178); //si se recibe 'd' => servo a 178°
                  break;
               case pote:
                  control = pote;   //si caracter recibido es "pote", paso control a potenciometro
                  break;
               case pulsador:
                  control = pulsador;  //si caracter recibido es "pulsador", paso control a pulsador
                  break;
               default:
                  control = uart;   //si caracter recibido no es "uart", "pulsador", o letra de posicionamiento, control sigue siendo uart
            }
         }
         delay( 100 );  //espero 100ms
      }
      //******************************************************************
      
      
      //TRATAMIENTO CONTROL CON PULSADORES
      //******************************************************************
      while( control == pulsador )  //mientras el control lo tengan los pulsadores...
      {
         //
         if( !gpioRead(TEC1) ) boolPWM = servoWrite( SERVO0, 2); //si TEC1 presionada => servo a 2°
         if( !gpioRead(TEC2) ) boolPWM = servoWrite( SERVO0, 60); //si TEC2 presionada => servo a 60°
         if( !gpioRead(TEC3) ) boolPWM = servoWrite( SERVO0, 120); //si TEC3 presionada => servo a 120°
         if( !gpioRead(TEC4) ) boolPWM = servoWrite( SERVO0, 178); //si TEC4 presionada => servo a 178°
         delay( 100 );  //espero 100ms
         
         //chequeo si llego algo por UART, y de ser asi si debo pasar el control a potenciometro o a UART
         if( uartReadByte( UART_USB, &recibido) ) //si llego un caracter por UART
         {
            switch( recibido )
            {
               case uart:
                  control = uart;   //si caracter recibido es "uart", paso control a UART
                  break;
               case pote:
                  control = pote;  //si caracter recibido es "pote", paso control a potenciometro
                  break;
               default:
                  control = pulsador;   //si caracter recibido no es "uart" o "pote", control sigue siendo pulsador
            }
         }
      }
      //******************************************************************
      
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

/*==================[fin del archivo]========================================*/