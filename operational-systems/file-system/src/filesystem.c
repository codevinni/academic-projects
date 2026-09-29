/* Implementa as funções do filesystem.

   Copyright (C) 2025 Vinícius Daniel da Silva
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include "filesystem.h"

/**
    Obtém a identificação do disco virtual onde o arquivo está armazenado.

	@param fileDescripton identificação do arquivo;
	@param systemDisks relação de discos do sistema de arquivos.

	@return a letra de identificação do disco, por exemplo C, D, E, etc., onde o arquivo está armazenado.
 */
char getDiskID(FileDescriptor fileDescriptor, SystemDisks systemDisks){

    for (int i = 0; i < NUMBER_OF_DISKS; i++) // Percorrendo discos.
        for (int j = 0; j < NUMBER_OF_FILES_OF_DISK; j++) // Percorrendo arquivos no disco
            if (systemDisks.fileIdentification[i][j] != NULL) 
                if (systemDisks.fileIdentification[i][j]->fileDescriptor == fileDescriptor) 
                    return systemDisks.diskID[i];
                
    return FALSE;
}

/**
   Cria um disco virtual.

   @param size especifica a capacidade máxima de armazenamento em bytes do disco;
   @param diskID letra de identificação do disco, por exemplo C, D, E, etc.

   @return um ponteiro para a estrutura de armazenamento criada para o disco, se ocorrer erro retorna NULL.
*/
VirtualDisk* createDisk(char diskID, Size size){
    
    VirtualDisk* novoDisco = getVirtualDisk(diskID, MODE_CREATE_DISK);

    if(novoDisco){
        novoDisco->disk.totalClusters = size/LOGICAL_BLOCK_SIZE; // Determina a quantidade de clusters.
        novoDisco->disk.availableClusters = novoDisco->disk.totalClusters;
        novoDisco->disk.capacity = size; 
        novoDisco->disk.freeSpace =  novoDisco->disk.totalClusters * LOGICAL_BLOCK_SIZE; // Determina o espaço disponível de acordo com o total de clusters, descartando espaço restante após o fim do ultimo cluster.
        
        SystemDisks* systemDisks = obterRelacaoDiscos();
        systemDisks->diskID[diskID - CHARACTER_CODE_A] = diskID;
    }

    return novoDisco;
}

/**
   Formata logicamente o disco virtual apagando todo o conteúdo de sua tabela de alocação de arquivos, consequentemente
   apagando todo os arquivos do disco. Reinicializa os dados da estrutura do disco. Toda a memória alocada dinamicamente 
   para armazenar a estrutura do disco e de seus arquivos devem ser liberadas, ou seja, devolvidas ao sistema operacional.

   @param diskID letra de identificação do disco, por exemplo C, D, E, etc.

   @return TRUE em caso de sucesso, caso contrário, FALSE.
*/
int format(char diskID){

    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);
    SystemDisks* systemDisks = obterRelacaoDiscos();

    if (!disco) 
        return FALSE;

    // Zera os dados do disco.
    disco->numberOfFiles = 0;
    disco->disk.usedSpace = 0;
    disco->disk.freeSpace = disco->disk.capacity;
    disco->disk.occupiedClusters = 0;
    disco->disk.availableClusters = disco->disk.totalClusters;

    // Limpando fileAllocationTable.
    for (int i = 0; i < NUMBER_OF_FILES_OF_DISK; i++) {
        disco->fileAllocationTable[i].fileIdentification.fileDescriptor = 0;
        disco->fileAllocationTable[i].fileIdentification.name[0] = '\0';
        disco->fileAllocationTable[i].size = 0;
        disco->fileAllocationTable[i].clusters = 0;
        disco->fileAllocationTable[i].type = NONE;
        disco->fileAllocationTable[i].open = CLOSE;
        disco->fileAllocationTable[i].hour[0] = '\0';
        disco->fileAllocationTable[i].content = NULL;
    }

    // Atualizando a relação de discos para remover a refereência aos arquivos. 
    int indexDisco = toupper(diskID) - CHARACTER_CODE_A;

    for (int j = 0; j < NUMBER_OF_FILES_OF_DISK; j++) {
        systemDisks->fileIdentification[indexDisco][j] = NULL;
    }

    return TRUE;
}

/**
   Cria um arquivo para leitura e escrita no disco.
   
   @param diskID letra de identificação do disco, por exemplo C, D, E, etc;
   @param fileName nome do arquivo.
   
   @return um inteiro que representa o descritor de arquivo ou ERROR_CREATE_FILE se o arquivo não pode ser criado.
 */
int create(char diskID, const char *fileName){

    // Variável estática para conferir aos arquivos, diferentes códigos de identificação.
    static FileDescriptor fd = 1;

    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);

    if(disco){
        unsigned int index = disco->numberOfFiles;
        disco->fileAllocationTable[index].fileIdentification.fileDescriptor = fd;
        disco->fileAllocationTable[index].type = NONE;
        obterTempo(disco->fileAllocationTable[index].hour);
        strncpy(disco->fileAllocationTable[index].fileIdentification.name, fileName, FILE_NAME_SIZE);

        adicionarArquivoNaRelacao(&(disco->fileAllocationTable[index].fileIdentification), index, diskID);

        disco->numberOfFiles++;

        return fd++;
    }

    return ERROR_CREATE_FILE;
}

/**
   Abre um arquivo para leitura e escrita.

   @param fileDescripton identificação do arquivo.

   @return TRUE se o arquivo foi aberto e ERROR_OPEN_FILE se ocorrer algum erro, por exemplo, o arquivo não existe.
*/
int open(FileDescriptor fileDescriptor){

    SystemDisks* systemDisks = obterRelacaoDiscos();
    char diskID = getDiskID(fileDescriptor, *systemDisks);
    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);

    if(disco)
        for(int index = 0; index < disco->numberOfFiles; index++)
            if(disco->fileAllocationTable[index].fileIdentification.fileDescriptor == fileDescriptor){
                disco->fileAllocationTable[index].open = OPEN;
                return TRUE;
            }
            
    return ERROR_OPEN_FILE;
}

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
int write(const void* buffer, Size size, Size number, FileDescriptor fileDescriptor) {
    
    if (size == 0 || number == 0)
        return ERROR_WRITE_FILE;

    SystemDisks* systemDisks = obterRelacaoDiscos();
    char diskID = getDiskID(fileDescriptor, *systemDisks);
    if (!diskID) 
        return ERROR_WRITE_FILE;

    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);

    File* file = NULL;
    // Procurando o arquivo no disco para obter sua referência.
    for (int index = 0; index < disco->numberOfFiles; index++) {
        if (disco->fileAllocationTable[index].fileIdentification.fileDescriptor == fileDescriptor) {
            file = &disco->fileAllocationTable[index];
            break;
        }
    }

    if (file == NULL || !file->open) 
        return ERROR_WRITE_FILE;

    Size bytesEscritos = size * number;
    Size novoTamanho = file->size + bytesEscritos;
    Size quantidadeClusters = (novoTamanho + LOGICAL_BLOCK_SIZE - 1) / LOGICAL_BLOCK_SIZE; 
    Size clustersAdicionais = quantidadeClusters - file->clusters; // Verifica se serão necessários mais clusters.

    if (disco->disk.freeSpace < bytesEscritos || disco->disk.availableClusters < clustersAdicionais) {
        printf("\nDisco %c cheio. Capacidade: %d bytes.\n\n", diskID, disco->disk.capacity);
        return ERROR_WRITE_FILE;
    }

    // Realocando memoria para o novo conteudo.
    char* novoConteudo = realloc(file->content, novoTamanho);
    if (!novoConteudo) 
        return ERROR_WRITE_FILE;

    // Copia os novos dados para o final.
    memcpy((char*)novoConteudo + file->size, buffer, bytesEscritos);   
    novoConteudo[novoTamanho] = '\0';

    file->content = novoConteudo;

    // Atualiza metadados
    file->size = novoTamanho;
    file->clusters = quantidadeClusters;

    // Atualiza o disco
    disco->disk.usedSpace += bytesEscritos;
    disco->disk.freeSpace -= bytesEscritos;
    disco->disk.occupiedClusters += clustersAdicionais;
    disco->disk.availableClusters -= clustersAdicionais;

    return bytesEscritos;
}

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
int read(void* buffer, Size size, Size number, FileDescriptor fileDescriptor) {
    
    SystemDisks* systemDisks = obterRelacaoDiscos();
    char diskID = getDiskID(fileDescriptor, *systemDisks);
    if (!diskID)
         return ERROR_READ_FILE;

    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);

    File* file = NULL;
    for(int i = 0; i < disco->numberOfFiles; i++){
        if(disco->fileAllocationTable[i].fileIdentification.fileDescriptor == fileDescriptor){
            file = &disco->fileAllocationTable[i];
            break;
        }
    }

    if(file == NULL || !file->open) 
        return ERROR_READ_FILE;

    Size bytesLidos = size * number;

    if (file->size < bytesLidos) 
        return END_OF_FILE;

    memcpy(buffer, file->content, bytesLidos);
    return bytesLidos;
}

/**
   Fecha o arquivo. 

   @param fileDescriptor identificação do arquivo.

   @return TRUE se a operação for bem sucedida ou ERROR_CLOSE_FILE se não conseguir fechar o arquivo.
*/
int close(FileDescriptor fileDescriptor){

    SystemDisks* systemDisks = obterRelacaoDiscos();
    char diskID = getDiskID(fileDescriptor, *systemDisks);
    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);

    if(disco)
        for(int index = 0; index < disco->numberOfFiles; index++)
            if(disco->fileAllocationTable[index].fileIdentification.fileDescriptor == fileDescriptor){
                disco->fileAllocationTable->open = CLOSE;
                return TRUE;
            }
            
    return ERROR_CLOSE_FILE;
}

/**
   Apaga o arquivo do disco.

   @param fileDescriptor identificação do arquivo.

   @return TRUE se a operação for bem sucedida ou ERROR_DELETE_FILE se não conseguir excluir o arquivo do disco virtual.
*/
int erase(FileDescriptor fileDescriptor) {
    
    SystemDisks* systemDisks = obterRelacaoDiscos();
    char diskID = getDiskID(fileDescriptor, *systemDisks);
    
    if(!diskID) 
        return ERROR_DELETE_FILE;

    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);

    int index = -1;
    for(int i = 0; i < disco->numberOfFiles; i++){
        if(disco->fileAllocationTable[i].fileIdentification.fileDescriptor == fileDescriptor){
            index = i;
            break;
        }
    }

    if(index == -1) 
        return ERROR_DELETE_FILE;

    File* arquivo = &disco->fileAllocationTable[index];

    if(arquivo->open == OPEN) 
        return ERROR_DELETE_FILE;

    // Atualiza o disco
    disco->disk.usedSpace -= arquivo->size;
    disco->disk.freeSpace += arquivo->size;
    disco->disk.occupiedClusters -= arquivo->clusters;
    disco->disk.availableClusters += arquivo->clusters;

    // Remove da fileAllocationTable
    for(int i = index; i < disco->numberOfFiles - 1; i++){
        disco->fileAllocationTable[i] = disco->fileAllocationTable[i + 1];
    }
    disco->numberOfFiles--;

    // Remove da relação de discos.
    for(int i = 0; i < NUMBER_OF_DISKS; i++){
        if(systemDisks->diskID[i] == diskID){
            for(int j = 0; j < NUMBER_OF_FILES_OF_DISK; j++){
                if(systemDisks->fileIdentification[i][j] != NULL && systemDisks->fileIdentification[i][j]->fileDescriptor == fileDescriptor){
                    systemDisks->fileIdentification[i][j] = NULL;
                    break;
                }
            }
            break;
        }
    }

    return TRUE;
}

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
DiskInformation* getDiskInformation(char diskID){

    DiskInformation* diskInfo = (DiskInformation*)calloc(1, sizeof(DiskInformation));
    
    if(diskInfo){
        VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);
        if(disco){
            diskInfo->virtualdisk = *disco;
            diskInfo->diskID = diskID;

            return diskInfo;
        }
    }

    return NULL;
}

/**
    Cria ou obtém o endereço da estrutura de armazenamento do disco.

    @param diskID letra de identificação do disco, por exemplo C, D, E, etc;
    @param mode informa o modo de acesso ao disco virtual, os valores válidos são: MODE_CREATE_DISK e MODE_GET_DISK 1.

    Se mode for MODE_CREATE_DISK e a letra de identificação do drive for válida, a função cria o disco e retorna um ponteiro para sua estrutura de armazenamento.
    Se mode for MODE_GET_DISK e a letra de identificação do drive for válida, a função obtém o ponteiro para a estrutura de armazenamento do disco previamente criada.

    @return NULL se ocorrer algum erro na criação ou no acesso ao disco ou se a identificação do drive for inválida.
*/
VirtualDisk* getVirtualDisk(char drive, char mode) 
{
   static VirtualDisk *virtualDisk[NUMBER_OF_DISKS];
   static unsigned char currentNumberOfDisks = 0;
   unsigned char indexOfDisk;

   if (drive >= CHARACTER_CODE_A && drive <= CHARACTER_CODE_Z && currentNumberOfDisks < NUMBER_OF_DISKS)
   {
      indexOfDisk = toupper(drive) - CHARACTER_CODE_A;

      switch (mode)
      {
      case MODE_CREATE_DISK:
            virtualDisk[indexOfDisk] = (VirtualDisk *)calloc(1, sizeof(VirtualDisk));
            currentNumberOfDisks++;

            return virtualDisk[indexOfDisk];

      case MODE_GET_DISK:
            return virtualDisk[indexOfDisk];
      }
   }
   return NULL;
}



// Executa a instrução cd (criar disco). Retorna TRUE caso o disco seja criado e FALSE caso não.
int criarDisco(const LinhaInstrucao linhaInstrucao){

    const unsigned ARGS_NUMBER = 3;

    if(obterNumeroDeArgumentos(linhaInstrucao) == ARGS_NUMBER){
        
        char diskID = validarIdentificador(substring(linhaInstrucao, strlen(linhaInstrucao), 2));
        if(diskID){
            diskID = toupper(diskID);
            Size tamanho = stringToInteger(substring(linhaInstrucao, strlen(linhaInstrucao), 3));
            if(tamanho) // É um inteiro válido
                if(tamanho < MINIMUM_DISK_SIZE){ // Verificando se o tamanho atende ao mínimo permitido.
                    printf("\nO tamanho mínimo é %d bytes\n\n", MINIMUM_DISK_SIZE);
                    return FALSE;
                }
                else // Tamanho mínimo aceito.
                    if(getVirtualDisk(diskID, MODE_GET_DISK) == NULL){ // Verificando se já existe um disco com este indentificador.
                        if(createDisk(diskID, tamanho)) // Cria o disco.
                            return TRUE;
                    }
                    else{
                        printf("\nJá existe um disco com este identificador.\n\n");
                        return FALSE;
                    }
        }
    }
    printf("\nUtilize a sintaxe - %s\n\n", CD_SYNTAX);
    return FALSE;
}

/* Executa a instrução ca (criar arquivo). Retorna o descritor de arquivo caso o arquivo seja criado, FALSE caso algum dos argumentos 
    fornecidos sejam inválidos, e ERROR_CREATE_FILE caso ocorra algum erro na criação do arquivo. */
int criarArquivo(const LinhaInstrucao linhaInstrucao){

    const unsigned ARGS_NUMBER = 3;
    char nomeArquivo[FILE_NAME_SIZE];

    if(obterNumeroDeArgumentos(linhaInstrucao) == ARGS_NUMBER){

        char diskID = validarIdentificador(substring(linhaInstrucao, strlen(linhaInstrucao), 2));
        if(diskID){
            diskID = toupper(diskID);
            if(getVirtualDisk(diskID, MODE_GET_DISK)){

                strncpy(nomeArquivo, substring(linhaInstrucao, strlen(linhaInstrucao), 3), FILE_NAME_SIZE);
                if(procurarArquivo(diskID, nomeArquivo) == 0)
                    return create(diskID, nomeArquivo);
                else{
                    printf("\nO arquivo '%s' já existe no disco '%c'\n\n", nomeArquivo, diskID);
                    return FALSE;
                }
            }
            else{
                printf("\nDisco virtual '%c' não é um disco válido.\n\n", diskID);
                return FALSE;
            }
        }
    }
    printf("\nUtilize a sintaxe - %s\n\n", CA_SYNTAX);
    return FALSE;
}

/* Executa as instruções fa (fechar arquivo) ou aa (abrir arquivo), com base no tipo de operação passado como parâmetro. 
    Se operation for igual a OPEN, abre o arquivo utilizando a função open.
    Se operation for igual a CLOSE, fecha o arquivo, utilizando a função close.
    Retorna TRUE se a operação for realizada com sucesso e FALSE caso não. */
int abrirOuFecharArquivo(LinhaInstrucao linhaInstrucao, unsigned char operation){

    const unsigned ARGS_NUMBER = 3;
    char nomeArquivo[FILE_NAME_SIZE];

    if(obterNumeroDeArgumentos(linhaInstrucao) == ARGS_NUMBER){
        char diskID = validarIdentificador(substring(linhaInstrucao, strlen(linhaInstrucao), 2));
        if(diskID){
            diskID = toupper(diskID);
            if(getVirtualDisk(diskID, MODE_GET_DISK)){
                strncpy(nomeArquivo, substring(linhaInstrucao, strlen(linhaInstrucao), 3), FILE_NAME_SIZE);
                FileDescriptor fd = procurarArquivo(diskID, nomeArquivo);
                if(fd){
                    if(operation)
                        return open(fd);
                    else
                        return close(fd);
                }
                else
                    printf("\nO arquivo '%s' não existe no disco '%c'\n\n", nomeArquivo, diskID); 
            }
            else
                printf("\nDisco virtual '%c' não é um disco válido.\n\n", diskID);
                
            return FALSE;
        }
    }
    if(operation)
        printf("\nUtilize a sintaxe - %s\n\n", AA_SYNTAX);
    else
        printf("\nUtilize a sintaxe - %s\n\n", FA_SYNTAX);

    return FALSE;
}

/* Executa a instrução ea (escrever arquivo). Escreve números ou strings em um arquivo, verificando o tipo.
    Retorna TRUE se a operação for realizada com sucesso e FALSE caso não.
*/
int escreverArquivo(LinhaInstrucao linhaInstrucao){
   
    const unsigned ARGS_NUMBER = 3;
    int argc = obterNumeroDeArgumentos(linhaInstrucao);
    char nomeArquivo[FILE_NAME_SIZE];
    char diskID;
    VirtualDisk* disco;
    File* file;

    if(argc <= ARGS_NUMBER){
        printf("\nUtilize a sintaxe - %s\n\n", EA_SYNTAX);
        return FALSE;
    }

    diskID = validarIdentificador(substring(linhaInstrucao, strlen(linhaInstrucao), 2));
    if(!diskID)
        return FALSE;
    
    diskID = toupper(diskID);

    disco = getVirtualDisk(diskID, MODE_GET_DISK);
    if(!disco){
        printf("\nDisco virtual '%c' não é um disco válido.\n\n", diskID);
        return FALSE;
    }

    strncpy(nomeArquivo, substring(linhaInstrucao, strlen(linhaInstrucao), 3), FILE_NAME_SIZE);
    file = obterArquivo(diskID, nomeArquivo);

    if(!file){
        printf("\nO arquivo '%s' não existe no disco '%c'\n\n", nomeArquivo, diskID); 
        return FALSE;
    }

    if(!file->open){
        printf("\nArquivo '%s' fechado.\n\n", nomeArquivo);
        return FALSE;
    }

    // Verifica se todos os argumentos são numéricos (para definir o tipo)
    int isNumeric = 1;
    char content[STRING_SIZE] = {0};
    for(int index = ARGS_NUMBER + 1; index <= argc; index++){
        char* arg = substring(linhaInstrucao, strlen(linhaInstrucao), index);
        if(!isNumber(arg))
            isNumeric = 0;
        
        strncat(content, arg, strlen(arg));
        
        if(index < argc)
            strncat(content, " ", 2);
    }

    //strcat(content, "\n");

    // Define o tipo do arquivo.
    if(file->type == NONE)
        file->type = isNumeric ? NUMERIC : STRING;
    else // Verificando se o tipo de dado condiz com o tipo do arquivo.
        if((file->type == NUMERIC && !isNumeric) || (file->type == STRING && isNumeric)){
            printf("\nErro: Tipo de dado incompatível.\n\n");
            return FALSE;
        }

    // Escreve o conteúdo no arquivo
    int bytesEscritos = write(content, sizeof(char), strlen(content) , file->fileIdentification.fileDescriptor);
    
    if(bytesEscritos == ERROR_WRITE_FILE){
        printf("\nFalha ao escrever.\n\n");
        return FALSE;
    }

    return TRUE;
}

/* Exectuta a instrução la (ler arquivo). Lê e exibe o conteúdo de um arquivo contido em um disco. 
    Retorna TRUE e exibe o conteúdo em caso de sucesso, e FALSE caso contrário. */
int lerArquivo(LinhaInstrucao linhaInstrucao){

    const unsigned ARGS_NUMBER = 3;
    char nomeArquivo[FILE_NAME_SIZE];

    if(obterNumeroDeArgumentos(linhaInstrucao) == ARGS_NUMBER){
        char diskID = validarIdentificador(substring(linhaInstrucao, strlen(linhaInstrucao), 2));
        if(diskID){
            diskID = toupper(diskID);
            if(getVirtualDisk(diskID, MODE_GET_DISK)){
                strncpy(nomeArquivo, substring(linhaInstrucao, strlen(linhaInstrucao), 3), FILE_NAME_SIZE);
                File* arquivo = obterArquivo(diskID, nomeArquivo);
                if(arquivo){
                    if(arquivo->open)
                        return showFileContent(arquivo, arquivo->size);
                    else
                        printf("\nO arquivo '%s' não está aberto\n\n", arquivo->fileIdentification.name);
                }
                else
                    printf("\nO arquivo '%s' não existe no disco '%c'\n\n", nomeArquivo, diskID); 
            }
            else
                printf("\nDisco virtual '%c' não é um disco válido.\n\n", diskID);
                
            return FALSE;
        }
    }
   printf("\nUtilize a sintaxe - %s\n\n", LA_SYNTAX);
}

/* Executa a instrução da (excluir arquivo). Apaga um arquivo armazenado no disco virtual após o usuário confirmar a operação.
    Retorna TRUE caso a exclusão seja realizada e ERROR_DELETE_FILE caso não. */
int excluirArquivo(LinhaInstrucao linhaInstrucao) {
    
    const unsigned ARGS_NUMBER = 3;
    char nomeArquivo[FILE_NAME_SIZE];

    if(obterNumeroDeArgumentos(linhaInstrucao) != ARGS_NUMBER){
        printf("\nUtilize a sintaxe - %s\n\n", DA_SYNTAX);
        return FALSE;
    }

    char diskID = validarIdentificador(substring(linhaInstrucao, strlen(linhaInstrucao), 2));
    if(!diskID){
        printf("\nIdentificador de disco inválido.\n\n");
        return FALSE;
    }

    diskID = toupper(diskID);

    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);
    if(!disco){
        printf("\nDisco virtual '%c' não é válido.\n\n", diskID);
        return FALSE;
    }

    strncpy(nomeArquivo, substring(linhaInstrucao, strlen(linhaInstrucao), 3), FILE_NAME_SIZE);
    File* arquivo = obterArquivo(diskID, nomeArquivo);
    if(!arquivo){
        printf("\nArquivo '%s' não existe no disco '%c'.\n\n", nomeArquivo, diskID);
        return FALSE;
    }

    if(arquivo->open == OPEN){
        printf("\nO arquivo '%s' está aberto. Feche-o antes de excluir.\n\n", nomeArquivo);
        return FALSE;
    }

    printf("\nConfirma a exclusão [S ou N]? ");
    fflush(stdout);
    char resposta;
    scanf(" %c", &resposta);
    while(getchar() != '\n'); // Limpar buffer de entrada

    if(toupper(resposta) != 'S'){
        printf("\nExclusão cancelada.\n\n");
        return FALSE;
    }

    FileDescriptor fd = arquivo->fileIdentification.fileDescriptor;
    int resultado = erase(fd);
    if(resultado == TRUE){
        printf("\nArquivo '%s' excluído com sucesso.\n\n", nomeArquivo);
        return TRUE;
    } 
    else{
        printf("\nErro ao excluir o arquivo '%s'.\n\n", nomeArquivo);
        return ERROR_DELETE_FILE;
    }
}

/* Executa a instrução ld (listar disco). Exibe um relatório com as informações sobre o disco e cada arquivo armazenado nele.
    Retorna TRUE caso a listagem seja feita com sucesso, e FALSE se não. */
int listarDisco(LinhaInstrucao linhaInstrucao){

    const unsigned ARGS_NUMBER = 2;

    if(obterNumeroDeArgumentos(linhaInstrucao) == ARGS_NUMBER){
        char diskID = validarIdentificador(substring(linhaInstrucao, strlen(linhaInstrucao), 2));
        if(diskID){
            diskID = toupper(diskID);
            if(getVirtualDisk(diskID, MODE_GET_DISK)){
                DiskInformation* diskInfo = getDiskInformation(diskID);
                if(diskInfo){
                    showReport(*diskInfo);
                    free(diskInfo); // Libera memória alocada em showReport.
                    return TRUE;
                }
            }
            else
                printf("\nDisco virtual '%c' não é um disco válido.\n\n", diskID); 
            
            return FALSE;
        }
    }

    printf("\nUtilize a sintaxe - %s\n\n", LD_SYNTAX);
    return FALSE;
}

/* Executa a instruçao fd (formatar disco). Formata logicamente o disco virtual apagando todo o conteúdo de sua tabela de alocação de
    arquivos, consequentemente apagando todos os arquivos do disco. Exige, antes disso, a confirmação do usuário.
    Retorna TRUE se o disco for formatado, e FALSE caso contrário.
*/
int formatarDisco(LinhaInstrucao linhaInstrucao) {
    const unsigned ARGS_NUMBER = 2;

    if(obterNumeroDeArgumentos(linhaInstrucao) != ARGS_NUMBER){
        printf("\nUtilize a sintaxe - %s\n\n", FD_SYNTAX);
        return FALSE;
    }

    char diskID = validarIdentificador(substring(linhaInstrucao, strlen(linhaInstrucao), 2));
    if(!diskID)
        return FALSE;

    diskID = toupper(diskID);

    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);
    if(!disco){
        printf("\nDisco virtual '%c' não existe.\n\n", diskID);
        return FALSE;
    }

    // Confirmação do usuário
    printf("\nTodos os dados serão excluídos, confirma a formatação [S ou N]? ");
    fflush(stdout);

    char resposta;
    scanf(" %c", &resposta);
    while(getchar() != '\n'); // Limpa o buffer de entrada

    if(toupper(resposta) != 'S'){
        printf("\nFormatação cancelada.\n\n");
        return FALSE;
    }

    int resultado = format(diskID);
    if(resultado == TRUE){
        printf("\nDisco '%c' formatado com sucesso.\n\n", diskID);
        return TRUE;
    } 
    else{
        printf("\nErro ao formatar o disco '%c'.\n\n", diskID);
        return FALSE;
    }
}

/* Executa a instrução et (exibir tamanho). Exibe o tamanho e o número de blocos de um arquivo armazenado no disco virtual. 
    Retorna TRUE se bem-sucedido e FALSE caso contrário. */
int exibirTamanho(LinhaInstrucao linhaInstrucao){

    const unsigned ARGS_NUMBER = 3;
    char nomeArquivo[FILE_NAME_SIZE];

    if(obterNumeroDeArgumentos(linhaInstrucao) == ARGS_NUMBER){
        char diskID = validarIdentificador(substring(linhaInstrucao, strlen(linhaInstrucao), 2));
        if(diskID){
            diskID = toupper(diskID);
            VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);
            if(disco){
                strncpy(nomeArquivo, substring(linhaInstrucao, strlen(linhaInstrucao), 3), FILE_NAME_SIZE);
                File* arquivo = obterArquivo(diskID, nomeArquivo);
                if(arquivo)
                    return showFileSize(arquivo);
                else
                    printf("\nO arquivo '%s' não existe no disco '%c'\n\n", nomeArquivo, diskID); 
            }
            else
                printf("\nDisco virtual '%c' não é um disco válido.\n\n", diskID);
                
            return FALSE;
        }
    }
    printf("\nUtilize a sintaxe - %s\n\n", ET_SYNTAX);
    return FALSE;
}

/*  Executa a instrução ia (importar arquivo). Importa um arquivo do sistema para o disco virtual.
    Retorna 
*/
int importarArquivo(LinhaInstrucao linhaInstrucao){

    const unsigned ARGS_NUMBER = 3;
    char diskID;
    char nomeArquivo[FILE_NAME_SIZE];
    VirtualDisk* disco;
    FILE* arq;
    char path[FILE_NAME_SIZE];

    if(obterNumeroDeArgumentos(linhaInstrucao) != ARGS_NUMBER){
        printf("\nUtilize a sintaxe: %s\n\n", IA_SYNTAX);
        return FALSE;
    }

    diskID = validarIdentificador(substring(linhaInstrucao, strlen(linhaInstrucao), 2));
    
    if (!diskID)
        return FALSE;
    
    diskID = toupper(diskID);
    disco = getVirtualDisk(diskID, MODE_GET_DISK);
    if (!disco) {
        printf("\nDisco virtual '%c' não é válido.\n\n", diskID);
        return FALSE;
    }

    strncpy(path, substring(linhaInstrucao, strlen(linhaInstrucao), 3), FILE_NAME_SIZE);

    // Extrai o nome do arquivo do caminho
    char* nomeDoArquivoLinux = strrchr(path, '/');
    strncpy(nomeArquivo, (nomeDoArquivoLinux ? nomeDoArquivoLinux + 1 : path), FILE_NAME_SIZE);

    if(procurarArquivo(diskID, nomeArquivo) != 0){
        printf("\nArquivo '%s' já existe.\n\n", nomeArquivo);
        return FALSE;
    }

    arq = fopen(path, "r");
    if(!arq){
        printf("\nErro ao abrir '%s'.\n\n", path);
        return FALSE;
    }

    // Cria o arquivo no disco virtual
    FileDescriptor fd = create(diskID, nomeArquivo);
    if(fd == ERROR_CREATE_FILE){
        fclose(arq);
        printf("\nErro ao criar arquivo.\n\n");
        return FALSE;
    }

    open(fd); // Abre o arquivo

    // Lê e escreve o conteúdo
    char buffer[LOGICAL_BLOCK_SIZE];
    size_t bytesLidos;
    while(1) {
        bytesLidos = fread(buffer, 1, LOGICAL_BLOCK_SIZE, arq);
        if (bytesLidos > 0) {
            if (write(buffer, 1, bytesLidos, fd) == ERROR_WRITE_FILE) {
                printf("\nErro ao escrever no disco virtual.\n\n");
                break;
            }
        }
    
        if (feof(arq)) 
            break;
        if (ferror(arq)) {
            printf("\nErro ao ler o arquivo de origem.\n\n");
            break;
        }
    }

    fclose(arq);
    close(fd);

    // Define o tipo como STRING
    File* arquivo = obterArquivo(diskID, nomeArquivo);
    if(arquivo) 
        arquivo->type = STRING;

    return TRUE;
}



// Mantém a relação de discos do sistema de arquivos e retorna seu endereço quando a função é chamada.
SystemDisks* obterRelacaoDiscos(){

    static SystemDisks systemDisks;
    return &systemDisks;
}

// Adiciona a referência ao campo fileIdentification de um arquivo (File) à relação de discos do sistema.
int adicionarArquivoNaRelacao(FileIdentification* fileIdentification, unsigned int position, char diskID){

    SystemDisks* systemDisks = obterRelacaoDiscos();
    systemDisks->fileIdentification[diskID - CHARACTER_CODE_A][position] = fileIdentification;
}

/* Obtem o horario atual e armazena no buffer. Retorna o número de caracteres armazenados ou 0 se 
    o tamanho HOUR_SIZE foi excedido na função strftime. */
size_t obterTempo(char* buffer){
    
    struct tm horario;
    time_t tempo;
    time(&tempo);
    localtime_r(&tempo, &horario);

    return strftime(buffer, HOUR_SIZE, "%H:%M:%S", &horario); // Armazena no buffer o horário formatado.
}

// Verifica se um arquivo está presente no disco 'diskID', retorna seu FileDescriptor caso exista e FALSE caso não.
int procurarArquivo(char diskID, char* fileName){

    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);

    for(int index = 0; index < disco->numberOfFiles; index++)
        if(strcmp(fileName, disco->fileAllocationTable[index].fileIdentification.name) == 0)
            return disco->fileAllocationTable[index].fileIdentification.fileDescriptor;

    return FALSE;
}

// Procura por um arquivo no disco 'diskID' e retorna sua referência caso seja encontrado, se não, retorna NULL.
File* obterArquivo(char diskID, char* fileName){

    VirtualDisk* disco = getVirtualDisk(diskID, MODE_GET_DISK);

    for(int index = 0; index < disco->numberOfFiles; index++)
        if(strcmp(fileName, disco->fileAllocationTable[index].fileIdentification.name) == 0)
            return &(disco->fileAllocationTable[index]);

    return NULL;
}

// Exibe na tela um relatório contendo todas as informações sobre um disco.
void showReport(DiskInformation diskInfo){
 
    File arq;
    char type[12] = {0};
    VirtualDisk disco = diskInfo.virtualdisk;

    printf("\nDisco %c\n", diskInfo.diskID);

    if(disco.numberOfFiles > 0)
        printf(REPORT_FILES_HEADER, "Nome", "Tipo", "Hora", "Tamanho", "Blocos");
    
    for(int index = 0; index < diskInfo.virtualdisk.numberOfFiles; index++){
        arq = diskInfo.virtualdisk.fileAllocationTable[index];

        if(arq.type == NUMERIC)
            strcpy(type, TYPE_NUMERIC);
        else
            if(arq.type == STRING)
                strcpy(type, TYPE_TEXT);
            else
                strcpy(type, "      ");

        printf(REPORT_FILES_BODY, arq.fileIdentification.name, type, arq.hour, arq.size, arq.clusters);
    }
    
    printf(REPORT_DISK_INFO, disco.numberOfFiles, disco.disk.capacity, disco.disk.usedSpace,
        disco.disk.freeSpace, disco.disk.totalClusters, disco.disk.occupiedClusters, disco.disk.availableClusters);
}

// Exibe na tela o conteúdo de um arquivo de acordo com seu tipo.
int showFileContent(const File* arquivo, Size size) {
    
    if(size <= 0){
        printf("\nArquivo vazio.\n\n");
        return FALSE;
    }

    if(arquivo->type == NUMERIC){
        
        pularLinha();
        char* conteudo = (char*)arquivo->content;
        char* token = strtok(conteudo, CARACTERES_SEPARADORES);
       
        while(token != NULL){
            printf("%s ", token);
            token = strtok(NULL, CARACTERES_SEPARADORES);
        }

        pularLinha();
        pularLinha();
    } 
    else 
        if(arquivo->type == STRING)
            printf("\n%s\n\n", (char*)(arquivo->content));
        else{
            printf("\nTipo desconhecido.\n\n");
            return FALSE;
        }

    return TRUE;
}

// Exibe na tela o tamanho e o número de blocos ocupados por um arquivo. Retorna TRUE.
int showFileSize(File* arquivo){

    printf("\nTamanho = %'d bytes\nBlocos = %'d\n\n", arquivo->size, arquivo->clusters);
    return TRUE;
}

// Verifica se uma string é na verdade um número inteiro ou real.
int isNumber(const char* str) {
    char* endptr;
    strtod(str, &endptr);
    return (*endptr == '\0');
}

/* Converte uma string para um número inteiro positivo, verificando se de fato essa string é composta somente por dígitos.
    Retorna o número representado na string como um inteiro, ou FALSE em caso de falha na conversão. */
int stringToInteger(const char* string){

    size_t stringSize = strlen(string);

    for(int index = 0; index < stringSize; index++)
        if(isdigit(string[index]) == 0)
            return FALSE;
        
    return atoi(string);
}

/* Verifica se uma string é composta por um único caractere, e se este representa uma letra (Identificação de Disco Válida).
    Retorna o caractere identificador caso estas condições sejam satisfeitas, e FALSE caso contrário. */
char validarIdentificador(const char* string){

    size_t stringSize = strlen(string);

    if(stringSize == 1 && isdigit(*string) == 0)
        return *string;
    
    return FALSE;
}

// Obtem a quantidade de argumentos em uma linha de instruções.
int obterNumeroDeArgumentos(const LinhaInstrucao linhaInstrucao){

    unsigned char argc = 0;

    while(substring(linhaInstrucao, strlen(linhaInstrucao), ++argc));

    return argc-1;
}

// Executa a função referente a instrução fornecida pelo usuário, e retorna o valor retornado por essa função.
int executarInstrucao(const LinhaInstrucao linhaInstrucao, const Instrucao instrucao){
    
    if(strcmp(instrucao, CD) == 0)
        return criarDisco(linhaInstrucao);
    else
        if(strcmp(instrucao, CA) == 0)
            return criarArquivo(linhaInstrucao);
        else
            if(strcmp(instrucao, AA) == 0)
                return abrirOuFecharArquivo(linhaInstrucao, OPEN);
            else
                if(strcmp(instrucao, EA) == 0)
                    return escreverArquivo(linhaInstrucao);
                else
                    if(strcmp(instrucao, LA) == 0)
                        return lerArquivo(linhaInstrucao); 
                    else
                        if(strcmp(instrucao, FA) == 0)
                            return abrirOuFecharArquivo(linhaInstrucao, CLOSE);
                        else
                            if(strcmp(instrucao, DA) == 0)
                                return excluirArquivo(linhaInstrucao); 
                            else
                                if(strcmp(instrucao, LD) == 0)
                                    return listarDisco(linhaInstrucao);
                                else
                                    if(strcmp(instrucao, FD) == 0)
                                        return formatarDisco(linhaInstrucao);
                                    else
                                        if(strcmp(instrucao, ET) == 0)
                                            return exibirTamanho(linhaInstrucao);
                                        else
                                            if(strcmp(instrucao, IA) == 0)
                                                return importarArquivo(linhaInstrucao);
}

/* Verifica se uma instrução está entre a lista de instruçoes válidas do sistema de arquivos.
    Retorna TRUE caso seja uma instrução válida e FALSE caso não */
int analisarInstrucao(const Instrucao instrucao){

    Instrucao instrucoesValidas[] = { CD, CA, AA, EA, LA, FA, DA, LD, FD, ET, EF, IA };
    size_t numeroInstrucoes = sizeof(instrucoesValidas)/sizeof(Instrucao);

    for(int index = 0; index < numeroInstrucoes; index++)
        if(strcmp(instrucao, instrucoesValidas[index]) == 0)
            return TRUE;

    return FALSE;
}

/* Obtém uma substring da string na posição indicada. A primeira posição válida é 1.
 	 O número de caracteres da string é definido por size, que deve ser menor do que STRING_SIZE. 
     Retorna a substring na posição fornecida ou NULL em caso de erro.
*/
char* substring(const char* string, size_t size, size_t position)
{
	char *context = NULL;
	size_t index = 0;

	if (position >= 1 && position < size && size < STRING_SIZE)
	{
		// Variável local estática mantém seu valor entre chamadas de função.
		static char substring[STRING_SIZE];

		// Salva a string original para não permitir sua alteração pela função strtok.
        strlcpy(substring, string, STRING_SIZE);

		// Obtém a primeira substring da string.
		char* substr = strtok(substring, CARACTERES_SEPARADORES);
		while (substr != NULL)
		{
			// Verifica se a substring obtida da string está na posição fornecida.
			if (++index == position) {
				strlcpy(substring, substr, size);
				break;
			}
			// Obtém a próxima substring da string.
			substr = strtok(NULL, CARACTERES_SEPARADORES);
		}
		return substr != NULL ? substring : NULL;
	}
	return NULL;
}

// Obtem o nome do comando da linha de instrução fornecida.
Instrucao obterNomeInstrucao(const LinhaInstrucao linhaInstrucao){
    return substring(linhaInstrucao, INSTRUCTION_LINE_LEN, 1);
}

/* Substitui a primeira ocorrência de um caractere em uma string, por outro caractere, retorna TRUE caso isso aconteça e FALSE caso
    a string não contenha o caractere passado como parâmetro. 
    
    character - caractere a ser substituido.
    replacement - caractere que substituirá character.
*/
int substituirCaractere(char* string, char character, char replacement){

    size_t stringLen = strlen(string);

    for(unsigned pos = 0; pos < stringLen; pos++)
        if(string[pos] == character){
            string[pos] = replacement;
            return TRUE;
        }

    return FALSE;
}

// Lê instrução fornecida pelo usuário, e remove caracteres de nova linha. 
void lerInstrucao(LinhaInstrucao linhaInstrucao){

    fflush(stdin);
    fgets(linhaInstrucao, INSTRUCTION_LINE_LEN, stdin);
    substituirCaractere(linhaInstrucao, NEW_LINE, NULL_CHAR);
}

// Exibe o prompt do sistema de arquivos.
void exibirPrompt(){
    printf(FS_PROMPT);
}

// Pula uma linha na tela.
void pularLinha(){
    printf("%c",NEW_LINE);
}

// Exibe o prompt do sistema de arquivos, aguarda e executa instruções do usuário até que o sistema de arquivos seja finalizado.
int filesystem() {

    LinhaInstrucao linhaInstrucao = (LinhaInstrucao)calloc(1, INSTRUCTION_LINE_LEN);
    Instrucao instrucao = (Instrucao)calloc(1, INSTRUCTION_LEN);

    pularLinha();

    do{
        exibirPrompt();
        lerInstrucao(linhaInstrucao);

        if(linhaInstrucao[0] == NULL_CHAR)
            continue;

        // Obtendo a partir da linha de instrução fornecida, o nome da instrução a ser executada.
        strlcpy(instrucao, obterNomeInstrucao(linhaInstrucao), INSTRUCTION_LEN);

        if(analisarInstrucao(instrucao)){
            executarInstrucao(linhaInstrucao, instrucao);
        }
        else
            puts("\nInstruções inválidas!\n");

    }while(strcmp(EF, instrucao));

    free(instrucao);
    free(linhaInstrucao);

    pularLinha();

    return EXIT_SUCCESS;
}

int main() {
    return filesystem();
}