/****************************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2024/2025, Enunciado Versão 4+
 **
 ** Aluno: Nº: 129869      Nome: Diogo Goncalves
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:  A explicacao encontra-se em comentarios ao longo do codigo
 **
 **
 ***************************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/*** Variáveis Globais ***/
Estacionamento clientRequest;           // Pedido enviado do Cliente para o Servidor
int recebeuRespostaServidor = FALSE;

/**
 * @brief  Processamento do processo Cliente.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
int main () {
    so_debug("<");

    // c1_IniciaCliente:
    c1_1_ValidaFifoServidor(FILE_REQUESTS);
    c1_2_ArmaSinaisCliente();

    // c2_CheckinCliente:
    c2_1_InputEstacionamento(&clientRequest);
    FILE *fFifoServidor;
    c2_2_AbreFifoServidor(FILE_REQUESTS, &fFifoServidor);
    c2_3_EscrevePedido(fFifoServidor, clientRequest);

    c3_ProgramaAlarme(MAX_ESPERA);

    // c4_AguardaRespostaServidor:
    c4_1_EsperaRespostaServidor();
    c4_2_DesligaAlarme();
    c4_3_InputEsperaCheckout();

    c5_EncerraCliente();

    so_error("Cliente", "O programa nunca deveria ter chegado a este ponto!");
    so_debug(">");
    return 0;
}

/**
 * @brief  c1_1_ValidaFifoServidor Ler a descrição da tarefa C1.1 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void c1_1_ValidaFifoServidor(char *filenameFifoServidor) {
    //nesta funcao iremos validar se o ficheiro existe ou nao
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    if (access(filenameFifoServidor, F_OK) == -1) { //aqyu vamos verificar se ele existe
        so_error("C1.1", "Ficheiro Fifo nao encontrado");
        exit(1);
    }

    //aqui vaomos verificar se o ficheiro segue a estrutura FIFO
    struct stat statsDoFicheiro; 

    if (stat(filenameFifoServidor, &statsDoFicheiro) != 0) {
        so_error("C1.1", "Erro ao obter info do ficheiro");
        exit(1);
    }

    if (!S_ISFIFO(statsDoFicheiro.st_mode)) {
        so_error("C1.1", "O ficheiro nao é um Fifo");
        exit(1);
    }

    so_success("C1.1", "Ficheiro fifo encontrado co sucesso");

    so_debug(">");
}

/**
 * @brief  c1_2_ArmaSinaisCliente Ler a descrição da tarefa C1.3 no enunciado
 */
void c1_2_ArmaSinaisCliente() {
    so_debug("<");
    //iguao ao servidor vamos armasr os sinais, a unica diferenca e que e usado o sigaction para o C6 paratermos o pid do SD
    if(signal(SIGINT, c8_TrataCtrlC) == SIG_ERR){
        so_error("C1.2","Erro a Tratar o sinal SIGINT");
        exit(1);
    }
    if(signal(SIGALRM, c9_TrataAlarme) == SIG_ERR){
        so_error("C1.2","Erro a Tratar o sinal SIGINT");
        exit(1);
    }

    if (signal(SIGHUP, c7_TrataSighup) == SIG_ERR) {
        so_error("C1.2", "Erro a tratar o sinal SIGHUP");
        exit(1);
    }

    struct sigaction sa_SIGUSR1;
    sa_SIGUSR1.sa_sigaction = c6_TrataSigusr1;
    sa_SIGUSR1.sa_flags = SA_SIGINFO;
    sigemptyset(&sa_SIGUSR1.sa_mask);
    if (sigaction(SIGUSR1, &sa_SIGUSR1, NULL) == -1) {
        so_error("C1.2", "Erro a tratar o sinal SIGUSR1");
        exit(1);
    }

    so_success("C1.2", "Sucesso a armar os sinais");
    so_debug(">");
}

/**
 * @brief  c2_1_InputEstacionamento Ler a descrição da tarefa C2.1 no enunciado
 * @param  pclientRequest (O) pedido a ser enviado por este Cliente ao Servidor
 */
void c2_1_InputEstacionamento(Estacionamento *pclientRequest) {
    //aqui iremos pedir ao usuario os impputs das informacoes necessarias para fazer o registo
    so_debug("<");

    printf("Park-IUL: Check-in Viatura\n");
    printf("----------------------------\n");
    printf("Introduza a matrícula da viatura: \n");
    //pedimos a matricula caso haja algum erro da error;
    if(so_gets(pclientRequest->viatura.matricula,sizeof(pclientRequest->viatura.matricula)) == NULL){
        so_error("C2.1", "Erro na informacao dada pelo usuario");
        exit (1);
    }
    //verificamos se e vazio, ou se e contituido apenas por espacos ou quebras de linha
    int vazio=1;
    for(int i = 0; pclientRequest->viatura.matricula[i] != '\0';i++){
        char charc = pclientRequest->viatura.matricula[i];
        if(charc != '\n' && charc != ' '){
            vazio = 0;
            break;
        }
    }

    if(vazio){
        so_error("C2.1","Matricula vazia");
        exit (1);
    }

    //++Daqui para a frente a idea é a mesma mas para a categoria pais e etc..
    printf("Introduza o país da viatura: \n");
    if(so_gets(pclientRequest->viatura.pais,sizeof(pclientRequest->viatura.pais)) == NULL){
        so_error("C2.1", "Erro na informacao dada pelo usuario");
        exit (1);
    }

    vazio=1;
    for(int i = 0; pclientRequest->viatura.pais[i] != '\0';i++){
        char charc = pclientRequest->viatura.pais[i];
        if(charc != '\n' && charc != ' '){
            vazio = 0;
            break;
        }
    }

    if(vazio){
        so_error("C2.1","Codigo de pais vazio");
        exit (1);
    }

    printf("Introduza a categoria da viatura: \n");
    if(scanf(" %c", &pclientRequest->viatura.categoria) == -1){
        so_error("C2.1", "Erro na informacao dada pelo usuario");
        exit (1);
    }

    

    while (getchar() != '\n'); // Limpa o '\n' que ficou no buffer

    printf("Introduza o nome do condutor: \n");
    if(so_gets(pclientRequest->viatura.nomeCondutor,sizeof(pclientRequest->viatura.nomeCondutor)) == NULL){
        so_error("C2.1", "Erro na informacao dada pelo usuario");
        exit (1);
    }

    vazio=1;
    for(int i = 0; pclientRequest->viatura.nomeCondutor[i] != '\0';i++){
        char charc = pclientRequest->viatura.nomeCondutor[i];
        if(charc != '\n' && charc != ' '){
            vazio = 0;
            break;
        }
    }

    if(vazio){
        so_error("C2.1","Nome vazio");
        exit (1);
    }
    //guardamops o pidCliente sendo o pid do processo a correr este programa
    pclientRequest->pidServidorDedicado = -1;
    pclientRequest->pidCliente = getpid();
    
    so_success("C2.1","%s %s %c %s %d %d",pclientRequest->viatura.matricula, pclientRequest->viatura.pais, pclientRequest->viatura.categoria, pclientRequest->viatura.nomeCondutor, pclientRequest->pidCliente, pclientRequest->pidServidorDedicado);

    so_debug("> [*pclientRequest:[%s:%s:%c:%s:%d:%d]]", pclientRequest->viatura.matricula, pclientRequest->viatura.pais, pclientRequest->viatura.categoria, pclientRequest->viatura.nomeCondutor, pclientRequest->pidCliente, pclientRequest->pidServidorDedicado);
}

/**
 * @brief  c2_2_AbreFifoServidor Ler a descrição da tarefa C2.2 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 * @param  pfFifoServidor (O) descritor aberto do ficheiro do FIFO do servidor
 */
void c2_2_AbreFifoServidor(char *filenameFifoServidor, FILE **pfFifoServidor) {
    //aqui vamos abrir o fifo para entao mais a frente coocar a informacao escrita pelo usuario
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    FILE *f = fopen(filenameFifoServidor, "w");
    if(f==NULL) {so_error("C2.2", "Nao foi possivel abrir o ficheiro"); exit (1);}
    //abrimos o ficheiro e se der algum eror da exit
    
    *pfFifoServidor = f; //como nos foi dado um pointer podemoso alterar
    
    so_success("C2.2","FIFO do servidor aberto com sucesso");

    so_debug("> [*pfFifoServidor:%p]", *pfFifoServidor);
}

/**
 * @brief  c2_3_EscrevePedido Ler a descrição da tarefa C2.3 no enunciado
 * @param  fFifoServidor (I) descritor aberto do ficheiro do FIFO do servidor
 * @param  clientRequest (I) pedido a ser enviado por este Cliente ao Servidor
 */
void c2_3_EscrevePedido(FILE *fFifoServidor, Estacionamento clientRequest) {
    //como ja abrirmps o ficheiro agora temos de escrever no mesmo
    so_debug("< [@param fFifoServidor:%p, clientRequest:[%s:%s:%c:%s:%d:%d]]", fFifoServidor, clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    if(fwrite(&clientRequest, sizeof(Estacionamento), 1, fFifoServidor) != 1){ //escrevemos no ficheiro em binario
        so_error("C2.3" , "Erro a escrever no ficheiro");
        exit(1);
    }
    //tentamos fechar o ficheiro, perane qualquer erro saimos e damos erro
    fclose(fFifoServidor);
    so_success("C2.3" , "Sucesso a escrever no ficheiro");

    so_debug(">");
}

/**
 * @brief  c3_ProgramaAlarme Ler a descrição da tarefa C3 no enunciado
 * @param  segundos (I) número de segundos a programar no alarme
 */
void c3_ProgramaAlarme(int segundos) {
    //aqui simplesmente programamos o alarm com o tempo dado em argumentos
    so_debug("< [@param segundos:%d]", segundos);

    alarm(segundos);
    so_success("C3","Espera resposta em %d segundos ", segundos);

    so_debug(">");
}

/**
 * @brief  c4_1_EsperaRespostaServidor Ler a descrição da tarefa C4 no enunciado
 */
void c4_1_EsperaRespostaServidor() {

    //aqui fazemos umciclo infinito usando o pause para nao estar a queimar o cpu ate que tenhamos resposta do servidor
    so_debug("<");

    while (!recebeuRespostaServidor) {
        pause();
        break;
    }
    so_success("C4.1","Check-in realizado com sucesso");

    so_debug(">");
}

/**
 * @brief  c4_2_DesligaAlarme Ler a descrição da tarefa C4.1 no enunciado
 */
void c4_2_DesligaAlarme() {
    //se tudo correr bem temos de desligar o alarme e é aqui que o fazemos
    so_debug("<");

    alarm(0);
    so_success("C4.2","Desliguei alarme");

    so_debug(">");
}

/**
 * @brief  c4_3_InputEsperaCheckout Ler a descrição da tarefa C4.2 no enunciado
 */
void c4_3_InputEsperaCheckout() {
    so_debug("<");
//aqui como pede o enunciado ficamos apenas a espera que o utilizador escreva "sair";
    while(1){
        printf("Escreva \"sair\" para sair do programa\n");
        char input[20];
        so_gets(input, sizeof(input));
        input[strcspn(input, "\n")] = '\0';
        if(strcmp("sair",input) == 0){
            so_success("C4.3","Utilizador pretende terminar estacionamento");
            c5_EncerraCliente();
            so_debug(">");
        }
    }
    c5_EncerraCliente();

    so_debug(">");
}

/**
 * @brief  c5_EncerraCliente      Ler a descrição da tarefa C5 no enunciado
 */
void c5_EncerraCliente() {
    //aqui chamamos apenas as ooutras funcoes
    so_debug("<");

    c5_1_EnviaSigusr1AoServidor(clientRequest);
    c5_2_EsperaRespostaServidorETermina();

    so_debug(">");
}

/**
 * @brief  c5_1_EnviaSigusr1AoServidor      Ler a descrição da tarefa C5.1 no enunciado
 * @param  clientRequest (I) pedido a ser enviado por este Cliente ao Servidor
 */
void c5_1_EnviaSigusr1AoServidor(Estacionamento clientRequest) {
    //enciamos o sinal necessario para fechar o servidor, sinal esse envaido para o SD
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    if(clientRequest.pidServidorDedicado != 0){
        if(kill(clientRequest.pidServidorDedicado,SIGUSR1)==-1){
            so_error("C5.1","Erro a enviar o sinal");
            exit(1);
        }
    }

    so_success("C5.1","Sucesso a enviar sinal");

    so_debug(">");
}

/**
 * @brief  c5_2_EsperaRespostaServidorETermina      Ler a descrição da tarefa C5.2 no enunciado
 */
void c5_2_EsperaRespostaServidorETermina() {
    so_debug("<");
//aqui vamos simplesmente esperar pela resposata do servidor 

    if(clientRequest.pidServidorDedicado == 0) {exit (0);}

    pause();

    so_success("C5.2","Sinal recebido com sucesso");
//fechamos o cliente
    exit(0);

    so_debug(">");
}

/**
 * @brief  c6_TrataSigusr1      Ler a descrição da tarefa C6 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void c6_TrataSigusr1(int sinalRecebido, siginfo_t *siginfo, void *context) {
    so_debug("< [@param sinalRecebido:%d, siginfo:%p]", sinalRecebido, siginfo);
    
    clientRequest.pidServidorDedicado=siginfo->si_pid;
    so_success("C6","Check-in concluído com sucesso pelo Servidor Dedicado %d", clientRequest.pidServidorDedicado);

    recebeuRespostaServidor = TRUE;

    so_debug(">");
}

/**
 * @brief  c7_TrataSighup      Ler a descrição da tarefa C7 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void c7_TrataSighup(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d, siginfo:%p]", sinalRecebido);
    
    so_success("C7","Estacionamento terminado");
    exit(0);

    so_debug(">");
}

/**
 * @brief  c8_TrataCtrlC      Ler a descrição da tarefa c8 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void c8_TrataCtrlC(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C8","Cliente: Shutdown");
    c5_EncerraCliente();

    so_debug(">");
}

/**
 * @brief  c9_TrataAlarme      Ler a descrição da tarefa c9 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void c9_TrataAlarme(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_error("C9","Cliente: Timeout");
    exit(0);

    so_debug(">");
}
