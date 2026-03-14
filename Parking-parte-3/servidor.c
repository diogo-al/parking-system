/****************************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2024/2025, Enunciado Versão 1+
 **
 ** Aluno: Nº:129869       Nome:Diogo Goncalves
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ***************************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "defines.h"

/*** Variáveis Globais ***/
int nrServidoresDedicados = 0;          // Número de servidores dedicados (só faz sentido no processo Servidor)
int shmId = -1;                         // Variável que tem o ID da Shared Memory
int msgId = -1;                         // Variável que tem o ID da Message Queue
int semId = -1;                         // Variável que tem o ID do Grupo de Semáforos
MsgContent clientRequest;               // Pedido enviado do Cliente para o Servidor
Estacionamento *lugaresEstacionamento = NULL;   // Array de Lugares de Estacionamento do parque
int dimensaoMaximaParque;               // Dimensão Máxima do parque (BD), recebida por argumento do programa
int indexClienteBD = -1;                // Índice do cliente que fez o pedido ao servidor/servidor dedicado na BD
long posicaoLogfile = -1;               // Posição no ficheiro Logfile para escrever o log da entrada corrente
LogItem logItem;                        // Informação da entrada corrente a escrever no logfile
int shmIdFACE = -1;                     // Variável que tem o ID da Shared Memory da entidade externa FACE
int semIdFACE = -1;                     // Variável que tem o ID do Grupo de Semáforos da entidade externa FACE
int *tarifaAtual = NULL;                // Inteiro definido pela entidade externa FACE com a tarifa atual do parque

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
 * @brief s1_iniciaServidor Ler a descrição da tarefa S1 no enunciado.
 *        OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param argc (I) número de Strings do array argv
 * @param argv (I) array de lugares de estacionamento que irá servir de BD
 */
void s1_IniciaServidor(int argc, char *argv[]) {
    so_debug("<");

    s1_1_ObtemDimensaoParque(argc, argv, &dimensaoMaximaParque);
    s1_2_ArmaSinaisServidor();
    s1_3_CriaMsgQueue(IPC_KEY, &msgId);
    s1_4_CriaGrupoSemaforos(IPC_KEY, &semId);
    s1_5_CriaBD(IPC_KEY, &shmId, dimensaoMaximaParque, &lugaresEstacionamento);

    so_debug(">");
}

/**
 * @brief s1_1_ObtemDimensaoParque Ler a descrição da tarefa S1.1 no enunciado
 * @param argc (I) número de Strings do array argv
 * @param argv (I) array de lugares de estacionamento que irá servir de BD
 * @param pdimensaoMaximaParque (O) número máximo de lugares do parque, especificado pelo utilizador
 */
void s1_1_ObtemDimensaoParque(int argc, char *argv[], int *pdimensaoMaximaParque) {
    so_debug("< [@param argc:%d, argv:%p]", argc, argv);

    if(argc != 2){so_error("S1.1","Argumento invalido!!"); exit(1);}

    char *lug = argv[1]; //pegamos o primeiro argumento que sera a dimensao do parque

    if (lug[0] == '\0') {
        so_error("S1.1", "Argumento invalido!!");
        exit(1);
    }   

    for(int i = 0; lug[i] != '\0'; i ++){
        if(lug[i] > '9' || lug[i] < '0'){so_error("S1.1","Argumento invalido!!"); exit(1);}
    }

    long dimensao = strtol(lug, NULL, 10);
    if(dimensao <= 0){so_error("S1.1","Dimensao Invalida"); exit(1);} //verificamos se a dimensao dada pelo user esta coerente com a realidade

    *pdimensaoMaximaParque = (int)dimensao;

    so_success("S1.1","Sucesso a carregar a dimensao do parque");
    so_debug("> [@return *pdimensaoMaximaParque:%d]", *pdimensaoMaximaParque);
}

/**
 * @brief s1_2_ArmaSinaisServidor Ler a descrição da tarefa S1.2 no enunciado
 */
void s1_2_ArmaSinaisServidor() {
    so_debug("<");

    if(signal(SIGINT, s3_TrataCtrlC)==SIG_ERR){ //tentamos armar o sinal CTRL+C para quando presionado nao terminar o servidor a bruta caso de erro entao damos so_error e saimos
        so_error("S1.2","Nao foi possivel armar o sinal"); exit(1);
    }

    if(signal(SIGCHLD, s5_TrataTerminouServidorDedicado)==SIG_ERR){ //tentamos armar o sinal responsavel por terminar os servidores dedicados caso de erro entao damos so_error e saimos
        so_error("S1.2","Nao foi possivel armar o sinal"); exit(1);
    }

    so_success("S1.2","Sucesso a armar os sinais");

    so_debug(">");
}

/**
 * @brief s1_3_CriaMsgQueue Ler a descrição da tarefa s1.3 no enunciado
 * @param ipcKey (I) Identificador de IPC a ser usada para o projeto
 * @param pmsgId (O) identificador aberto de IPC
 */
void s1_3_CriaMsgQueue(key_t ipcKey, int *pmsgId) {
    so_debug("< [@param ipcKey:0x0%x]", ipcKey);

    int Id=msgget(ipcKey,0); //tentamso aceder a MSG
    if(Id == -1){//se nao der erro o acesso entao ela provavelmente nao existe
        if(errno != ENOENT){so_error("S1.3","Nao foi possivel conectar a MSG "); exit(1);}//se der erro verificamos so se o erro nao e por outro motivo a nao ser o de a MSG nao existir
    }
    else{if(msgctl(Id, IPC_RMID, NULL)==-1){so_error("S1.3","Nao foi possivel apagar a MSG"); exit(1);}}

    Id=msgget(ipcKey, IPC_CREAT | 0600 | IPC_EXCL);
    if(Id == -1){so_error("S1.3","Nao foi possivel criar a MSG | %d | 0x%x", *pmsgId, ipcKey); exit(1);}
    
    *pmsgId=Id;
    so_success("S1.3","Message Queue criada com sucesso");
    so_debug("> [@return *pmsgId:%d]", *pmsgId);
}

/**
 * @brief s1_4_CriaGrupoSemaforos Ler a descrição da tarefa s1.4 no enunciado
 * @param ipcKey (I) Identificador de IPC a ser usada para o projeto
 * @param psemId (O) identificador aberto de IPC
 */
 void s1_4_CriaGrupoSemaforos(key_t ipcKey, int *psemId) {
    so_debug("< [@param ipcKey:0x0%x]", ipcKey);

    *psemId=semget(ipcKey,0,0); //da mesma forma que fizesmos para as MSQ vamos tentar aceder
    if(*psemId == -1){//se der erro o acesso entao ele provavelmente nao existe
        if(errno != ENOENT){so_error("S1.4","Nao foi possivel conectar aos SEM"); exit(1);}//se o erro dado nao foi pelo facto de um semaforo nao existir e sim por outro caso qualquer damos erro
        *psemId=semget(ipcKey,4,0666 | IPC_CREAT | IPC_EXCL); //se tudo correr bem criamos ujm conjunto de quatro semaforos
        if(*psemId == -1){so_error("S1.4","Nao foi possivel criar o grupo SEM"); exit(1);}
    }
    else{
        if(semctl(*psemId, 0, IPC_RMID)==-1){so_error("S1.4","Nao foi possivel apagar o grupo SEM"); exit(1);}
        *psemId=semget(ipcKey,4,0666 | IPC_CREAT | IPC_EXCL);
        if(*psemId == -1){so_error("S1.4","Nao foi possivel criar o grupo SEM "); exit(1);}
    }
    //se chegamos ate aqui entao temos um grupo de 4 semaforos, vamos inciar-los
    int status = semctl(*psemId, SEM_MUTEX_BD , SETVAL, 1);
    if(status == -1){so_error("S1.4","Nao foi possivel inciar valor do semaforo 0"); exit(1);}

    status = semctl(*psemId, SEM_MUTEX_LOGFILE , SETVAL, 1);
    if(status == -1){so_error("S1.4","Nao foi possivel inciar valor do semaforo 1"); exit(1);}

    status = semctl(*psemId, SEM_SRV_DEDICADOS , SETVAL, 0);
    if(status == -1){so_error("S1.4","Nao foi possivel inciar valor do semaforo 2"); exit(1);}

    status = semctl(*psemId, SEM_LUGARES_PARQUE , SETVAL, dimensaoMaximaParque);
    if(status == -1){so_error("S1.4","Nao foi possivel inciar valor do semaforo 3"); exit(1);}

    so_success("S1.4","grupo SEM criado com sucesso");
    so_debug("> [@return *psemId:%d]", *psemId);
}

/**
 * @brief s1_5_CriaBD Ler a descrição da tarefa S1.5 no enunciado
 * @param ipcKey (I) Identificador de IPC a ser usada para o projeto
 * @param pshmId (O) identificador aberto de IPC
 * @param dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 * @param plugaresEstacionamento (O) array de lugares de estacionamento que irá servir de BD
 */
void s1_5_CriaBD(key_t ipcKey, int *pshmId, int dimensaoMaximaParque, Estacionamento **plugaresEstacionamento) {
    so_debug("< [@param ipcKey:0x0%x, dimensaoMaximaParque:%d]", ipcKey, dimensaoMaximaParque);

    *pshmId = shmget(ipcKey,0,0);
    if(*pshmId==-1){ //tentamos ver se a shm existe
        if(errno != ENOENT){so_error("S1.5","Nao foi possivel conectar a SHM"); exit(1);} //se o erro dado nao foi por uma shm nao existir entao damos erro

        *pshmId=shmget(ipcKey,dimensaoMaximaParque*sizeof(Estacionamento), IPC_CREAT | IPC_EXCL | 0666); // alteramos o id da variavel globasl para o id da shm
        if(*pshmId == -1){so_error("S1.5","Nao foi possivel criar SHM"); exit(1);} // perante qualquer erro damos exit e anunciamos o erro

        *plugaresEstacionamento = (Estacionamento*) shmat(*pshmId, NULL, 0); // mudamos o apontador da base de dados para o endereco da shm
        if(*plugaresEstacionamento == (Estacionamento*)-1){so_error("S1.5","Nao foi possivel conectar a SHM"); exit(1);}//verificamos se o attach deu bem

        for(int i =0; i < dimensaoMaximaParque; i ++){(*plugaresEstacionamento)[i].pidCliente = DISPONIVEL;} //colocamos os lugares como disponiveis
    }
    else{
        *plugaresEstacionamento=(Estacionamento*)shmat(*pshmId,NULL,0);
        if(*plugaresEstacionamento == (Estacionamento*)-1){so_error("S1.5","Nao foi possivel conectar a SHM"); exit(1);}
    }

    so_success("S1.5","Sucesso a criar BD");
    so_debug("> [@return *pshmId:%d, *plugaresEstacionamento:%p]", *pshmId, *plugaresEstacionamento);
}

/**
 * @brief s2_MainServidor Ler a descrição da tarefa S2 no enunciado.
 *        OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO
 */
void s2_MainServidor() {
    so_debug("<");

    while (TRUE) {
        s2_1_LePedidoCliente(msgId, &clientRequest);
        s2_2_CriaServidorDedicado(&nrServidoresDedicados);
    }

    so_debug(">");
}

/**
 * @brief s2_1_LePedidoCliente Ler a descrição da tarefa S2.1 no enunciado.
 * @param msgId (I) identificador aberto de IPC
 * @param pclientRequest (O) pedido recebido, enviado por um Cliente
 */
void s2_1_LePedidoCliente(int msgId, MsgContent *pclientRequest) {
    so_debug("< [@param msgId:%d]", msgId);

    while(1){ //ciclozito infinito apra caso haja erro a ler mensagem
        int status = msgrcv(msgId,pclientRequest,sizeof(pclientRequest->msgData), MSGTYPE_LOGIN,0);
        if (status == -1) {
            if (errno == EINTR) {
                continue; //se deu erro pois foi inerrompida por um sinal tetnamos novamente
            } 
            else {
                so_error("S2.1", "Erro a ler a mensagem");
                s4_EncerraServidor();
            }
        }
        else{break;} // se tudo correu bem sai do ciclo e faz o restante
    }

    //sleep(10);  // TEMPORÁRIO, os alunos deverão comentar este statement apenas
                // depois de terem a certeza que não terão uma espera ativa

    so_success("S2.1","%s %d", pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.pidCliente);
    so_debug("> [@return *pclientRequest:[%s:%s:%c:%s:%d.%d]]", pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.viatura.pais, pclientRequest->msgData.est.viatura.categoria, pclientRequest->msgData.est.viatura.nomeCondutor, pclientRequest->msgData.est.pidCliente, pclientRequest->msgData.est.pidServidorDedicado);
}

/**
 * @brief s2_2_CriaServidorDedicado Ler a descrição da tarefa S2.2 no enunciado
 * @param pnrServidoresDedicados (O) número de Servidores Dedicados que foram criados até então
 */
void s2_2_CriaServidorDedicado(int *pnrServidoresDedicados) {
    so_debug("<");

    int pid = fork();//craimos um novo processo
    if(pid == -1){so_error("S2.2","Erro a criar no processo"); s4_EncerraServidor();} // se deu erro a criar o processo filho
    *pnrServidoresDedicados += 1;
    if(pid == 0){ //codigo do filho
        so_success("S2.2","SD: Nasci com PID %d", getpid());
        sd7_MainServidorDedicado();
    }
    else{
        so_success("S2.2","Servidor: Iniciei SD %d", pid);
    }

    so_debug("> [@return *pnrServidoresDedicados:%d", *pnrServidoresDedicados);
}

/**
 * @brief s3_TrataCtrlC Ler a descrição da tarefa S3 no enunciado
 * @param sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void s3_TrataCtrlC(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("S3","Servidor: Start Shutdown");
    s4_EncerraServidor();

    so_debug(">");
}

/**
 * @brief s4_EncerraServidor Ler a descrição da tarefa S4 no enunciado
 *        OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO
 */
void s4_EncerraServidor() {
    so_debug("<");

    s4_1_TerminaServidoresDedicados(lugaresEstacionamento, dimensaoMaximaParque);
    s4_2_AguardaFimServidoresDedicados(nrServidoresDedicados);
    s4_3_ApagaElementosIPCeTermina(shmId, semId, msgId);

    so_debug(">");
}

/**
 * @brief s4_1_TerminaServidoresDedicados Ler a descrição da tarefa S4.1 no enunciado
 * @param lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 */
void s4_1_TerminaServidoresDedicados(Estacionamento *lugaresEstacionamento, int dimensaoMaximaParque) {
    so_debug("< [@param lugaresEstacionamento:%p, dimensaoMaximaParque:%d]", lugaresEstacionamento, dimensaoMaximaParque);

    struct sembuf up={SEM_MUTEX_BD,1,0};
    struct sembuf down={SEM_MUTEX_BD,-1,0};

    if(lugaresEstacionamento != NULL){
        if(semop(semId,&down,1)==-1){so_error("S4.1", "Erro na operacaodo do semaforo"); exit(1);}
        for(int i = 0; i < dimensaoMaximaParque; i ++){ //se a base de dados existir
            //so_debug("i=%d | pidSD: %d | pidCliente: %d", i,lugaresEstacionamento[i].pidServidorDedicado,lugaresEstacionamento[i].pidCliente);
            if(lugaresEstacionamento[i].pidCliente != -1 && lugaresEstacionamento[i].pidServidorDedicado > 0){//vamos percorrer e qunado tivermos um pidCliente valido e um pidSD valido entao mandsmos o sinal;
                if(kill(lugaresEstacionamento[i].pidServidorDedicado, SIGUSR2)==-1){so_error("S4.1", "Erro a enviar sinal"); exit(1);}
                so_debug("sinal enviado para %d", lugaresEstacionamento[i].pidServidorDedicado);
            }
        }
        if(semop(semId,&up,1)==-1){so_error("S4.1", "Erro na operacaodo do semaforo"); exit(1);}
    }

    so_success("S4.1","Sucesso a mandar sinais aos SDs");
    so_debug(">");
}

/**
 * @brief s4_2_AguardaFimServidoresDedicados Ler a descrição da tarefa S4.2 no enunciado
 * @param nrServidoresDedicados (I) número de Servidores Dedicados que foram criados até então
 */
void s4_2_AguardaFimServidoresDedicados(int nrServidoresDedicados) {
    so_debug("< [@param nrServidoresDedicados:%d]", nrServidoresDedicados);

    if(nrServidoresDedicados!=0){
        struct sembuf operation = {SEM_SRV_DEDICADOS, -nrServidoresDedicados, 0};
        so_debug("%d", semctl(semId, SEM_SRV_DEDICADOS, GETVAL, 0));
        int status=semop(semId, &operation,1); //tentamos fazer down da barreira ou sej aso seguimos qundo todos tiverem termiando
        if(status==-1){
            so_error("S4.2","Erro na operacao do semaforo");
        }
    }

    so_success("S4.2","Sucesso a aguardar pelo termino do SD'S");
    so_debug(">");
}

/**
 * @brief s4_3_ApagaElementosIPCeTermina Ler a descrição da tarefa S4.2 no enunciado
 * @param shmId (I) identificador aberto de IPC
 * @param semId (I) identificador aberto de IPC
 * @param msgId (I) identificador aberto de IPC
 */
void s4_3_ApagaElementosIPCeTermina(int shmId, int semId, int msgId) {
    so_debug("< [@param shmId:%d, semId:%d, msgId:%d]", shmId, semId, msgId);
    //aqui vamos remover os IPCS como nao pedem para tratar erros entao apenas os removemos 
    semctl(semId,0, IPC_RMID);
    msgctl(msgId,IPC_RMID,NULL);
    shmctl(shmId,IPC_RMID,NULL);

    so_debug(">");
    so_success("S4.3", "Servidor: End Shutdown");
    exit(0);
}

/**
 * @brief s5_TrataTerminouServidorDedicado Ler a descrição da tarefa S5 no enunciado
 * @param sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void s5_TrataTerminouServidorDedicado(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    int pid = wait(NULL); //esperamos que o sd termione e guardamos o pid

    nrServidoresDedicados--;
    
    so_success ("S5","Servidor: Confirmo que terminou o SD %d", pid);
    so_debug("> [@return nrServidoresDedicados:%d", nrServidoresDedicados);
}

/**
 * @brief sd7_ServidorDedicado Ler a descrição da tarefa SD7 no enunciado
 *        OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void sd7_MainServidorDedicado() {
    so_debug("<");

    // sd7_IniciaServidorDedicado:
    sd7_1_ArmaSinaisServidorDedicado();
    sd7_2_ValidaPidCliente(clientRequest);
    sd7_3_GetShmFACE(KEY_FACE, &shmIdFACE);
    sd7_4_GetSemFACE(KEY_FACE, &semIdFACE);
    sd7_5_ProcuraLugarDisponivelBD(semId, clientRequest, lugaresEstacionamento, dimensaoMaximaParque, &indexClienteBD);

    // sd8_ValidaPedidoCliente:
    sd8_1_ValidaMatricula(clientRequest);
    sd8_2_ValidaPais(clientRequest);
    sd8_3_ValidaCategoria(clientRequest);
    sd8_4_ValidaNomeCondutor(clientRequest);

    // sd9_EntradaCliente:
    sd9_1_AdormeceTempoRandom();
    sd9_2_EnviaSucessoAoCliente(msgId, clientRequest);
    sd9_3_EscreveLogEntradaViatura(FILE_LOGFILE, clientRequest, &posicaoLogfile, &logItem);

    // sd10_AcompanhaCliente:
    sd10_1_AguardaCheckout(msgId);
    sd10_2_EscreveLogSaidaViatura(FILE_LOGFILE, posicaoLogfile, logItem);

    sd11_EncerraServidorDedicado();

    so_error("Servidor Dedicado", "O programa nunca deveria ter chegado a este ponto!");
    so_debug(">");
}

/**
 * @brief sd7_1_ArmaSinaisServidorDedicado Ler a descrição da tarefa SD7.1 no enunciado
 */
void sd7_1_ArmaSinaisServidorDedicado() {
    so_debug("<");
//armamos os sinais como se fez no servidor princiapl p[erante qualque r erro damos erro e exit
    if(signal(SIGINT, SIG_IGN)==SIG_ERR){
        so_error("SD7.1","Erro a armar sinal CTRL + C"); exit(0);
    }

    if(signal(SIGUSR2, sd12_TrataSigusr2)==SIG_ERR){
        so_error("SD7.1","Erro a armar sinal SIGUSR2"); exit(0);
    }

    if(signal(SIGALRM, sd10_1_1_TrataAlarme)==SIG_ERR){
        so_error("SD7.1","Erro a armar sinal SIGALRM"); exit(0);
    }
    so_success("SD7.1","Sucesso a armar os sinais");

    so_debug(">");
}

/**
 * @brief sd7_2_ValidaPidCliente Ler a descrição da tarefa SD7.2 no enunciado
 * @param clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd7_2_ValidaPidCliente(MsgContent clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado);

    if(clientRequest.msgData.est.pidCliente > 0){so_success("SD7.2","PID Valido");} //validamos o pidcliente
    else{so_error("SD7.2","PID Invalido"); exit(0);}

    so_debug(">");
}

/**
 * @brief sd7_3_GetShmFACE Ler a descrição da tarefa SD7.3 no enunciado
 * @param ipcKeyFace (I) Identificador de IPC a ser definida pela FACE
 * @param pshmIdFACE (O) identificador aberto de IPC da FACE
 */
void sd7_3_GetShmFACE(key_t ipcKeyFace, int *pshmIdFACE) {
    so_debug("< [@param ipcKeyFace:0x0%x]", ipcKeyFace);

    *pshmIdFACE = shmget(ipcKeyFace,0,0);//guardamos o id da shm da face
    if(*pshmIdFACE == -1){so_error("SD7.3","erro a ligar a FACE"); exit(0);} 

    tarifaAtual = (int *) shmat(*pshmIdFACE,NULL,0); //tentamos nos ligar a SHM da FAce
    if(tarifaAtual == (int*)-1){so_error("SD7.3","erro a ligar a FACE"); exit(0);}

    so_success("SD7.3","sucesso a ligar a FACE");

    so_debug("> [@return *pshmIdFACE:%d]", *pshmIdFACE);
}

/**
 * @brief sd7_4_GetSemFACE Ler a descrição da tarefa SD7.4 no enunciado
 * @param ipcKeyFace (I) Identificador de IPC a ser definida pela FACE
 * @param psemIdFACE (O) identificador aberto de IPC da FACE
 */
void sd7_4_GetSemFACE(key_t ipcKeyFace, int *psemIdFACE) {
    so_debug("< [@param ipcKeyFace:0x0%x]", ipcKeyFace);

    *psemIdFACE=semget(ipcKeyFace,0,0); //guardamos o id dos sems da face
    if(*psemIdFACE == -1){so_error("SD7.4","erro a ligar ao semaforo FACE"); exit(0);}

    so_success("SD7.4","sucesso a ligar ao semaforo FACE");

    so_debug("> [@return *psemIdFACE:%d]", *psemIdFACE);
}

/**
 * @brief sd7_5_ProcuraLugarDisponivelBD Ler a descrição da tarefa SD7.5 no enunciado
 * @param semId (I) identificador aberto de IPC
 * @param clientRequest (I) pedido recebido, enviado por um Cliente
 * @param lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 * @param pindexClienteBD (O) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void sd7_5_ProcuraLugarDisponivelBD(int semId, MsgContent clientRequest, Estacionamento *lugaresEstacionamento, int dimensaoMaximaParque, int *pindexClienteBD) {
    so_debug("< [@param semId:%d, clientRequest:[%s:%s:%c:%s:%d:%d], lugaresEstacionamento:%p, dimensaoMaximaParque:%d]", semId, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, lugaresEstacionamento, dimensaoMaximaParque);

    struct sembuf mutexDown = {SEM_MUTEX_BD,-1,0};//operacao de down no semaforo
    struct sembuf mutexUp = {SEM_MUTEX_BD,1,0};//operacao de up
    struct sembuf lugaresDown = {SEM_LUGARES_PARQUE,-1,0};//operacao de down no semaforo
    *pindexClienteBD = -1;//cao nao se encontre lugar nao se altera

    //so_debug("BD: %d | lug: %d", semctl(semId,SEM_MUTEX_BD,GETVAL,0),semctl(semId,SEM_LUGARES_PARQUE,GETVAL,0));
    if(semop(semId,&lugaresDown,1)==-1){so_error("SD7.5","erro na operacao 1 do semaforo"); exit(0);} //para controlar se ha lugares ou nao
    so_debug("<lugar disponivel>");
    if(semop(semId,&mutexDown,1)==-1){so_error("SD7.5","erro na operacao 2 do semaforo"); exit(0);}
    for(int i = 0; i < dimensaoMaximaParque; i ++){
        if(lugaresEstacionamento[i].pidCliente==-1){
            lugaresEstacionamento[i]=clientRequest.msgData.est; //se encontrar-se lugar alterarmos a bd
            *pindexClienteBD=i;//novo index
            lugaresEstacionamento[i].pidServidorDedicado=getpid();
            break;
        }
    }
    if(semop(semId,&mutexUp,1)==-1){so_error("SD7.5","erro na operacao 3 do semaforo"); exit(0);} //damos up ao sem
    
    so_success("SD7.5","Reservei Lugar: %d", *pindexClienteBD);
    so_debug("> [*pindexClienteBD:%d]", *pindexClienteBD);
}

/**
 * @brief  sd8_1_ValidaMatricula Ler a descrição da tarefa SD8.1 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_1_ValidaMatricula(MsgContent clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado);

    char* matricula=clientRequest.msgData.est.viatura.matricula; //guardamos a matricula e o tamanho
    int N = strlen(matricula);
    if(N<=0){so_error("SD8.1","matricula invalida"); sd11_EncerraServidorDedicado();} //verificamos o tamanho da string

    for(int i = 0; i < N ; i ++) // vamos verificar se cada caractere segue a regra imposta
        if((matricula[i] < 'A'  || matricula[i] > 'Z') && (matricula[i] < '0'  || matricula[i] > '9')){so_error("SD8.1","matricula invalida"); sd11_EncerraServidorDedicado();}
    so_success("SD8.1","Matricula valida");
    so_debug(">");
}

/**
 * @brief  sd8_2_ValidaPais Ler a descrição da tarefa SD8.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_2_ValidaPais(MsgContent clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado);

    char* pais=clientRequest.msgData.est.viatura.pais;
    int N = strlen(pais);
    if(N != 2){so_error("SD8.1","codigo de pais invalido"); sd11_EncerraServidorDedicado();}

    for(int i = 0; i < N ; i ++)
        if(pais[i] < 'A' || pais[i] > 'Z'){so_error("SD8.2","codigo de pais invalido"); sd11_EncerraServidorDedicado();}

    so_success("SD8.2","codigo de pais valido");
    so_debug(">");
}

/**
 * @brief  sd8_3_ValidaCategoria Ler a descrição da tarefa SD8.3 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_3_ValidaCategoria(MsgContent clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado);

    char categoria= clientRequest.msgData.est.viatura.categoria;
    if(categoria != 'P' && categoria != 'M' && categoria != 'L'){so_error("SD8.3","categoria invalida"); sd11_EncerraServidorDedicado();}

    so_success("SD8.3","categoria valida");
    so_debug(">");
}

/**
 * @brief  sd8_4_ValidaNomeCondutor Ler a descrição da tarefa SD8.4 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_4_ValidaNomeCondutor(MsgContent clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado);

    FILE* f = fopen(FILE_USERS, "r");//abrimos o ficheikro que contem os nomes
    if(f==NULL){so_error("SD8.4","nao foi possivel abrir o ficheiro"); fclose(f); sd11_EncerraServidorDedicado();}
    char *nome= clientRequest.msgData.est.viatura.nomeCondutor;
    nome[strcspn(nome, "\n")] = '\0'; //tira o enter que pode vir agarrado do cliente


    char linha[256];
    int tester=0;
    while(fgets(linha, sizeof(linha), f)){ //le linha a linha
        if(strstr(linha, nome) != NULL){ // se a string for encontrada dentro da linha entao damos sucesso
            tester = 1;
        }
    }
    if(tester){ 
        so_success("SD8.4","Nome encontrado com sucesso");
        fclose(f);
    }
    else{ // se no fichiero nao estava o nome inserido entao damos erro e vamos para sd11
    so_error("SD8.4","nome nao encontrado");fclose(f); sd11_EncerraServidorDedicado();
    }
    so_debug(">");
}

/**
 * @brief sd9_1_AdormeceTempoRandom Ler a descrição da tarefa SD9.1 no enunciado
 */
void sd9_1_AdormeceTempoRandom() {
    so_debug("<");

    if (MAX_ESPERA <= 0) { //verificamos se a variavel está certa
        so_error("SD9.1","Tempo invalido");
        sd11_EncerraServidorDedicado();
    }

    int numero=so_random_between_values(1,MAX_ESPERA-1);
    so_success("SD9.1","%d",numero);
    sleep(numero);

    so_debug(">");
}

/**
 * @brief sd9_2_EnviaSucessoAoCliente Ler a descrição da tarefa SD9.2 no enunciado
 * @param msgId (I) identificador aberto de IPC
 * @param clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd9_2_EnviaSucessoAoCliente(int msgId, MsgContent clientRequest) {
    so_debug("< [@param msgId:%d, clientRequest:[%s:%s:%c:%s:%d:%d]]", msgId, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado);

    clientRequest.msgData.status=CLIENT_ACCEPTED;
    clientRequest.msgType=clientRequest.msgData.est.pidCliente;
    clientRequest.msgData.est.pidServidorDedicado=getpid();

    if(msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData),0)==-1){so_error("SD9.2","erro a enviar mensagem"); sd11_EncerraServidorDedicado();}
    so_success("SD9.2","SD: Confirmei Cliente Lugar %d ", indexClienteBD);
    so_debug(">");
}

/**
 * @brief sd9_3_EscreveLogEntradaViatura Ler a descrição da tarefa SD9.3 no enunciado
 * @param logFilename (I) O nome do ficheiro de Logfile (i.e., FILE_LOGFILE)
 * @param clientRequest (I) pedido recebido, enviado por um Cliente
 * @param pposicaoLogfile (O) posição do ficheiro Logfile mesmo antes de inserir o log desta viatura
 * @param plogItem (O) registo de Log para esta viatura
 */
void sd9_3_EscreveLogEntradaViatura(char *logFilename, MsgContent clientRequest, long *pposicaoLogfile, LogItem *plogItem) {
    so_debug("< [@param logFilename:%s, clientRequest:[%s:%s:%c:%s:%d:%d]]", logFilename, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado);

    FILE* fb = fopen("estacionamentos.txt", "ab");
    if(fb==NULL){so_error("SD9.3","nao foi possivel abrir o ficheiro"); sd11_EncerraServidorDedicado();}
    fseek(fb,0,SEEK_END);
    *pposicaoLogfile=ftell(fb);
    if(*pposicaoLogfile == -1L){so_error("SD9.3","erro a guardar log"); fclose(fb); sd11_EncerraServidorDedicado();}

    plogItem->viatura = clientRequest.msgData.est.viatura;
    
    //aqui vamos buscar o time stamp com o formato pedido;
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    strftime(plogItem->dataEntrada, sizeof(plogItem -> dataEntrada), "%Y-%m-%dT%Hh%M", t); //colocamos o timeStamp da entrada na estrutura

    if(fwrite(plogItem,sizeof(LogItem),1,fb)!=1){so_error("SD9.3","erro a escrever log");fclose(fb); sd11_EncerraServidorDedicado();}

    if(fclose(fb) != 0){
        so_error("SD9.3" , "Erro a fechar o ficheiro");
        sd11_EncerraServidorDedicado();
    } 

    so_success("SD9.3", "SD: Guardei log na posição %ld: Entrada Cliente %s em %s", *pposicaoLogfile, plogItem->viatura.matricula, plogItem->dataEntrada);

    so_debug("> [*pposicaoLogfile:%ld, *plogItem:[%s:%s:%c:%s:%s:%s]]", *pposicaoLogfile, plogItem->viatura.matricula, plogItem->viatura.pais, plogItem->viatura.categoria, plogItem->viatura.nomeCondutor, plogItem->dataEntrada, plogItem->dataSaida);
}

/**
 * @brief  sd10_1_AguardaCheckout Ler a descrição da tarefa SD10.1 no enunciado
 * @param msgId (I) identificador aberto de IPC
 */
void sd10_1_AguardaCheckout(int msgId) {
    so_debug("< [@param msgId:%d]", msgId);

    alarm(60);
    while(1){ //ciclo infinito pra precaver que a mensagem seja interrompida
        if(msgrcv(msgId, &clientRequest, sizeof(clientRequest.msgData), getpid(),0)==-1){
            if (errno == EINTR) {//se o erro deu se por ter sido interompido volta 
                continue;
            } 
            else {
                so_error("SD10.1","erro a receber mensagem"); sd11_EncerraServidorDedicado();
                alarm(0);
            }
        }
        if(clientRequest.msgData.status == TERMINA_ESTACIONAMENTO){alarm(0); break;}
    }

    so_success("SD10.1","SD: A viatura %s deseja sair do parque", clientRequest.msgData.est.viatura.matricula);
    so_debug(">");
}

/**
 * @brief  sd10_1_1_TrataAlarme Ler a descrição da tarefa SD10.1.1 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */

void sd10_1_1_TrataAlarme(int sinalRecebido) { 
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);  
    //configurar as operacoes dos semaforos FACE
    struct sembuf FACEDown = {SEM_MUTEX_FACE,-1,IPC_NOWAIT};//operacao de down no semaforo 
    struct sembuf FACEUp = {SEM_MUTEX_FACE,1,IPC_NOWAIT};//operacao de up

    char hora[20];

    //aqui vamos buscar o time stamp com o formato pedido;
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    strftime(hora, sizeof(hora), "%Y-%m-%dT%Hh%M ", t); //colocamos o timeStamp da entrada na estrutura
    so_debug("%s", hora);
    char tarifamsg[50];
    int tarifa;

    if(semop(semIdFACE,&FACEDown,1)==-1){so_error("SD10.1.1","erro na operacao 1 do semaforo"); sd11_EncerraServidorDedicado();}
    so_debug("Zona critica");
    tarifa = *tarifaAtual; //gaurdamos a tarifa no formato pedido
    if(semop(semIdFACE,&FACEUp,1)==-1){so_error("SD10.1.1","erro na operacao 1 do semaforo"); sd11_EncerraServidorDedicado();}

    snprintf(tarifamsg,sizeof(tarifamsg) ,"Tarifa atual:%d", tarifa);
    //juntamos a string da tarifa com a string da data
    snprintf(clientRequest.msgData.infoTarifa,sizeof(clientRequest.msgData.infoTarifa) ,"%s%s", hora,tarifamsg);
    
    shmdt(tarifaAtual);
    clientRequest.msgData.est.pidServidorDedicado=getpid(); //colocamos o pid do SD para gareantir que o Cliente consiga mandar a mensagem de volta
    clientRequest.msgData.status = INFO_TARIFA; //definimos o status como pedido
    clientRequest.msgType = (float) clientRequest.msgData.est.pidCliente; //definimos o destino da mensagem como o pidCliente
    
    //finalmente enviamos a mensagem
    if(msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData), 0)==-1){so_error("SD10.1.1","erro a enviar mensagem"); sd11_EncerraServidorDedicado();}
    so_success("SD10.1.1","Info Tarifa");
    alarm(60); //voltamos a definir o alarme como pedido
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

    FILE *fb = fopen(logFilename,"r+b"); //vamos abrir o fichoeiro og em binario
    if(fb==NULL){so_error("SD10.2","erro a abrir ficheiro"); sd11_EncerraServidorDedicado();}

    if(fseek(fb,posicaoLogfile,SEEK_SET)==-1){so_error("SD10.2","erro a posicionar no fichiero");fclose(fb); sd11_EncerraServidorDedicado();} //tentamos colocar-nos na posicao deficnida anteriormente

    time_t agora = time(NULL); //vamos buscar o tempo no formatopedido para a saida
    struct tm *t = localtime(&agora);
    strftime(logItem.dataSaida, sizeof(logItem.dataSaida), "%Y-%m-%dT%Hh%M ", t);

    if(fwrite(&logItem, sizeof(LogItem), 1, fb)!=1){so_error("SD10.2","erro a atualizar data");fclose(fb); sd11_EncerraServidorDedicado();} //tentamos escrever no fichieroo a log de saida
    if(fclose(fb) == -1){so_error("SD10.2","erro a fechar logfile"); sd11_EncerraServidorDedicado();} //fechamos o ficheiro

    so_success ("SD10.2","SD: Atualizei log na posição %ld: Saída Cliente %s em %s",posicaoLogfile, logItem.viatura.matricula, logItem.dataSaida );
    so_debug(">");
    sd11_EncerraServidorDedicado();
}

/**
 * @brief  sd11_EncerraServidorDedicado Ler a descrição da tarefa SD11 no enunciado
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void sd11_EncerraServidorDedicado() {
    so_debug("<");

    sd11_1_LibertaLugarViatura(semId, lugaresEstacionamento, indexClienteBD);
    sd11_2_EnviaTerminarAoClienteETermina(msgId, clientRequest);

    so_debug(">");
}

/**
 * @brief sd11_1_LibertaLugarViatura Ler a descrição da tarefa SD11.1 no enunciado
 * @param semId (I) identificador aberto de IPC
 * @param lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void sd11_1_LibertaLugarViatura(int semId, Estacionamento *lugaresEstacionamento, int indexClienteBD) {
    so_debug("< [@param semId:%d, lugaresEstacionamento:%p, indexClienteBD:%d]", semId, lugaresEstacionamento, indexClienteBD);

    if(indexClienteBD < 0){so_error("SD11.1","Erro a libertar lugar"); exit(1);}
    struct sembuf up = {SEM_MUTEX_BD,1,0};
    struct sembuf down = {SEM_MUTEX_BD,-1,0};
    struct sembuf operation = {SEM_LUGARES_PARQUE, 1, 0};

    so_debug("BD: %d | lug: %d", semctl(semId,SEM_MUTEX_BD,GETVAL,0),semctl(semId,SEM_LUGARES_PARQUE,GETVAL,0));
    if(semop(semId,&down,1)==-1){so_error("SD11.1","Erro na operacao 1 do semaforo"); exit(1);} //fazemos down do semaforo para evitar excusao mutua na BD
    lugaresEstacionamento[indexClienteBD].pidCliente=-1; //tornar o lugar disponivel
    if(semop(semId,&up,1)==-1){so_error("SD11.1","Erro na operacao 2 do semaforo"); exit(1);}

    if(semop(semId,&operation,1)==-1){so_error("SD11.1","Erro na operacao 3 do semaforo"); exit(1);} //damos up no semLugares para que possa estacionar mais pessoas 

    so_success("SD11.1","SD: Libertei Lugar: %d", indexClienteBD);

    so_debug(">");
}

/**
 * @brief sd11_2_EnviaTerminarAoClienteETermina Ler a descrição da tarefa SD11.2 no enunciado
 * @param msgId (I) identificador aberto de IPC
 * @param clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd11_2_EnviaTerminarAoClienteETermina(int msgId, MsgContent clientRequest) {
    so_debug("< [@param msgId:%d, clientRequest:[%s:%s:%c:%s:%d:%d]]", msgId, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado);

    clientRequest.msgData.status =  ESTACIONAMENTO_TERMINADO; //definimos o status
    clientRequest.msgType=clientRequest.msgData.est.pidCliente; //definimos o tipo de mensagem

    if(msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData),0)==-1){so_error("SD11.2","Erro a enviar mensagem ao cliente"); exit(1);}

    so_success("SD11.2","SD: Shutdown");
    so_debug(">");
    exit(0);
}

/**
 * @brief  sd12_TrataSigusr2    Ler a descrição da tarefa SD12 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void sd12_TrataSigusr2(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("SD12","SD: Recebi pedido do Servidor para terminar");

    struct sembuf UpSRV = {SEM_SRV_DEDICADOS, 1, 0}; //operacao para libertar o Servidor quadno esse for encerrar
    if(semop(semId,&UpSRV,1)==-1){so_debug("id: %d | num: %d | %s" ,semId, UpSRV.sem_num, strerror(errno)); so_error("SD11.2","Erro na operacao 4 do semaforo" ); exit(1); }
    sd11_EncerraServidorDedicado();

    so_debug(">");
}