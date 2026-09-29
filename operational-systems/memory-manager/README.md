
# Simulador de Paginação - Projeto de Sistemas Operacionais

Este projeto simula um sistema básico de gerenciamento de memória com paginação, desenvolvido como parte da disciplina de **Sistemas Operacionais** na minha graduação. O objetivo é interpretar arquivos de instruções de tarefas, alocar páginas de memória para código e dados, registrar acessos e exibir um relatório detalhado com informações de mapeamento entre endereços lógicos e físicos.

---

## 📂 Estrutura do Projeto

```
├── gm.c              # Implementação principal do simulador
├── gm.h              # Definições e estruturas de dados
└── samples/           # Arquivos de teste 
    ├── t1.tsk        
    └── t2.tsk        
```

---

## ⚙️ Compilação

Use `gcc` para compilar:

```bash
gcc gm.c -o gm
```

---

## 🚀 Execução

```bash
./gm t1 t2 ...
```

> **Importante:** não inclua a extensão `.tsk` ao passar os nomes dos arquivos, o programa a adiciona automaticamente.
> Além disso, presume-se que estes arquivos estão no diretório [`samples/`](./samples/).

---

## 🧠 Funcionamento

Cada arquivo `.tsk` representa uma tarefa e deve seguir o seguinte formato:

```txt
#T=512
vetor1 new 100
vetor1[10]
vetor1[99]
```

- A primeira linha define o tamanho do segmento de código em bytes (deve obrigatóriamente ser a primeira linha).
- Linhas com `new` alocam vetores no segmento de dados.
- Linhas no formato `vetor[posicao]` representam acessos a memória.

---

## 🧮 Como funciona a simulação

### 1. **Paginação**

- Tamanho da página: `512 bytes`
- Memória máxima por tarefa: `4096 bytes`
- Total de páginas físicas disponíveis: `88` (de PF 40 a 127)

### 2. **Etapas**

 **Inicialização:** Vetor de páginas livres é populado com as páginas físicas disponíveis.
 **Processamento da Tarefa:**
   - Lê o segmento de código e calcula quantas páginas serão necessárias.
   - Interpreta instruções de alocação e acesso a vetores.
   - Verifica erros como: alocação duplicada, estouro de memória, acesso inválido, etc.
 **Alocação:** As páginas lógicas são mapeadas para páginas físicas.
 
 **Relatório:** O simulador imprime:
   - Tamanhos e mapeamentos de memória
   - Endereços lógicos e físicos de vetores e acessos
   - Tabela de páginas

---

## 📋 Exemplo de Saída

```txt
- Tarefa: t4
     - Memória
          Número de páginas lógicas = 2

          - vetor1
          Endereços Lógicos = 512 a 611 ( 1 : 0 a 1 : 99 )
          Endereços Físicos = 20992 a 21091 ( 41 : 0 a 41 : 99 )

          - vetor1[10]
          Endereço Lógico = 1 : 10
          Endereço Físico = 41 : 10

          - vetor1[99]
          Endereço Lógico = 1 : 99
          Endereço Físico = 41 : 99

          - Tabela de Páginas
               PL 0 (0 a 511) --> PF 40 (20480 a 20991)
               PL 1 (512 a 1023) --> PF 41 (20992 a 21503)
               ...
```

---

## 🛠️ Possíveis Erros a Serem Detectados

- Tarefa excede a memória permitida (`4096 bytes`)
- Tentativa de alocar um vetor já existente
- Acesso a uma variável inexistente
- Acesso fora dos limites do vetor
- Instrução com formato inválido

---

## 📎 Amostras de Entrada

Os arquivos `.tsk` de exemplo estão na pasta [`samples/`](./samples/), com tarefas prontas para teste.

---

## 🧑‍💻 Autor

Projeto desenvolvido por Vinícius Daniel da Silva.

---

## 📄 Licença

Este projeto é apenas para fins educacionais.



