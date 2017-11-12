/*============================================================================
 * Autor: Rodrigo Furlani
 * Fecha: 28/09/2017
 *===========================================================================*/
 
/*===========================================================================

PROGRAMA QUE IMPLEMENTA UNA CALCULADORA

//*****************
SECCION PARA LEER TECLADO MATRICIAL 4x4
 
El programa escanea regularmente el teclado para determinar si hay una tecla
presionada, y de ser así la envía por la UART.

El teclado se conecta a la CIAA segun las GPIO indicadas en "#define"

Para escanear el teclado, se activa por turnos (en 1) una de las 4 filas
(las otras tres en 0), y para cada fila activada se va mirando si alguna de las
cuatro columnas está activada (en 1); de ser así hay una tecla presionada.

El programa esta hecho de forma que si una tecla se presiona permanentemente,
solo responde a la primera vez que se la ve pulsada. Para que vuelva a tomarse
como válida la presión de esa tecla, debe soltarse y volverse a presionar.
Es decir que si una tecla se presiona permanentemente, solo se envia una sola
vez por UART. Debe soltarse y volverse a presionar para que se envíe otra vez.

Para ello el programa se asegura que haya trancurrido un tiempo sin que se haya
presionado alguna tecla (si detecta que se presiona una tecla antes de ese tiempo,
no le da pelota).
Luego de ese tiempo sin teclas presionadas, escanea el teclado para ver si hay algo
presionado (es decir que detecta si hay alguna tecla presionada, pero no cual).
De detectar que hay una tecla presionada, espera un tiempo de antirrebote y luego
escanea nuevamente el teclado buscando CUAL tecla esta presionada. Si encuentra una
tecla presionada pasa su posición al resto del programa (que la envía por UART), si no
encuentra una tecla presionada es porque la misma no pasó el antirrebote, y entonces 
vuelve a empezar el proceso de escanear el teclado.


//*****************
SECCION PARA CALCULADORA

Cada vez que se presiona una tecla, la misma se guarda en un vector de 7 elementos
(7 porque se espera como máximo dos números de tres dígitos mas el operador). Si se
presionan mas teclas que los elementos del vector, se sobreescribe la tecla mas vieja
guardada en el vector, se hace un desplazamiento de elementos, y se guarda la nueva tecla
(como sea, el vector almacena las últimas 7 teclas presionadas). Paralelamente se va
llevando la cuenta de cuantos de los 7 elementos del vector se van llenando; esto porque 
pueden no llenarse los 7 elementos ya que los numeros a operar pueden ser de menos de
tres dígitos.

Lo anterior se realiza indefinidamente hasta que la tecla presionada sea la de igual (#).
La tecla "igual" no se guarda en el vector, lo que hace es saltar a un proceso que primero
chequea que lo ingresado en el vector sea válido, y de ser así realiza el cálculo.

El chequeo del vector consiste en leer cada una de los 7 elementos para determinar la ubicación
del operador (si no hay operador => invalido). Una vez ubicada la posición del operador, se chequea
si los elementos de las posiciones anteriores al operador corresponden a un número de 1 a 3 dígitos
(si no => invalido). Luego chequea si los elementos en las posiciones posteriores al operador
corresponden a un número de 1 a 3 dígitos (sin no => invalido).

Si alguno de los chequeos anteriores es inválido, se muestra "Error" por consola, se
inicializa la calculadora, y se queda a la espera de la próxima entrada de teclado.

Si los chequeos son todos válidos, se reraliza el cálculo, se muesstra el resultado por
consola, se inicializa la calculadora, y se queda a la espera de la próxima entrada de teclado
 
/*===========================================================================*/

/*==================[inclusiones]============================================*/

#include "sapi.h"        // <= Biblioteca sAPI

/*==================[definiciones y macros]==================================*/

CONSOLE_PRINT_ENABLE

// Alias para GPIO destinadas a teclado. De usar otras GPIO, se cambian acá
#define  TECfila0 RS232_TXD   //salida RS232_TXD para fila 0 teclado
#define  TECfila1 CAN_RD      //salida CAN_RD para fila 1 teclado
#define  TECfila2 CAN_TD      //salida CAN_TD para fila 2 teclado
#define  TECfila3 T_COL1      //salida T_COL1 para fila 3 teclado
#define  TECcol0  T_FIL0      //entrada T_FIL0 para columna 0 teclado (en modo GPIO_INPUT_PULLDOWN)
#define  TECcol1  T_FIL3      //entrada T_FIL3 para columna 1 teclado (en modo GPIO_INPUT_PULLDOWN)
#define  TECcol2  T_FIL2      //entrada T_FIL2 para columna 2 teclado (en modo GPIO_INPUT_PULLDOWN)
#define  TECcol3  T_COL0      //entrada T_COL0 para columna 3 teclado (en modo GPIO_INPUT_PULLDOWN)

// Usadas para lectura de teclado
#define  delayEscaneoTeclado  100 //tiempo en ms, entre llamadas a escaneo de teclado
#define  recargaConteoNoPresionado  2  //tiempo (en multiplos de delayEscaneoTeclado) que debe pasar para considerar valido que el teclado no estubo presionado
#define  antirreboteTeclado   25 //tiempo en ms, para antirrebote de teclado

// Usadas para calculadora
#define  Tigual   '='   //tecla = de calculadora
#define  Tsuma    '+'   //tecla + de calculadora
#define  Tresta   '-'   //tecla - de calculadora
#define  Tmult    '*'   //tecla * de calculadora
#define  Tdiv     '/'   //tecla / de calculadora

/*==================[definiciones de datos internos]=========================*/

// Para lectura teclado
// GPIO conectadas a las filas del teclado (modo GPIO_OUTPUT)
uint8_t tecladoFilas[4] =  
{
   TECfila0,
   TECfila1,
   TECfila2,
   TECfila3
};

// GPIO conectadas a las columnas del teclado (modo GPIO_INPUT_PULLDOWN)
uint8_t tecladoColumnas[4] =  
{
   TECcol0,
   TECcol1,
   TECcol2,
   TECcol3
};

// Vector para mostrar tecla presionada por UART
// MODIFICADO PARA QUE MUESTRE OPERACIONES CALCULADORA
uint8_t TECascii[16] = 
{
   '1', '2', '3', '+',
   '4', '5', '6', '-',
   '7', '8', '9', '*',
   'E', '0', '=', '/'
};

// Estructura para teclado matricial
struct Tmatricial
{
   uint8_t conteoNoPresionado;   //contador para saber si el teclado NO estuvo presionado
   uint8_t tecla; //guarda la tecla presionada
   bool_t procesarTecla;   //si es 1 => se presiono una tecla y => se deber hacer algo
} teclado;


// Para calculadora
struct Calculux
{
   uint8_t entrada[7];   //guarda operandos y operador de calculadora. Dos operandos de máximo 3 dígitos y un operador => 7 elementos
   int8_t indice;  //para direccionar elementos de entrada
   bool_t calcular;  //si es 1 => hay que realizar calculo con lo cargado en entrada
   uint8_t operadorIndex;  //indice del operador dentro de entrada
   bool_t error;
   uint16_t num1; //primer operando
   uint16_t num2; //segundo operando
   int32_t resultado;   //resultado
} calculadora;



/*==================[declaraciones de funciones internas]====================*/

// Usadas para teclado
void configurarTeclado(void);
void escanearTeclado(void);
bool_t tecladoPresionado(void);
uint8_t buscarTecla(void);

// Usadas para calculadora
void inicializarCalculadora(void);
void cargarCalculadora(uint8_t);
void desplazarEntrada(void);
void chequearEntrada(void);
bool_t esNumero(uint8_t);
uint8_t indexOperador(void);
void realizarCalculo(void);
void mostrarResultado(void);
void mostrarError(void);

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
   // ---------- CONFIGURACIONES ------------------------------

   boardConfig();    //Inicializar y configurar la plataforma
   uartConfig(UART_USB, 115200);  //Inicializar UART_USB
   configurarTeclado();    //configuro los GPIO para el teclado
   
   teclado.conteoNoPresionado = recargaConteoNoPresionado;  //inicializo contador teclado no presionado
   teclado.procesarTecla = 0; //inicializo que no se presiono tecla
   
   inicializarCalculadora();

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
      delay(delayEscaneoTeclado);   //tiempo entre escaneos de teclado
      escanearTeclado();   //busca si hay tecla presionada
      if (teclado.procesarTecla) //si hubo tecla presionada...
      {
         uartWriteByte(UART_USB, TECascii[teclado.tecla]);   //saca por UART la tecla presionada
         teclado.procesarTecla = 0; //indica que ya se atendio funcion de tecla pulsada
         cargarCalculadora(TECascii[teclado.tecla]);  //ingresa a calculadora la tecla presionada
         if (calculadora.calcular)  //si debo realizar cálculo...
         {
            chequearEntrada();  //primero chequeo entradas validas
            if (!calculadora.error) //...si no hubo error en los datos de entrada...
            {
               realizarCalculo();   //realiza el calculo
               mostrarResultado();
            }
            else  //si hubo error en datos de entrada...
            {
               mostrarError();
            }
            inicializarCalculadora();
         }
      }
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamente sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

void configurarTeclado(void)
{
   //Configura como entradas y salidas las GPIO que van conectadas al teclado
   //Las GPIO de entrada se configuran con resistencia PULLDOWN activadas
   //
   uint8_t i;  //auxiliar para for
   //
   for (i=0; i<4; i++) {gpioConfig(tecladoFilas[i], GPIO_OUTPUT);}  //configura como salida los GPIO para las filas
   for (i=0; i<4; i++) {gpioConfig(tecladoColumnas[i], GPIO_INPUT_PULLDOWN);}  //configura como entrada con PULLDOWN los GPIO para las columnas
}

void escanearTeclado(void)
{
   //Por un lado se encarga de esperar que pase un tiempo sin teclas presionadas.
   //Si ya pasó ese tiempo, escanea el teclado para determinar que tecla esta presionada (si la hubiese)
   //
   if (!tecladoPresionado())  //si teclado no esta presionado => decremento conteo de tiempo no presionado
   {
      if (teclado.conteoNoPresionado) teclado.conteoNoPresionado = teclado.conteoNoPresionado - 1;   //decremento si no es 0
   }
   else  //si teclado esta presionado....
   {
      if (!teclado.conteoNoPresionado) //si hubo un tiempo sin presionar tecla...
      {
         delay(antirreboteTeclado); //espero tiempo de antirrebote
         teclado.tecla = buscarTecla();   //me fijo que tecla esta presionada (si hubiera)
         if (teclado.tecla != 16)  //si encontro tecla presionada...
         {
            teclado.procesarTecla = 1; //indico a otra parte del programa que hay tecla valida a procesar
            teclado.conteoNoPresionado = recargaConteoNoPresionado;  //inicializo contador teclado no presionado
         }
      }
   }
}


bool_t tecladoPresionado(void)
{
   //Determina si hay al menos una tecla presionada (pero no determina cual)
   //Devuelve un 1 si hay tecla presionada, o un 0 si no hay tecla presionada
   bool_t presionado = 0;
   uint8_t i;  //auxiliar para for
   //
   for (i=0; i<4; i++) {gpioWrite(tecladoFilas[i], 1);} //pone en 1 las 4 filas del teclado
   for (i=0; i<4; i++)
   {
      if (gpioRead(tecladoColumnas[i])) presionado = 1; //si alguna de las columnas es 1 => tecla presionada
   }
   for (i=0; i<4; i++) {gpioWrite(tecladoFilas[i], 0);} //pone en 0 las 4 filas del teclado
   return presionado;
}

uint8_t buscarTecla(void)
{
   //Determina CUAL tecla es la que esta presionada (puede que no haya ninguna)
   //Si hay una tecla presionada, devuelve su posición en la matriz (0..15)
   //Si no hay tecla presionada, devuelve 16
   //La matriz es:
   //    C0 C1 C2 C3
   // F0  0  1  2  3
   // F1  4  5  6  7
   // F2  8  9 10 11
   // F3 12 13 14 15
   //
   bool_t encontroTecla = 0;  //flag para saber si encontro tecla presionada
   uint8_t fil, col, i; //para for escaneo teclado
   uint8_t tecla; //tecla encontrada -> 0..15 valida, 16 no encontro tecla
   //
   for (fil=0; fil<4; fil++)  //para cada fila...
   {
      for (i=0; i<4; i++) {gpioWrite(tecladoFilas[i], 0);} //pone en 0 las 4 filas del teclado
      gpioWrite(tecladoFilas[fil], 1); //pone en 1 la fila "fil" del teclado
      for (col= 0; col<4; col++) //para cada columna...
      {
         if (gpioRead(tecladoColumnas[col])) //si la columna esta en 1 (encontro tecla)....
         {
            encontroTecla = 1;   //encontré la tecla
            tecla = fil*4 + col;
         }
      }
   }
   if (encontroTecla) return tecla; else  //si encontro tecla retorna su posicion 0..15, sino retorna 16
   {
      teclado.conteoNoPresionado = recargaConteoNoPresionado;  //inicializo contador teclado no presionado
      return 16;   
   }
}

void inicializarCalculadora(void)
{
   //pone en 0 el vector calculadora.entrada e inicializa otras variables
   //
   for (calculadora.indice=0; calculadora.indice<7; calculadora.indice++) {calculadora.entrada[calculadora.indice] = 0;}
   calculadora.indice = -1;   //-1 indica que calculadora.entrada esta vacio
   calculadora.calcular = 0;  //indica que no hay que realizar calculo
}

void cargarCalculadora(uint8_t tecla)
{
   //LLega aca cuando se presiona una tecla
   //Si la tecla es la de igual, setea calculadora.calcular para indicar que se debe realizar el calculo
   //Si la tecla es cualquier otra, la guarda en el vector calculadora.entrada
   if (tecla == Tigual) calculadora.calcular = 1;  //si se ingreso tecla igual => indico que debe realizarse cálculo
      else
      {
         calculadora.indice++;   //incremento indice para guardar tecla 
         if (calculadora.indice > 6)   //si calculadora.entrada esta lleno, desplazo para ingresar nueva tecla y sacar la mas vieja
         {  
            desplazarEntrada();
            calculadora.indice = 6;
         }
         calculadora.entrada[calculadora.indice] = tecla;   //guardo tecla presionada
      }
      
}

void desplazarEntrada(void)
{
   //si se siguen presionando teclas cuando calculadora.entrada ya esta lleno, esta función quita la tecla mas vieja
   //desplazando calculadora.entrada un lugar, de forma que se pueda guardar la nueva tecla en calculadora.entrada[6]
   for (uint8_t aux=0; aux<6; aux++) {calculadora.entrada[aux] = calculadora.entrada[aux+1];}
}

bool_t esNumero(uint8_t tecla)
{
   //chequea si "tecla" es un numero del '0' al '9'. Devuelve 1 si es un nº, 0 si no lo es
   bool_t esUnNumero = 0; //asumo inicialmente que "tecla" no es un nº
   uint8_t numeros[10] = {'0','1','2','3','4','5','6','7','8','9'};  //numeros validos a comparar
   for (uint8_t aux=0; aux<10; aux++)
   {
      if (tecla == numeros[aux]) //...si "tecla" es un nº...
      {
         esUnNumero = 1;  //indico que es un nº
         break;   //corto el for, no tiene sentido seguir buscando
      }
   }
   return esUnNumero;
}

uint8_t indexOperador(void)
{
   //devuelve la posicion del operador (si lo hubiera) dentro de calculadora.entrada
   //rangos validos de posicion son de 1 a calculadora.indice-1 (o sea 5 maximo)
   //si no encuentra operador, devuelve 6
   uint8_t operadores[4] = {Tsuma, Tresta, Tmult, Tdiv}; //operadores validos
   uint8_t aux = 0;
   bool_t hayOperador = 0;
   //
   while ((aux<5) && (!hayOperador))
   {
	   aux++;
	   for (uint8_t aux2=0; aux2<4; aux2++) {if (calculadora.entrada[aux] == operadores[aux2]) hayOperador = 1;}
   }
   if (hayOperador) return aux; else return 6;
}

void chequearEntrada(void)
{
   //chequea que en calculadora.entrada haya dos numeros de 3 digitos máximo, y un operador valido entre ellos (+-*/)
   //devuelve en calculadora.error si esta bien (0) o si hay error (1)
   uint8_t aux;   //auxiliar para for
   calculadora.error = 0;
   if (!esNumero(calculadora.entrada[0]) || !esNumero(calculadora.entrada[calculadora.indice])) //si el primer o ultimo elemento de calculadora.entrada no es nº => error
      calculadora.error = 1;  //indico que hubo error
   calculadora.operadorIndex = indexOperador();
   if (calculadora.operadorIndex > 3) calculadora.error = 1;  //si no encontro operador (6) o su indice es mayor a 3 (=> 1º operando de mas de 3 digitos) => error
   if ((calculadora.indice - calculadora.operadorIndex) > 3) calculadora.error = 1; //si 2º operando es de mas de 3 digitos => error
   for (aux=1; aux<calculadora.operadorIndex; aux++) {if (!esNumero(calculadora.entrada[aux])) calculadora.error = 1;}  //si el 1ºoperando no tiene numeros => error
   for (aux=calculadora.operadorIndex+1; aux<calculadora.indice; aux++) {if (!esNumero(calculadora.entrada[aux])) calculadora.error = 1;}   //si 2º operando no tiene numeros => error
}

void realizarCalculo(void)
{
   //pasa de ascii a entero los dos operandos num1 y num2, y determina y realiza la operación (guarda en resultado)
   //para pasar de numeros en ascii en calculadora.entrada a enteros, resto 48 que es el ascii del cero
   //
   switch (calculadora.operadorIndex)  //determino valor operando 1 (num1)
   {
      case 1:
         calculadora.num1 = calculadora.entrada[0]-48;   //num1 de 1 digito
         break;
      case 2:
         calculadora.num1 = (calculadora.entrada[0]-48)*10 + (calculadora.entrada[1]-48);   //num1 de 2 digitos
         break;
      case 3:
         calculadora.num1 = (calculadora.entrada[0]-48)*100 + (calculadora.entrada[1]-48)*10 + (calculadora.entrada[2]-48);   //num1 de 3 digitos
   }
   
   switch (calculadora.indice - calculadora.operadorIndex)  //determino valor operando 2 (num2)
   {
      case 1:
         calculadora.num2 = calculadora.entrada[calculadora.indice]-48; //num2 de 1 digito
         break;
      case 2:
         calculadora.num2 = (calculadora.entrada[calculadora.indice]-48) + (calculadora.entrada[calculadora.indice-1]-48)*10; //num2 de 2 digitos
         break;
      case 3:
         calculadora.num2 = (calculadora.entrada[calculadora.indice]-48) + (calculadora.entrada[calculadora.indice-1]-48)*10 + (calculadora.entrada[calculadora.indice-2]-48)*100; //num2 de 3 digitos
   }
   
   switch (calculadora.entrada[calculadora.operadorIndex])  //determino y hago operacion
   {
      case Tsuma: //si hay que sumar
         calculadora.resultado = calculadora.num1 + calculadora.num2;
         break;
      case Tresta: //si hay que restar
         calculadora.resultado = calculadora.num1 - calculadora.num2;
         break;
      case Tmult: //si hay que multiplicar
         calculadora.resultado = calculadora.num1 * calculadora.num2;
         break;
      case Tdiv: //si hay que dividir
         calculadora.resultado = calculadora.num1 / calculadora.num2;
   }
}

void mostrarError(void)
{
   //muestra "Error" por consola
   uartWriteString(UART_USB, "\r\n");
   uartWriteString(UART_USB, "Error\r\n");
   uartWriteString(UART_USB, "\r\n");
}

void mostrarResultado(void)
{
   //muestra el resultado por consola
   consolePrintEnter();
   consolePrintlnInt(calculadora.resultado);
   consolePrintEnter();
}

/*==================[fin del archivo]========================================*/