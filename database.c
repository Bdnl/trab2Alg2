#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "misc.h"
#include "database.h"

// funções do misc
#define fopen _fopen
#define fclose _fclose

/**
 * Inicializa database com os valores padrões
 * @param db necessariamente não inicializada
 */
void initDB(database_t *db) {
	// zera completamente db
	memset(db, 0, sizeof(database_t));
	// abre o arquivo para garantir que ele vai existir
	fclose(fopen(DBFILENAME, "a"));
	fclose(fopen(IDXFILENAME, "a"));
	fclose(fopen(IDADEFILENAME, "a"));
	fclose(fopen(GENEROSFILENAME, "a"));
	fclose(fopen(GENEROSTABLEFILENAME, "a"));
	// carrega todos os idx
	// carrega idx principal
	FILE *fd = fopen(IDXFILENAME, "r+");
	// menos um porque o primeiro byte é o flag q diz se tá ordenado ou não
	db->num_id = (_file_size(fd) - 1) / sizeof(idx_id_t);
	db->ordenado = fgetc(fd);
	if(db->ordenado == -1) {
		// não foi inserido nenhum arquivo por enquanto, não há necessidade de carregar conteúdos em RAM
		// mas deve inserir a flag no arquivo
		rewind(fd);
		fputc(1, fd);
		db->ordenado = 1;
		db->num_id = 0;
		fclose(fd);
		return;
	}
	db->idx_id = malloc(db->num_id * sizeof(idx_id_t));
	fread(db->idx_id, sizeof(idx_id_t), db->num_id, fd);
	fclose(fd);
	// carrega os indices secundarios
	loadSecundaryIdx(db, &db->idx_idade, IDADEFILENAME);
	loadSecundaryIdx(db, &db->idx_genero, GENEROSFILENAME);
	// carrega a table de generos
	fd = fopen(GENEROSTABLEFILENAME, "r");
	db->genero_table.num_node = _file_size(fd) / sizeof(genero_node_t);
	db->genero_table.nodes = NULL;
	// leitura binária
	// fread(db->genero_table.nodes, sizeof(genero_node_t), db->genero_table.num_node, fd);
	// leitura IU
	char buffer[256];
	while(_fgets(buffer, 256, fd)) {
		// i vai de 0 até o número de generos-1
		int i = db->genero_table.num_node++;
		db->genero_table.nodes = realloc(db->genero_table.nodes, db->genero_table.num_node * sizeof(genero_node_t));
		char *igual = buffer; // procura o igual na frase
		while(*igual != '=') igual++;	
		*igual = 0;
		// copia o nome do genero para seu devido lugar
		strcpy(db->genero_table.nodes[i].str, buffer);
		igual++;
		db->genero_table.nodes[i].cod = atoi(igual);
		#ifdef DEBUG
			printf("Novo genero carregado: '%s' => %d\n", db->genero_table.nodes[i].str, db->genero_table.nodes[i].cod);
		#endif // DEBUG
	}
	fclose(fd);
}

/* ====================================================
   FUNÇÕES PARA ABRIR ARQUIVOS
   ==================================================== */

FILE *abrirArquivoDB(database_t *db, char *mode) {
	if(db->file_db != NULL) {
		return db->file_db;
	}
	db->file_db = fopen(DBFILENAME, mode);
	return db->file_db;
}

FILE *abrirArquivoIdx(database_t *db, char *mode) {
	if(db->file_id != NULL) {
		return db->file_id;
	}
	db->file_id = fopen(IDXFILENAME, mode);
	return db->file_id;
}

FILE *abrirArquivoIdade(database_t *db, char *mode) {
	if(db->file_idade != NULL) {
		return db->file_idade;
	}
	db->file_idade = fopen(IDADEFILENAME, mode);
	return db->file_idade;
}

FILE *abrirArquivoGeneros(database_t *db, char *mode) {
	if(db->file_generos != NULL) {
		return db->file_generos;
	}
	db->file_generos = fopen(GENEROSFILENAME, mode);
	return db->file_generos;
}

/* ====================================================
   FUNÇÕES PARA FECHAR ARQUIVO
   ==================================================== */

void fecharArquivoDB(database_t *db) {
	fclose(db->file_db);
	db->file_db = NULL;
}

void fecharArquivoIdx(database_t *db) {
	fclose(db->file_id);
	db->file_id = NULL;
}

void fecharArquivoIdade(database_t *db) {
	fclose(db->file_idade);
	db->file_idade = NULL;
}

void fecharArquivoGeneros(database_t *db) {
	fclose(db->file_generos);
	db->file_generos = NULL;
}

/**
 * inicializa um nó de índices secundarios
 * @param node necessariamente não inicializada
 */
void initSecundaryNode(secundary_node_t *node) {
	memset(node, 0, sizeof(secundary_node_t));
}

/**
 * cria um novo índice secundário
 * @param db        previamente inicializada
 * @param secundary previamente inicializada
 * @param cod       cod do novo índice
 * @param id        id que apontará o índice
 * @param file_name nome do arquivo principal de índice secundário
 */
void newSecundaryIdx(database_t *db, secundary_t *secundary, int cod, id_type id, char *file_name) {
	#ifdef DEBUG
		printf("Inserindo no secundario: %d => %d\n", cod, id);
	#endif // DEBUG
	// obtem a nova posicao do nó
	int pos_node = secundary->num_node;
	// cria um novo nó
	secundary->num_node++;
	secundary->nodes = realloc(secundary->nodes, secundary->num_node * sizeof(secundary_node_t));
	initSecundaryNode(secundary->nodes + pos_node);
	// seta as nocas condições
	secundary->nodes[pos_node].cod = cod;
	secundary->nodes[pos_node].id = id;
	// atualiza todo o arquivo de idx
	FILE *fd = fopen(file_name, "a");
	fwrite(secundary->nodes+pos_node, sizeof(secundary_node_t), 1, fd);
	fclose(fd);
}

/**
 * carrega para a memória RAM um índice secundário
 * @param db        previamente inicializada
 * @param secundary não necessariamente inicializada, será alterada
 * @param file_name nome do arquivo principal de índice secundário
 */
void loadSecundaryIdx(database_t *db, secundary_t *secundary, char *file_name) {
	// carregando o arquivo principal
	FILE *fd_principal = fopen(file_name, "r");

	secundary->num_node = _file_size(fd_principal) / sizeof(secundary_node_t);
	// aloca
	secundary->nodes = malloc(secundary->num_node * sizeof(secundary_node_t));
	// carrega o principal
	fread(secundary->nodes, sizeof(secundary_node_t), secundary->num_node, fd_principal);

	fclose(fd_principal);
}

/**
 * libera um índice secundário
 * @param secundary previamente inicializado
 */
void freeSecundaryIdx(secundary_t *secundary) {
	free(secundary->nodes);
}

/**
 * libera e encerra o db
 * @param db previamente inicializado
 */
void closeDB(database_t *db) {
	// fecha todos os arquivos
	fecharArquivoDB(db);
	fecharArquivoIdx(db);
	fecharArquivoIdade(db);
	fecharArquivoGeneros(db);

	// libera na memória
	freeSecundaryIdx(&db->idx_idade);
	freeSecundaryIdx(&db->idx_genero);
	free(db->idx_id);
}

/* ====================================================
   FUNCOES DE ORDENACAO
   ==================================================== */
int qsort_secundary(const void *p1, const void *p2) {
	secundary_node_t *lt = (secundary_node_t *) p1;
	secundary_node_t *gt = (secundary_node_t *) p2;
	if(lt->cod < gt->cod) {
		return -1;
	} else if(lt->cod > gt->cod) {
		return 1;
	}
	return 0;
}

int qsort_idx(const void *p1, const void *p2) {
	idx_id_t *lt = (idx_id_t *) p1;
	idx_id_t *gt = (idx_id_t *) p2;
	if(lt->id < gt->id) {
		return -1;
	} else if(lt->id > gt->id) {
		return 1;
	}
	return 0;
}

/**
 * funcao que retorna se tem registro ou não dentro do programa
 * @param  db previamente inicializado
 * @return    verdadeiro se tem registros dentro do arquivo
 */
bool temRegistro(database_t *db) {
	return db->num_id;
}

void ordenarSecundario(database_t *db, secundary_t *secundary, FILE *fd) {
	qsort(secundary->nodes, secundary->num_node, sizeof(secundary_node_t), qsort_secundary);
	fwrite(secundary->nodes, sizeof(secundary_node_t), secundary->num_node, fd);
}

void ordenarDB(database_t *db) {
	if(db->ordenado) {
		// já está ordenado
		return ;
	}
	if(!temRegistro(db)) {
		// nao há arquivos para serem ordenados
		return ;
	}
	// ordena idx_id_t
	qsort(db->idx_id, db->num_id, sizeof(idx_id_t), qsort_idx);

	// atualiza os arquivos
	FILE *fd = abrirArquivoIdx(db, "w");
	fputc(1, fd); // flag de ordenado
	fwrite(db->idx_id, sizeof(idx_id_t), db->num_id, fd);
	fecharArquivoIdx(db);
	fd = abrirArquivoIdade(db, "w");
	ordenarSecundario(db, &db->idx_idade, fd);
	fecharArquivoIdade(db);
	fd = abrirArquivoGeneros(db, "w");
	ordenarSecundario(db, &db->idx_genero, fd);
	fecharArquivoGeneros(db);
}

/**
 * set na flag do arquivo idx, essa flag diz se os indices estao atualizados ou não
 * @param db   inicializado previamente
 * @param flag nova flag
 */
void setFlag(database_t *db, char flag) {
	if(db->ordenado == flag) {
		// já é igual, não precisa fazer nada
		return ;
	}
	if(flag == 1) {
		// se for para ordenar, que ordene!!
		ordenarDB(db);
	}
	#ifdef DEBUG
		printf("Modificando a FLAG para '%d'\n", flag);
	#endif // DEBUG
	// seta a nova flag
	db->ordenado = flag;
	FILE *fd = abrirArquivoIdx(db, "r+");
	// insere no arquivo idx o status da flag
	fputc(flag, fd);
	fecharArquivoIdx(db);
}