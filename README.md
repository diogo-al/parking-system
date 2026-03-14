# 🅿️ Park-IUL

Sistema de gestão de parque de estacionamento desenvolvido no âmbito da
disciplina de Sistemas Operativos (ISCTE-IUL, 2024/2025).

## Descrição

Plataforma automatizada de gestão de lugares de estacionamento, implementada
em três fases com complexidade crescente:

- **Parte 1** – Shell scripts (Bash) para registo de passagens, manutenção
  da base de dados e geração de estatísticas em HTML
- **Parte 2** – Sistema cliente-servidor em C com processos concorrentes,
  sinais UNIX e comunicação via Named Pipes (FIFOs)
- **Parte 3** – Extensão com IPC: memória partilhada, semáforos e filas
  de mensagens, com modelo cliente/servidor dedicado

## Tecnologias

- **C** — processos (`fork`/`wait`), sinais (`sigaction`), ficheiros binários
- **Bash** — scripts de administração, expressões regulares, cron jobs
- **IPC UNIX** — `shmget`/`shmat`, `semget`/`semop`, `msgget`/`msgsnd`/`msgrcv`

## Arquitetura (Parte 3)
```
Cliente  ──MSG──►  Servidor  ──fork──►  Servidor Dedicado
                      │                        │
                    SHM ◄────────────────────►SHM
                   (lugaresEstacionamento)
```

Cada cliente tem um Servidor Dedicado exclusivo. A exclusão mútua é garantida
por semáforos IPC, sem esperas ativas.

## Como compilar
```bash
make
```

## Como executar
```bash
# Terminal 1 — iniciar o servidor com capacidade para 10 lugares
./servidor 10

# Terminal 2 — cliente faz check-in
./cliente
```

## Estrutura do repositório
```
parte-1/    # Shell scripts (Bash)
parte-2/    # C — processos e sinais (FIFOs)
parte-3/    # C — IPC (SHM + SEM + MSG)
```
