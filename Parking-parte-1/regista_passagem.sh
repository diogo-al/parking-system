#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:129869       Nome:Diogo Gonçalves
## Nome do Módulo: S1. Script: regista_passagem.sh
## Descrição/Explicação do Módulo: A explicacao do modulo encontra-se ao longo do mesmo, mas basicamente neste modulo faz se a implementacao da parte chave do trabalho que é a verificação dos argumentos dados pelo user, onde para tal é usado comandos com o awk, cut, grep, sed, etc...
## na segunda parte é feito então a parte do registo de viaturas, para tal usa-se o comando echo para escrever em ficheiros e o sed para colocar a data de saida no registo do veiculo que sai, ao longo do modulo é usado também sempre se tenta modificar ficheiros um if, para caso
## o ficheiro nao tenha perms ou até outro problema da erro.
#####################################################################################

## Este script é invocado quando uma viatura entra/sai do estacionamento Park-IUL. Este script recebe todos os dados por argumento, na chamada da linha de comandos, incluindo os <Matrícula:string>, <Código País:string>, <Categoria:char> e <Nome do Condutor:string>.

## S1.1. Valida os argumentos passados e os seus formatos:
## • Valida se os argumentos passados são em número suficiente (para os dois casos exemplificados), assim como se a formatação de cada argumento corresponde à especificação indicada. O argumento <Categoria> pode ter valores: L (correspondente a Ligeiros), P (correspondente a Pesados) ou M (correspondente a Motociclos);
## • A partir da indicação do argumento <Código País>, valida se o argumento <Matrícula> passada cumpre a especificação da correspondente <Regra Validação Matrícula>;
## • Valida se o argumento <Nome do Condutor> é o “primeiro + último” nomes de um utilizador atual do Tigre;
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.1 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.1.


## S1.2. Valida os dados passados por argumento para o script com o estado da base de dados de estacionamentos especificada no ficheiro estacionamentos.txt:
## • Valida se, no caso de a invocação do script corresponder a uma entrada no parque de estacionamento, se ainda não existe nenhum registo desta viatura na base de dados;
## • Valida se, no caso de a invocação do script corresponder a uma saída do parque de estacionamento, se existe um registo desta viatura na base de dados;
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.2 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.2.


## S1.3. Atualiza a base de dados de estacionamentos especificada no ficheiro estacionamentos.txt:
## • Remova do argumento <Matrícula> passado todos os separadores (todos os caracteres que não sejam letras ou números) eventualmente especificados;
## • Especifique como data registada (de entrada ou de saída, conforme o caso) a data e hora do sistema Tigre;
## • No caso de um registo de entrada, crie um novo registo desta viatura na base de dados;
## • No caso de um registo de saída, atualize o registo desta viatura na base de dados, registando a data de saída;
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.3 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.3.


## S1.4. Lista todos os estacionamentos registados, mas ordenados por saldo:
## • O script deve criar um ficheiro chamado estacionamentos-ordenados-hora.txt igual ao que está no ficheiro estacionamentos.txt, com a mesma formatação, mas com os registos ordenados por ordem crescente da hora (e não da data) de entrada das viaturas.
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.4 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.4.
if (($#==4)); then #se forem 4 arrgumentos corresponde a uma entrada

#Aqui iremos verificar se o ficheiro paises.txt tem permissoes de leitura e se existe
    if ! [ -f paises.txt ]; then
        so_error S1.1 "ficheiro paises.txt não encontrado"
        exit 1
    fi

    if ! [ -r paises.txt ]; then
        so_error S1.1 "ficheiro sem permissões de leitura"
        exit 1
    fi


    paises=$(awk -F"###" '{print $1}' paises.txt) #retiramos todos os codigos de pais possiveis
    categorais=("L P M") #as categorias sao apenas L, P e M por isso basta guardar-las
    nomesTiger=$(cat _etc_passwd | cut -d":" -f5 | sed "s/,,,//" | awk '{print $1, $NF}') #aqui extraimos todos os primeiros e ultimos nomes de usuarios do Tigre
      
    if ! ( echo "$categorais" | grep -q "$3" ); then #aqui verificamos se a categoria inserida pelo utilizador é L, P ou M se não da erro
        so_error S1.1 "categoria inválida"
        exit 1
    fi

    if ! ( echo "$paises" | grep -q "$2" ); then #aqui verificamos se o codigo de pais inserido pelo utilizador é um dos que está presente no paises.txt
        so_error S1.1 "codigo de país inválido"
        exit 1
    fi

    if ! ( echo "$nomesTiger" | grep -q "$4" ); then #aqui verificamos se o nome inserido é um usuario do tigre se não da erro
        so_error S1.1 "O nome inserido não é um usuário do Tigre"
        exit 1
    fi

    FormatoMatriculaPais=$( cat paises.txt | grep "$2" | awk -F"###" '{print $3}' ) #guarda o fomrato de matricula do referente codigo de pais

    if ! ( echo $1 | grep -qE "$FormatoMatriculaPais" ); then #aqui vamos verificar se a matricula cumpre a regra de validação (-E pois estamos a comparar Regex e não Strings)
        so_error S1.1 "Formato de matricula invalido"
        exit 1
    fi

    so_success S1.1 "Argumentos de Entrada Validos"

    matricula=$( echo $1 | sed "s/[ -]//g" ) #deixamos a matricula com o formato da base de dados

    if [ -f estacionamentos.txt ]; then #se o ficheiro da BD existir então temos de verificar se a matricula já está registada

        if ! (( $(cat estacionamentos.txt | awk -F":" 'NF==5' | grep $matricula | wc -l) == 0 )); then #verifica se exsite alguma entrada da matricula mas sem saida ou seja 5 campos, isso dá-se pois uma matricula pode ja ter um registo mas nesse registo ter saido o que entao pode entrar de novo
            so_error S1.2 "Matricula tem registo de entrada"
            exit 1
        fi

    fi

    so_success S1.2 "Matricula sem registo de Entrada"

#se chegamos ate aqui esta tudo impec para ser guardadinhho na base de dados :)

    data=$( date "+%Y-%m-%dT%Hh%M" ) #guardamos a data atual do tigre no formato pedido

    if ! echo "$matricula:$2:$3:$4:$data" >> estacionamentos.txt ; then #como nao existe um registo adicionamos um novo com o formato pedido com no final a data atual do tigre que é a data de entrada; se o if falhar da so_error
        so_error S1.3 "Erro a guardar na base de Dados"
        exit 1
    fi

so_success S1.3 "Sucesso a guardar o registo do veiculo na base de dados"

#aqui iremos passar a data de entrada para a frente ou seja ficamos com (dataEntrada Resto), de seguida tiramos o primeiro h, o das horas, e ficamos com horas do tipo (hhmm)
#o que fica super facil de ordenar pois basta usar o sort usando como separadar "T" por fim com tudo ordenado basta retirar a parte adiocionada usando o cut

    if ! awk -F":" '{print $5, $0}' estacionamentos.txt | sed "s/h//" | sort -t"T" -n -k2 | cut -d" " -f2- >> estacionamentos-ordenados-hora.txt ; then
        so_error S1.4 "Erro a criar o ficheiro "estacionamentos-ordenados-hora.txt""
        exit 1
    fi
    
so_success S1.4 "Sucesso a criar o ficheiro "estacionamentos-ordenados-hora.txt" "

elif (($#==1)); then #se for 1 argumento corresponde a uma saida
#aqui separamos e guardamos individualmente os dois argumentos dados
    A1_pais=$( echo $1 | cut -d"/" -f1 )
    A2_matricula=$( echo $1 | cut -d"/" -f2 | sed "s/[ -]//g" )

    if ! [[ -n $A1_pais && -n $A2_matricula ]]; then
        so_error S1.1 "Argumento com formato invalido"
        exit 1
    fi
#como na entrada vamos verificar se o fichieor paises.txt existe e se tem permissoes de leitura
    if ! [ -f paises.txt ]; then
        so_error S1.1 "ficheiro paises.txt não encontrado"
        exit 1
    fi

    if ! [ -r paises.txt ]; then
        so_error S1.1 "ficheiro sem permissões de leitura"
        exit 1
    fi

#vamos guardar os codigos de paises validos e verificar se o codigo dado como argumento é valido
    paises=$( awk -F"###" '{ print $1 }' paises.txt )
    if ! ( echo "$paises" | grep -q  $A1_pais ); then
        so_error S1.1 "codigo de país inválido"
        exit 1
    fi

#se chegou ate aqui é porque o codigo de pais e valido entao vamos guardar o codigo de matricula referente a esse codigo de pais
    FormatoMatriculaPais=$( cat paises.txt | grep "$A1_pais" | awk -F"###" '{print $3}' ) #guarda o fomrato de matricula do referente codigo de pais

#de forma identica ao que se fez em 1.1 vamos verificar se a matricula dada como argumento é valida
    if ! ( echo "$A2_matricula" | grep -qE "$FormatoMatriculaPais" ); then
        so_error S1.1 "Formato de matricula invalido"
        exit 1
    fi

    so_success S1.1 "Argumentos de saida Validos"

    if ! [ -f estacionamentos.txt ]; then #se o ficheiro da BD não existir então éimpossivelalgum veiculo dar saida
        so_error S1.2 "Ficheiro da base de dados não encontrado"
        exit 1
    fi

    if ! ( cat estacionamentos.txt | grep -q "$A2_matricula" ); then
        so_error S1.2 "Matricula nao encontrada na base de dados"
        exit 1
    fi

    if (( $(cat estacionamentos.txt | awk -F":" 'NF==5' | grep "$A2_matricula" | wc -l) == 0 )); then #verifica se exsite alguma entrada da matricula mas sem saida ou seja 5 campos, se não houver então um veiculo nao pode sair se não entrou
        so_error S1.2 "Matricula não tem registo de entrada"
        exit 1
    fi

    so_success S1.2 "Matricula nao tem registo de saida"

    data=$( date "+%Y-%m-%dT%Hh%M" ) #gauradmos a data com o formato pedido

#aqui iremos usar o awk para passar como output o numero das linhas que contenham a matricula e que tenham 5 campos , ou seja, as linhas cujo o veiculo que tenha a matricula A2_matricula nao tenha ainda saido
#com o numero das linhas usamos um while read apra percorrer por esse output e para cada linha que respeite essas condicoes vai se adicionar a data de saida
    if ! ( awk -F":" -v matricula="$A2_matricula" '{ if (NF==5 && $1==matricula){print NR}}' estacionamentos.txt | \
    while read line; do
        sed -i "${line}s/$/:$data/" estacionamentos.txt #adiciona internamente no final da linha a data de saida na linha numero "line"
    done ); then 
        so_error S1.3 "Erro a alterar o ficheiro"
        exit 1
    fi

so_success S1.3 "Sucesso a alterar o regidto do veiculo"

    if ! awk -F":" '{print $5, $0}' estacionamentos.txt | sed "s/h//" | sort -t"T" -n -k2 | cut -d" " -f2- >> estacionamentos-ordenados-hora.txt ; then
        so_error S1.4 "Erro a criar o ficheiro "estacionamentos-ordenados-hora.txt""
        exit 1
    fi

so_success S1.4 "Sucesso a criar o ficheiro "estacionamentos-ordenados-hora.txt" "

else
    so_error S1.1 "Argumento com formato inválido"
    exit 1
fi