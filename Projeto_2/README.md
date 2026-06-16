# SEL0433_microprocessadores
Este repositório é referente à matéria SEL0433 - Aplicação de microprocessadores - Professor Pedro Oliveira C. Junior.

# Projeto 2: Aferidor de temperatura de forno industrial
Eduardo Yumoto Carvalheira - 15636150; Thayson Pereira Alves - 14681087

## Sobre o Projeto
Este projeto consiste no desenvolvimento de um dispositivo de aferição de temperatura de tempo para um forno industrial, que é um processo amplamente utilizado em processos de fabricação metálica e pintura eletrostática. O sistema foi desenvolvido em C para o microcontrolador PIC18F4550, implementando no software do SimulIDE e baseado no hardware do Kit EasyPic v7.

## Requisitos implementados:
O código desenvolvido atende às seguintes necessidades do projeto:
- Exibição da contagem regressiva e da temperatura em display LCD em formatação de XX.X°C
- Leitura analógica da temperatura na faixa de 0 a 100 graus Celsius
- Configuração de um conversor analógico digital utilizando uma tensão externa de 1V
- Contagem de longa (60 segundos) e curta (10 segundos) duração acionada por botão
- Botão de acionamento geral para iniciar simultaneamente a contagem regressiva e a leitura de temperatura
- Formatação de valores sem o uso de dados do tipo float, para economizar memória
- Tratamento do efeitou bouncing (debounce) nos botões
- Inclusão de um LED representando a resistência do forno, configurado para acender em temperaturas maiores que 50°C
