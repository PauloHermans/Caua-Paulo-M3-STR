# M3 Sistemas em Tempo Real

Integrantes: Cauã Domingos e Paulo Martino Hermans<br>
Data de entrega: 05/12/2024<br>

# Relatório de Desenvolvimento

O relatório completo do projeto está disponível em: https://drive.google.com/file/d/1SRI5GhOwBoUZdleQ2P2hHnubcYBZjvDz/view?usp=sharing

# Instruções de Compilação
A implementação foi feita utilizando o Linux Xubuntu PREEMPT RT 64-bit em uma máquina virtual, na qual possuía uma memória principal de 4096 MB e 4 processadores.

Após a escrita do código, foi aberto o terminal do SO e compilado o código com a seguinte linha:
```bash
g++ -o siscar -g sistema_abs_airbag.c -lpthread
```
Em seguida foi executado o código:
```bash
.\siscar
```
