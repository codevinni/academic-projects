#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gm.h"


/* Inicializa o vetor de páginas livres. Preenche este vetor com os identificadores das páginas físicas disponíveis, 
    ou seja, de PAGES_BEGIN a PAGES_TOTAL. */
void inicializarPaginas(int* paginasLivres, int* numeroPaginasLivres){
    
    *numeroPaginasLivres = PAGES_TOTAL;

    for(int index = 0; index < PAGES_TOTAL; index++)
        paginasLivres[index] = PAGES_BEGIN + index;
}

/* Aloca dinamicamente um vetor de páginas e preenche o mesmo com as páginas disponíveis no vetor paginasLivres. 
    Remove as páginas utilizadas do vetor de páginas livres. 
    Retorna TRUE caso a alocação seja bem sucedida e FALSE caso contrário. */
int alocarPaginas(int num, int** paginas, int* paginasLivres, int* numeroPaginasLivres){
    
    // Verificando se há páginas suficientes
    if(*numeroPaginasLivres < num)
        return FALSE;

    // Armazena os identificadores das páginas alocadas.
    *paginas = malloc(num * sizeof(int));

    if(!(*paginas))
        return FALSE;

    for(int index = 0; index < num; index++) 
        (*paginas)[index] = paginasLivres[index];
    
    // Atualizando o vetor
    memmove(paginasLivres, paginasLivres + num, (*numeroPaginasLivres - num) * sizeof(int));
    *numeroPaginasLivres-=num;

    return TRUE;
}

// Verifica se um vetor já foi criado em uma tarefa. Retorna o índice onde foi encontrado ou NOT_FOUND caso não tenha sido localizado.
int encontrarVetor(const Tarefa* tarefa, const char* nomeVetor){

    for(int index = 0; index < tarefa->numeroVetores; index++) 
        if(strcmp(tarefa->vetores[index].nome, nomeVetor) == 0)
            return index;

    return NOT_FOUND;
}

/* Verifica se uma instrução contendo o comando "new" tem o formato correto, para alocar um novo vetor. Caso tenha, verifica se o tamanho desse vetor
    a ser criado excede o tamanho máximo para uma tarefa. Verifica também se o novo vetor já existe na tarefa.
    Retorna TRUE e armazena as informações do vetor em caso de sucesso, e retorna FALSE em caso de falha. */
int instrucaoAlocarVetor(Tarefa* tarefa, const char* linha, const char* nomeTarefa){
    
    Name nomeVetor;
    Size tamanhoVetor;

    // Verifica se é um comando válido, além de armazenar informações sobre o vetor criado.
    if(sscanf(linha, "%s new %d", nomeVetor, &tamanhoVetor) != 2){
        printf("\nA tarefa %s não será executada, pois tem instruções diferentes do tipo 1 e 2.\n", nomeTarefa);
        return FALSE;
    }
    if((tarefa->memoriaCodigo + tarefa->memoriaDados + tamanhoVetor) > MAX_TASK_MEM){ // Verificando se excede o máximo de memória
        printf("\nA tarefa %s excedeu o limite de memória de %dKB\n", nomeTarefa, MAX_TASK_MEM/1024);
        return FALSE;
    }
    if(encontrarVetor(tarefa, nomeVetor) != NOT_FOUND){ // Verifica se o "novo" vetor já existe.
        printf("\nA tarefa %s tentou alocar memória novamente para um vetor que já existe (%s).\n", nomeTarefa, nomeVetor);
        return FALSE;
    }

    strcpy(tarefa->vetores[tarefa->numeroVetores].nome, nomeVetor);
    tarefa->vetores[tarefa->numeroVetores].tamanho = tamanhoVetor;
    tarefa->vetores[tarefa->numeroVetores].inicio = tarefa->memoriaDados;
    tarefa->memoriaDados += tamanhoVetor;
    tarefa->numeroVetores++;

    return TRUE;
}

/* Verifica se uma instrução representa acesso a um vetor. Verifica também se o vetor existe na estrutura 
    da tarefa e se a posição acessada está dentro dos limites do vetor. 
    Retorna TRUE e registra o acesso em caso de sucesso, e retorna FALSE em caso de falha. */
int instrucaoAcessarVetor(Tarefa* tarefa, const char* linha, const char* nomeTarefa){
   
    Name nomeVetor;
    int posicao;

    if(sscanf(linha, "%[^[][%d]", nomeVetor, &posicao) != 2){
        printf("\nA tarefa %s não será executada, pois tem instruções diferentes do tipo 1 e 2.\n", nomeTarefa);
        return FALSE;
    }

    int vetorEncontrado = encontrarVetor(tarefa, nomeVetor);

    // Verificando se o vetor existe.
    if(vetorEncontrado != NOT_FOUND){
        if(posicao < 0 || posicao >= tarefa->vetores[vetorEncontrado].tamanho) { // Verifica se o acesso é feito a umaa posição válida.
            printf("\nA tarefa %s tentou realizar um acesso inválido à memória %s[%d].\n", nomeTarefa, nomeVetor, posicao);
            return FALSE;
        }
    }
    else{
        printf("\nA tarefa %s tentou acesso a uma variável que não existe (%s).\n", nomeTarefa, nomeVetor);
        return FALSE;
    }
    
    strcpy(tarefa->acessos[tarefa->numeroAcessos].nomeVetor, nomeVetor);
    tarefa->acessos[tarefa->numeroAcessos].posicao = posicao;
    tarefa->numeroAcessos++;

    return TRUE;
}

// Gera e exibe um relatório contendo as informações de uma tarefa.
void gerarRelatorio(const Tarefa* tarefa, const char* nomeTarefa){
    
    int totalPaginas = tarefa->paginasCodigo + tarefa->paginasDados;

    printf(BOLD "\n- Tarefa: %s\n", nomeTarefa);
    printf("     - Memória\n" RESET_BOLD);
    printf("          Número de páginas lógicas = %d\n", totalPaginas);

    // Vetores
    for(int index = 0; index < tarefa->numeroVetores; index++){

        const Vetor *v = &tarefa->vetores[index]; // Para simpllificar.

        int inicioEnderecosLogicos = tarefa->paginasCodigo * PAGE_SIZE + v->inicio,
            fimEnderecosLogicos = inicioEnderecosLogicos + v->tamanho - 1;
        printf("\n          - %s\n", v->nome);
        printf("          Endereços Lógicos = %d a %d ", inicioEnderecosLogicos, fimEnderecosLogicos);

        int paginaLogicaInicial = inicioEnderecosLogicos / PAGE_SIZE, 
            paginaLogicaFinal = fimEnderecosLogicos / PAGE_SIZE,
            posLogicaInicial = inicioEnderecosLogicos % PAGE_SIZE,
            posLogicaFinal = fimEnderecosLogicos % PAGE_SIZE;

        printf("( %d : %d a %d : %d )\n", paginaLogicaInicial, posLogicaInicial, paginaLogicaFinal, posLogicaFinal);

        int paginaFisicaInicial = tarefa->tabelaPaginas[paginaLogicaInicial],
            paginaFisicaFinal = tarefa->tabelaPaginas[paginaLogicaFinal];
        int inicioEnderecosFisicos =  paginaFisicaInicial * PAGE_SIZE + posLogicaInicial,
            fimEnderecosFisicos = paginaFisicaFinal * PAGE_SIZE + posLogicaFinal;
        printf("          Endereços Físicos = %d a %d ", inicioEnderecosFisicos, fimEnderecosFisicos);
        
        printf("( %d : %d a %d : %d )\n", paginaFisicaInicial, posLogicaInicial, paginaFisicaFinal, posLogicaFinal);
    }

    // Acessos aos vetores
    for(int index = 0; index < tarefa->numeroAcessos; index++){

        AcessoMemoria acesso = tarefa->acessos[index];
        Vetor vetor;

        for(int j = 0; j < tarefa->numeroVetores; j++){
            if(strcmp(tarefa->vetores[j].nome, acesso.nomeVetor) == 0){
                vetor = tarefa->vetores[j];
                break;
            }
        }

        int enderecoLogico = tarefa->paginasCodigo * PAGE_SIZE + vetor.inicio + acesso.posicao,
            paginaLogica = enderecoLogico / PAGE_SIZE,
            posicaoLogica = enderecoLogico % PAGE_SIZE,
            paginaFisica = tarefa->tabelaPaginas[paginaLogica];
        printf("\n          - %s[%d]\n", acesso.nomeVetor, acesso.posicao);
        printf("          Endereço Lógico = %d : %d\n", paginaLogica, posicaoLogica);
        printf("          Endereço Físico = %d : %d\n", paginaFisica, posicaoLogica);
    }

    printf("\n          - Tabela de Páginas\n");
    for(int index = 0; index < totalPaginas; index++){

        int primeiroEnderecoLogico = index * PAGE_SIZE,
            ultimoEnderecoLogico = (index + 1) * PAGE_SIZE - 1,
            paginaFisica = tarefa->tabelaPaginas[index],
            primeiroEnderecoFisico = paginaFisica * PAGE_SIZE,
            ultimoEnderecoFisico = primeiroEnderecoFisico + PAGE_SIZE - 1;
        printf("               PL %d (%d a %d) --> PF %d (%d a %d)\n", index, primeiroEnderecoLogico, ultimoEnderecoLogico, paginaFisica, primeiroEnderecoFisico, ultimoEnderecoFisico);
    }
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

/* Processa a tarefa, lendo o arquivo, interpretando suas instruções e gerando um relatório sobre o mesmo.
    Retorna TRUE se a tarefa for processada com sucesso, ou um código de erro em caso de falha. */
int processarTarefa(const char* nomeTarefa, int* paginasLivres, int* numeroPaginasLivres){
    
    Name nomeArquivo;
    snprintf(nomeArquivo, NAME_LEN, "../samples/%s.tsk", nomeTarefa);

    FILE *arquivo = fopen(nomeArquivo, "r");
    if(!arquivo){
        printf("\nErro ao abrir o arquivo %s\n", nomeArquivo);
        return FILE_ERROR;
    }

    // Inicializa todos os campos com 0
    Tarefa tarefa = {0};
    char linha[LINE_LEN];

    // Lê a primeira linha
    if(!fgets(linha, sizeof(linha), arquivo)){
        printf("\nErro ao ler o arquivo %s\n", nomeArquivo);
        fclose(arquivo);
        return FILE_ERROR;
    } // Verifica se essa linha tem o formato esperado
    if(sscanf(linha, "#T=%d", &tarefa.memoriaCodigo) != 1){
        printf("\nA tarefa %s tem formato inválido (não começa com os caracteres #T=<tamanho>).\n", nomeTarefa);
        fclose(arquivo);
        return FILE_ERROR;
    }
    else
        if(tarefa.memoriaCodigo > MAX_TASK_MEM){
            printf("\nA tarefa %s excedeu o limite de memória de %dKB.\n", nomeTarefa, MAX_TASK_MEM/1024);
            return MEMORY_ERROR;
        }

    // Lê as outras linhas do arquivo
    while(fgets(linha, sizeof(linha), arquivo)){

        substituirCaractere(linha, '\n', '\0');

        if(linha[0] == '\0') // Permite linhas vazias no arquivo
            continue;

        if(strstr(linha, "new") != NULL){ // Procura por new primeiro e verifica se é uma instrução de alocação
            if(!instrucaoAlocarVetor(&tarefa, linha, nomeTarefa)){ 
                fclose(arquivo);
                return INSTRUCTION_ERROR;
            }
        } 
        else // Verifica se é uma instruçao de acesso
            if(!instrucaoAcessarVetor(&tarefa, linha, nomeTarefa)){
                fclose(arquivo);
                return INSTRUCTION_ERROR;
            }
    }
    fclose(arquivo);

    tarefa.paginasCodigo = (tarefa.memoriaCodigo + PAGE_SIZE - 1) / PAGE_SIZE;
    tarefa.paginasDados = (tarefa.memoriaDados + PAGE_SIZE - 1) / PAGE_SIZE;
    int totalPaginas = tarefa.paginasCodigo + tarefa.paginasDados;

    if(totalPaginas > MAX_LOG_PAGES){
        printf("\nA tarefa %s excedeu o limite de %d páginas lógicas.\n", nomeTarefa, MAX_LOG_PAGES);
        return MEMORY_ERROR;
    }

    int *paginasAlocadas;
    if(!alocarPaginas(totalPaginas, &paginasAlocadas, paginasLivres, numeroPaginasLivres)){
        printf("\nMemória física insuficiente para a tarefa %s.\n\n", nomeTarefa);
        return MEMORY_ERROR;
    }

    tarefa.paginasFisicas = paginasAlocadas;
    for(int index = 0; index < totalPaginas; index++) 
        tarefa.tabelaPaginas[index] = paginasAlocadas[index];

    gerarRelatorio(&tarefa, nomeTarefa);
    free(paginasAlocadas);

    return TRUE;
}

// Inicia o sistema de gerencia de memória, processando cada tarefa passada como argumento através da linha de comando do SO.
int gm(int argc, char* tarefas[]){
    
    if(argc <= 1)
        puts("\nForneça ao menos um arquivo de instruções.\n");
    else
        if(argc > 5) 
            puts("\nForneça no máximo 4 arquivos de instruções.\n");
        else{

            // Representa a relação de páginas físicas do sistema.
            int paginasLivres[PAGES_TOTAL];
            int numeroPaginasLivres;

            inicializarPaginas(paginasLivres, &numeroPaginasLivres);

            for(int index = 1; index < argc; index++)
                processarTarefa(tarefas[index], paginasLivres, &numeroPaginasLivres);

            puts(""); // Pular uma linha no final.

            return EXIT_SUCCESS;
        }

    return EXIT_FAILURE;
}

int main(int argc, char* argv[]){
    return gm(argc, argv);
} 