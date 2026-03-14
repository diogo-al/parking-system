#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:129869       Nome:Diogo Gonçalves
## Nome do Módulo: S2. Script: manutencao.sh
## Descrição/Explicação do Módulo: A explicacao do modulo encontra-se ao longo do mesmo, mas resumidamente este modulo serve basicamente para fazer a verificação do conteudo da base de dados, para fazer essa verificação
## foi utilizado um while read, para ler linha a linha da base de dados, para cada linha entao recolhemos as varias informacoes que necessitamos verificar, recorrendo sempre aos comandos grep, cut, awk e etc... e ao ficheiro paises.txt
## para verificar a veracidade das informacoes de cada linha, com tudo verificado movemos as linhas cujos veiculos ja tenham saido do parque e guardamos no arquivo-<AnoSaida>-<MesSaida>.park essa linha mais o tempo de estacionamento total, e removemos do estacionamentos.txt a linha que acabara de ser adicionada
## para calcular o tempo foi usado a funcao "+%s" do date que nso dá o numero de segundos entre 1/1/1970 á 00:00 até uma dada data, o que torna facil fazer esse calculo
#####################################################################################

## Este script não recebe nenhum argumento, e permite realizar a manutenção dos registos de estacionamento. 

## S2.1. Validações do script:
## O script valida se, no ficheiro estacionamentos.txt:
## • Todos os registos referem códigos de países existentes no ficheiro paises.txt;
## • Todas as matrículas registadas correspondem à especificação de formato dos países correspondentes;
## • Todos os registos têm uma data de saída superior à data de entrada;

## • Em caso de qualquer erro das condições anteriores, dá so_error S2.1 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S2.1.
##so_success S2.1


## S2.2. Processamento:
## • O script move, do ficheiro estacionamentos.txt, todos os registos que estejam completos (com registo de entrada e registo de saída), mantendo o formato do ficheiro original, para ficheiros separados com o nome arquivo-<Ano>-<Mês>.park, com todos os registos agrupados pelo ano e mês indicados pelo nome do ficheiro. Ou seja, os registos são removidos do ficheiro estacionamentos.txt e acrescentados ao correspondente ficheiro arquivo-<Ano>-<Mês>.park, sendo que o ano e mês em questão são os do campo <DataSaída>. 
## • Quando acrescentar o registo ao ficheiro arquivo-<Ano>-<Mês>.park, este script acrescenta um campo <TempoParkMinutos> no final do registo, que corresponde ao tempo, em minutos, que durou esse registo de estacionamento (correspondente à diferença em minutos entre os dois campos anteriores).

## • Em caso de qualquer erro das condições anteriores, dá so_error S2.2 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S2.2.
##so_success S2.2

## • O registo em cada ficheiro arquivo-<Ano>-<Mês>.park, tem então o formato:
## <Matrícula:string>:<Código País:string>:<Categoria:char>:<Nome do Condutor:string>: <DataEntrada:AAAA-MM-DDTHHhmm>:<DataSaída:AAAA-MM-DDTHHhmm>:<TempoParkMinutos:int>
## • Exemplo de um ficheiro arquivo-<Ano>-<Mês>.park, para janeiro de 2025:

#aqui verificamos se os ficheiros paises.txt e estacionamentos.txt podem ser lidos e se existem


if ! [ -f estacionamentos.txt ]; then
    so_success S2.1 "ficheiro da base de dados nao existe, por isso nao ha conteudo para verificar"
else  #se o ficheiro estacionamentos.txt existir entao vamos ter de verificar o seu conteudo

#verifica-mos se o ficheiro paises.txt existe
    if ! [ -f paises.txt ]; then 
        so_error S2.1 "ficheiro paises.txt nao encontrado"
        exit 1
    fi

    if ! [ -r paises.txt ]; then
        so_error S2.1 "ficheiro paises.txt sem permissoes de leitura"
        exit 1
    fi

    #aqui iremos utilizar um while read para percorrer pelas linhas do ficheiro estacionamentos.txt, e para cada linha verificar se o seu conteudo é valido
    while read line ; do
    
    #aqui de forma identica á parte S1.* vamos guardar os varios codigos de paises, guardamos para cada linha a matricula e o codigo
        paises=$(awk -F"###" '{print $1}' paises.txt)
        codPais=$(echo "$line" | cut -d":" -f2 )
        matricula=$(echo "$line" | cut -d":" -f1)

        if ! ( echo "$paises" | grep -q "$codPais"  ); then #neste codigo verificamos se o codigo de pais da linha N existe em paises.txt
            so_error S2.1 "Codigo de país inválido"
            exit 1
        fi

    #aqui a idea é a mesma guardamos o fomrato de matricula de um dado codigo e confirmamos se o formato da matricula bate certo com o de paises.txt
        FormatoMatriculaPais=$( cat paises.txt | grep "$codPais" | awk -F"###" '{print $3}' )

        if ! ( echo "$matricula" | grep -qE "$FormatoMatriculaPais" ); then
            so_error S2.1 "Codigo de país inválido"
            exit 1
        fi

        if (( $(echo $line | awk -F":" 'NF==6'| wc -l ) == 1)); then # se a linha tiver 6 campos entao tem uma saida entao vamos verificar se a data de saida é superior á de entrada
        #guardamos as datas de entrada e de saida no formato possivel para se poder fazer um timeStamp
            dataEntrada=$(echo "$line" | cut -d":" -f5 | sed -e "s/T/ /" -e "s/h/:/")
            dataSaida=$(echo "$line" | cut -d":" -f6 | sed -e "s/T/ /" -e "s/h/:/")
        #com as datas no formato correto podemos fazer o timeStamp, ou seja obeter o numero de segundos entre a data X e 1/1/1970 as 00:00
            timeStamp1=$( date -d "$dataEntrada" +%s)
            timeStamp2=$( date -d "$dataSaida" +%s)

        #com os valores dos segundos entre as datas e 1/1/1970 00:00 torna se facil para fazer o calculo dos minutos entre as datas e verificar se uma data é depois ou não da outra
            if ((timeStamp1 >= timeStamp2)); then
                so_error S2.1 "Data de entrada superior à data de saída"
                exit 1
            fi

        fi


    done < estacionamentos.txt
    so_success S2.1 "base de dados válida"
fi

#se chegou ate aqui entao temos que a base de dados tem conteudo valido, entao vamos agora tratar do ficheiro arquivo

if ! [ -f estacionamentos.txt ]; then
    so_success S2.2 "ficheiro da base de dados nao existe, por isso nao ha conteudo para adicionar"
    exit 1
else

    while read line; do

        if (( $(echo $line | awk -F":" 'NF==6'| wc -l ) == 1)); then # se a linha tiver 6 campos entao tem uma saida entao vamos calcular o tempo e colocar no arquivo
        #da mesma forma como foi feito em cima vamos guardar em variaveis para cada linha informacoes
            matricula=$(echo "$line" | cut -d":" -f1)
            dataEntrada=$(echo "$line" | cut -d":" -f5 | sed -e "s/T/ /" -e "s/h/:/")
            dataSaida=$(echo "$line" | cut -d":" -f6 | sed -e "s/T/ /" -e "s/h/:/")

            #com as datas no formato correto podemos fazer o timeStamp, ou seja obeter o numero de segundos entre a data X e 1/1/1970 as 00:00
            timeStamp1=$( date -d "$dataEntrada" +%s)
            timeStamp2=$( date -d "$dataSaida" +%s)

            #aqui guardamos o ano e mes de saida pois vai ser util para o nome do ficheiro que vamos criar
            anoSaida=$( echo "$dataSaida" | cut -d"T" -f1 | cut -d"-" -f1 )
            mesSaida=$(echo "$dataSaida" | cut -d"T" -f1 | cut -d"-" -f2 )

            deltaTempo=$(((timeStamp2-timeStamp1)/60)) #subtraimos os dois timeStamps e tempo o tempo em segundos entre as duas datas entao basta dividir por 60 e temos o tempo em minutos que o automovel teve no parque
            
            #com tudo pronto basta colocar as coisas no ficheiro:
            if ! echo "$line:$deltaTempo" >> arquivo-$anoSaida-$mesSaida.park ; then
                so_error S2.2 "erro a criar o arquivo"
                exit 1
            fi
            #como ja colocamos a linha no arquivo basta utilizar o sed e remover a linha de estacionamentos.txt que contenha no inicio a matricula e pós esta tenha apenas 4 campos separado por ":"
            if ! sed -i "/^$matricula:[^:]*:[^:]*:[^:]*:[^:]*:[^:]*$/d" estacionamentos.txt ; then
                so_error S2.2 "erro a apagar informação de estacionamentos.txt"
                exit 1
            fi

        fi
    done < estacionamentos.txt

    so_success S2.2 "sucesso a criar o arquivo"
fi