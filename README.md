# Ohmímetor na Placa BitDogLab

## Descrição
Este projeto monta um ohmímetro através da leitura da resistência pela GPIO 28, que contém ADC, e por meio de cálculos, determina de maneira razoável 
o valor da resistência montada em um circuito.

## Funcionalidades
- **Botão A**: Alterna entre os modos de exibição no display.
- **Botão B**: Faz com que a placa entre em modo Bootsel.

## Estrutura do Código
O código apresenta diversas funções, das quais vale a pena citar:

- `mostrar_serie()`: Aproxima o valor lido em R_x (valor do ADC) para o padrão E24 de resistores.
- `achar_cor()`: Mostra no display o nome das cores que o resistor aproximado para a série E24 tem.
- `modo_padrao()`: Modo inicial do projeto, apresenta o valor aproximado e lido pelo ADC.

## Desenvolvedor
Guilherme Miller Gama Cardoso

## Link com o vídeo explicativo
https://drive.google.com/file/d/1p3ClYKP0K_hSHX37an257BhhqBniDbVw/view?usp=sharing