#include "fila.h"

void cria_filaPF();
void destroi_filaPF();
Pagina* primeiro_filaPF();
TabelaDePaginas* cria_TabelaDePaginas(int processo);
TabelaDePaginas* attach_TabelaDePaginas(int processo);
int trans (int processo, int index, int offset, char acesso);
void liberaSHM (void);
