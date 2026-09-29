#include <time.h>
#include <dirent.h>

#define TRUE 1
#define FALSE 0

#define BOLD_TEXT "\033[1m" // Constante para deixar o texto em negrito.
#define TURN_OFF_BOLD "\033[m" // Constante para limitar até onde o texto em negrito se expande.

// Tamanho máximo do caminho para um diretório.
#define MAX_PATH_LENGHT 5000
// Tamanho máximo a ser considerado para cada linha de um arquivo.
#define MAX_LINE_LENGHT 2000
// Tamanho máximo para um argumento passado pela linha de comando.
#define ARGUMENT_LENGTH 5000
// Tmanaho máximo para o nome de um arquivo.
#define FILE_NAME_SIZE 256

// Caracteres ocorrentes em códigos C.
#define SIMPLE_COMENT "//"
#define OPEN_LONG_COMENT "/*"
#define CLOSE_LONG_COMENT "*/"
#define NEW_LINE '\n'
#define NULL_CHAR '\0'
#define SPACE ' '

// Estrutura do relatório final.
#define RELATORY_SUBTITTLE_1 "\n- Código-fonte C"
#define RELATORY_SUBTITTLE_2 "\n- Tempo"
#define RELATORY_OF_LINES_BODY "\n   N° de arquivos = %d\n   Linhas vazias = %d\n   Linhas de comentários = %d\n   Linhas de instruções = %d\n"
#define RELATORY_OF_TIME "\n   Início.....: %02d:%02d:%02d\n   Término: %02d:%02d:%02d\n   Duração: %d segundos\n\n"



typedef unsigned char status;
typedef char* CommandLineArgument;
typedef char* DirectoryName;

/* Representa a quantidade de linhas em branco, comentadas ou de código em um 
    único arquivo, ou o total destas propriedades em um código fonte. */
typedef struct {
    unsigned emptyLines,
        comentedLines,
        codeLines;
}Properties;  

typedef Properties FileProperties;
typedef Properties TotalOfLines;

/* Representa os atributos de um arquivo que será lido, o que inclui, seu status (se já foi lido ou não), 
    seu nome, e a quantidade de linhas. */
typedef struct {
    char name[FILE_NAME_SIZE];
    char path[MAX_PATH_LENGHT];
    status read;
    FileProperties properties;
}FileElement; 

typedef FileElement* FileElements; 

// Representa informações sobre o tempo inicial, final e decorrido a partir do início do cálculo das linhas de um código fonte.
typedef struct {
    struct tm *start, *end;
    int elapsed;
}TimeInfo;



/* Inicia o programa recebendo um nome de arquivo ou diretório, fornecidos através da linha de comando do SO.
    Cria um processo filho para executar a contagem do total de linhas de um código fonte. */
int clsc(int argc, char* argv[]);

/* Recebe o argumento passado pela linha de comando e verifica se é um arquivo ou diretório existente.
    Indica por meio de uma váriavel se o argumento representa um arquivo (valor 1), ou se representa um diretório (valor 0).
    Retorna FALSE caso o argumento não represente um arquivo ou diretório válido, e TRUE se obtiver sucesso */
int analyseArgs(const CommandLineArgument commandLineArg, int* argType);

// Verifica se o nome de um arquivo contém a extensão .c. Retorna TRUE caso sim e FALSE no caso contrário. 
int isValidFile(char* fileName);

// Cria um novo processo e retorna o seu PID.
pid_t createChild();

/* Calcula o total de linhas de um único arquivo ou um diretório, criando threads (ou chamando função para tal) proporcionalmente à quantidade 
    de arquivos válidos, para que executem a função countLines. Após isso, obtem o resultado final, o tempo de execução, e exibe um relatório. */
int calculateTotalLines(int argType, CommandLineArgument commandLineArg);

void readDirectory(DirectoryName path, FileElements** ptr, unsigned* number_of_files);

// Copia src1 para str, e então concatena src2 e src3 à str, respectivamente.
void strCopyAndCat(char* str, size_t size, char* src1, char* src2, char* src3);

// Aguarda que todas as threads criadas terminem de executar, verificando o campo status de cada arquivo.
void waitThreads(const FileElements* files, int number_of_thrds);

/* Realiza a contagem de linhas de código, linhas em branco e linhas comentadas em um código fonte */
int countLines(void* files);

// Inicializa os campos de um dado do tipo FileElement com valores padrões.
void defaultFileElement(FileElement *file);

/* Realiza a soma de todas as diferentes classificações de linhas de cada arquivo no vetor de ponteiros.
    Retorna uma estrutura com o resultado final. */
TotalOfLines sumTotalLines(FileElements* files, unsigned number_of_files);

// Converte o tempo do formato time_t para um formato local, e armazena os valores obtidos. Calcula também a duração do programa.
void setTimeValues(TimeInfo *time, time_t *start, time_t *end);

// Exibe o relatório final do programa.
void showRelatory(TotalOfLines total, unsigned number_of_files, TimeInfo time);

// Libera memória utilizada durante execução da função executada pelo processo filho.
void freeAllocatedMemory(FileElements* files, TimeInfo* programTime, unsigned number_of_files);