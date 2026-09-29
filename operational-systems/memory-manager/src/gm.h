#define TRUE 1
#define FALSE 0

#define BOLD "\033[1m"
#define RESET_BOLD "\033[0m"

#define NAME_LEN 64
#define LINE_LEN 256

#define MAX_TASK_MEM 4096
#define PAGE_SIZE 512
#define MAX_LOG_PAGES (MAX_TASK_MEM / PAGE_SIZE)

#define PAGES_BEGIN 40
#define PAGES_END 127
#define PAGES_TOTAL (PAGES_END - PAGES_BEGIN + 1)

#define MAX_VECTORS 101
#define MAX_ACCESS 101

#define NOT_FOUND -1
#define FILE_ERROR -2
#define INSTRUCTION_ERROR -3
#define MEMORY_ERROR -4

typedef unsigned int Pages;
typedef unsigned int Size;
typedef char Name[NAME_LEN];

// Representa um vetor criado em uma tarefa.
typedef struct {
    Name nome;
    Size tamanho;
    int inicio;
} Vetor;

// Respresenta os acessos realizados a um vetor, em uma tarefa.
typedef struct {
    Name nomeVetor;
    int posicao;
} AcessoMemoria;

// Representa uma tarefa e suas propriedades.
typedef struct {
    Size memoriaCodigo;
    Size memoriaDados;

    Vetor vetores[MAX_VECTORS];
    int numeroVetores;
    AcessoMemoria acessos[MAX_ACCESS];
    int numeroAcessos;

    Pages tabelaPaginas[MAX_LOG_PAGES];
    Pages paginasCodigo;
    Pages paginasDados;
    Pages *paginasFisicas;
} Tarefa;

// Inicia o sistema de gerencia de memória, processando cada tarefa passada como argumento através da linha de comando do SO.
int gm(int argc, char* tarefas[]);

/* Processa a tarefa, lendo o arquivo, interpretando suas instruções e gerando um relatório sobre o mesmo.
    Retorna TRUE se a tarefa for processada com sucesso, ou um código de erro em caso de falha. */
int processarTarefa(const char* nomeTarefa, int* paginasLivres, int* numeroPaginasLivres);

/* Substitui a primeira ocorrência de um caractere em uma string, por outro caractere, retorna TRUE caso isso aconteça e FALSE caso
    a string não contenha o caractere passado como parâmetro. 
    
    character - caractere a ser substituido.
    replacement - caractere que substituirá character.
*/
int substituirCaractere(char* string, char character, char replacement);

// Gera e exibe um relatório contendo as informações de uma tarefa.
void gerarRelatorio(const Tarefa* tarefa, const char* nomeTarefa);

/* Verifica se uma instrução representa acesso a um vetor. Verifica também se o vetor existe na estrutura 
    da tarefa e se a posição acessada está dentro dos limites do vetor. 
    Retorna TRUE e registra o acesso em caso de sucesso, e retorna FALSE em caso de falha. */
int instrucaoAcessarVetor(Tarefa* tarefa, const char* linha, const char* nomeTarefa);

/* Verifica se uma instrução contendo o comando "new" tem o formato correto, para alocar um novo vetor. Caso tenha, verifica se o tamanho desse vetor
    a ser criado excede o tamanho máximo para uma tarefa. Verifica também se o novo vetor já existe na tarefa.
    Retorna TRUE e armazena as informações do vetor em caso de sucesso, e retorna FALSE em caso de falha. */
int instrucaoAlocarVetor(Tarefa* tarefa, const char* linha, const char* nomeTarefa);

// Verifica se um vetor já foi criado em uma tarefa. Retorna o índice onde foi encontrado ou NOT_FOUND caso não tenha sido localizado.
int encontrarVetor(const Tarefa* tarefa, const char* nomeVetor);

/* Aloca dinamicamente um vetor de páginas e preenche o mesmo com as páginas disponíveis no vetor paginasLivres. 
    Remove as páginas utilizadas do vetor de páginas livres. 
    Retorna TRUE caso a alocação seja bem sucedida e FALSE caso contrário. */
int alocarPaginas(int num, int** paginas, int* paginasLivres, int* numeroPaginasLivres);

/* Inicializa o vetor de páginas livres. Preenche este vetor com os identificadores das páginas físicas disponíveis, 
    ou seja, de PAGES_BEGIN a PAGES_TOTAL. */
void inicializarPaginas(int* paginasLivres, int* numeroPaginasLivres);