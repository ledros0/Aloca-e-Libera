#include <stdio.h>
#include <stddef.h>

#define TAM_MEMORIA (16 * 1024) // 16 KB

static char memoria_pool[TAM_MEMORIA];

typedef struct BlocoHeader {
    size_t tamanho;          // Tamanho útil do bloco
    int livre;               // 1 se estiver livre, 0 se alocado
    struct BlocoHeader *prox;
    struct BlocoHeader *ant;
} BlocoHeader;

static BlocoHeader *lista_livre = NULL;
static int inicializado = 0;

void inicializar_alocador() {
    if (inicializado) return;
    
    BlocoHeader *primeiro = (BlocoHeader *)memoria_pool;
    primeiro->tamanho = TAM_MEMORIA - sizeof(BlocoHeader);
    primeiro->livre = 1;
    primeiro->prox = NULL;
    primeiro->ant = NULL;
    
    lista_livre = primeiro;
    inicializado = 1;
}

void* aloca(size_t tamanho) {
    if (!inicializado) {
        inicializar_alocador();
    }
    
    if (tamanho == 0) return NULL;
    
        size_t tamanho_alinhado = (tamanho + 7) & ~7;
    
    BlocoHeader *atual = lista_livre;
    while (atual != NULL) {
        if (atual->livre && atual->tamanho >= tamanho_alinhado) {
            // Verifica se o bloco sobra espaço suficiente para ser dividido
            if (atual->tamanho >= tamanho_alinhado + sizeof(BlocoHeader) + 8) {
                BlocoHeader *novo_bloco = (BlocoHeader *)((char *)(atual + 1) + tamanho_alinhado);
                novo_bloco->tamanho = atual->tamanho - tamanho_alinhado - sizeof(BlocoHeader);
                novo_bloco->livre = 1;
                novo_bloco->prox = atual->prox;
                novo_bloco->ant = atual;
                
                if (atual->prox != NULL) {
                    atual->prox->ant = novo_bloco;
                }
                atual->prox = novo_bloco;
                atual->tamanho = tamanho_alinhado;
            }
            
            atual->livre = 0;
            return (void *)(atual + 1);
        }
        atual = atual->prox;
    }
    
    return NULL; }

void libera(void *ptr) {
    if (ptr == NULL) return;
    
    BlocoHeader *header = (BlocoHeader *)ptr - 1;
    header->livre = 1;
    
    if (header->prox != NULL && header->prox->livre) {
        header->tamanho += sizeof(BlocoHeader) + header->prox->tamanho;
        header->prox = header->prox->prox;
        if (header->prox != NULL) {
            header->prox->ant = header;
        }
    }
    
    if (header->ant != NULL && header->ant->livre) {
        header->ant->tamanho += sizeof(BlocoHeader) + header->tamanho;
        header->ant->prox = header->prox;
        if (header->prox != NULL) {
            header->prox->ant = header->ant;
        }
    }
}

typedef struct No {
    int dado;
    struct No *prox;
} No;

No* inserir(No *cabeca, int valor) {
    No *novo_no = (No*) aloca(sizeof(No));
    if (novo_no == NULL) {
        printf("Erro: Memória esgotada no vetor global!\n");
        return cabeca;
    }
    novo_no->dado = valor;
    novo_no->prox = cabeca;
    return novo_no;
}

No* remover(No *cabeca) {
    if (cabeca == NULL) return NULL;
    No *temp = cabeca;
    cabeca = cabeca->prox;
    libera(temp);
    return cabeca;
}

void imprimir(No *cabeca) {
    No *atual = cabeca;
    printf("Lista: ");
    while (atual != NULL) {
        printf("%d -> ", atual->dado);
        atual = atual->prox;
    }
    printf("NULL\n");
}

int main() {
    No *lista = NULL;

    printf("--- Inserindo elementos ---\n");
    lista = inserir(lista, 10);
    lista = inserir(lista, 20);
    lista = inserir(lista, 30);
    imprimir(lista);

    printf("\n--- Removendo o primeiro elemento (30) ---\n");
    lista = remover(lista);
    imprimir(lista);

    printf("\n--- Inserindo novos elementos (40 e 50) ---\n");
    lista = inserir(lista, 40);
    lista = inserir(lista, 50);
    imprimir(lista);

    printf("\n--- Limpando toda a lista ---\n");
    while (lista != NULL) {
        lista = remover(lista);
    }
    imprimir(lista);

    return 0;
}
