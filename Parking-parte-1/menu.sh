#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:129869       Nome:Diogo Gonçalves
## Nome do Módulo: S4. Script: menu.sh
## Descrição/Explicação do Módulo:A descricao do moculo encontra-se ao longo do mesmo, mas basicamente este é o metodo responsável por comunicar com o user, aqui usamos um "while true" para o menu correr até instrução contrária
## de seguida dependendo da opção (que é validada) iremos correr um script com ou sem argumentos, os que necessitarem de argumentos pedimos ao usuario, os que não precisarem apenas rodamos o script
##
#####################################################################################

## Este script invoca os scripts restantes, não recebendo argumentos.
## Atenção: Não é suposto que volte a fazer nenhuma das funcionalidades dos scripts anteriores. O propósito aqui é simplesmente termos uma forma centralizada de invocar os restantes scripts.
## S4.1. Apresentação:
## S4.1.1. O script apresenta (pode usar echo, cat ou outro, sem “limpar” o ecrã) um menu com as opções abaixo indicadas.


## S4.2. Validações:
## S4.2.1. Aceita como input do utilizador um número. Valida que a opção introduzida corresponde a uma opção válida. Se não for, dá so_error <opção> (com a opção errada escolhida), e volta ao passo S4.1 (ou seja, mostra novamente o menu). Caso contrário, dá so_success <opção>.
## S4.2.2. Analisa a opção escolhida, e mediante cada uma delas, deverá invocar o sub-script correspondente descrito nos pontos S1 a S3 acima. No caso das opções 1 e 4, este script deverá pedir interactivamente ao utilizador as informações necessárias para execução do sub-script correspondente, injetando as mesmas como argumentos desse sub-script:
## S4.2.2.1. Assim sendo, no caso da opção 1, o script deverá pedir ao utilizador sucessivamente e interactivamente os dados a inserir:


## Este script não deverá fazer qualquer validação dos dados inseridos, já que essa validação é feita no script S1. Após receber os dados, este script invoca o Sub-Script: regista_passagem.sh com os argumentos recolhidos do utilizador. Após a execução do sub-script, dá so_success e volta ao passo S4.1.
## S4.2.2.2. No caso da opção 2, o script deverá pedir ao utilizador sucessivamente e interactivamente os dados a inserir:
##  Este script não deverá fazer qualquer validação dos dados inseridos, já que essa validação é feita no script S1. Após receber os dados, este script invoca o Sub-Script: regista_passagem.sh com os argumentos recolhidos do utilizador. Após a execução do sub-script, dá so_success e volta ao passo S4.1.
## S4.2.2.3. No caso da opção 3, o script invoca o Sub-Script: manutencao.sh. Após a execução do sub-script, dá so_success e volta para o passo S4.1.
## S4.2.2.4. No caso da opção 4, o script deverá pedir ao utilizador as opções de estatísticas a pedir, antes de invocar o Sub-Script: stats.sh. Se uma das opções escolhidas for a 8, o menu deverá invocar o Sub-Script: stats.sh sem argumentos, para que possa executar TODAS as estatísticas, caso contrário deve respeitar a ordem.
## Após a execução do Sub-Script: stats.sh, dá so_success e volta para o passo S4.1.


## Apenas a opção 0 (zero) permite sair deste Script: menu.sh. Até escolher esta opção, o menu deverá ficar em ciclo, permitindo realizar múltiplas operações iterativamente (e não recursivamente, ou seja, não deverá chamar o Script: menu.sh novamente). 

while ((0==0)) ; do #comecamos por fazer um "while true" para ficar sempre a correr o script ate instrucao contraria 
    echo -e -n "=====PARK-IUL=====
=====Bem Vindo=====
MENU:
1: Regista passagem – Entrada estacionamento
2: Regista passagem – Saída estacionamento
3: Manutenção
4: Estatísticas
0: Sair
Opção: "
    read escolha #mostramos na consola o menuzito com as varias opcoes e lemos o imput do user e guardamos em escolha

    #verificar se a opcao selecionada é valida

    #verifica se é um numero inteiro:
    if ! echo "$escolha" | grep -qE "^[0-9]+$" ; then
        so_error S4.2.1 "$escolha"
        continue #ao contrario dos scripts anteriores se der erro voltamos ao inicio do while e nao damos exit
    fi

    if (("$escolha" < 0 || "$escolha" > 4)); then #se a escolha nao tiver entre 0 e 4 da erro
        so_error S4.2.1 "$escolha"
        continue
    else
        so_success S4.2.1 "$escolha"
    fi

#daqui para a frente nao tem segredo,  usamos um case e verificamos caso a caso, quando necessario recolhemos imputs e chamamos os respetivos scripts
    case "$escolha" in
        0)
            exit #se for 0 termina
            ;;
        1)
            echo -e -n " 
Regista passagem de Entrada estacionamento:
Indique a matrícula da viatura: "
            read matricula
            echo -n "Indique o código do país de origem da viatura: "
            read codPais
            echo -n "Indique a categoria da viatura [L(igeiro)|P(esado)|M(otociclo)]: "
            read catg
            echo -n "Indique o nome do condutor da viatura: "
            read nome
            ./regista_passagem.sh "$matricula" "$codPais" "$catg" "$nome" #usamos os imputs do utilizador para rodar o script com esses valores
            so_success S4.3
            ;;
        2)
            echo -e -n "
Regista passagem de Saída estacionamento:
Indique a matrícula da viatura: "
            read matricula
            echo -n "Indique o código do país de origem da viatura: "
            read codPais
            ./regista_passagem.sh "$codPais"/"$matricula"
            so_success S4.4
            ;; 
        3)
            ./manutencao.sh 
            so_success S4.5
            
            ;;
        4)
            echo -e -n "
Estatísticas:
1: matrículas e condutores cujas viaturas estão ainda estacionadas no parque
2: top3 das matrículas das viaturas que passaram mais tempo estacionadas
3: tempos de estacionamento de ligeiros e pesados agrupadas por país
4: top3 das matrículas das viaturas que estacionaram mais tarde num dia
5: tempo total de estacionamento por utilizador
6: matrículas e tempo total de estacionamento delas, agrupadas por país da matrícula
7: top3 nomes de condutores mais compridos
8: todas as estatísticas anteriores, na ordem numérica indicada
Indique quais as estatísticas a incluir, opções separadas por espaço: "
        read opcoes
        teste=0 # eu uso esta variavel para caso dê algum erro ou caso algum dos argumentos seja 8 ele ja nao corre o stats e volta ao inicio
        if ! [[ -n "$opcoes" ]] ; then #se não der opcoes entao obviamente nao é valido
            so_error S4.6 "escolha invalida"
            teste=1
        else #se houverem opcoes vamos verificar se alguma é 8
            for i in $opcoes ; do #corremos uma a uma para verificar se é 8
                if (( i == 8 )); then
                    ./stats.sh
                    so_success S4.6
                    teste=1  # se for 8 executa todas e para e usamos o teste para garantir que nao se executa mais nada
                    break  # Sai do loop
                fi
            done
        fi
        if (( teste == 0 )); then #la esta se o teste for 0 entao nao havia 8 ou entao os argumentosnao sao validos, nao havendo 8 e os argumentos sendo validos usamos o script com as opcoes dadas 
            ./stats.sh $opcoes
            so_success S4.6
        fi ;;
    esac
done