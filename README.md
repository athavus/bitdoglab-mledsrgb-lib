
# Biblioteca para Controle de Matriz de LEDs RGB na **BitdogLab** com **Raspberry Pi Pico W**

Esta biblioteca foi desenvolvida para a plataforma **BitdogLab** que utiliza a **Raspberry Pi Pico W (RP2040)**. Ela permite controlar uma matriz de LEDs RGB e exibir mensagens rolando, além de ter um modo demo. O sistema utiliza dois botões para alternar entre as funcionalidades e permite capturar logs para análise.

## Estrutura do Projeto

O projeto é composto pelos seguintes arquivos:

1. [**main.c**](main.c) - Contém o código principal do programa, que gerencia a inicialização da matriz de LEDs, os botões e o ciclo de exibição de mensagens.
2. [**logs.py**](logs.py) - Script Python que captura e registra os logs dos dados da matriz de LEDs via comunicação serial.
4. [**frames.h**](frames.h) - Arquivo de cabeçalho com os quadros e animações que podem ser exibidos na matriz de LEDs.
5. [**letters.h**](letters.h) - Arquivo de cabeçalho que define os caracteres e as funções de mapeamento de letras para a matriz.
6. [**led_functions.h**](led_functions.h) - Arquivo de cabeçalho contendo funções para controlar a exibição da mensagem e das animações na matriz de LEDs.

## Dependências

- **Raspberry Pi Pico W (RP2040)**
- **Serial Communication** para os logs via Python.

## Funcionalidades

### 1. Inicialização do Sistema

A inicialização do sistema é feita pela função `matrix_init()` que configura o clock e a inicialização da comunicação da matriz de LEDs. A função principal que deve ser chamada é:

```c
matrix_init(&pio, &sm, &offset);
```

### 2. Modos de Operação

O sistema possui dois modos principais de operação: o **modo demo** e o **modo mensagem em rolagem**. O botão A alterna entre esses dois modos.

#### Modo Demo

O modo demo exibe animações pré-configuradas. A função utilizada para ativá-lo é:

```c
demo_test();
```

#### Modo Mensagem em Rolagem

O modo mensagem exibe uma mensagem rolando na matriz de LEDs. O texto é configurado pela constante `PHRASE`, e a cor dos LEDs é ajustada pelas variáveis `COLOR_LED_R`, `COLOR_LED_G`, e `COLOR_LED_B`. Para exibir a mensagem, a função `message_test()` é utilizada:

```c
message_test(message_color);
```

### 3. Captura de Logs

O script **logs.py** captura os dados via comunicação serial e registra em um arquivo de log. O script pode ser configurado para a porta COM correta e para o nome do arquivo de log.

```python
PORTA = 'COM7'
BAUD = 115200
ARQUIVO_LOG = 'logs_matriz_leds_rgb.txt'
```

### 4. Controle dos Botões

Dois botões são utilizados para alternar entre os modos:

- **Botão A**: Alterna para o modo de demo.
- **Botão B**: Alterna para o modo de mensagem em rolagem.

O tempo de debounce para os botões é configurado pela constante `DEBOUNCE_TIME_MS`.

### 5. Controle da Intensidade e Velocidade

A intensidade dos LEDs é controlada pela constante `INTENSITY`, que varia de 0.0 a 1.0, e a velocidade da rolagem da mensagem é ajustada pela constante `SPEED` (em milissegundos).

## Como Usar

1. **Compilar e carregar o código**: Compile o código C e carregue-o na **Raspberry Pi Pico W**.
2. **Configurar a porta serial**: Conecte a placa via USB e ajuste a porta no script Python **logs.py**.
3. **Iniciar o programa**: Execute o código e use os botões para alternar entre os modos.