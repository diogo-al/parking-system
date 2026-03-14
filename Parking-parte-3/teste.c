void sd10_1_1_TrataAlarme(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    int tar;

    if (sinalRecebido != SIGALRM) {
        so_error("SD10.1.1", "Sinal recebido não é SIGALRM: %d", sinalRecebido);
        return; 
    }

    clientRequest.msgType = clientRequest.msgData.est.pidCliente;
    clientRequest.msgData.status = INFO_TARIFA;

    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timestamp_str[64];
    strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%dT%Hh%M", tm_now);

    
    struct sembuf sb_lock = {SEM_MUTEX_FACE, -1, IPC_NOWAIT};
    if (semop(semIdFACE, &sb_lock, 1) == -1) {
        so_error("SD10.1.1", "Erro ao adquirir semáforo da FACE para ler tarifa: %s", strerror(errno));
        strcpy(clientRequest.msgData.infoTarifa, "Erro ao obter tarifa");
    } else {
        // Aceder à memória partilhada
        if (tarifaAtual == (int*) -1) {
            so_error("SD10.1.1", "Erro ao aceder à memória partilhada da FACE: %s", strerror(errno));
            strcpy(clientRequest.msgData.infoTarifa, "Tarifa Indisponível");
        } else {
            tar = *tarifaAtual;
            snprintf(clientRequest.msgData.infoTarifa, sizeof(clientRequest.msgData.infoTarifa), "%s Tarifa atual:%d", timestamp_str, *tarifaAtual);
            if (shmdt(tarifaAtual) == -1) {
                so_error("SD10.1.1", "Erro ao desligar da memória partilhada da FACE: %s", strerror(errno));
            }
        }
        // Libertar o semáforo
        struct sembuf sb_unlock = {SEM_MUTEX_FACE, 1, IPC_NOWAIT};
        if (semop(semIdFACE, &sb_unlock, 1) == -1) {
            so_error("SD10.1.1", "Erro ao libertar semáforo da FACE após leitura da tarifa: %s", strerror(errno));
        }
    }

    if (msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData), 0) == -1) {
        so_error("SD10.1.1", "Erro ao enviar info tarifa ao cliente: %s", strerror(errno));
        return;
    }
    so_success("SD10.1.1", "%d", tar);

    so_debug(">");
}