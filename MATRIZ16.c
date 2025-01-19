#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"


//define o LED de saída
#define GPIO_LED 13

// Configuração do pino do buzzer
#define BUZZER_PIN 21

// Configuração da frequência do buzzer (em Hz)
#define BUZZER_FREQUENCY 100


// Define os pinos dos LEDs
#define LED_VERDE 11
#define LED_AZUL 12
#define LED_VERMELHO 13


//define os pinos do teclado com as portas GPIO
uint columns[4] = {11, 10, 9, 8}; 
uint rows[4] = {19, 18, 17, 16};

//mapa de teclas
char KEY_MAP[16] = {
    '1', '2' , '3', 'A',
    '4', '5' , '6', 'B',
    '7', '8' , '9', 'C',
    '*', '0' , '#', 'D'
};

uint _columns[4];
uint _rows[4];
char _matrix_values[16];
uint all_columns_mask = 0x0;
uint column_mask[4];

// Função para inicializar os LEDs
void init_leds() {
    gpio_init(LED_VERDE);
    gpio_init(LED_AZUL);
    gpio_init(LED_VERMELHO);

    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);

    gpio_put(LED_VERDE, false);
    gpio_put(LED_AZUL, false);
    gpio_put(LED_VERMELHO, false);
}

// Função para acender todos os LEDs
void acender_branco() {
    gpio_put(LED_VERDE, true);
    gpio_put(LED_AZUL, true);
    gpio_put(LED_VERMELHO, true);
}

// Função para apagar todos os LEDs
void apagar_leds() {
    gpio_put(LED_VERDE, false);
    gpio_put(LED_AZUL, false);
    gpio_put(LED_VERMELHO, false);
}

//imprimir valor binário
void imprimir_binario(int num) {
 int i;
 for (i = 31; i >= 0; i--) {
  (num & (1 << i)) ? printf("1") : printf("0");
 }
}

//inicializa o keypad
void pico_keypad_init(uint columns[4], uint rows[4], char matrix_values[16]) {

    for (int i = 0; i < 16; i++) {
        _matrix_values[i] = matrix_values[i];
    }

    for (int i = 0; i < 4; i++) {

        _columns[i] = columns[i];
        _rows[i] = rows[i];

        gpio_init(_columns[i]);
        gpio_init(_rows[i]);

        gpio_set_dir(_columns[i], GPIO_IN);
        gpio_set_dir(_rows[i], GPIO_OUT);

        gpio_put(_rows[i], 1);

        all_columns_mask = all_columns_mask + (1 << _columns[i]);
        column_mask[i] = 1 << _columns[i];
    }
}

// Definição de uma função para inicializar o PWM no pino do buzzer
void pwm_init_buzzer(uint pin) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

//coleta o caracter pressionado
char pico_keypad_get_key(void) {
    int row;
    uint32_t cols;
    bool pressed = false;

    cols = gpio_get_all();
    cols = cols & all_columns_mask;
    imprimir_binario(cols);
    
    if (cols == 0x0) {
        return 0;
    }

    for (int j = 0; j < 4; j++) {
        gpio_put(_rows[j], 0);
    }

    for (row = 0; row < 4; row++) {

        gpio_put(_rows[row], 1);

        busy_wait_us(10000);

        cols = gpio_get_all();
        gpio_put(_rows[row], 0);
        cols = cols & all_columns_mask;
        if (cols != 0x0) {
            break;
        }   
    }

    for (int i = 0; i < 4; i++) {
        gpio_put(_rows[i], 1);
    }

    if (cols == column_mask[0]) {
        return (char)_matrix_values[row * 4 + 0];
    }
    else if (cols == column_mask[1]) {
        return (char)_matrix_values[row * 4 + 1];
    }
    if (cols == column_mask[2]) {
        return (char)_matrix_values[row * 4 + 2];
    }
    else if (cols == column_mask[3]) {
        return (char)_matrix_values[row * 4 + 3];
    }
    else {
        return 0;
    }
}

// Definição de uma função para emitir um beep com duração especificada
void beep(uint pin, uint duration_ms) {
    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(100); // Pausa de 100ms
}

//função principal
int main() {
    stdio_init_all();
    pico_keypad_init(columns, rows, KEY_MAP);
    char caracter_press;
    char buffer[4];
    uint use_uart = 1;
    gpio_init(GPIO_LED);
    gpio_set_dir(GPIO_LED, GPIO_OUT);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    init_leds();

    // Inicializar o PWM no pino do buzzer
    pwm_init_buzzer(BUZZER_PIN);

    while (true) {

        if (use_uart == 0) {
            caracter_press = pico_keypad_get_key();
            printf("\nTecla pressionada: %c\n", caracter_press);

        }
        else {
            scanf("%4s", buffer);
            printf("\nTecla pressionada: %s\n", buffer);

        }

        //Avaliação de caractere para o LED
        if (caracter_press=='B' || buffer[0] == 'B')
        {
            beep(BUZZER_PIN, 1000); // Bipe de 500ms
            gpio_put(GPIO_LED,true);

        }else
        {
            gpio_put(GPIO_LED,false);
        }
        if (caracter_press == 'A' || caracter_press == 'a' || buffer[0] == 'a' || buffer[0] == 'A') { // Verifica se a tecla pressionada é "A" ou "a"
            printf("Tecla 'A' pressionada! Acendendo luz branca...\n");
            acender_branco(); // Acende os LEDs
            sleep_ms(5000);   // Fica aceso por 5 segundos
            apagar_leds();    // Apaga os LEDs após o intervalo
        }

        busy_wait_us(500000);
    }
}