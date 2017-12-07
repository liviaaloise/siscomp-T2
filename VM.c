#include "VM.h" 

TabelaDePaginas *tabela[4];
Fila *filaPF;
int seg[4];

void cria_filaPF(){
	filaPF = fila_cria();
}

void destroi_filaPF(){
	fila_libera(filaPF);
}

Pagina* primeiro_filaPF(){
	return fila_pop(filaPF);
}

TabelaDePaginas* cria_TabelaDePaginas(int processo){
	int i;

	seg[processo] = shmget(IPC_PRIVATE, sizeof(TabelaDePaginas),IPC_CREAT | IPC_EXCL | S_IRWXU);
	tabela[processo]=(TabelaDePaginas*)shmat(seg[processo],0,0);
	tabela[processo]->proc=processo;
	for(i=0;i<PGS;i++)
		tabela[processo]->indexFrame[i]=-1;
	return tabela[processo];
}

TabelaDePaginas* attach_TabelaDePaginas(int processo){
	return tabela[processo];
}


int trans (int processo, int index, int offset, char acesso){
	int indexFr = tabela[processo]->indexFrame[index];
	Pagina *PG;
	
	if(indexFr != -1){ //significa que essa pagina tem na memoria
		printf("%d, %04x%04x, %c\n", processo+1, indexFr, offset, acesso);
		return 1; // se retornar 1 signifca q tem adc um no acesso do frame
	}
	
	//else, nao tem -> page fault
	
	PG = (Pagina*) malloc(sizeof(Pagina));
	PG->indexTP = index;
	PG->offset = offset;
	PG->acesso = acesso;
	PG->proc = processo;
	fila_push(filaPF, PG);
	
	kill(getppid(),SIGUSR1);
	return 0;
}

void liberaSHM (void){
	int i;
	for(i=0;i<4;i++){
		shmdt(tabela[i]);
		shmctl(seg[i],IPC_RMID,0);
	}
}






















