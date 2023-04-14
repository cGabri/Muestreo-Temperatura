// Inclusion de librerias
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>

// GPIO del 7 segmentos
#define SEG_A   0
#define SEG_B   1
#define SEG_C   2
#define SEG_D   3
#define SEG_E   4
#define SEG_F   5
#define SEG_G   6
#define SEG_DP  7
#define DIG_1   8
#define DIG_2   9
#define DIG_3   10

// GPIO del ADC
#define ANALOG_GPIO 26
// Canal del ADC
#define ANALOG_CH   0

// Mascara para GPIO de segmentos
#define SEGMENTS  (1 << SEG_A) | (1 << SEG_B) | (1 << SEG_C) | (1 << SEG_D) | \  
                  (1 << SEG_E) | (1 << SEG_F) | (1 << SEG_G) | (1 << SEG_DP)

// Mascara para cada digito
#define DIGIT_1   (1 << DIG_1)
#define DIGIT_2   (1 << DIG_2)
#define DIGIT_3   (1 << DIG_3)

// Mascara para GPIO de digitos
#define DIGITS    DIGIT_1 | DIGIT_2 | DIGIT_3

// Tiempo de refresco para el siete segmentos
#define SLEEP_MS 10

// Coeficiente de temperatura del termistor
#define BETA 3950

/*
 * @brief Display temperature value in 7 segment display
 * @param temperature: value to display
 */
void display_temp(float temperatura) {
  // Array con valores de segmentos para cada numero (prende con cero el segmento)
  const uint8_t digitos[10] = { 
    ~0x3f, ~0x06, ~0x5b, ~0x4f, ~0x66, 
    ~0x6d, ~0x7d, ~0x07, ~0x7f, ~0x6f 
  };
  // Variables para cada digito individualmente
  uint8_t digit_1, digit_2, digit_3;
  // Obtengo el valor de la decena

  // Para calcular la decena
  int unidad; // Declaro la variable unidad para guardar el valor de unidad del numero
  int y; // Declaro y como una variable auxiliar 
  if (temperatura > 10) { // Declaro este if por si la temperatura vale mas de 10 grados que pase eso
    unidad = (int)temperatura % 10;  // Calculo la unidad pasando la temperatura a un numero entero descartando los valores despues de la coma y calculo el resto de la division de temperatura entre 10
    y = temperatura - unidad; // Calculo la decena haciendo la temperatura menos la unidad y lo guardo en la variable auxiliar
    digit_1 = y / 10; // Despues de restarle al numero la unidad lo divido entre 10 y obtengo el valor de la decena
  }
else { // Si la temperatura es menor a 10 grados pasa esto
  digit_1 = 0; // La decena en el caso de que la temperatura sea menor a 10 siempre va a ser 0
  unidad = (int)temperatura; // Con esto lo que hago es quitarle la coma al numero que equivale a la unidad

}
  // Prendo el digito 1
  gpio_put_masked(DIGITS, DIGIT_1);
  // Dibujo el digito
  gpio_put_masked(SEGMENTS, digitos[digit_1]);
  // Espero
  sleep_ms(SLEEP_MS);
  // Obtengo el valor de la unidad
  digit_2 = unidad; // Asigno el valor de unidad
  // Prendo el digito 2
  gpio_put_masked(DIGITS, DIGIT_2);
  // Dibujo el digito 2 con el punto
  gpio_put_masked(SEGMENTS, digitos[digit_2] & ~(1 << SEG_DP));
  // Espero
  sleep_ms(SLEEP_MS);
  // Obtengo el valor del primer decimal
  digit_3 = (temperatura - (int)temperatura)*10; // Aca calculo el valor del primer decimal haciendo (int)temperatura(numero sin coma) - temperatura numero con coma ej 4.3 - 4 y el resultad seria el primer decimal 0.3
  // Prendo el digito 3
  gpio_put_masked(DIGITS, DIGIT_3);
  // Dibujo el digito
  gpio_put_masked(SEGMENTS, digitos[digit_3]);
  // Espero
  sleep_ms(SLEEP_MS);
}

/*
 * @brief Programa principal
 */
int main() {
  // Variable para almacenar el resultado del ADC
  uint16_t adc_reading = 0;
  // Variable para guardar el valor de temperatura
  float temperatura = 0.0;
  // Inicializacion de UART
  stdio_init_all();
  // Inicializacion de GPIO
  gpio_init_mask(SEGMENTS | DIGITS);
  // Configuro GPIO como salida
  gpio_set_dir_out_masked(SEGMENTS | DIGITS);
  // Valores iniciales de GPIO cero
  gpio_put_masked(SEGMENTS | DIGITS, 0x0);
  // Inicializo ADC
  adc_init(); // Le aviso al micro controlador que voy a usar el adc
  // Inicializo GPIO26 como entrada analogica
  adc_gpio_init(26); 
  // Selecciono canal analogico
  adc_select_input(0);
  // Enciendo todos los digitos
  gpio_put_masked(DIGITS, 0x0);

  // Bucle infinito
  while (true) {
    // Leer NTC
    adc_reading = adc_read();
    // Calculo temperatura
    temperatura = 1 / (log(1 / (4095. / adc_reading - 1)) / BETA + 1.0 / 298.15) - 273.15;
    // Puede verificarse el valor antes de mostrar en el display con el siguiente printf
    printf("Temperatura: %.2f\n", temperatura); // En esta linea imprimo el valor de la tempertura en la terminal para verificar el resultado de la cuenta y que funcione bien el display
    printf("analogvalue: %.2f\n", adc_reading); // En esta linea imprimo en la terminal el valor de la lectura que hace el adc
    // Muestro temperatura
    display_temp(temperatura);
    // Espero
    sleep_ms(SLEEP_MS);
  }
}
