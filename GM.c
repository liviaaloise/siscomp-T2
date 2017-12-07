
#include "VM.h"

MemoriaFisica *MF;
TabelaDePaginas *tabela[4];
pid_t pidFilho[4];
int swap=0, pf=0;
int segMF;


void cria_MemoriaFisica(void);
void pageFaultHandler (int signal);
void quitHandler (int signal);
int LFU (void);

int main (void){
    FILE *compilador, *compressor,*matriz,*simulador;
    unsigned addr;
    char acesso;
    int index, offset,i;
    
    cria_MemoriaFisica();
    cria_filaPF();
    
    if ((pidFilho[0] = fork())==0){/*Filho1*/
        tabela[0]=attach_TabelaDePaginas(0);
        
        compilador=fopen("ArqLogs/compilador.log","r");
        
        if (compilador==NULL){
            printf("Erro na abertura de compilador.log\n");
            exit(1);
        }
        
        while(fscanf(compilador,"%x %c",&addr,&acesso)>0){
            index = addr>>16;
            offset = addr - (index<<16);
            
            if (trans(0,index,offset,acesso) == 1){
                MF->frame[tabela[0]->indexFrame[index]].count_R++;
                if(acesso == 'W')
                    MF->frame[tabela[0]->indexFrame[index]].M=1;
            }
        }
        
    }
    else if ((pidFilho[1] = fork())==0){/*Filho2*/
        tabela[1]=attach_TabelaDePaginas(1);
        
        compressor=fopen("ArqLogs/compressor.log","r");
        
        if (compressor==NULL){
            printf("Erro na abertura de compressor.log\n");
            exit(1);
        }
        
        while(fscanf(compressor,"%x %c",&addr,&acesso)>0){
            index=addr>>16;
            offset=addr - (index<<16);
            
            if (trans(1,index,offset,acesso) == 1){
                MF->frame[tabela[1]->indexFrame[index]].count_R++;
                if(acesso == 'W')
                    MF->frame[tabela[1]->indexFrame[index]].M=1;
            }
        }
    }
    else if ((pidFilho[2]=fork())==0){/*Filho3*/
        tabela[2]=attach_TabelaDePaginas(2);
        
        matriz=fopen("ArqLogs/matriz.log","r");
        
        if (matriz==NULL){
            printf("Erro na abertura de matriz.log\n");
            exit(1);
        }
        while(fscanf(matriz,"%x %c",&addr,&acesso)>0){
            index=addr>>16;
            offset=addr - (index<<16);
            
            if (trans(2,index,offset,acesso) == 1){
                MF->frame[tabela[2]->indexFrame[index]].count_R++;
                if(acesso == 'W')
                    MF->frame[tabela[2]->indexFrame[index]].M=1;
            }
        }
    }
    else if ((pidFilho[3]=fork())==0){/*Filho4*/
        tabela[3]=attach_TabelaDePaginas(3);
        
        simulador=fopen("ArqLogs/simulador.log","r");
        
        if (simulador==NULL){
            printf("Erro na abertura de simulador.log\n");
            exit(1);
        }
        
        while(fscanf(simulador,"%x %c",&addr,&acesso)>0){
            index=addr>>16;
            offset=addr - (index<<16);
            
            if (trans(3,index,offset,acesso) == 1){
                MF->frame[tabela[3]->indexFrame[index]].count_R++;
                if(acesso == 'W')
                    MF->frame[tabela[3]->indexFrame[index]].M=1;
            }
        }
        
    }
    else{ //pai
        signal(SIGUSR1, pageFaultHandler);
        signal(SIGQUIT, quitHandler);
        signal(SIGINT, quitHandler);
        
        //cria a memoria compartilhada dos 4 filhos, mas cada filho só tem acesso a sua.
        for(i=0;i<4;i++)
            tabela[i]=cria_TabelaDePaginas(i);
        
    }
    return 0;
}

void cria_MemoriaFisica(void){
    segMF = shmget(IPC_PRIVATE, sizeof(MemoriaFisica),IPC_CREAT | IPC_EXCL | S_IRWXU);
    MF = (MemoriaFisica*)shmat(segMF,0,0);
    MF->indexVazio = 0;
}

void pageFaultHandler(int signal){
    Pagina *PG = primeiro_filaPF();
    int index, pagSubs_proc,pagSubs_indexTP;
    unsigned int slp=1;
    pf++;
    
    kill(pidFilho[PG->proc],SIGSTOP);
    
    if ((index = MF->indexVazio)<FRM){ //se ainda tem espaco vazio no frame é só botar la
        
        //atualizando a tabela do processo
        tabela[PG->proc]->indexFrame[PG->indexTP]=index;
        
        
        MF->frame[index].pag.proc = PG->proc;
        MF->frame[index].pag.indexTP = PG->indexTP;
        MF->frame[index].pag.offset = PG->offset;
        MF->frame[index].pag.acesso = PG->acesso;
        
        if(PG->acesso == 'W')
            MF->frame[index].M=1;
        else
            MF->frame[index].M=0;
        
        MF->frame[index].count_R=1;
        
        MF->indexVazio++;
    }
    else{ // nao tem mais espaço vazio -> LFU
        index = LFU();
        if(MF->frame[index].M == 1){//se o que vai ser substituido foi modificado -> area de swap
            slp++;
            swap++;
        }
        
        //atualizando tabelas
        pagSubs_proc = MF->frame[index].pag.proc;
        pagSubs_indexTP = MF->frame[index].pag.indexTP;
        
        tabela[PG->proc] -> indexFrame[PG->indexTP]=index;
        tabela[pagSubs_proc] -> indexFrame[pagSubs_indexTP] = -1;
        
        
        //substituindo na Memoria Física
        MF->frame[index].pag.proc = PG->proc;
        MF->frame[index].pag.indexTP = PG->indexTP;
        MF->frame[index].pag.offset = PG->offset;
        MF->frame[index].pag.acesso = PG->acesso;
        MF->frame[index].count_R=1;
        
        if(PG->acesso == 'W')
            MF->frame[index].M=1;
        else
            MF->frame[index].M=0;
        
    }
    printf("%d, %04x%04x, %c\n", MF->frame[index].pag.proc, MF->frame[index].pag.indexTP, MF->frame[index].pag.offset, MF->frame[index].pag.acesso);
    
    sleep(slp);
    kill(pidFilho[PG->proc],SIGCONT);
    free(PG);
    
}

int LFU (void){
    int i, index = 0;
    for (i=1;i<FRM;i++)
        if(MF->frame[i].count_R < MF->frame[index].count_R)
            index = i;
    /*
     else if(MF->frame[i].count_R == MF->frame[index].count_R) //se forem iguais
     if((MF->frame[index].M==1) && (MF->frame[i].M==0)) //escolhe o q tem M==0
     index = i;
     */
    
    return index;
}

void quitHandler (int signal){
    destroi_filaPF();
    liberaSHM();
    shmdt(MF);
    shmctl(segMF,IPC_RMID,0);
    
    printf("\n---Quantidade de Swap: %d\n---Quantidade de Page Faults: %d\n",swap,pf);
    exit(0);
}













