/****************************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2024/2025, Enunciado Versão 1+
 **
 ** Aluno: Nº129869    Nome: Diogo Goncalbves
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ***************************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "defines.h"

/*** Variáveis Globais ***/
int msgId = -1;                         // Variável que tem o ID da Message Queue
MsgContent clientRequest;               // Pedido enviado do Cliente para o Servidor
int recebeuRespostaServidor = FALSE;    // Variável que determina se o Cliente já recebeu uma resposta do Servidor

/**
 * @brief Processamento do processo Cliente.
 *        OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
int main () {
    so_debug("<");

    // c1_IniciaCliente:
    c1_1_GetMsgQueue(IPC_KEY, &msgId);
    c1_2_ArmaSinaisCliente();

    // c2_CheckinCliente:
    c2_1_InputEstacionamento(&clientRequest);
    c2_2_EscrevePedido(msgId, clientRequest);

    c3_ProgramaAlarme(MAX_ESPERA);

    // c4_AguardaRespostaServidor:
    c4_1_EsperaRespostaServidor(msgId, &clientRequest);
    c4_2_DesligaAlarme();

    c5_MainCliente(msgId, &clientRequest);

    so_error("Cliente", "O programa nunca deveria ter chegado a este ponto!");
    so_debug(">");
    return 0;
}

/**
 * @brief c1_1_GetMsgQueue Ler a descrição da tarefa C1.1 no enunciado
 * @param ipcKey (I) Identificador de IPC a ser usada para o projeto
 * @param pmsgId (O) identificador aberto de IPC
 */
void c1_1_GetMsgQueue(key_t ipcKey, int *pmsgId) {
    so_debug("< [@param ipcKey:0x0%x]", ipcKey);

    *pmsgId=msgget(ipcKey,0);//tentamos obter a msg queue para comunicar com os servidores
    if(*pmsgId == -1){so_error("C1.1","Erro a ligar a msg"); exit(1);}

    so_success("C1.1","Sucesso a ligar a msg");
    so_debug("> [@return *pmsgId:%d]", *pmsgId);
}

/**
 * @brief c1_2_ArmaSinaisCliente Ler a descrição da tarefa C1.2 no enunciado
 */
void c1_2_ArmaSinaisCliente() {
    so_debug("<");

    if(signal(SIGINT,c6_TrataCtrlC)==SIG_ERR){so_error("C1.2","Erro a tratar sinal SIGINT"); exit(1);}

    if(signal(SIGALRM,c7_TrataAlarme)==SIG_ERR){so_error("C1.2","Erro a tratar sinal SIGALRM"); exit(1);}

    so_success("C1.2","Sucesso a armar sinais");
    so_debug(">");
}

/**
 * @brief c2_1_InputEstacionamento Ler a descrição da tarefa C2.1 no enunciado
 * @param pclientRequest (O) pedido a ser enviado por este Cliente ao Servidor
 */
void c2_1_InputEstacionamento(MsgContent *pclientRequest) {
    so_debug("<");

    //Aqui como na 2 parte do trabalho vamos pedir algumas informacaoes ao usuario e verificar se sao ou nao vazias

    printf("BUENAS TARDES COMPAÑERO\nPark-IUL: Check-in Viatura\n----------------------------\n");

    //=======================MATRICULA=======================

    printf("Introduza a matrícula da viatura: ");
    if(so_gets(pclientRequest->msgData.est.viatura.matricula,sizeof(pclientRequest->msgData.est.viatura.matricula))==NULL){}
    //verificamos se e vazio, ou se e contituido apenas por espacos ou quebras de linha
    int vazio=1;
    for(int i = 0; pclientRequest->msgData.est.viatura.matricula[i] != '\0';i++){
        char charc = pclientRequest->msgData.est.viatura.matricula[i];
        if(charc != '\n' && charc != ' '){vazio = 0; break;}
    }
    if(vazio){ so_error("C2.1","Matricula vazia"); exit (1);}

    //=======================PAIS=======================

    printf("Introduza o país da viatura: ");
    if(so_gets(pclientRequest->msgData.est.viatura.pais,sizeof(pclientRequest->msgData.est.viatura.pais))==NULL){}
    //verificamos se e vazio, ou se e contituido apenas por espacos ou quebras de linha
    vazio=1;
    for(int i = 0; pclientRequest->msgData.est.viatura.pais[i] != '\0';i++){
        char charc = pclientRequest->msgData.est.viatura.pais[i];
        if(charc != '\n' && charc != ' '){vazio = 0; break;}
    }
    if(vazio){ so_error("C2.1","pais vazio"); exit (1);}

    //=======================CATEGORIA=======================

    printf("Introduza a categoria da viatura: ");
    if(scanf("%c", &pclientRequest->msgData.est.viatura.categoria) == -1){
        so_error("C2.1", "Erro na informacao dada pelo usuario");
        exit (1);
    }
    if(pclientRequest->msgData.est.viatura.categoria == ' ' || pclientRequest->msgData.est.viatura.categoria == '\n'){so_error("C2.1","categoria vazia"); exit (1);}
    while (getchar() != '\n');

    //=======================NOME=======================

    printf("Introduza o nome do condutor: ");
    if(so_gets(pclientRequest->msgData.est.viatura.nomeCondutor,sizeof(pclientRequest->msgData.est.viatura.nomeCondutor))==NULL){}
    //verificamos se e vazio, ou se e contituido apenas por espacos ou quebras de linha
    vazio=1;
    for(int i = 0; pclientRequest->msgData.est.viatura.nomeCondutor[i] != '\0';i++){
        char charc = pclientRequest->msgData.est.viatura.nomeCondutor[i];
        if(charc != '\n' && charc != ' '){vazio = 0; break;}
    }
    if(vazio){ so_error("C2.1","nome vazio"); exit (1);}

    pclientRequest->msgData.est.pidCliente = getpid();
    pclientRequest->msgData.est.pidServidorDedicado =-1;

    so_success("C2.1","%s %s %c %s %d %d", pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.viatura.pais, pclientRequest->msgData.est.viatura.categoria, pclientRequest->msgData.est.viatura.nomeCondutor,pclientRequest->msgData.est.pidCliente, pclientRequest->msgData.est.pidServidorDedicado);
    
    so_debug("> [*pclientRequest:[%s:%s:%c:%s:%d:%d]]", pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.viatura.pais, pclientRequest->msgData.est.viatura.categoria, pclientRequest->msgData.est.viatura.nomeCondutor, pclientRequest->msgData.est.pidCliente, pclientRequest->msgData.est.pidServidorDedicado);
}

/**
 * @brief c2_2_EscrevePedido Ler a descrição da tarefa C2.2 no enunciado
 * @param msgId (I) identificador aberto de IPC
 * @param clientRequest (I) pedido a ser enviado por este Cliente ao Servidor
 */
void c2_2_EscrevePedido(int msgId, MsgContent clientRequest) {
    so_debug("< [@param msgId:%d, clientRequest:[%s:%s:%c:%s:%d:%d]]", msgId, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado);

    clientRequest.msgType=MSGTYPE_LOGIN; //mudamos o tipo de mensagem
    if(msgsnd(msgId,&clientRequest, sizeof(clientRequest.msgData),0)==-1){so_error("C2.2","erro a enviar mensagem"); exit (1);}//emviamos a mensagem

    so_success("C2.2","Sucesso a enviar mensagem");
    so_debug(">");
}

/**
 * @brief c3_ProgramaAlarme Ler a descrição da tarefa C3 no enunciado
 * @param segundos (I) número de segundos a programar no alarme
 */
void c3_ProgramaAlarme(int segundos) {
    so_debug("< [@param segundos:%d]", segundos);

    alarm(segundos);//definimos o alrm
    so_success("C3","Espera resposta em %d segundos ", segundos);
    so_debug(">");
}

/**
 * @brief c4_1_EsperaRespostaServidor Ler a descrição da tarefa C4.1 no enunciado
 * @param msgId (I) identificador aberto de IPC
 * @param pclientRequest (O) mensagem enviada por um Servidor Dedicado
 */
void c4_1_EsperaRespostaServidor(int msgId, MsgContent *pclientRequest) {
    so_debug("< [@param msgId:%d]", msgId);

    while(1){
        if(msgrcv(msgId,pclientRequest,sizeof(pclientRequest->msgData),getpid(),0)==-1){
            if(errno == EINTR){
                continue; 
            }
            so_error("C4.1","erro a receber mensagem"); 
            exit (1);
        }
        break;
        
    }

    int status = pclientRequest->msgData.status;//definimos o status da mensagem recebida 
    if(status == CLIENT_ACCEPTED){so_success("C4.1","Check-in realizado com sucesso");} // se a mensagem recebida foi de aceitacao da a mensagem pedida
    else if(status == ESTACIONAMENTO_TERMINADO){so_success("C4.1","Não é possível estacionar"); exit(0);}

    so_debug("> [*pclientRequest:[%s:%s:%c:%s:%d:%d]]", pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.viatura.pais, pclientRequest->msgData.est.viatura.categoria, pclientRequest->msgData.est.viatura.nomeCondutor, pclientRequest->msgData.est.pidCliente, pclientRequest->msgData.est.pidServidorDedicado);
}

/**
 * @brief c4_2_DesligaAlarme Ler a descrição da tarefa C4.2 no enunciado
 */
void c4_2_DesligaAlarme() {
    so_debug("<");

    alarm(0); //desligamos o alarm
    so_success("C4.2","Desliguei alarme");
    so_debug(">");
}

/**
 * @brief c5_MainCliente Ler a descrição da tarefa C5 no enunciado
 * @param msgId (I) identificador aberto de IPC
 * @param pclientRequest (O) mensagem enviada por um Servidor Dedicado
*/
void c5_MainCliente(int msgId, MsgContent *pclientRequest) {
    so_debug("< [@param msgId:%d]", msgId);

    while(1){ //ciclo infinito
        if(msgrcv(msgId, pclientRequest, sizeof(pclientRequest->msgData),getpid(), 0)==-1){
            if(errno == EINTR) {continue; }
            so_error("C5","erro a receber mensagem"); exit (1);} //ficamos a espera ate que se receba a mesnagem
        int status = pclientRequest->msgData.status;

        if(status == INFO_TARIFA){so_success("C5","%s",pclientRequest->msgData.infoTarifa);} // se receber info da tarifa entao tudo bem da sucesso e volta a esperar por mensagem
        if(status == ESTACIONAMENTO_TERMINADO){so_success("C5","Estacionamento terminado administrativamente");exit(0);}//se receber a de terminar termina
    }

    so_debug("> [*pclientRequest:[%s:%s:%c:%s:%d:%d]]", pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.viatura.pais, pclientRequest->msgData.est.viatura.categoria, pclientRequest->msgData.est.viatura.nomeCondutor, pclientRequest->msgData.est.pidCliente, pclientRequest->msgData.est.pidServidorDedicado);
}

/**
 * @brief  c6_TrataCtrlC Ler a descrição da tarefa C6 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void c6_TrataCtrlC(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d, msgId:%d, clientRequest:[%s:%s:%c:%s:%d:%d]]", sinalRecebido, msgId, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado);

    clientRequest.msgType=clientRequest.msgData.est.pidServidorDedicado;
    clientRequest.msgData.status=TERMINA_ESTACIONAMENTO;
    if(msgsnd(msgId,&clientRequest,sizeof(clientRequest.msgData),0)==-1){so_error("C6","erro a enviar mensagem"); exit (1);} // envia mensagem ai SD a dizer que quer terminar
    so_success("C6","Cliente: Shutdown");

    so_debug(">");
}

/**
 * @brief  c7_TrataAlarme Ler a descrição da tarefa C7 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void c7_TrataAlarme(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_error("C7","Cliente: Timeout"); // se o alarm acabou o tempo avisa e termina
    exit(0);

    so_debug(">");
}