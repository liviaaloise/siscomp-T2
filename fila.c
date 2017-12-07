#include "fila.h"
#include <stdlib.h>

struct no{
	Pagina *pag;
	No *prox;
};

struct fila{
	No *ini;
};


Fila* fila_cria(void)
{
    Fila *f;
    f=(Fila*)malloc(sizeof(Fila));
    if(f==NULL){printf("Memoria Insuficiente\n");exit(1);}
    f->ini=NULL;
    return f;
}

void fila_push (Fila* f, Pagina *p)
{
	No *n, *aux;
    n=(No*)malloc(sizeof(No));
    if (!f->ini)
        f->ini = n;
    else{
        for(aux = f->ini; aux->prox; aux=aux->prox);
        aux->prox = n;
    }
    n->pag = p;
    n->prox = NULL;
}

Pagina* fila_pop (Fila* f)
{
	No *aux;
    Pagina *p;
    if(fila_vazia(f))
    {
        printf("Fila Vazia");
        exit(1);
    }
    aux = f->ini;
    p = aux->pag;
	f->ini=aux->prox;
	free(aux);
    return p;
}

int fila_vazia (Fila* f)
{
    return (f->ini==NULL);
}

void fila_libera (Fila* f)
{
    No *n, *aux;
    n = f->ini;
    while(n){
    	aux = n;
    	n= aux->prox;
    	free(aux);
    }
    free(f);
}


