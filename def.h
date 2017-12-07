#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/shm.h>
#include<time.h>
#define PGS 65536  // temos 2^16 paginas numa tabela de paginas
#define FRM 256


typedef struct pagina{
	int proc;
	int indexTP;
	int offset;
	char acesso;
} Pagina;

typedef struct tabelaDePaginas{
	int proc;
	int indexFrame[PGS];
} TabelaDePaginas;

typedef struct frame{
	int count_R;
	int M;
	Pagina pag;
} Frame;

typedef struct memoriaFisica{
	int indexVazio; // index do primeiro frame vazio
	Frame frame[FRM];
} MemoriaFisica;
