/* Define as constantes, os tipos de dados e os protótipos de funções do filesystem.

   Copyright (C) 2025 Vinícius Daniel da Silva
*/
#pragma once 

#define FALSE 0
#define TRUE 1
#define ERROR_OPEN_FILE -1
#define ERROR_CREATE_FILE -2
#define ERROR_CLOSE_FILE -3
#define ERROR_WRITE_FILE -4
#define ERROR_DELETE_FILE -5
#define ERROR_READ_FILE -6
#define END_OF_FILE -7

// Número máximo de caracteres no nome do arquivo.
#define FILE_NAME_SIZE 128

// Comprimento da hora de criação do arquivo no formato hh:mm:ss. 
#define HOUR_SIZE 9

// Tamanho do bloco lógico em bytes.
#define LOGICAL_BLOCK_SIZE 512

// Tamanho mínimo de um disco em bytes.
#define MINIMUM_DISK_SIZE 65536

// Número máximo de arquivos do disco virtual.
#define NUMBER_OF_FILES_OF_DISK 1024

// Número máximo de discos virtuais que podem ser criados.
#define NUMBER_OF_DISKS 26

// Representação descritiva dos tipos de dados que um arquivo pode armazenar. 
#define TYPE_NUMERIC "Numérico "
#define TYPE_TEXT "Texto"

// Código UTF-8 do caractere A maiúsculo.
#define CHARACTER_CODE_A 0x41

// Código UTF-8 do caractere Z maiúsculo.
#define CHARACTER_CODE_Z 0x5A

// Indica que o disco virtual deve ser criado.
#define MODE_CREATE_DISK 0 

// Indica que o disco virtual previamente criado pode ser acessado.
#define MODE_GET_DISK 1

// Representa os tipos de dados que um arquivo pode armazenar. Os tipos são numérico (inteiro ou real) e string.
typedef enum { NONE, NUMERIC, STRING } FileType; 

// Tamanho representado como um inteiro sem sinal.
typedef unsigned int Size;

// O descritor de arquivo é usado para identificar um arquivo no sistema de arquivos. 
typedef unsigned int FileDescriptor;

// Identificação do arquivo.
typedef struct {
	/* Descritor de arquivo usado para identificar o arquivo no sistema de arquivos. 
	   ATENÇÃO: Deve ser gerado um código único de identificação para cada arquivo do sistema de arquivos. 
	*/ 
	FileDescriptor fileDescriptor;

	// Nome do arquivo. 
	char name[FILE_NAME_SIZE];

} FileIdentification;

// Atributos do arquivo.
typedef struct 
{
	// Nome e descritor do arquivo.
	FileIdentification fileIdentification;

	// Data de criação do arquivo no formato hh:mm:ss. 
	char hour[HOUR_SIZE];

	// Tamanho do arquivo em bytes.
	Size size; 

	// Número de blocos lógicos do arquivo.
	Size clusters; 

	// Conteúdo do arquivo. 
	void* content;

	// Representa o tipo de dado do conteúdo do arquivo. 
	FileType type;

	// Representa o estado de um arquivo, se está aberto ou não.
	unsigned char open;

} File;

// Atributos do disco.
typedef struct {
	// Capacidade máxima de armazenamento em bytes do disco.
    Size capacity;

	// Espaço ocupado do disco.
	Size usedSpace;
	
	// Espaço livre do disco.
	Size freeSpace;

	// Número total de blocos lógicos, ocupados e livres.
	Size totalClusters;

	// Número de blocos lógicos ocupados do disco.
	Size occupiedClusters;

	// Número de blocos lógicos livres do disco.
	Size availableClusters;

} Disk;

// Atributos do disco virtual.
typedef struct {
	// Dados do disco.
	Disk disk;

	// Número de arquivos do disco.
	Size numberOfFiles;

	// A tabela de alocação de arquivos é usada para armazenar todos os arquivos do disco. 
	File fileAllocationTable[NUMBER_OF_FILES_OF_DISK];

} VirtualDisk;

// Informações sobre o disco virtual.
typedef struct {
	// Letra de identificação do disco virtual.
	char diskID;

	// Dados do disco virtual.
	VirtualDisk virtualdisk;

} DiskInformation;

// Relação de discos do sistema de arquivos.
typedef struct {
	// Letra de identificação de todos os discos do sistema de arquivos.
	char diskID[NUMBER_OF_DISKS];

	// Nome e descritor de cada arquivo armazenado em cada disco do sistema de arquivos.
	FileIdentification* fileIdentification[NUMBER_OF_DISKS][NUMBER_OF_FILES_OF_DISK];

} SystemDisks;

/**
    Obtém a identificação do disco virtual onde o arquivo está armazenado.

	@param fileDescriptor identificação do arquivo;
	@param systemDisks relação de discos do sistema de arquivos.

	@return a letra de identificação do disco, por exemplo C, D, E, etc., onde o arquivo está armazenado.
 */
char getDiskID(FileDescriptor fileDescriptor, SystemDisks systemDisks);

/**
   Cria um disco virtual.

   @param size especifica a capacidade máxima de armazenamento em bytes do disco;
   @param diskID letra de identificação do disco, por exemplo C, D, E, etc.

   @return um ponteiro para a estrutura de armazenamento criada para o disco, se ocorrer erro retorna NULL.
*/
VirtualDisk* createDisk(char diskID, Size size);

/**
   Formata logicamente o disco virtual apagando todo o conteúdo de sua tabela de alocação de arquivos, consequentemente
   apagando todo os arquivos do disco. Reinicializa os dados da estrutura do disco. Toda a memória alocada dinamicamente 
   para armazenar a estrutura do disco e de seus arquivos devem ser liberadas, ou seja, devolvidas ao sistema operacional.

   @param diskID letra de identificação do disco, por exemplo C, D, E, etc.

   @return TRUE em caso de sucesso, caso contrário, FALSE.
*/
int format(char diskID);

/**
   Cria um arquivo para leitura e escrita no disco.
   
   @param diskID letra de identificação do disco, por exemplo C, D, E, etc;
   @param fileName nome do arquivo.
   
   @return um inteiro que representa o descritor de arquivo ou ERROR_CREATE_FILE se o arquivo não pode ser criado.
 */
int create(char diskID, const char *fileName);

/**
   Abre um arquivo para leitura e escrita.

   @param fileDescriptor identificação do arquivo.

   @return TRUE se o arquivo foi aberto e ERROR_OPEN_FILE se ocorrer algum erro, por exemplo, o arquivo não existe.
*/
int open(FileDescriptor fileDescriptor);

/**
	Escreve os dados no arquivo. 

	NOTA: O bloco lógico é a unidade básica de alocação de espaço (memória) no disco, portanto sempre que uma escrita é feita no 
	      disco deve-se escrever no mínimo um bloco lógico de tamanho definido em LOGICAL_BLOCK_SIZE. Sendo assim, por exemplo, se o tamanho do bloco lógico 
		  for 4 bytes, quando o usuário solicita que seja escrito um número inteiro de 4 bytes, usa-se um bloco lógico para armazená-lo no arquivo, mas quando 
		  é escrito a string "Férias!", usa-se 2 blocos lógicos. Como essa string possui 7 caracteres e cada caractere (char) ocupa 1 byte de memória, ela precisa
		  de 7 bytes de memória. No entanto, como nesse exemplo cada bloco lógico possui 4 bytes, a string exige 2 blocos lógicos (8 bytes) para ser armazenada 
		  no disco, logo deve-se escrever dois blocos.

	@param buffer ponteiro para o primeiro dado do buffer a ser escrito no arquivo;
	@param size tamanho de cada dado;
	@param number o número de dados a serem escritos no arquivo;
	@param fileDescriptor identificação do arquivo.
	
	@return o número de bytes escritos em caso de sucesso ou ERROR_WRITE_FILE se ocorrer algum erro.
			Se size ou number for zero, a função retorna ERROR_WRITE_FILE e a operação não é executada.
*/
int write(const void* buffer, Size size, Size number, FileDescriptor fileDescriptor);

/**
	Lê os dados do arquivo. 

	NOTA: O bloco lógico é a unidade básica de alocação de espaço (memória) no disco, portanto sempre que uma leitura é feita no 
	      disco deve-se ler no mínimo um bloco lógico de tamanho definido em LOGICAL_BLOCK_SIZE. Sendo assim, por exemplo, se o tamanho do bloco lógico 
		  for 4 bytes, quando o usuário solicita que seja lido um número inteiro de 4 bytes, lê-se um bloco lógico para recuperá-lo do arquivo, mas quando 
		  é lido a string "Férias!", lê-se 2 blocos lógicos. Como essa string possui 7 caracteres e cada caractere (char) ocupa 1 byte de memória, ela precisa 
		  de 7 bytes de memória. No entanto, como nesse exemplo como cada bloco lógico possui 4 bytes, a string exige 2 blocos lógicos (8 bytes) para ser armazenada
		  no disco, logo deve-se ler os dois blocos.

	@param buffer ponteiro para o buffer onde os dados lidos são armazenados;
	@param size tamanho de cada dado;
	@param number o número de dados a serem lidos do arquivo;
	@param fileDescriptor identificação do arquivo.

	@return o número de bytes lidos em caso de sucesso ou ERROR_READ_FILE se ocorrer algum erro ou 
	        END_OF_FILE se for feito uma tentativa de ler além do fim do arquivo.
			Se size ou number for zero, a função retorna ERROR_READ_FILE e a operação não é executada.
*/
int read(void* buffer, Size size, Size number, FileDescriptor fileDescriptor);

/**
   Fecha o arquivo. 

   @param fileDescriptor identificação do arquivo.

   @return TRUE se a operação for bem sucedida ou ERROR_CLOSE_FILE se não conseguir fechar o arquivo.
*/
int close(FileDescriptor fileDescriptor);

/**
   Apaga o arquivo do disco.

   @param fileDescriptor identificação do arquivo.

   @return TRUE se a operação for bem sucedida ou ERROR_DELETE_FILE se não conseguir excluir o arquivo do disco virtual.
*/
int erase(FileDescriptor fileDescriptor);

/**
   Obtém um relatório com as seguintes informações:

   - Dados do Arquivo
   --------------------
   1. nome;
   2. tipo;
   3. tamanho em bytes;
   4. número de blocos lógicos;
   5. hora de criação.

   - Dados do Disco
   ------------------
   1. número de arquivos;
   2. a capacidade de armazenamento;
   3. o espaço ocupado do disco;
   4. o espaço livre do disco;
   5. o número total de blocos lógicos;
   6. o número de blocos lógicos ocupados;
   7. o número de blocos lógicos livres.

   Os dados devem ser obtidos para cada arquivo do disco. 

   @param diskID letra de identificação do disco, por exemplo C, D, E, etc.

   @return o conteúdo do relatório em caso de sucesso ou NULL se ocorrer um erro.
*/
DiskInformation* getDiskInformation(char diskID);

/**
   Cria ou obtém o endereço da estrutura de armazenamento do disco.
   
   @param diskID letra de identificação do disco, por exemplo C, D, E, etc;
   @param mode informa o modo de acesso ao disco virtual, os valores válidos são: MODE_CREATE_DISK e MODE_GET_DISK 1.

   Se mode for MODE_CREATE_DISK e a letra de identificação do drive for válida, a função cria o disco e retorna um ponteiro para sua estrutura de armazenamento.
   Se mode for MODE_GET_DISK e a letra de identificação do drive for válida, a função obtém o ponteiro para a estrutura de armazenamento do disco previamente criada.

   @return NULL se ocorrer algum erro na criação ou no acesso ao disco ou se a identificação do drive for inválida.
*/
VirtualDisk* getVirtualDisk(char diskID, char mode);


/*-------------------------------------------*/

// Prompt do sistema de arquivos (file system)
#define FS_PROMPT "fs> "

#define INSTRUCTION_LINE_LEN 256
#define INSTRUCTION_LEN 32
#define STRING_SIZE (INSTRUCTION_LINE_LEN + INSTRUCTION_LEN)

#define NEW_LINE '\n'
#define NULL_CHAR '\0'
#define WHITE_SPACE " " 
#define CARACTERES_SEPARADORES " \t"

// Comandos a serem executados pelo sistema de arquivos.
#define CD "cd"
#define CA "ca"
#define AA "aa"
#define EA "ea"
#define LA "la"
#define FA "fa"
#define DA "da"
#define LD "ld"
#define FD "fd"
#define ET "et"
#define EF "ef"
#define IA "ia"

// Sitaxe dos comandos.
#define CD_SYNTAX "cd <disco> <tamanho>" 
#define CA_SYNTAX "ca <disco> <arquivo>"
#define AA_SYNTAX "aa <disco> <arquivo>"
#define EA_SYNTAX "ea <disco> <arquivo> <número | string>"
#define LA_SYNTAX "la <disco> <arquivo>" 
#define FA_SYNTAX "fa <disco> <arquivo>"
#define DA_SYNTAX "da <disco> <arquivo>"
#define LD_SYNTAX "ld <disco>"
#define ET_SYNTAX "et <disco> <arquivo>"
#define FD_SYNTAX "fd <disco>"
#define IA_SYNTAX "ia <disco> <caminho_arquivo_linux>"

// Estrutura do relatório de disco
#define REPORT_FILES_HEADER "\n  %-10s %-10s %-10s %-15s %-10s\n" 
#define REPORT_FILES_BODY "  %-10s %-10s %-10s %6d bytes    %-10d\n"
#define REPORT_DISK_INFO "\nNúmero de arquivos: %d\n\nCapacidade do disco: %d bytes\nEspaço ocupado: %d bytes\nEspaço livre: %d bytes\n\nTotal de blocos lógicos: %d\nBlocos lógicos ocupados: %d\nBlocos lógicos livres: %d\n\n"

// Indicam se um arquivo está aberto ou fechado
#define CLOSE 0
#define OPEN 1

typedef char* LinhaInstrucao;
typedef char* Instrucao;


// Exibe o prompt do sistema de arquivos, aguarda e executa instruções do usuário até que o sistema de arquivos seja finalizado.
int filesystem();

// Pula uma linha na tela.
void pularLinha();

// Exibe o prompt do sistema de arquivos.
void exibirPrompt();

// Lê instrução fornecida pelo usuário, e remove caracteres de nova linha. 
void lerInstrucao(LinhaInstrucao linhaInstrucao);

/* Substitui a primeira ocorrência de um caractere em uma string, por outro caractere, retorna TRUE caso isso aconteça e FALSE caso
    a string não contenha o caractere passado como parâmetro. 
    
    character - caractere a ser substituido.
    replacement - caractere que substituirá character.
*/
int substituirCaractere(char* string, char character, char replacement);

// Obtem o nome do comando da linha de instrução fornecida.
Instrucao obterNomeInstrucao(const LinhaInstrucao linhaInstrucao);

/* Obtém uma substring da string na posição indicada. A primeira posição válida é 1.
 	 O número de caracteres da string é definido por size, que deve ser menor do que STRING_SIZE. 
     Retorna a substring na posição fornecida ou NULL em caso de erro.
*/
char* substring(const char* string, size_t size, size_t position);

/* Verifica se uma instrução está entre a lista de instruçoes válidas do sistema de arquivos.
    Retorna TRUE caso seja uma instrução válida e FALSE caso não */
int analisarInstrucao(const Instrucao instrucao);

// Executa a função referente a instrução fornecida pelo usuário, e retorna o valor retornado por essa função.
int executarInstrucao(const LinhaInstrucao linhaInstrucao, const Instrucao instrucao);

// Obtem a quantidade de argumentos em uma linha de instruções.
int obterNumeroDeArgumentos(const LinhaInstrucao linhaInstrucao);

/* Verifica se uma string é composta por um único caractere, e se este representa uma letra (Identificação de Disco Válida).
    Retorna o caractere identificador caso estas condições sejam satisfeitas, e FALSE caso contrário. */
char validarIdentificador(const char* string);

/* Converte uma string para um número inteiro positivo, verificando se de fato essa string é composta somente por dígitos.
    Retorna o número representado na string como um inteiro, ou FALSE em caso de falha na conversão. */
int stringToInteger(const char* string);

// Verifica se uma string é na verdade um número inteiro ou real.
int isNumber(const char* str);

// Exibe na tela o tamanho e o número de blocos ocupados por um arquivo. Retorna TRUE.
int showFileSize(File* arquivo);

// Exibe na tela o conteúdo de um arquivo de acordo com seu tipo.
int showFileContent(const File* arquivo, Size size);

// Exibe na tela um relatório contendo todas as informações sobre um disco.
void showReport(DiskInformation diskInfo);

// Procura por um arquivo no disco 'diskID' e retorna sua referência caso seja encontrado, se não, retorna NULL.
File* obterArquivo(char diskID, char* fileName);

// Verifica se um arquivo está presente no disco 'diskID', retorna seu FileDescriptor caso exista e FALSE caso não.
int procurarArquivo(char diskID, char* fileName);

/* Obtem o horario atual e armazena no buffer. Retorna o número de caracteres armazenados ou 0 se 
    o tamanho HOUR_SIZE foi excedido na função strftime. */
size_t obterTempo(char* buffer);

// Adiciona a referência ao campo fileIdentification de um arquivo (File) à relação de discos do sistema.
int adicionarArquivoNaRelacao(FileIdentification* fileIdentification, unsigned int position, char diskID);

// Mantém a relação de discos do sistema de arquivos e retorna seu endereço quando a função é chamada.
SystemDisks* obterRelacaoDiscos();

// Executa a instrução cd (criar disco). Retorna TRUE caso o disco seja criado e FALSE caso não.
int criarDisco(const LinhaInstrucao linhaInstrucao);

/* Executa a instrução ca (criar arquivo). Retorna o descritor de arquivo caso o arquivo seja criado, FALSE caso algum dos argumentos 
    fornecidos sejam inválidos, e ERROR_CREATE_FILE caso ocorra algum erro na criação do arquivo. */
int criarArquivo(const LinhaInstrucao linhaInstrucao);

/* Executa as instruções fa (fechar arquivo) ou aa (abrir arquivo), com base no tipo de operação passado como parâmetro. 
    Se operation for igual a OPEN, abre o arquivo utilizando a função open.
    Se operation for igual a CLOSE, fecha o arquivo, utilizando a função close.
    Retorna TRUE se a operação for realizada com sucesso e FALSE caso não. */
int abrirOuFecharArquivo(LinhaInstrucao linhaInstrucao, unsigned char operation);

/**
   Escreve números ou strings em um arquivo, verificando o tipo.
*/
int escreverArquivo(LinhaInstrucao linhaInstrucao);

/* Exectuta a instrução la (ler arquivo). Lê e exibe o conteúdo de um arquivo contido em um disco. 
    Retorna TRUE e exibe o conteúdo em caso de sucesso, e FALSE caso contrário. */
int lerArquivo(LinhaInstrucao linhaInstrucao);

/* Executa a instrução da (excluir arquivo). Apaga um arquivo armazenado no disco virtual após o usuário confirmar a operação.
    Retorna TRUE caso a exclusão seja realizada e ERROR_DELETE_FILE caso não. */
int excluirArquivo(LinhaInstrucao linhaInstrucao);

/* Executa a instrução ld (listar disco). Exibe um relatório com as informações sobre o disco e cada arquivo armazenado nele.
    Retorna TRUE caso a listagem seja feita com sucesso, e FALSE se não. */
int listarDisco(LinhaInstrucao linhaInstrucao);

/* Executa a instruçao fd (formatar disco). Formata logicamente o disco virtual apagando todo o conteúdo de sua tabela de alocação de
    arquivos, consequentemente apagando todos os arquivos do disco. Exige, antes disso, a confirmação do usuário.
    Retorna TRUE se o disco for formatado, e FALSE caso contrário.
*/
int formatarDisco(LinhaInstrucao linhaInstrucao);

/* Executa a instrução et (exibir tamanho). Exibe o tamanho e o número de blocos de um arquivo armazenado no disco virtual. 
    Retorna TRUE se bem-sucedido e FALSE caso contrário. */
int exibirTamanho(LinhaInstrucao linhaInstrucao);

/*  Executa a instrução ia (importar arquivo). Importa um arquivo do sistema para o disco virtual.
    Retorna 
*/
int importarArquivo(LinhaInstrucao linhaInstrucao);