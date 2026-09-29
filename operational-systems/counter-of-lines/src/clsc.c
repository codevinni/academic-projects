#include <stdio.h>      
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <threads.h>
#include "clsc.h"
int linhasT = 0;
// Libera memória alocada durante execução da função executada pelo processo filho.
void freeAllocatedMemory(FileElements* files, TimeInfo* programTime, unsigned number_of_files){

    // Liberando memória alocada para cada ponteiro no vetor.
    for(int index = 0; index < number_of_files; index++)
        free(files[index]);
    
    free(files);

    free(programTime->end); 
    free(programTime->start);
}

// Exibe o relatório final do programa.
void showRelatory(TotalOfLines total, unsigned number_of_files, TimeInfo time){

    printf(BOLD_TEXT "%s" TURN_OFF_BOLD, RELATORY_SUBTITTLE_1);
    printf(RELATORY_OF_LINES_BODY, number_of_files, total.emptyLines, total.comentedLines, total.codeLines);
    
    printf(BOLD_TEXT "%s" TURN_OFF_BOLD, RELATORY_SUBTITTLE_2);
    printf(RELATORY_OF_TIME, time.start->tm_hour, time.start->tm_min, time.start->tm_sec, time.end->tm_hour, time.end->tm_min, time.end->tm_sec , time.elapsed);
}

// Converte o tempo do formato time_t para um formato local, e armazena os valores obtidos. Calcula também a duração do programa.
void setTimeValues(TimeInfo *time, time_t *start, time_t *end){

    time->start = (struct tm*)malloc(sizeof(struct tm));
    time->end = (struct tm*)malloc(sizeof(struct tm));

    time->elapsed = *end-*start; // Duração do programa em segundos.
    localtime_r(start, time->start);
    localtime_r(end, time->end);
    
    /* Alternativa acessando o conteúdo da struct tm que está sendo retornada.
        *(time->start) = *localtime(start); 
        *(time->end) = *localtime(end); 
    */
}

/* Realiza a soma de todas as diferentes classificações de linhas de cada arquivo no vetor de ponteiros.
    Retorna uma estrutura com o resultado final. */
TotalOfLines sumTotalLines(FileElements* files, unsigned number_of_files){

    TotalOfLines total = {0}; // Inicializando todos os campos com 0.

    for(int index = 0; index < number_of_files; index++){
        total.emptyLines += files[index]->properties.emptyLines;
        total.comentedLines += files[index]->properties.comentedLines;
        total.codeLines += files[index]->properties.codeLines;
    }

    return total;
}

// Inicializa os campos de um dado do tipo FileElement com valores padrões.
void defaultFileElement(FileElement *file){

    file->read = 0;
    file->properties.emptyLines = 0;
    file->properties.codeLines = 0;
    file->properties.comentedLines = 0;
}

/* Realiza a contagem de linhas de código, linhas em branco e linhas comentadas em um código fonte */
int countLines(void* files){

    FileElement* file = (FileElements)files; 
    FILE* userFile = fopen(file->path, "r");

    if(userFile){

        defaultFileElement(file); // Inicializando atributos da estrutura.

        char* fileLine = (char*)malloc(MAX_LINE_LENGHT); // Representa uma linha do arquivo.
        char miniBuffer[3]; // Vetor que armazena de dois em dois caracteres de uma linha ao percorrer a mesma.
        
        // Váriaveis para indicar o estado de uma linha ou caractere individual.
        status inComent = 0, // Indica que um caractere está em um comentário.
            hasInstruction, // Indica que há uma instrução (correta ou não) em uma linha.
            hadComent, // Indica que na linha havia um comentário.
            is_simpleComent; // Indica que havia um comentário simples na linha (//).
    
        rewind(userFile);

        // Armazenando uma linha
        while(fgets(fileLine, MAX_LINE_LENGHT, userFile)){ 
            linhasT++;
            if(feof(userFile)) // Caso seja a ultima linha, adiciona '\n' à linha para verificar corretamente seu conteúdo.
                strcat(fileLine, "\n"); 
            
            // Reinicializando variáveis.
            miniBuffer[0] = NULL_CHAR; 
            miniBuffer[1] = NULL_CHAR;
            hasInstruction = 0;
            
            if(inComent == 0)
                hadComent = 0;
            
            is_simpleComent = 0;
            
            // Percorrendo, por meio de um ponteiro, pela linha armazenada.
            for(char* cursor = fileLine; *cursor != NULL_CHAR; cursor++){
                
                miniBuffer[0] = *cursor; 
                miniBuffer[1] = *(cursor + 1);
            
                if(strncmp(OPEN_LONG_COMENT, miniBuffer, 2) == 0){ // Iniciando comentário longo.
                    inComent = 1;
                    cursor++; continue; /* Pula duas posições a partir da atual caso alguma dessas condições seja verdadeira. 
                                           O uso do continue é para o miniBuffer armazenar corretamente os próximos caracteres. */
                }
                else if(strncmp(CLOSE_LONG_COMENT, miniBuffer, 2) == 0 && inComent == 1){ // Terminando comentário longo.
                    inComent = 0;
                    hadComent = 1;
                    cursor++; continue;
                }
                else if(strncmp(SIMPLE_COMENT, miniBuffer, 2) == 0 && inComent == 0){ // Iniciando comentário simples.
                    is_simpleComent = 1;
                    cursor++; continue;
                }
                
                // Se algum caractere estiver fora de um comentário e for diferente destes abaixo, significa que é uma instrução.
                if(inComent == 0 && is_simpleComent==0 && (*cursor != ' ' &&  *cursor != '\t' && *cursor != '\n')){
                    hasInstruction = 1;
                }

                // Determinando a classificação da linha.
                if(*cursor == NEW_LINE){
                    if((inComent == 1 || is_simpleComent == 1 || hadComent == 1) && hasInstruction == 0)
                        file->properties.comentedLines++; // Linha de comentário.
                    else if(hasInstruction)
                        file->properties.codeLines++; // Linha de instrução.
                    else
                        file->properties.emptyLines++; // Linha vazia.
                }
            }
        }

        free(fileLine);
        file->read = 1; // Indica que o arquivo foi lido.
    }
    else{
        printf("\nErro ao abrir o arquivo \"%s\".\n", file->name);
        thrd_exit(EXIT_FAILURE);
    }

    fclose(userFile);
    thrd_exit(EXIT_SUCCESS);
}

// Aguarda que todas as threads criadas terminem de executar verificando o campo status de cada arquivo.
void waitThreads(const FileElements* files, int number_of_thrds){

    status allRead = 0; // Indica se todos arquivos ja foram lidos ou não.

    while(!allRead){

        allRead= 1;
        for(unsigned i = 0; i < number_of_thrds; i++)
            if(files[i]->read != 1)
                allRead = 0;
    }
}

// Copia src1 para str, e então concatena src2 e src3 à str, respectivamente.
void strCopyAndCat(char* str, size_t size, char* src1, char* src2, char* src3){
    snprintf(str, size, "%s%s%s", src1, src2, src3);
    /*strncpy(str, src1, size);
    strcat(str, src2);
    strcat(str, src3);*/
}

/* Lê um diretório recursivamente, ou seja, se houver um diretório dentro do diretório passado por parâmetro, a função será chamada novamente,
    até que não hajam mais diretórios. Além disso, lê também os arquivos, criando uma thread para executar a função countLines para cada um deles. */
void readDirectory(DirectoryName path, FileElements** ptr, unsigned* number_of_files){

    DIR* directory;
    struct dirent *dirFile; // Contém informaçoes sobre elementos de um diretório.
    char newPath[MAX_PATH_LENGHT]; // Armazena o caminho até o diretório, ou subdiretório a medida que a função se aprofunda.
    thrd_t tid;

    directory = opendir(path);

    if(directory){
        while((dirFile = readdir(directory)) != NULL) // Percorrendo o conteúdo do diretório.
            if(dirFile->d_type == DT_REG && isValidFile(dirFile->d_name)){ // Verificando se é um arquivo válido.

                if(*number_of_files == 0){ // Caso seja o primeiro arquivo a ser lido.
                    *ptr = (FileElements*)malloc(sizeof(FileElements)); // Cria um vetor de ponteiros.
                    (*ptr)[0] = (FileElements)malloc(sizeof(FileElement)); // Criando o primeiro ponteiro do vetor.
                }
                else{                                
                    *ptr = (FileElements*)realloc(*ptr, (*number_of_files+1) * sizeof(FileElements)); // Adiciona um novo ponteiro ao vetor.
                    (*ptr)[*number_of_files] = (FileElements)malloc(sizeof(FileElement)); /* Aloca espaço para este novo ponteiro. 
                                                                                           Obs: o número de arquivos vai acompanhar o índice).*/
                }

                strCopyAndCat((*ptr)[*number_of_files]->path, MAX_PATH_LENGHT, path, "/", dirFile->d_name); // Copia o caminho para o arquivo, para a estrutura FileElement.

                strncpy((*ptr)[*number_of_files]->name, dirFile->d_name, FILE_NAME_SIZE); // Copia o nome do arquivo para a estrutura FileElement. Será usado somente para exibir mensagens de erro, se houver.

                thrd_create(&tid, countLines, (*ptr)[*number_of_files]); // Criando thread para ler e calcular as linhas do arquivo.
                (*number_of_files)++;
                
            }else
                if(dirFile->d_type == DT_DIR) // Verificando se dirFile aponta para um diretório.
                    if(strcmp(dirFile->d_name, ".") != 0 && strcmp(dirFile->d_name, "..") != 0){
                        strCopyAndCat(newPath, MAX_PATH_LENGHT, path, "/", dirFile->d_name); // Armazena o caminho para um subdiretório.
                        readDirectory(newPath, ptr, number_of_files); // Chama novamente a função passando o novo diretório.
                    }     
        
        closedir(directory);
    }
}

/* Calcula o total de linhas de um único arquivo ou um diretório, criando threads (ou chamando função para tal) proporcionalmente à quantidade 
    de arquivos válidos, para que executem a função countLines. Após isso, obtem o resultado final, o tempo de execução, e exibe um relatório. */
int calculateTotalLines(int argType, CommandLineArgument commandLineArg){
    
    FileElements* files; // Vetor de ponteiros que apontam para uma estrutura que representa as propriedades de um arquivo.
    time_t start, end; // Tempo de início e fim da função.
   
    time(&start); // Armazenando tempo de início.

    TimeInfo programTime; // Armazena tanto o tempo de início e fim da função, quanto o tempo decorrido.
    unsigned number_of_files = 0; // Número de arquivos em um diretório.
    TotalOfLines total; // Resultado final da contagem de linhas do código fonte.
    thrd_t tid;
    int result;

    if(argType){ // argType = 1 representa um arquivo.
       
        number_of_files = 1;
        files = (FileElement**)malloc(sizeof(FileElement*));
        *files = (FileElement*)malloc(sizeof(FileElement));

        if(*files){
            strcpy((*files)->name, commandLineArg);
            strcpy((*files)->path, commandLineArg); // CORREÇÃO
            // Cria thread para executar a função countLines.
            thrd_create(&tid, countLines, *files);
            // Aguarda a finalização da thread única.
            thrd_join(tid, &result);
        }
    }
    else
        if(!argType){ // argType = 0 representa um diretório.

            files = NULL;
            readDirectory(commandLineArg, &files, &number_of_files);
            waitThreads(files, number_of_files); // Aguardando threads terminarem.
        }
        else{ 
            puts("\nErro desconhecido\n");
            exit(EXIT_FAILURE);
        }

    if(number_of_files == 0){ // Se nenhum arquivo válido for encontrado no diretório.
        puts("\nNenhum arquivo \".c\" foi encontrado!\n");
        exit(EXIT_FAILURE);
    }

    total = sumTotalLines(files, number_of_files); // Somando o total de linhas.

    time(&end); // Armazenando tempo final.
    setTimeValues(&programTime, &start, &end);
    showRelatory(total, number_of_files, programTime); // Exibe o relatório final.

    freeAllocatedMemory(files, &programTime, number_of_files); // Liberando memória alocada.
printf("Total: %d\n\n", linhasT);
    exit(EXIT_SUCCESS);
}

// Cria um novo processo e retorna o seu PID.
pid_t createChild(){

    pid_t pid = fork(); // Armazena PID do processo criado.

    if(pid == -1)
        puts("\nErro na criaçao do processo filho!\n");

    return pid;
}

// Verifica se o nome de um arquivo contém a extensão .c. Retorna TRUE caso sim e FALSE no caso contrário. 
int isValidFile(char* fileName){

    size_t size = strlen(fileName);

    if(fileName[size-2] == '.' && fileName[size-1] == 'c') // Verificando se os dois ultimos caracteres indicam um arquivo .c
        return TRUE;
    
    return FALSE;
}

/* Recebe o argumento passado pela linha de comando e verifica se é um arquivo ou diretório existente.
    Indica por meio de uma váriavel se o argumento representa um arquivo (valor 1), ou se representa um diretório (valor 0).
    Retorna FALSE caso o argumento não represente um arquivo ou diretório válido, e TRUE se obtiver sucesso */
int analyseArgs(const CommandLineArgument commandLineArg, int* argType){

    // Abre o argumento passado, como um diretório.
    DIR* directory = opendir(commandLineArg);

    // Verificando se é um diretório válido.
    if(directory){
        closedir(directory);
        *argType = 0;
    }
    else{ // Verifica se é um arquivo válido
        
        FILE* file = fopen(commandLineArg, "r");

        if(file){
            fclose(file);
            *argType = 1;

            if(!isValidFile(commandLineArg)){
                printf("\nO arquivo \"%s\" não é um arquivo .c!\n\n", commandLineArg);
                return FALSE;
            }
        }
        else{
            printf("\nO arquivo ou diretório \"%s\" não existe.\n\n", commandLineArg);
            return FALSE;
        }
    }
    return TRUE;
}

/* Inicia o programa recebendo um nome de arquivo ou diretório, fornecidos através da linha de comando do SO.
    Cria um processo filho para executar a contagem do total de linhas de um código fonte */
int clsc(int argc, char* argv[]){

    int argType = -1; // Variável que indica se é um arquivo ou um diretório

    if(argc == 2){
        
        // Verifica se o argumento passado como parâmetro excede o tamanho máximo aceitável.
        if(strlen(argv[1]) > ARGUMENT_LENGTH){
            printf("\nO tamanho do argumento excede o máximo de %d caracteres.\n", ARGUMENT_LENGTH);
            return EXIT_FAILURE;
        }

        // Armazena argumento passado.
        CommandLineArgument commandLineArg = (CommandLineArgument)malloc(ARGUMENT_LENGTH);
        strcpy(commandLineArg, argv[1]);

        if(analyseArgs(commandLineArg, &argType)){

            pid_t pid = createChild();
            int status;

            if(pid > 0){
                wait(&status);
            }
            else 
                if(pid == 0) // Executando instruções do processo filho.
                    calculateTotalLines(argType, commandLineArg);
        }

        free(commandLineArg);

        return EXIT_SUCCESS;
    }
    else
        if(argc > 2)
            printf("\nVocê deve fornecer somente um arquivo ou diretório!\n\n");
        else
            printf("\nVocê deve fornecer pelo menos um arquivo ou diretório!\n\n");

    return EXIT_FAILURE;
}

int main(int argc, char* argv[]){
    return clsc(argc, argv);   
}