# SimpleAssembler

O **SimpleAssembler** é um projeto solo desenvolvido com fins educacionais voltado ao estudo da arquitetura RISC-V, focado nos processos de tradução de linguagem Assembly para código de máquina e na execução de instruções em nível de ISA (*Instruction Set Architecture*).

O projeto consiste em dois componentes principais:

- Um *Assembler* simples para instruções RISC-V RV32I, responsável por traduzir código Assembly para código de máquina em hexadecimal;
- Um *Instruction Set Simulator (ISS)* desenvolvido em linguagem C, projetado para interpretar e executar as instruções geradas pelo assembler.

À medida que o projeto evolui, novas funcionalidades e objetivos podem surgir, influenciando a premissa inicial deste repositório e/ou gerando novos projetos com propostas mais acadêmicas. No momento, considere este projeto como um passatempo sem finalidades sérias ou funcionais.

---

## Objetivos

- Estudar a arquitetura RISC-V
- Compreender o funcionamento de *Assemblers*
- Explorar codificação e decodificação de instruções
- Trabalhar com manipulação de bits em linguagem C
- Implementar parsing e tradução de Assembly
- Simular a execução de instruções RV32I

---

## Funcionalidades

- Tradução de instruções RISC-V RV32I para código de máquina hexadecimal (*Assembler*)
- Simulação básica de execução de instruções RV32I

---

## Funcionamento

O *assembler* traduz instruções contidas no arquivo *instructions.txt* e gera o arquivo *machine_code.txt* com as instruções Assembly em código de máquina. No momento não existe suporte à pseudo-instruçoes, syscalls, segmento .data e várias outras coisas. Considere o arquivo *instructions.txt* como o segmento .text do código assembly.

---

## Objetivos futuros

- Melhorar o interpretador de instruções RV32I para acomodar syscalls
- Implementar um runtime simulator simples para acompanhar o valor dos registradores
- Implementar suporte ao segmento .data
- Suporte à pseudo-instruções
