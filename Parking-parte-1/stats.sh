#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:129869       Nome:Diogo Gonçalves
## Nome do Módulo: S3. Script: stats.sh
## Descrição/Explicação do Módulo: A descricao do modulo encontra-se ao longo do mesmo, mas basicamente este é um metodo que coloca num ficheiro html, que posteriormente poderá ser visto online num site, as estatisticas pedidas pelo usuario
## para a implementação deste modulo foram utilizadas divermos comandos, mas majoritariamente foi usado o comando awk devido a este ter uma linguagem que na minha opinião é acessivel, e tambem por ter arrays associativos o que facilita guardar as informações necessarias
## para a questão de ordenar e mostrar o TOP3 foi usado o sort e o head/tail respetivamente
#####################################################################################

## Este script obtém informações sobre o sistema Park-IUL, afixando os resultados das estatísticas pedidas no formato standard HTML no Standard Output e no ficheiro stats.html. Cada invocação deste script apaga e cria de novo o ficheiro stats.html, e poderá resultar em uma ou várias estatísticas a serem produzidas, todas elas deverão ser guardadas no mesmo ficheiro stats.html, pela ordem que foram especificadas pelos argumentos do script.

## S3.1. Validações:
## O script valida se, na diretoria atual, existe algum ficheiro com o nome arquivo-<Ano>-<Mês>.park, gerado pelo Script: manutencao.sh. Se não existirem ou não puderem ser lidos, dá so_error S3.1 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S3.1.


## S3.2. Estatísticas:
## Cada uma das estatísticas seguintes diz respeito à extração de informação dos ficheiros do sistema Park-IUL. Caso não haja informação suficiente para preencher a estatística, poderá apresentar uma lista vazia.
## S3.2.1.  Obter uma lista das matrículas e dos nomes de todos os condutores cujas viaturas estão ainda estacionadas no parque, ordenados alfabeticamente por nome de condutor:
## <h2>Stats1:</h2>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b>Condutor:</b> <Nome do Condutor></li>
## <li><b>Matrícula:</b> <Matrícula> <b>Condutor:</b> <Nome do Condutor></li>
## ...
## </ul>


## S3.2.2. Obter uma lista do top3 das matrículas e do tempo estacionado das viaturas que já terminaram o estacionamento e passaram mais tempo estacionadas, ordenados decrescentemente pelo tempo de estacionamento (considere apenas os estacionamentos cujos tempos já foram calculados):
## <h2>Stats2:</h2>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b>Tempo estacionado:</b> <TempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b>Tempo estacionado:</b> <TempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b>Tempo estacionado:</b> <TempoParkMinutos></li>
## </ul>


## S3.2.3. Obter as somas dos tempos de estacionamento das viaturas que não são motociclos, agrupadas pelo nome do país da matrícula (considere apenas os estacionamentos cujos tempos já foram calculados):
## <h2>Stats3:</h2>
## <ul>
## <li><b>País:</b> <Nome País> <b>Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## <li><b>País:</b> <Nome País> <b>Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## ...
## </ul>


## S3.2.4. Listar a matrícula, código de país e data de entrada dos 3 estacionamentos, já terminados ou não, que registaram uma entrada mais tarde (hora de entrada) no parque de estacionamento, ordenados crescentemente por hora de entrada:
## <h2>Stats4:</h2>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b>País:</b> <Código País> <b>Data Entrada:</b> <DataEntrada></li>
## <li><b>Matrícula:</b> <Matrícula> <b>País:</b> <Código País> <b>Data Entrada:</b> <DataEntrada></li>
## <li><b>Matrícula:</b> <Matrícula> <b>País:</b> <Código País> <b>Data Entrada:</b> <DataEntrada></li>
## </ul>


## S3.2.5. Tendo em consideração que um utilizador poderá ter várias viaturas, determine o tempo total, medido em dias, horas e minutos gasto por cada utilizador da plataforma (ou seja, agrupe os minutos em dias e horas).
## <h2>Stats5:</h2>
## <ul>
## <li><b>Condutor:</b> <NomeCondutor> <b>Tempo  total:</b> <x> dia(s), <y> hora(s) e <z> minuto(s)</li>
## <li><b>Condutor:</b> <NomeCondutor> <b>Tempo  total:</b> <x> dia(s), <y> hora(s) e <z> minuto(s)</li>
## ...
## </ul>


## S3.2.6. Liste as matrículas das viaturas distintas e o tempo total de estacionamento de cada uma, agrupadas pelo nome do país com um totalizador de tempo de estacionamento por grupo, e totalizador de tempo global.
## <h2>Stats6:</h2>
## <ul>
## <li><b>País:</b> <Nome País></li>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## ...
## </ul>
## <li><b>País:</b> <Nome País></li>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## ...
## </ul>
## ...
## </ul>


## S3.2.7. Obter uma lista do top3 dos nomes mais compridos de condutores cujas viaturas já estiveram estacionadas no parque (ou que ainda estão estacionadas no parque), ordenados decrescentemente pelo tamanho do nome do condutor:
## <h2>Stats7:</h2>
## <ul>
## <li><b> Condutor:</b> <Nome do Condutor mais comprido></li>
## <li><b> Condutor:</b> <Nome do Condutor segundo mais comprido></li>
## <li><b> Condutor:</b> <Nome do Condutor terceiro mais comprido></li>
## </ul>


## S3.3. Processamento do script:
## S3.3.1. O script cria uma página em formato HTML, chamada stats.html, onde lista as várias estatísticas pedidas.
## O ficheiro stats.html tem o seguinte formato:
## <html><head><meta charset="UTF-8"><title>Park-IUL: Estatísticas de estacionamento</title></head>
## <body><h1>Lista atualizada em <Data Atual, formato AAAA-MM-DD> <Hora Atual, formato HH:MM:SS></h1>
## [html da estatística pedida]
## [html da estatística pedida]
## ...
## </body></html>
## Sempre que o script for chamado, deverá:
## • Criar o ficheiro stats.html.
## • Preencher, neste ficheiro, o cabeçalho, com as duas linhas HTML descritas acima, substituindo os campos pelos valores de data e hora pelos do sistema.
## • Ciclicamente, preencher cada uma das estatísticas pedidas, pela ordem pedida, com o HTML correspondente ao indicado na secção S3.2.
## • No final de todas as estatísticas preenchidas, terminar o ficheiro com a última linha “</body></html>”

#como pedido e necessario vamos verificar se o ficheiro da base de dados existe e se tem permissoes de leitura
if ! [ -f estacionamentos.txt ]; then
    so_error S3.1 "ficheiro estacionamentos.txt não encontrado"
    exit 1
fi
if ! [ -r estacionamentos.txt ]; then
    so_error S3.1 "nao foi possivel ler o ficheiro estacionamentos.txt"
    exit 1
fi

#como pedido e necessario vamos verificar se o ficheiro paises.txt existe e se tem permissoes de leitura
if ! [ -f paises.txt ]; then
    so_error S3.1 "ficheiro estacionamentos.txt não encontrado"
    exit 1
fi
if ! [ -r paises.txt ]; then
    so_error S3.1 "nao foi possivel ler o ficheiro estacionamentos.txt"
    exit 1
fi

#aqui comecamos por criar a variavel tester que ira guardar o valor logico que verifica se existe pelo menos um arquivo

arquivo="arquivo-[0-9]*-[0-9]*.park"

tester=0
for fich in $arquivo ; do #vamos percorrer todos os arquivos que existem
    if [ -f "$fich" ]; then 
        if ! [ -r "$fich" ]; then #entretanto alem de verificar se os ficheiros existem temos de ver se eles existirem se têm permissoes de leitura
            so_error S3.1 "Arquivo sem permissoes de leitura" # se pelo menos um nao tiver perms entao da erro e termina
            exit 1
        fi
        tester=1 # se existir pelo menos um ficheiro o tester fica a 1
    fi
done

# se ele nao terminar ou seja se chegou ate aquik quer dizer que ou nao existem ficheiros ou existem e é aí que entra o tester

if ((tester==0)); then #se o tester for zero entao nao existem ficheiros arquivo
    so_error S3.1 "Arquivo nao encontrado"
    exit 1
else #se existir e poder ser lido porreiro então podemos fazer as estatisticas, para isso temos de verificar se os argumentos dados sao validos
    for i in "$@" ; do
        if ! echo $i | grep -qE "^[0-9]+$" ; then 
            so_error S3.1 "Argumentos invalidos"
            exit 1
        fi
        if (( $i <= 0 || $i > 7 )); then
            so_error S3.1 "Argumentos invalidos"
            exit 1
        fi
    done
    so_success S3.1 "o arquivo foi encontrado"
fi

if [ -f stats.html ]; then #se o ficheiro existir entao vamos eliminar-lo para depois voltar a criar-lo
    rm stats.html
fi

#===========Funcoezinhas===============


Stats1(){
    #para implementar esta funcao foi utilizado o awk para mostrar no formato pedido as linhas do estacionamento.txt que têm 5 campos, ou seja, dos veiculos que ainda estao estacionados, e o sort para ordenar os nomes
    
    echo "<h2>Stats1:</h2>" >> stats.html
    echo "<ul>" >> stats.html
    cat estacionamentos.txt | sort -t":" -k4 | awk -F":" 'NF == 5 {print "<li><b>Matrícula:</b> "$1" <b>Condutor:</b> "$4"</li>"}' >> stats.html
    echo "</ul>" >> stats.html
    echo  >> stats.html
}

Stats2(){
    #para implementar esta funcao usei o comando awk,dado a este comando permitir arrays associativos torna se facil resolver este problema, pois podemos fazer
    #tempoPorMatricula[AA00BB]+=tempo ou seja vai guardar para cada matricula o tempo da mesma, com tudo feito no END do awk decidi utilizar antes do formato pedido o tempo
    #dessa matricula separando-os com um "#" para facilitar o ordenamento
    echo "<h2>Stats2:</h2>" >> stats.html
    echo "<ul>" >> stats.html
    awk -F":" '{
        tempoPorMatricula[$1] += $7
    }
    END{
        for (i in tempoPorMatricula){
            print ""tempoPorMatricula[i]"#<li><b>Matrícula:</b> "i" <b>Tempo estacionado:</b> "tempoPorMatricula[i]"</li>"
        }
    }' arquivo*.park | sort -t"#" -k1 -n -r | head -3 | cut -d"#" -f2- >> stats.html
    echo "</ul>" >> stats.html
    echo  >> stats.html 
}

Stats3() {
#para variar :) na implementacao desta funcao usei o awk, recebendo este dois ficheiros, o paises.txt e o(s) arquivo(s), fazemos (NR == FNR) que basicamente compara o numero da linha no ficheiro que ele esta a ler com o numero da linha geral
#se for igual estamos entao no primeiro ficheiro, ou seja, no paises.txt entao usando o split, guardamos em array[1] o codigo de pais e em array[2] o repetivo nome e posteeriormente em paises guardamos para cada codigo o seu nome, ou seja,
#paises[PT]=Portugal, depois verificamos se a linha em questao é um motociclo, se nao guardamos em somaTempos o tempo referente a cada codigo de pais, com isso basta fazer print com o formato pedido
    echo "<h2>Stats3:</h2>" >> stats.html
    echo "<ul>" >> stats.html
    awk -F: '{
        if(NR == FNR){
            split($0,array,"###")
            paises[array[1]]=array[2]
            next
        } 
        if ($3 != "M")                   #aqui verificamos se a viatura nao é uma mota
            somaTempos[$2] += $7         #se nao for somamos o tempo desse pais ou seja por exemplo teriamos somaTempos[PT]=tempo das matriculas portuguesas
        } 
    END { 
        for (i in somaTempos) {           # Percorre os países existentes no arquivo
            print "<li><b>País:</b> "paises[i]" <b>Total tempo estacionado:</b> "somaTempos[i]"</li>" 
        }
    }' paises.txt arquivo*.park >> stats.html
    echo "</ul>" >> stats.html
    echo  >> stats.html  
}

Stats4() {
    #mais uma vez usamos o awk para colocar a frente de cada linha a data de entrada, de forma identica ao S1.4 tiramos o h ordenamos as horas e retiramos o adicionado
    #com isso basta dar print no formato pedido

    echo "<h2>Stats4:</h2>" >> stats.html
    echo "<ul>" >> stats.html
    awk -F":" '{print $5, $0}' estacionamentos.txt arquivo*.park | sed "s/h//" | sort -t"T" -n -k2 | cut -d" " -f2- | \
    awk -F":" '{print "<li><b>Matrícula:</b> "$1" <b>País:</b> "$2" <b>Data Entrada:</b> "$5"</li>"}' | tail -3 >> stats.html
    echo "</ul>" >> stats.html
    echo  >> stats.html 
}

Stats5(){
    #mais outra vez para variar o comando awk foi usado para guardar em variaveis inteira os dias, horas e minutos correspondentes a um dado numero de minutos e damos print no formato pedido
    echo "<h2>Stats5:</h2>" >> stats.html
    echo "<ul>" >> stats.html
    awk -F":" '{
        tempoPorNome[$4] += $7 #guardamos neste array o tempo correspondente a cada nome
    }
    END{
        for (i in tempoPorNome){
            tempo=tempoPorNome[i]

            dias=int(tempo/(60*24))
            horas=int((tempo/60)%24)
            minutos=int(tempo%60)
            print "<li><b>Condutor:</b> "i" <b>Tempo total:</b> "dias" dia(s), "horas" hora(s) e "minutos" minuto(s)</li>"
        }
    }' arquivo*.park >> stats.html
    echo "</ul>" >> stats.html
    echo  >> stats.html
}

Stats6(){
#aqui é muito parecido com o Stats3 onde vamos guardar em varios arrays coisas referentes a coisas e depois fazer um print em funcao disso, a explicacao esta em comentarios no awk:
    echo "<h2>Stats6:</h2>" >> stats.html
    echo "<ul>" >> stats.html
    awk -F: '{
            if (NR == FNR){              #igual ao 3) onde vamos guardar em paises para cada codigo o seu nome
                split($0,array,"###")
                paises[array[1]]=array[2]
                next
            }
            paisesExistentes[$2]=paises[$2] #aqui guardamos para cada codigo existente no arquivo o seu nome

            if( index(matriculasPorPais[$2],$1)==0) { #este comando do awk verifica se $1 esta contido na String matriculasPorPais[$2], se não tiver devolve zero 
                matriculasPorPais[$2] = ""matriculasPorPais[$2]" "$1""  #aqui guardamos para cada pais uma string com as suas varias matriculas, tendo em atencao que nao pode haver matriculas repetidas
            }
            
            tempoPorMatricula[$1] += $7 #guardamos para cada matricula o seu tempo de estacionameento total
            tempoPorPais[$2] += $7 #para cada pais guardamos o seu tempo total
        }
        END{
            for (i in paisesExistentes){
                print "<li><b>País:</b> "paisesExistentes[i]" <b>Total tempo estacionado:</b> "tempoPorPais[i]"</li>"
                print "<ul>"
                split(matriculasPorPais[i],matriculas," ") #como tinhamos uma string usamos o split para termos num array todas as matriculas por pais
                for(j in matriculas){
                    print "<li><b>Matrícula:</b> "matriculas[j]" <b> Total tempo estacionado:</b> "tempoPorMatricula[matriculas[j]]"</li>"
                }
                print "</ul>"
            }
        }
        ' paises.txt arquivo*.park >> stats.html
    echo "</ul>" >> stats.html
    echo  >> stats.html
}
Stats7(){
    #para implementar esta funcao usamos a funcao length do awk e colocamos á frente de cada linha o tamanho da String referente ao nome depois é facil basta ordenar e retirar o que foi adicionado
    #e dar print com o formato pedido

    echo "<h2>Stats7:</h2>" >> stats.html
    echo "<ul>" >> stats.html
    cat estacionamentos.txt arquivo*.park | awk -F":" '{print length($4), $0}' | sort -t" " -k1 -n -r| cut -d" " -f2- | head -3 |\
    awk -F":" '{print "<li><b> Condutor:</b> "$4"</li>"}' >> stats.html
    echo "</ul>" >> stats.html
    echo  >> stats.html
}

#=====colocar nos stats=======
data=$(date "+%Y-%m-%d")
hora=$(date "+%H:%M:%S")

echo "<html><head><meta charset=\"UTF-8\"><title>Park-IUL: Estatísticas de estacionamento</title></head>" >> stats.html
echo "<body><h1>Lista atualizada em "$data" "$hora"</h1>" >> stats.html
echo >> stats.html

if (( $# == 0 )); then #se o numero de argumentos for zero entao corremos de forma ordenada todas as stats
    Stats1
    Stats2
    Stats3
    Stats4
    Stats5
    Stats6
    Stats7
else
    for i in "$@" ; do #corremos todos os imputs e usamos um case para selecionar de forma correta a stat pedida
        case "$i" in 
            1)
                Stats1;;
            2)
                Stats2;;
            3)
                Stats3;;
            4)
                Stats4;;
            5)
                Stats5;;
            6)
                Stats6;;
            7)
                Stats7;;
        esac
    done
fi

echo "</body></html>" >> stats.html

so_success S3.3 "Sucesso a criar as estatisticas"