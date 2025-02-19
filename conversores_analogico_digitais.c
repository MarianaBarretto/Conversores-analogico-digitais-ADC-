#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "inc/ssd1306.h" // Biblioteca para controle do display OLED
#include "hardware/adc.h" // Biblioteca para controle do ADC (Conversor Analógico-Digital)
#include "hardware/pwm.h" // Biblioteca para controle do PWM (Modulação por Largura de Pulso)
#include "hardware/i2c.h" // Biblioteca para controle do I2C
#include "hardware/gpio.h" // Biblioteca para controle de GPIO

// Definição dos pinos dos LEDs
const uint PINO_LED_VERDE = 11;  // Pino para o LED verde
const uint PINO_LED_AZUL = 12;   // Pino para o LED azul
const uint PINO_LED_VERMELHO = 13; // Pino para o LED vermelho

// Definição dos pinos dos botões
const uint PINO_BOTAO_A = 5; // Pino para o botão A
const uint PINO_JOYSTICK_X = 26; // Pino para o eixo X do joystick
const uint PINO_JOYSTICK_Y = 27; // Pino para o eixo Y do joystick
const uint PINO_BOTAO_JOYSTICK = 22; // Pino para o botão do joystick

// Configuração do barramento I2C e o endereço do display OLED
#define PORTA_I2C i2c1
#define PINO_SDA 14 // Pino SDA para o I2C
#define PINO_SCL 15 // Pino SCL para o I2C
#define ENDERECO_DISPLAY 0x3C // Endereço do display OLED

// Variáveis de controle global
static volatile uint32_t ultimo_tempo = 0; // Para controle do tempo de debounce
volatile bool desliga_leds = false;  // Flag para indicar se os LEDs devem ser desligados
volatile bool apenas_led_verde = false;  // Flag para indicar se apenas o LED verde deve ficar aceso

// Enumeração para definir os estilos de borda do display
typedef enum {
    NENHUMA,  // Nenhuma borda
    PONTILHADA,  // Borda pontilhada
    TRACEJADA  // Borda tracejada
} estilo_borda_t;

estilo_borda_t estilo_borda = NENHUMA;  // Inicializa com a borda sem estilo

// Variáveis para o controle do display OLED
ssd1306_t display;
uint coordenada_x = 60, coordenada_y = 30; // Coordenadas do quadrado a ser desenhado

// Flag para armazenar o último estado do botão do joystick (utilizado para evitar múltiplas leituras rápidas)
bool ultimo_estado_botao = false;

// Função para tratar as interrupções nos botões
void tratador_de_irq_botao(uint gpio, uint32_t eventos) {
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time()); // Obtém o tempo atual

    // Verifica se o tempo desde a última interrupção é maior que 300ms (debounce)
    if (tempo_atual - ultimo_tempo > 300000) {
        ultimo_tempo = tempo_atual;

        // Verifica qual botão foi pressionado
        if (gpio == PINO_BOTAO_A) {
            desliga_leds = !desliga_leds;  // Alterna o estado de desligamento dos LEDs
            apenas_led_verde = false;  // Se os LEDs forem alternados, desativa o modo "apenas verde"
        }

        // Verifica se o botão do joystick foi pressionado
        if (gpio == PINO_BOTAO_JOYSTICK) {
            apenas_led_verde = !apenas_led_verde;  // Alterna o estado de "apenas LED verde"
            desliga_leds = apenas_led_verde;  // Se o modo "apenas verde" estiver ativo, desliga os outros LEDs

            // Ajusta o estilo da borda dependendo do estado do LED verde
            if (apenas_led_verde) {
                estilo_borda = PONTILHADA;  // Quando o LED verde está aceso, a borda será pontilhada
            } else {
                estilo_borda = TRACEJADA;  // Quando o LED verde está apagado, a borda será tracejada
            }
        }
    }
}

// Função para desenhar borda pontilhada
void desenhar_borda_pontilhada(ssd1306_t *ssd) {
    for (int i = 3; i <= 122; i += 6) {  // Desenha os pontos na parte superior e inferior
        ssd1306_pixel(ssd, i, 3, true);  // Linha superior
        ssd1306_pixel(ssd, i, 58, true); // Linha inferior
    }
    for (int i = 3; i <= 58; i += 6) {  // Desenha os pontos nas laterais
        ssd1306_pixel(ssd, 3, i, true);  // Linha esquerda
        ssd1306_pixel(ssd, 122, i, true); // Linha direita
    }
}

// Função para desenhar borda tracejada
void desenhar_borda_tracejada(ssd1306_t *ssd) {
    for (int i = 3; i <= 122; i += 10) {  // Desenha segmentos horizontais
        ssd1306_line(ssd, i, 3, i + 6, 3, true);   // Linha superior
        ssd1306_line(ssd, i, 58, i + 6, 58, true);  // Linha inferior
    }
    for (int i = 3; i <= 58; i += 10) {  // Desenha segmentos verticais
        ssd1306_line(ssd, 3, i, 3, i + 6, true);   // Linha esquerda
        ssd1306_line(ssd, 122, i, 122, i + 6, true); // Linha direita
    }
}

// Função para configurar o PWM do LED
void configurar_pwm_led(uint pino) {
    gpio_set_function(pino, GPIO_FUNC_PWM);  // Define o pino como saída PWM
    uint slice = pwm_gpio_to_slice_num(pino); // Obtém o número do "slice" PWM associado ao pino
    pwm_set_wrap(slice, 4095);  // Define o valor máximo para o contador PWM
    pwm_set_enabled(slice, true);  // Ativa o PWM
}

// Função para ajustar o brilho do LED com PWM
void atualizar_pwm_led(uint pino, uint16_t valor) {
    uint slice = pwm_gpio_to_slice_num(pino);  // Obtém o número do "slice" PWM associado ao pino
    pwm_set_chan_level(slice, pwm_gpio_to_channel(pino), valor);  // Define o valor do PWM para o pino
}

// Função principal que controla a lógica do sistema
int main() {
    stdio_init_all();  // Inicializa a comunicação padrão (serial, etc.)

    // Configuração do barramento I2C para comunicação com o display OLED
    i2c_init(PORTA_I2C, 400 * 1000);  // Inicializa o I2C com taxa de 400 kHz
    gpio_set_function(PINO_SDA, GPIO_FUNC_I2C);  // Configura o pino SDA para a função I2C
    gpio_set_function(PINO_SCL, GPIO_FUNC_I2C);  // Configura o pino SCL para a função I2C
    gpio_pull_up(PINO_SDA);  // Ativa o resistor pull-up no pino SDA
    gpio_pull_up(PINO_SCL);  // Ativa o resistor pull-up no pino SCL
    ssd1306_t display; // Cria uma variável para o display OLED
    ssd1306_init(&display, 128, 64, false, ENDERECO_DISPLAY, PORTA_I2C);  // Inicializa o display OLED
    ssd1306_config(&display); // Configura as opções do display OLED
    ssd1306_fill(&display, false);  // Limpa o display
    ssd1306_send_data(&display);  // Atualiza o display com o conteúdo atual
    
    // Configuração do ADC para leitura dos valores dos eixos X e Y do joystick
    adc_init();  // Inicializa o ADC
    adc_gpio_init(PINO_JOYSTICK_X);  // Configura o pino X do joystick
    adc_gpio_init(PINO_JOYSTICK_Y);  // Configura o pino Y do joystick

    // Configuração do LED verde como saída
    gpio_init(PINO_LED_VERDE);  // Inicializa o pino do LED verde
    gpio_set_dir(PINO_LED_VERDE, GPIO_OUT);  // Define o pino como saída
    gpio_put(PINO_LED_VERDE, 0);  // Inicializa o LED verde apagado

    // Configuração dos botões com resistores pull-up
    gpio_init(PINO_BOTAO_A);  // Inicializa o pino do botão A
    gpio_set_dir(PINO_BOTAO_A, GPIO_IN);  // Define o pino como entrada
    gpio_pull_up(PINO_BOTAO_A);  // Ativa o resistor pull-up no pino do botão A

    gpio_init(PINO_BOTAO_JOYSTICK);  // Inicializa o pino do botão do joystick
    gpio_set_dir(PINO_BOTAO_JOYSTICK, GPIO_IN);  // Define o pino como entrada
    gpio_pull_up(PINO_BOTAO_JOYSTICK);  // Ativa o resistor pull-up no pino do botão do joystick

    // Configuração das interrupções para os botões
    gpio_set_irq_enabled_with_callback(PINO_BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &tratador_de_irq_botao);  // Interrupção para o botão A
    gpio_set_irq_enabled_with_callback(PINO_BOTAO_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &tratador_de_irq_botao);  // Interrupção para o botão do joystick

    // Configuração do PWM para os LEDs azul e vermelho
    configurar_pwm_led(PINO_LED_AZUL);  // Configura o LED azul para PWM
    configurar_pwm_led(PINO_LED_VERMELHO);  // Configura o LED vermelho para PWM

    while (true) {  // Loop principal de execução
        adc_select_input(0);  // Seleciona o canal do joystick para o eixo X
        uint16_t adc_x = adc_read();  // Lê o valor do eixo X do joystick
        adc_select_input(1);  // Seleciona o canal do joystick para o eixo Y
        uint16_t adc_y = adc_read();  // Lê o valor do eixo Y do joystick

        // Se o modo "apenas_led_verde" estiver ativado
        if (apenas_led_verde) {
            // Desliga os LEDs vermelho e azul, acendendo apenas o LED verde
            atualizar_pwm_led(PINO_LED_VERMELHO, 0);
            atualizar_pwm_led(PINO_LED_AZUL, 0);
            gpio_put(PINO_LED_VERDE, 1);  // Acende o LED verde
            ssd1306_send_data(&display);  // Atualiza o display
            sleep_ms(100);  // Atraso de 100ms antes de continuar
        }
        
        // Caso contrário, se todos os LEDs estiverem desligados        
        else if (!desliga_leds) {
            // Controle normal dos LEDs com base nas leituras do joystick
            uint16_t brilho_vermelho = abs((int)adc_y - 2048) * 2; // Ajusta o brilho do LED vermelho com base na posição Y
            uint16_t brilho_azul = abs((int)adc_x - 2048) * 2;    // Ajusta o brilho do LED azul com base na posição X

            // Se o brilho for muito baixo, desliga o LED
            if (brilho_vermelho < 500) brilho_vermelho = 0;
            if (brilho_azul < 500) brilho_azul = 0;

            // Atualiza o PWM dos LEDs com os novos valores de brilho
            atualizar_pwm_led(PINO_LED_VERMELHO, brilho_vermelho);
            atualizar_pwm_led(PINO_LED_AZUL, brilho_azul);
            gpio_put(PINO_LED_VERDE, 0);  // Desliga o LED verde
        }
        else {
            // Se a flag "desliga_leds" estiver ativa, desliga todos os LEDs
            atualizar_pwm_led(PINO_LED_VERMELHO, 0);
            atualizar_pwm_led(PINO_LED_AZUL, 0);
            gpio_put(PINO_LED_VERDE, 0);
        }

        // Atualiza as coordenadas do quadrado no display OLED
        coordenada_x = 120 - ((adc_x * 50) / 4095 + 3);  // A posição X do quadrado será mapeada de acordo com o valor de adc_x
        coordenada_y = (adc_y * 100) / 4095 + 3;        // A posição Y do quadrado será mapeada de acordo com o valor de adc_y
        ssd1306_fill(&display, false); // Limpa o display

        // Desenha a borda do display com base no estilo atual
        if (estilo_borda == PONTILHADA) {
            desenhar_borda_pontilhada(&display);  // Desenha a borda pontilhada
        } else if (estilo_borda == TRACEJADA) {
            desenhar_borda_tracejada(&display);  // Desenha a borda tracejada
        }

        // Desenha o quadrado dentro da borda
        ssd1306_rect(&display, coordenada_x, coordenada_y + 6, 8, 8, true, true); // Desenha o quadrado de 8x8 pixels
        ssd1306_send_data(&display);  // Envia os dados para o display OLED

        // Aguarda 100ms antes de realizar a próxima atualização
        sleep_ms(100);
    }

    return 0;  // Retorno da função main (nunca será alcançado, pois o loop é infinito)
}

