/****************************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2024/2025, Enunciado Versão 4+
 **
 ** Aluno: Nº:  129869     Nome: Diogo Goncalves
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:A explicacao encontra-se em comentarios ao longo do codigo
 **
 **
 ***************************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/*** Variáveis Globais ***/
Estacionamento clientRequest;           // Pedido enviado do Cliente para o Servidor
Estacionamento *lugaresEstacionamento;  // Array de Lugares de Estacionamento do parque
int dimensaoMaximaParque;               // Dimensão Máxima do parque (BD), recebida por argumento do programa
int indexClienteBD;                     // Índice do cliente que fez o pedido ao servidor/servidor dedicado na BD
long posicaoLogfile;                    // Posição no ficheiro Logfile para escrever o log da entrada corrente
LogItem logItem;                        // Informação da entrada corrente a escrever no logfile

/**
 * @brief  Processamento do processo Servidor e dos processos Servidor Dedicado
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param  argc (I) número de Strings do array argv
 * @param  argv (I) array de lugares de estacionamento que irá servir de BD
 * @return Success (0) or not (<> 0)
 */
int main(int argc, char *argv[]) {
    so_debug("<");

    s1_IniciaServidor(argc, argv);
    s2_MainServidor();

    so_error("Servidor", "O programa nunca deveria ter chegado a este ponto!");
    so_debug(">");
    return 0;
}

/**
 * @brief  s1_iniciaServidor Ler a descrição da tarefa S1 no enunciado.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param  argc (I) número de Strings do array argv
 * @param  argv (I) array de lugares de estacionamento que irá servir de BD
 */
void s1_IniciaServidor(int argc, char *argv[]) {
    so_debug("<");

    s1_1_ObtemDimensaoParque(argc, argv, &dimensaoMaximaParque);
    s1_2_CriaBD(dimensaoMaximaParque, &lugaresEstacionamento);
    s1_3_ArmaSinaisServidor();
    s1_4_CriaFifoServidor(FILE_REQUESTS);

    so_debug(">");
}

/**
 * @brief  s1_1_ObtemDimensaoParque Ler a descrição da tarefa S1.1 no enunciado
 * @param  argc (I) número de Strings do array argv
 * @param  argv (I) array de lugares de estacionamento que irá servir de BD
 * @param  pdimensaoMaximaParque (O) número máximo de lugares do parque, especificado pelo utilizador
 */
void s1_1_ObtemDimensaoParque(int argc, char *argv[], int *pdimensaoMaximaParque) {
    //esta funcao ira receber do usuario um argumento e primeiro iremos verificar se o argumento e valido
    so_debug("< [@param argc:%d, argv:%p]", argc, argv);

    if(argc != 2){ //vemos se o foi colocado um argumento
        so_error("S1.1","Argumento invalidos");
        exit(1);
    }

    char *arg = argv[1]; //guardamos o 1 argumento

    int i = 0;
    
    while(arg[i]){

        if(!isdigit(arg[i])){ //vamos o percorrendo para ver se é um numero
            so_error("S1.1","Argumento invalido");
            exit(1); 
        }
        i++;
    }

    
    *pdimensaoMaximaParque = atoi(argv[1]); //se tudo correu bem passamos para digito e damos sucesso
    if(*pdimensaoMaximaParque <= 0){ // verificamos apenas que a dimensao fornecida tem sentido
        so_error("S1.1","Argumento invalido");
        exit(1); 
    }

    so_success("S1.1","Sucesso a obter a dimensao do parque");

    so_debug("> [@param +pdimensaoMaximaParque:%d]", *pdimensaoMaximaParque);
}

/**
 * @brief  s1_2_CriaBD Ler a descrição da tarefa S1.2 no enunciado
 * @param  dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 * @param  plugaresEstacionamento (O) array de lugares de estacionamento que irá servir de BD
 */
void s1_2_CriaBD(int dimensaoMaximaParque, Estacionamento **plugaresEstacionamento) {
    //esta funcao recebe a dimensao maxima do parque e o endereço da base de dados parra assim podermos alterar-la
    so_debug("< [@param dimensaoMaximaParque:%d]", dimensaoMaximaParque);

    //vamos aqui reservar um espaço especifico para a base de dados usando o malloc
    *plugaresEstacionamento = malloc(dimensaoMaximaParque * sizeof(Estacionamento)); //aqui basicamente alocamos memoria com o tamnho certo a partir do endereco lugaresEstacionamento
    //caso plugares seja null entao o malloc nao funcionou como esperado por isso damos erro
    if(*plugaresEstacionamento == NULL){
        so_error("S1.2","Nao foi possivel alocar meoria para a base de dados");
        exit(1); 
    }
    
    for(int i = 0; i < dimensaoMaximaParque; i ++){ //aqui vamos percorrer por todos os lugare e consideramos que um lugar disponivel tem pidCliente =-1
        (*plugaresEstacionamento)[i].pidCliente = -1;
    }

    so_success("S1.2","Sucesso a criar base de dados");

    so_debug("> [*plugaresEstacionamento:%p]", *plugaresEstacionamento);
}

/**
 * @brief  s1_3_ArmaSinaisServidor Ler a descrição da tarefa S1.3 no enunciado
 */
void s1_3_ArmaSinaisServidor() {
    //nesta funcao iremos definir como o servidor ira tratar os varios sinais que podera receber
    so_debug("<");

    //caso ele receba o SIGINT (CTRL + C) ira fazer oq esta na funcao S3
    if( signal(SIGINT, s3_TrataCtrlC) == SIG_ERR){
        so_error("S1.3","Erro a tratar o sinal");
        exit(1); 
    }
    //o reciproco acontece para o SIGCHLD onde este é tratado em S5
    if( signal(SIGCHLD, s5_TrataTerminouServidorDedicado) == SIG_ERR){
        so_error("S1.3","Erro a tratar o sinal");
        exit(1); 
    }

    so_success("S1.3","Sucesso a armar os sinais");

    so_debug(">");
}

/**
 * @brief  s1_4_CriaFifoServidor Ler a descrição da tarefa S1.4 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void s1_4_CriaFifoServidor(char *filenameFifoServidor) {
    //Aqui iremos finalmente criar o ficheiro fifo
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    //para removero ficheiro como diz o enunciado usei a funcao remove que mesmo que o ficheiro nao exista ela nao da um erro fatal logo é plausivel o seu uso
    remove(filenameFifoServidor);

    //entao criamos o ficheiro FIFo do 0, o codigo 0666 qeur dizer que em codigo hexadecimal vamos dar permissoes de leitura eescrita a toidos os usuarios 
    if(mkfifo(filenameFifoServidor, 0666) == -1){
        so_error("S1.4","Nao foi possivel crair ficheiro fifo");
        exit(1); 
    }

    so_success("S1.4","Sucesso a criar o ficheiro Fifo");

    so_debug(">");
}

/**
 * @brief  s2_MainServidor Ler a descrição da tarefa S2 no enunciado.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void s2_MainServidor() {
    so_debug("<");

    FILE *fFifoServidor;
    while (TRUE) {
        s2_1_AbreFifoServidor(FILE_REQUESTS, &fFifoServidor);
        s2_2_LePedidosFifoServidor(fFifoServidor);
        //sleep(10);
    }

    so_debug(">");
}

/**
 * @brief  s2_1_AbreFifoServidor Ler a descrição da tarefa S2.1 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 * @param  pfFifoServidor (O) descritor aberto do ficheiro do FIFO do servidor
 */
void s2_1_AbreFifoServidor(char *filenameFifoServidor, FILE **pfFifoServidor) {
    //como ja criamos o ficheiro fifo vamos entao abrir-lo
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    //como o processo bloqueia se ninguem tiver a ler o fifo, caso se recebe um sinal qualquer ele vai ser interrompido e dar o erro EINTR como os sinais que realmente interessam têm handler
    //é interessante que outros sinais nao interrompam a abertura
    while (1) {
        *pfFifoServidor = fopen(filenameFifoServidor, "r");
        if (*pfFifoServidor != NULL) break;

        if (errno == EINTR) {
            // Foi interrompido por um sinal, tentamos de novo
            continue;
        }

        // Outro erro real, que relamente implica o fechamento do servidor
        so_error("S2.1", "Nao foi possivel abrir ficheiro fifo");
        s4_EncerraServidor(FILE_REQUESTS);
    }

    so_success("S2.1","Sucesso a abrir o ficheiro Fifo");

    so_debug("> [*pfFifoServidor:%p]", *pfFifoServidor);
}

/**
 * @brief  s2_2_LePedidosFifoServidor Ler a descrição da tarefa S2.2 no enunciado.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param  fFifoServidor (I) descritor aberto do ficheiro do FIFO do servidor
 */
void s2_2_LePedidosFifoServidor(FILE *fFifoServidor) {
    so_debug("<");

    int terminaCiclo2 = FALSE;
    while (TRUE) {
        terminaCiclo2 = s2_2_1_LePedido(fFifoServidor, &clientRequest);
        if (terminaCiclo2)
            break;
        s2_2_2_ProcuraLugarDisponivelBD(clientRequest, lugaresEstacionamento, dimensaoMaximaParque, &indexClienteBD);
        s2_2_3_CriaServidorDedicado(lugaresEstacionamento, indexClienteBD);
    }
    

    so_debug(">");
}

/**
 * @brief  s2_2_1_LePedido Ler a descrição da tarefa S2.2.1 no enunciado
 * @param  fFifoServidor (I) descritor aberto do ficheiro do FIFO do servidor
 * @param  pclientRequest (O) pedido recebido, enviado por um Cliente
 * @return TRUE se não conseguiu ler um pedido porque o FIFO não tem mais pedidos.
 */
int s2_2_1_LePedido(FILE *fFifoServidor, Estacionamento *pclientRequest) {
    int naoHaMaisPedidos = FALSE;
    so_debug("< [@param fFifoServidor:%p]", fFifoServidor);

    if (fread(pclientRequest, sizeof(Estacionamento), 1, fFifoServidor) <= 0) { // se conseguiu ler da TRUE
        //se nao leu nada ou e porque chegou ao fim ou entao pois tivemos um erro
        if(feof(fFifoServidor)){ //se ja chegamos ao final do ficheiro
            naoHaMaisPedidos = TRUE;
            so_success("S2.2.1","Não há mais registos no FIFO");
            fclose(fFifoServidor);
        }
        else{
            so_error("S2.2.1","Nao foi possivel ler o conteudo do FIFO");
            s4_EncerraServidor(FILE_REQUESTS);
        }
    }
    so_success("S2.2.1","Li Pedido do FIFO");
    so_debug("> [naoHaMaisPedidos:%d, *pclientRequest:[%s:%s:%c:%s:%d.%d]]", naoHaMaisPedidos, pclientRequest->viatura.matricula, pclientRequest->viatura.pais, pclientRequest->viatura.categoria, pclientRequest->viatura.nomeCondutor, pclientRequest->pidCliente, pclientRequest->pidServidorDedicado);
    return naoHaMaisPedidos;
}

/**
 * @brief  s2_2_2_ProcuraLugarDisponivelBD Ler a descrição da tarefa S2.2.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 * @param  lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param  dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 * @param  pindexClienteBD (O) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void s2_2_2_ProcuraLugarDisponivelBD(Estacionamento clientRequest, Estacionamento *lugaresEstacionamento, int dimensaoMaximaParque, int *pindexClienteBD) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d], lugaresEstacionamento:%p, dimensaoMaximaParque:%d]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado, lugaresEstacionamento, dimensaoMaximaParque);

    *pindexClienteBD = -1;

    for(int i = 0; i < dimensaoMaximaParque; i ++){
        if(lugaresEstacionamento[i].pidCliente == -1){ // se o lugar for disponivel...
            lugaresEstacionamento[i] = clientRequest;
            *pindexClienteBD = i;   
            so_success("S2.2.2","Reservei Lugar: %d", i);
            break;
        }
    }

    so_debug("> [*pindexClienteBD:%d]", *pindexClienteBD);
}

/**
 * @brief  s2_2_3_CriaServidorDedicado    Ler a descrição da tarefa S2.2.3 no enunciado
 * @param  lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param  indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void s2_2_3_CriaServidorDedicado(Estacionamento *lugaresEstacionamento, int indexClienteBD) {
    so_debug("< [@param lugaresEstacionamento:%p, indexClienteBD:%d]", lugaresEstacionamento, indexClienteBD);

    int pid = fork(); //criamos um novo processo

    if(pid < 0){ //se o fork nao foi porreiro
        so_error("S2.2.3","Nao foi possivel criar Servidor Dedicado");
        s4_EncerraServidor(FILE_REQUESTS);
    }

    if(pid == 0){
        so_success("S2.2.3", "SD: Nasci com PID %d ", getpid()); //Processo Filho
        sd7_MainServidorDedicado();
    }
    else {  // Processo pai (Servidor principal)
        if(indexClienteBD != -1){ //se foi encontrado um lugar
            lugaresEstacionamento[indexClienteBD].pidServidorDedicado = pid; //guarda o pid do SD
        }
        so_success("S2.2.3", "Servidor: Iniciei SD %d", pid);
    }

    so_debug(">");
}

/**
 * @brief  s3_TrataCtrlC    Ler a descrição da tarefa S3 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void s3_TrataCtrlC(int sinalRecebido) {
    //esta é a funcao responsavel por tratar do sinal configurado em S1.3 que recebendo um ctrl + c ira mandar o sinal SIGUSR2 para os servidores dedicados e prosseguir para a funcao s4
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("S3","Servidor: Start Shutdown");
    for(int i = 0 ; i < dimensaoMaximaParque; i ++){
        if(lugaresEstacionamento[i].pidCliente != -1){
            if (lugaresEstacionamento[i].pidServidorDedicado > 0) {
                so_debug("< [@param i:%d | pidCliente:%d | pidServidorDedicado: %d]", i, lugaresEstacionamento[i].pidCliente, lugaresEstacionamento[i].pidServidorDedicado);
                if(kill(lugaresEstacionamento[i].pidServidorDedicado, SIGUSR2) == -1){
                    so_error("S3","Erro a mandar sinal");
                    s4_EncerraServidor(FILE_REQUESTS);
                }
            }
        }
    }

    s4_EncerraServidor(FILE_REQUESTS);

    so_debug(">");
}

/**
 * @brief  s4_EncerraServidor Ler a descrição da tarefa S4 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void s4_EncerraServidor(char *filenameFifoServidor) {
    //esta aqui, a famosa funcao 4 ira simplesmemente apagar o FIFO e fechar o processo do servidor principal
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    if(remove(filenameFifoServidor) == -1){
        so_error("S4","Erro a remover o ficheiro FIFO");
        exit(0);
    }
    else{
        so_success("S4","Servidor: End Shutdown");
    }

    exit(0);

    so_debug(">");
}

/**
 * @brief  s5_TrataTerminouServidorDedicado    Ler a descrição da tarefa S5 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
 void s5_TrataTerminouServidorDedicado(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d Pointer:%p]", sinalRecebido, lugaresEstacionamento);
    for(int i=0; i < dimensaoMaximaParque; i++){
        so_debug("ANTES -> Lugar: %d | pidCliente: %d | pidServidorDedicado: %d", i,lugaresEstacionamento[i].pidCliente , lugaresEstacionamento[i].pidServidorDedicado);
    }
    int status;
    int pid = wait(&status);
    so_success("S5", "Servidor: Confirmo que terminou o SD %d", pid);
    for(int i=0; i < dimensaoMaximaParque; i++){
        so_debug("DEPOIS -> Lugar: %d | pidCliente: %d | pidServidorDedicado: %d", i,lugaresEstacionamento[i].pidCliente , lugaresEstacionamento[i].pidServidorDedicado);
        lugaresEstacionamento[i].pidCliente=-1;
        lugaresEstacionamento[i].pidServidorDedicado=-1;
    }
    so_debug(">");
}

/**
 * @brief  sd7_ServidorDedicado Ler a descrição da tarefa SD7 no enunciado
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void sd7_MainServidorDedicado() {
    so_debug("<");

    // sd7_IniciaServidorDedicado:
    sd7_1_ArmaSinaisServidorDedicado();
    sd7_2_ValidaPidCliente(clientRequest);
    sd7_3_ValidaLugarDisponivelBD(indexClienteBD);

    // sd8_ValidaPedidoCliente:
    sd8_1_ValidaMatricula(clientRequest);
    sd8_2_ValidaPais(clientRequest);
    sd8_3_ValidaCategoria(clientRequest);
    sd8_4_ValidaNomeCondutor(clientRequest);

    // sd9_EntradaCliente:
    sd9_1_AdormeceTempoRandom();
    sd9_2_EnviaSigusr1AoCliente(clientRequest);
    sd9_3_EscreveLogEntradaViatura(FILE_LOGFILE, clientRequest, &posicaoLogfile, &logItem);

    // sd10_AcompanhaCliente:
    sd10_1_AguardaCheckout();
    sd10_2_EscreveLogSaidaViatura(FILE_LOGFILE, posicaoLogfile, logItem);

    sd11_EncerraServidorDedicado();

    so_error("Servidor Dedicado", "O programa nunca deveria ter chegado a este ponto!");

    so_debug(">");
}

/**
 * @brief  sd7_1_ArmaSinaisServidorDedicado    Ler a descrição da tarefa SD7.1 no enunciado
 */
void sd7_1_ArmaSinaisServidorDedicado() {
    so_debug("<");

    if (signal(SIGINT, SIG_IGN) == SIG_ERR) { //como pedido no encunciado caso o servido dedicado receba o ctrl +c nao termina
        so_error("SD7.1","Erro a tratar sinais");
        exit (0);
    }

    if(signal(SIGUSR2, sd12_TrataSigusr2) == SIG_ERR){
        so_error("SD7.1","Erro a tratar sinais");
        exit (0);
    }

    if(signal(SIGUSR1, sd13_TrataSigusr1)== SIG_ERR){
        so_error("SD7.1","Erro a tratar sinais");
        exit (0);
    }

    so_success("SD7.1","Sucesso a armar sinais");

    so_debug(">");
}

/**
 * @brief  sd7_2_ValidaPidCliente    Ler a descrição da tarefa SD7.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd7_2_ValidaPidCliente(Estacionamento clientRequest) {
    //esta e a funcnao responsavelpor validar o pid do cliente
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    if(clientRequest.pidCliente <= 0){ //se o pid for zero ou negaivo da erro
        so_error("SD7.2","Pid invalido");
        exit (0);
    }
    so_success("SD7.2","Pid Valido");
    so_debug(">");
}

/**
 * @brief  sd7_3_ValidaLugarDisponivelBD    Ler a descrição da tarefa SD7.3 no enunciado
 * @param  indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void sd7_3_ValidaLugarDisponivelBD(int indexClienteBD) {
    so_debug("< [@param indexClienteBD:%d]", indexClienteBD);

    if(indexClienteBD == -1){ // se o lugar nao tiver disponivel
        so_debug("A");
        if(kill(clientRequest.pidCliente, SIGHUP) == -1){
            so_error("SD7.3","Erro a validar lugar disponivel");
        }
        exit(1); //termina
    }
    so_success("SD7.3","Lugares validados com sucesso");

    so_debug(">");
}

/**
 * @brief  sd8_1_ValidaMatricula Ler a descrição da tarefa SD8.1 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_1_ValidaMatricula(Estacionamento clientRequest) {
    //nesta funcao e  nas proximas iremos validar as informacoes dadas pelo Cliente
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    char matricula[10];
    strncpy(matricula, clientRequest.viatura.matricula, sizeof(matricula) - 1);
    matricula[sizeof(matricula) - 1] = '\0'; // Garante null termination
    int tam = strlen(matricula); //extraimos o tamanho da string 

    if(tam == 0){ //se o tamanho for zero nao faz sentido por isso da erro e vai para sd11
        so_error("SD8.1","Matricula Invalida");
        sd11_EncerraServidorDedicado();
    }

    for(int i = 0; i < tam ; i++){ //se o tamanho nao for zero vamo percorrer a string para ver se esta cumpre os requiisitos
        if (!((matricula[i] >= 'A' && matricula[i] <= 'Z') || (matricula[i] >= '0' && matricula[i] <= '9'))) {
            so_error("SD8.1","Matricula Invalida");
            sd11_EncerraServidorDedicado();
        }
    }

    so_success("SD8.1","Sucesso a validar Matricula");

    so_debug(">");
}

/**
 * @brief  sd8_2_ValidaPais Ler a descrição da tarefa SD8.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_2_ValidaPais(Estacionamento clientRequest) {
    //Nesta funcao iremos validar o codigo de Pais
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    char *pais=clientRequest.viatura.pais;
    int tam = strlen(pais);

    if(tam != 2){ //Duas letras
        so_error("SD8.2","Erro a validar codigo de pais");
        sd11_EncerraServidorDedicado();
    }

    for(int i = 0; i < tam; i ++){ //maiusculas
        if(!(pais[i] >= 'A' && pais[i] <= 'Z')){
            so_error("SD8.2","Erro a validar codigo de pais");
            sd11_EncerraServidorDedicado();
        }
    }

    so_success("SD8.2", "Sucesso a validar o codigo de pais");

    so_debug(">");
}

/**
 * @brief  sd8_3_ValidaCategoria Ler a descrição da tarefa SD8.3 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_3_ValidaCategoria(Estacionamento clientRequest) {
    //aqui iremos validar a categoria
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    char categoria=clientRequest.viatura.categoria;

    if(!(categoria == 'L' || categoria == 'P' || categoria == 'M')){
        so_error("SD8.3","Categoria invalida");
        sd11_EncerraServidorDedicado();
    }

    so_success("SD8.3","Categorio valida");
    so_debug(">");
}

/**
 * @brief  sd8_4_ValidaNomeCondutor Ler a descrição da tarefa SD8.4 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_4_ValidaNomeCondutor(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    char *nome = clientRequest.viatura.nomeCondutor;
    nome[strcspn(nome, "\n")] = '\0';

    if (strlen(nome) == 0){ //se o nnome triver vazio
        so_error("SD8.4","Nome invalido");
        sd11_EncerraServidorDedicado();
    }

    int tester = 0;

    FILE *f = fopen(FILE_USERS, "r"); 
    if (f==NULL) {//se der erro a abrir o ficheiro
        so_error("SD8.4","Erro a abrir o ficheiro");
        sd11_EncerraServidorDedicado();
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), f)) { //le linha a linha
        char *p = strtok(linha, ":"); // usamos o strtok para dividir a linhas em substrings divididas por ":"
        for (int i = 1; i <= 4 && p != NULL; i++) {
            p = strtok(NULL, ":"); // vamos dividindo consecutuvamente ate chegarmos ao nome
            if (i == 4 && p != NULL) {
                char *nomeFich = strtok(p, ","); // dividirmos mais uma vez para tirarmos as virgulas
                if (nomeFich && strcmp(nome, nomeFich) == 0) { // se o nome nao for null e for igual ao dado
                    fclose(f);
                    so_success("SD8.4","Nome valido");
                    tester = 1;
                    break;
                }
            }
        }
    }

    if(tester == 0){
        so_error("SD8.4","Nome Invalido");
        fclose(f);
        sd11_EncerraServidorDedicado();
    }


    so_debug(">");
}

/**
 * @brief  sd9_1_AdormeceTempoRandom Ler a descrição da tarefa SD9.1 no enunciado
 */
void sd9_1_AdormeceTempoRandom() {
    so_debug("<");

    if (MAX_ESPERA <= 0) { //verificamos se a variavel está certa
        so_error("SD9.1","Tempo invalido");
        sd11_EncerraServidorDedicado();
    }

    srand(time(NULL));

    int numeroRand = so_random_between_values(1,5); //escolhemos o numero aleatoro x E [1,MAX[, intervalo aberto pois caso for 5, acaba por dar time out no cliente
    so_success("SD9.1", "%d", numeroRand);
    sleep(numeroRand);

    so_debug(">");
}

/**
 * @brief  sd9_2_EnviaSigusr1AoCliente Ler a descrição da tarefa SD9.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd9_2_EnviaSigusr1AoCliente(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

     
    if(kill(clientRequest.pidCliente, SIGUSR1) == -1){
        so_error("SD9.2","Erro a enviar sinal");
        sd11_EncerraServidorDedicado();
    }
    
    so_success("SD9.2","SD: Confirmei Cliente Lugar %d \n", indexClienteBD);

    so_debug(">");
}

/**
 * @brief  sd9_3_EscreveLogEntradaViatura Ler a descrição da tarefa SD9.3 no enunciado
 * @param  logFilename (I) O nome do ficheiro de Logfile (i.e., FILE_LOGFILE)
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 * @param  pposicaoLogfile (O) posição do ficheiro Logfile mesmo antes de inserir o log desta viatura
 * @param  plogItem (O) registo de Log para esta viatura
 */
void sd9_3_EscreveLogEntradaViatura(char *logFilename, Estacionamento clientRequest, long *pposicaoLogfile, LogItem *plogItem) {
    //Esta funcao e quando vamos escrevr as logs da viatura
    so_debug("< [@param logFilename:%s, clientRequest:[%s:%s:%c:%s:%d:%d]]", logFilename, clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    FILE *fb = fopen(logFilename, "ab"); //aqui abrimos o ficheiro e verificamos se obtiveos sucesso
    if (fb == NULL){
        so_error("SD9.3","Nao foi possivel abrir o ficheiro");
        sd11_EncerraServidorDedicado();
    }

    fseek(fb,0, SEEK_END);//vamos para o fim do ficheiro
    *pposicaoLogfile = ftell(fb); //pegamos na posicao atual

    if(*pposicaoLogfile == -1L){
        so_error("SD9.3","Nao foi possivel abrir o ficheiro");
        sd11_EncerraServidorDedicado();
    }

    plogItem->viatura = clientRequest.viatura;

        //aqui vamos buscar o time stamp com o formato pedido;
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    strftime(plogItem->dataEntrada, sizeof(plogItem -> dataEntrada), "%Y-%m-%dT%Hh%M", t); //colocamos o timeStamp da entrada na estrutura

    if(fwrite(plogItem, sizeof(LogItem), 1, fb)!=1){
        so_error("SD9.3","Nao foi possivel escrever no ficheiro");
        sd11_EncerraServidorDedicado();
    } //escrevemos no ficheiro a LogItem
    if(fclose(fb) != 0){
        so_error("SD9.3" , "Erro a fechar o ficheiro");
        sd11_EncerraServidorDedicado();
    } 

    so_success("SD9.3", "SD: Guardei log na posição %ld: Entrada Cliente %s em %s", *pposicaoLogfile, plogItem->viatura.matricula, plogItem->dataEntrada);

    so_debug("> [*pposicaoLogfile:%ld, *plogItem:[%s:%s:%c:%s:%s:%s]]", *pposicaoLogfile, plogItem->viatura.matricula, plogItem->viatura.pais, plogItem->viatura.categoria, plogItem->viatura.nomeCondutor, plogItem->dataEntrada, plogItem->dataSaida);
}

/**
 * @brief  sd10_1_AguardaCheckout Ler a descrição da tarefa SD10.1 no enunciado
 */
void sd10_1_AguardaCheckout() {
    so_debug("<");

    while(1){
        pause();
        break;
    }
    so_success ("SD10.1","SD: A viatura %s deseja sair do parque", clientRequest.viatura.matricula);

    so_debug(">");
}

/**
 * @brief  sd10_2_EscreveLogSaidaViatura Ler a descrição da tarefa SD10.2 no enunciado
 * @param  logFilename (I) O nome do ficheiro de Logfile (i.e., FILE_LOGFILE)
 * @param  posicaoLogfile (I) posição do ficheiro Logfile mesmo antes de inserir o log desta viatura
 * @param  logItem (I) registo de Log para esta viatura
 */
void sd10_2_EscreveLogSaidaViatura(char *logFilename, long posicaoLogfile, LogItem logItem) {
    so_debug("< [@param logFilename:%s, posicaoLogfile:%ld, logItem:[%s:%s:%c:%s:%s:%s]]", logFilename, posicaoLogfile, logItem.viatura.matricula, logItem.viatura.pais, logItem.viatura.categoria, logItem.viatura.nomeCondutor, logItem.dataEntrada, logItem.dataSaida);

    FILE *fb = fopen(logFilename, "r+b"); //aqui abrimos o ficheiro e verificamos se obtiveos sucesso
    if(fb == NULL){
        so_error("SD10.2", "Erro a abrir o ficheiro");
        sd11_EncerraServidorDedicado();
    }

    if (fseek(fb, posicaoLogfile, SEEK_SET) != 0) { //colocamos o descirtor na posicaoLogFile desde o inicio do ficheiro
        so_error("SD10.2", "Erro ao posicionar no ficheiro de log");
        fclose(fb);
        sd11_EncerraServidorDedicado();
    }

    //aqui vamos buscar o time stamp com o formato pedido;
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    strftime(logItem.dataSaida, sizeof(logItem.dataSaida), "%Y-%m-%dT%Hh%M", t);

    if(fwrite(&logItem, sizeof(LogItem), 1, fb)!=1){
        so_error("SD10.2", "Erro ao posicionar no ficheiro de log");
        fclose(fb);
        sd11_EncerraServidorDedicado();
    } //escrevemos no ficheiro a LogItem
    if(fclose(fb) != 0){
        so_error("SD10.2" , "Erro a fechar o ficheiro");
        sd11_EncerraServidorDedicado();
    } 

    so_success ("SD10.2","SD: Atualizei log na posição %ld: Saída Cliente %s em %s",posicaoLogfile, logItem.viatura.matricula, logItem.dataSaida );
    sd11_EncerraServidorDedicado();
    so_debug(">");
}

/**
 * @brief  sd11_EncerraServidorDedicado Ler a descrição da tarefa SD11 no enunciado
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void sd11_EncerraServidorDedicado() {
    so_debug("<");

    sd11_1_LibertaLugarViatura(lugaresEstacionamento, indexClienteBD);
    sd11_2_EnviaSighupAoClienteETermina(clientRequest);

    so_debug(">");
}

/**
 * @brief  sd11_1_LibertaLugarViatura Ler a descrição da tarefa SD11.1 no enunciado
 * @param  lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param  indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void sd11_1_LibertaLugarViatura(Estacionamento *lugaresEstacionamento, int indexClienteBD) {
    so_debug("< [@param lugaresEstacionamento:%p, indexClienteBD:%d]", lugaresEstacionamento, indexClienteBD);

    if(indexClienteBD < 0 || indexClienteBD >= dimensaoMaximaParque){
        so_error("Sd11.1", "Erro a libertar lugar");
    }

    lugaresEstacionamento[indexClienteBD].pidCliente = -1;

    so_success ("SD11.1","SD: Libertei Lugar: %d ", indexClienteBD);

    so_debug(">");
}

/**
 * @brief  sd11_2_EnviaSighupAoClienteETerminaSD Ler a descrição da tarefa SD11.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd11_2_EnviaSighupAoClienteETermina(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    if(kill(clientRequest.pidCliente, SIGHUP) == - 1){
        so_error("SD11.2","Erro a mandar sinal para cliente");
    }
    else{
        so_success("SD11.2","SD: Shutdown");
    }

    exit(0);

    so_debug(">");
}

/**
 * @brief  sd12_TrataSigusr2    Ler a descrição da tarefa SD12 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void sd12_TrataSigusr2(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success ("SD12","SD: Recebi pedido do Servidor para terminar");
    sd11_EncerraServidorDedicado();

    so_debug(">");
}

/**
 * @brief  sd13_TrataSigusr1    Ler a descrição da tarefa SD13 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void sd13_TrataSigusr1(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success ("SD13","SD: Recebi pedido do Cliente para terminar o estacionamento");

    so_debug(">");
}
