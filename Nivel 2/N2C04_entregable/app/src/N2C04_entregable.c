/*============================================================================
 * Autor: Rodrigo Furlani
 * Fecha: 25/10/2017
 *===========================================================================*/

/*============================================================================
El programa, cada un segundo, lee las coordenadas del magnetómetro, le agrega
estampa de tiempo (fecha y hora), y a esos datos los saca por consola y los
guarda en un archivo en tarjeta SD
 *===========================================================================*/

/*==================[inlcusiones]============================================*/

#include "sapi.h"        // <= Biblioteca sAPI

#include "ff.h"          // Biblioteca FAT

#include <string.h>     // Biblioteca estandar, para poder usar función "strlen"

/*==================[definiciones y macros]==================================*/

CONSOLE_PRINT_ENABLE

#define ADDRESS_MAG  0x0D  //direccion magnetometro

#define FILENAME "Muestras.txt"  //archivo para guardar muestras en tarjeta SD

/*==================[definiciones de datos internos]=========================*/

static FATFS fs;  //para SD - area de trabajo del volumen
static FIL fp;    //para SD - objeto para cada archivo abierto
UINT nbytes;	//para SD - cantidad de caracteres escritos

uint8_t largoCadena; //para determinar largo de cadena a escribir en trajeta SD

/*==================[definiciones de datos externos]=========================*/

uint8_t bufferTexto[41];    //buffer de caracteres a eacribir en UART y en archivo
                            //formato SXXXX;SYYYY;SZZZZ;yyyy/mm/dd_HH:MM:SS; (mas \r\n\0). Máximo 41 elementos
                            //S es signo de coordenadas magnetometro (+ o -)
                            //XXXX, YYYY, ZZZZ son las coordenadas del magnetometro
                            //yyyy mm dd son año mes y dia de la fecha
                            //HH MM SS son horas minutos y segundos de la hora

rtc_t tiempoReal; //estructura tipo rtc_t, para leer/escribir reloj tiempo real. Contiene año, mes, dia, hora, etc

delay_t miRetardo;   //para retardo de tiempo no bloqueante

//para magnetometro
uint8_t bufferTransmision[2];
uint8_t registroAleer;
int16_t xValue;
int16_t yValue;
int16_t zValue;
uint8_t x_MSB, x_LSB;
uint8_t y_MSB, y_LSB;
uint8_t z_MSB, z_LSB;


/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

void ajustarRTC(void);

void leerMagnetometro(void);

void formatearBufferTexto(void);

void enviarBufferTextoaUart(void);

void guardarBufferTextoSD(void);

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void ){

   // ---------- CONFIGURACIONES ------------------------------

   
   boardConfig(); // Inicializar y configurar la plataforma

   consolePrintConfigUart( UART_USB, 115200 );  // Inicializar UART_USB como salida de consola
      
   ajustarRTC();  //pongo en hora rtc de CIAA
   
   delayConfig(&miRetardo, 1000);   //inicializo para retardo no bloqueante de 1 segundo
   
   //para magnetometro
   i2cConfig(I2C0, 100000);   //inicializar modulo I2C0
   bufferTransmision[0] = 0x0B;  //registro a escribir 0x0B
   bufferTransmision[1] = 1;  //valor a escribir en registro (set/reset period)
   i2cWrite( I2C0, ADDRESS_MAG, bufferTransmision, 2, TRUE );  //escribe 1 en registro 0x0B
   bufferTransmision[0] = 0x09;  //registro a escribir 0x09
   bufferTransmision[1] = 0x11;  //valor a escribir en registro
                                 //over sample ratio 512
                                 //scale 8 gauss
                                 //output data rate 10Hz
                                 //mode continuous
   i2cWrite( I2C0, ADDRESS_MAG, bufferTransmision, 2, TRUE );  //escribe 0x11 en registro 0x09
   
   //SPI para tarjeta SD
   spiConfig(SPI0);
   
   //Para SD - asigna area de trabajo en SD
   if (f_mount( &fs, "", 0 ) != FR_OK){
	   //si falla, la funcion no pudo registar un objeto de sistema de archivos
	   //chequear si esta conectada la SD
	   gpioWrite(LEDR, 1);	//prendo RGB rojo para indicar falla
   } else {gpioWrite (LEDG, 1);}	//prendo RGB verde para indicar OK
      
   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
      if(delayRead(&miRetardo))  //...cada un segundo...
      {
         rtcRead(&tiempoReal);   //leo rtc CIAA
         leerMagnetometro();	//leo magnetometro
         formatearBufferTexto();	//formateo cadena de texto a enviar a archivo y a UART
         enviarBufferTextoaUart();	//envia cadena de texto a UART
         guardarBufferTextoSD();	//guarda cadena de texto en SD
      }
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

void ajustarRTC(void){
   //pone en hora el reloj de tiempo real
   
   //establezco los valores de fecha y hora
   tiempoReal.year = 2017;
   tiempoReal.month = 10;
   tiempoReal.mday = 21;
   tiempoReal.wday = 6;
   tiempoReal.hour = 16;
   tiempoReal.min = 0;
   tiempoReal.sec = 0;
   
   //copio los valores establecidos al rtc de la CIAA
   rtcConfig(&tiempoReal);
}

void leerMagnetometro(void){
   //Lee cordenadas XYZ del magnetómetro, las guarda en xValue, yValue, y zValue
   registroAleer = 0;   //registro X LSB
   i2cRead( I2C0, ADDRESS_MAG, &registroAleer, 1, TRUE, &x_LSB, 1, TRUE);  //leo valor
   //   
   registroAleer = 1;   //registro X MSB
   i2cRead( I2C0, ADDRESS_MAG, &registroAleer, 1, TRUE, &x_MSB, 1, TRUE);  //leo valor
   //  
   registroAleer = 2;   //registro Y LSB
   i2cRead( I2C0, ADDRESS_MAG, &registroAleer, 1, TRUE, &y_LSB, 1, TRUE);  //leo valor
   //   
   registroAleer = 3;   //registro Y MSB
   i2cRead( I2C0, ADDRESS_MAG, &registroAleer, 1, TRUE, &y_MSB, 1, TRUE);  //leo valor
   //   
   registroAleer = 4;   //registro Z LSB
   i2cRead( I2C0, ADDRESS_MAG, &registroAleer, 1, TRUE, &z_LSB, 1, TRUE);  //leo valor
   //   
   registroAleer = 5;   //registro Z MSB
   i2cRead( I2C0, ADDRESS_MAG, &registroAleer, 1, TRUE, &z_MSB, 1, TRUE);  //leo valor
	//     
   xValue = x_MSB;
   xValue = (xValue << 8)|x_LSB;
   //   
   yValue = y_MSB;
   yValue = (yValue << 8)|x_LSB;
   //   
   zValue = z_MSB;
   zValue = (zValue << 8)|z_LSB;
}

void formatearBufferTexto(void){
   //formato SXXXX;SYYYY;SZZZZ;yyyy/mm/dd_HH:MM:SS; (mas \r\n\0)
   //
   //Guarda en vector "bufferTexto[]" el equivalente en ASCII de las coordenadas del
   //magnetometro, así como la fecha y hora.
   //Para convertir los números en ASCII va obteniendo cada dígito por separado, haciendo
   //divisiones enteras ( / ) y divisiones de resto ( % ), y al dígito obtenido se le
   // suma '0' que es el ASCII de donde empiezan los números => el resultado es el ASCII
   // del dígito obtenido.
   
   //formateo de eje x magnetometro
   if (xValue < 0)   //signo de la coordenada. Y si es negativo, lo paso a positivo
   {
     bufferTexto[0] = '-';
     xValue = -xValue;
   } else bufferTexto[0] = '+';	
   bufferTexto[1] = (xValue / 1000) + '0';   //obtengo unidades de mil. Ej, si xValue es 1234 => obtengo '1'
   bufferTexto[2] = (xValue % 1000)/100 + '0';  //obtengo centenas. Ej si xValue es 1234 => 1234%1000=234; y 234/100 => obtengo '2'
   bufferTexto[3] = (xValue % 100)/10 + '0'; //obtengo decenas. Ej si xValue es 1234 => 1234%100=34; y 34/10 => obtengo '3'
   bufferTexto[4] = (xValue % 10) + '0';  //obtengo unidades. Ej si xValue es 1234 => 1234%10=4 => obtengo '4'
   bufferTexto[5] = ';';
   
   //formateo de eje y magnetometro
   if (yValue < 0)   //signo de la coordenada. Y si es negativo, lo paso a positivo
   {
     bufferTexto[6] = '-';
     yValue = -yValue;
   } else bufferTexto[6] = '+';	
   bufferTexto[7] = (yValue / 1000) + '0';
   bufferTexto[8] = (yValue % 1000)/100 + '0';
   bufferTexto[9] = (yValue % 100)/10 + '0';
   bufferTexto[10] = (yValue % 10) + '0';
   bufferTexto[11] = ';';
   
   //formateo de eje z magnetometro
   if (zValue < 0)   //signo de la coordenada. Y si es negativo, lo paso a positivo
   {
     bufferTexto[12] = '-';
     zValue = -zValue;
   } else bufferTexto[12] = '+';	
   bufferTexto[13] = (zValue / 1000) + '0';
   bufferTexto[14] = (zValue % 1000)/100 + '0';
   bufferTexto[15] = (zValue % 100)/10 + '0';
   bufferTexto[16] = (zValue % 10) + '0';
   bufferTexto[17] = ';';
   
   //formateo año
   bufferTexto[18] = (tiempoReal.year / 1000) + '0';
   bufferTexto[19] = (tiempoReal.year % 1000)/100 + '0';
   bufferTexto[20] = (tiempoReal.year % 100)/10 + '0';
   bufferTexto[21] = (tiempoReal.year %10) + '0';
   bufferTexto[22] = '/';
   
   //formateo mes
   bufferTexto[23] = (tiempoReal.month / 10) + '0';
   bufferTexto[24] = (tiempoReal.month % 10) + '0';
   bufferTexto[25] = '/';
   
   //formateo dia
   bufferTexto[26] = (tiempoReal.mday / 10) + '0';
   bufferTexto[27] = (tiempoReal.mday % 10) + '0';
   bufferTexto[28] = '_';
   
   //formateo hora
   bufferTexto[29] = (tiempoReal.hour / 10) + '0';
   bufferTexto[30] = (tiempoReal.hour % 10) + '0';
   bufferTexto[31] = ':';
   
   //formateo minutos
   bufferTexto[32] = (tiempoReal.min / 10) + '0';
   bufferTexto[33] = (tiempoReal.min % 10) + '0';
   bufferTexto[34] = ':';
   
   //formateo segundos
   bufferTexto[35] = (tiempoReal.sec / 10) + '0';
   bufferTexto[36] = (tiempoReal.sec % 10) + '0';
   bufferTexto[37] = ';';
   
   //retorno de carro, avance de linea, fin de cadena
   bufferTexto[38] = '\r';
   bufferTexto[39] = '\n';
   bufferTexto[40] = '\0';
}

void enviarBufferTextoaUart(void){
   //Saca por consola la misma línea que se va a guardar en tarjeta SD (coordenadas, fecha y hora)
   consolePrintString(bufferTexto);
}

void guardarBufferTextoSD(void){
	//Guarda en tarjeta SD las coordenadas del magnetometro con su estampa de tiempo
	disk_timerproc(); //no se que hace esto, pero tiene que estar
   
   largoCadena = strlen(bufferTexto);  //determino la longitud de la cadena de texto. No cuenta fin de cadena \0.
                                       //Por ejemplo, si bufferTexto es "Hola\r\n\0", strlen da 6
   
	//Para SD - abre archivo (o lo crea si no existe), agrega un string al final, y lo cierra
   if ( f_open ( &fp, FILENAME, FA_WRITE | FA_OPEN_APPEND ) == FR_OK ){
   //if ( f_open ( &fp, FILENAME, FA_WRITE | FA_CREATE_ALWAYS ) == FR_OK ){
	   f_write ( &fp, bufferTexto, largoCadena, &nbytes );
	   
	   f_close (&fp);
	   
	   if ( nbytes == largoCadena){
		   gpioWrite(LED3, 1);	//prendo Led verde para indicar OK (escribio toda la cadena)
	   } else {
		   gpioWrite(LED2, 1);	//prendo led rojo para indicar error (no escribio toda la cadena)
	   }
   }
	
}

/*==================[fin del archivo]========================================*/