/*
 * Por: Wilton Lacerda Silva
 *    Ohmímetro utilizando o ADC da BitDogLab
 *
 * 
 * Neste exemplo, utilizamos o ADC do RP2040 para medir a resistência de um resistor
 * desconhecido, utilizando um divisor de tensão com dois resistores.
 * O resistor conhecido é de 10k ohm e o desconhecido é o que queremos medir.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o voltímetro
#define Botao_A 5  // GPIO para botão A

// Variáveis globais
int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
volatile bool modo = true; // Variável para alternar os modos 
volatile bool cor = true;  // Variável para utilizar funções no display
ssd1306_t ssd;             // Inicializa a estrutura do display
char str_x[8]; // Buffer para armazenar a string
char str_y[8]; // Buffer para armazenar a string
char str_R[8]; // Buffer para armazenar a string
absolute_time_t last_time_A = 0;


// Trecho para modo BOOTSEL com botão B & alternar modos do display
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events){
  if(gpio == botaoB)
    reset_usb_boot(0, 0);

  if(gpio == Botao_A){
      absolute_time_t current = to_ms_since_boot(get_absolute_time());
      if(current - last_time_A < 200000)
        modo = !modo;
      last_time_A = current;
  }
}

float mostrar_serie(){
  int tamanho, i = 0;

  int resistores_e24[] = {
    510, 560, 620, 680, 750, 820, 910,
    1000, 1100, 1200, 1300, 1500, 1600, 1800,
    2000, 2200, 2400, 2700, 3000, 3300, 3600,
    3900, 4300, 4700, 5100, 5600, 6200, 6800,
    7500, 8200, 9100, 10000, 11000, 12000, 13000,
    15000, 16000, 18000, 20000, 22000, 24000, 27000,
    30000, 33000, 36000, 39000, 43000, 47000, 51000,
    56000, 62000, 68000, 75000, 82000, 91000, 100000
  };

  tamanho = sizeof(resistores_e24) / sizeof(resistores_e24[0]);

  for(i = 0; i < tamanho; i++){
      int valor = resistores_e24[i];
      float tolerancia = valor * 5 / 100;

      if(R_x >= (valor - tolerancia) && R_x <= (valor + tolerancia)){
          printf("O valor aproximado e de %d Ohms.\n", valor);
          return valor;
      }
  }

  return 0;
}

void achar_cor(float resistencia){
  int digito1 = 0, digito2 = 0, multiplicador = 0, valor_int;

  char cores[10][10] = {
    "Preto",
    "Marrom",
    "Vermelho",
    "Laranja",
    "Amarelo",
    "Verde",
    "Azul",
    "Violeta",
    "Cinza",
    "Branco"
  };

  while(resistencia >= 100.0f){
    resistencia /= 10.0f;
    multiplicador++;
  }

  while(resistencia < 10.0f){
    resistencia *= 10.0f;
    multiplicador--;
  }

  valor_int = (int)(resistencia);

  digito1 = valor_int / 10;
  digito2 = valor_int % 10;

      ssd1306_fill(&ssd, !cor);                          // Limpa o display
      ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
      ssd1306_draw_string(&ssd, cores[digito1], 10, 10); // Desenha uma string
      ssd1306_draw_string(&ssd, cores[digito2], 10, 30);          // Desenha uma string
      ssd1306_draw_string(&ssd, cores[multiplicador], 10, 50);    // Desenha uma string
      //ssd1306_draw_string(&ssd, str_x, 8, 52);           // Desenha uma string
      //ssd1306_draw_string(&ssd, str_y, 59, 52);          // Desenha uma string
      ssd1306_send_data(&ssd);                           // Atualiza o display
      mostrar_serie();
      sleep_ms(700);
}

void modo_padrao(float R_aprox){
    
  ssd1306_fill(&ssd, !cor);                          // Limpa o display
  ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
  ssd1306_draw_string(&ssd, "R_Lido:", 10, 16);  // Desenha uma string
  ssd1306_draw_string(&ssd, str_R, 70, 16);  // Desenha uma string
  ssd1306_draw_string(&ssd, "R_E24:", 10, 41);          // Desenha uma string

  if(R_aprox)
    ssd1306_draw_string(&ssd, str_y, 60, 41);  // Desenha uma string
  else
    ssd1306_draw_string(&ssd, "---", 60, 41);  // Desenha uma string

  ssd1306_send_data(&ssd);                           // Atualiza o display
  sleep_ms(700);
}
void setup_inicial(){
  // Para ser utilizado o modo BOOTSEL com botão B
  gpio_init(botaoB);
  gpio_set_dir(botaoB, GPIO_IN);
  gpio_pull_up(botaoB);
  gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  // Aqui termina o trecho para modo BOOTSEL com botão B

  gpio_init(Botao_A);
  gpio_set_dir(Botao_A, GPIO_IN);
  gpio_pull_up(Botao_A);
  gpio_set_irq_enabled_with_callback(Botao_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);
}

int main(){
  float tensao;

  setup_inicial();

  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  

  while (true){
    adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

    float soma = 0.0f;

    for (int i = 0; i < 500; i++){
      soma += adc_read();
      sleep_ms(1);
    }

    float media = soma / 500.0f;

      // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
      R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);

    sprintf(str_x, "%1.0f", media); // Converte o inteiro em string
    sprintf(str_y, "%1.0f", mostrar_serie());   // Converte o float em string
    sprintf(str_R, "%1.0f", R_x);   // Converte o float em string

    if(modo){
    // cor = !cor;
    //  Atualiza o conteúdo do display com animações
    /* ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
    ssd1306_line(&ssd, 3, 25, 123, 25, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
    ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6); // Desenha uma string
    ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);  // Desenha uma string
    ssd1306_draw_string(&ssd, "  Ohmimetro", 10, 28);  // Desenha uma string
    ssd1306_draw_string(&ssd, "ADC", 13, 41);          // Desenha uma string
    ssd1306_draw_string(&ssd, "Resisten.", 50, 41);    // Desenha uma string
    ssd1306_line(&ssd, 44, 37, 44, 60, cor);           // Desenha uma linha vertical
    ssd1306_draw_string(&ssd, str_x, 8, 52);           // Desenha uma string
    ssd1306_draw_string(&ssd, str_y, 59, 52);          // Desenha uma string
    ssd1306_send_data(&ssd);                           // Atualiza o display
    mostrar_serie();
    sleep_ms(700); */
    modo_padrao(mostrar_serie());
    }
    else{
      achar_cor(mostrar_serie());
    }
  }
}