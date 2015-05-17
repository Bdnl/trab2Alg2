#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "misc.h"
#include "database.h"

// troca o fopen
#define fopen _fopen
#define malloc _malloc
#define realloc _realloc
#define calloc _calloc

void initDB(database_t *db) {
	// zera completamente db
	memset(db, 0, sizeof(database_t));
	// abre o arquivo para garantir que ele vai existir
	fclose(fopen(DBFILENAME, "a"));
	fclose(fopen(IDXFILENAME, "a"));
	fclose(fopen(IDADEFILENAME, "a"));
	fclose(fopen(IDADELISTFILENAME, "a"));
	fclose(fopen(GENEROSFILENAME, "a"));
	fclose(fopen(GENEROSLISTFILENAME, "a"));
	fclose(fopen(GENEROSTABLEFILENAME, "a"));
	// carrega todos os idx
	// carrega idx principal
	FILE *fd = fopen(IDXFILENAME, "r");
	db->num_id = _file_size(fd) / sizeof(idx_id_t);
	db->idx_id = malloc(db->num_id * sizeof(idx_id_t));
	fread(db->idx_id, sizeof(idx_id_t), db->num_id, fd);
	fclose(fd);
	// carrega os indices secundarios
	loadSecundaryIdx(db, &db->idx_idade, IDADEFILENAME, IDADELISTFILENAME);
	loadSecundaryIdx(db, &db->idx_genero, GENEROSFILENAME, GENEROSLISTFILENAME);
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

FILE *abrirArquivoGenerosTable(database_t *db, char *mode) {
	if(db->file_generos_table != NULL) {
		return db->file_generos_table;
	}
	db->file_generos_table = fopen(GENEROSFILENAME, mode);
	return db->file_generos_table;
}

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

void fecharArquivoGenerosTable(database_t *db) {
	fclose(db->file_generos_table);
	db->file_generos_table = NULL;
}

int getNodePos(secundary_t *secundary, int cod) {
	int i;
	for(i=0; i<secundary->num_node; i++) {
		if(secundary->nodes[i].cod == cod) {
			return i;
		}
	}
	return -1;
}

void zerarSecundaryNode(secundary_node_t *node) {
	memset(node, 0, sizeof(secundary_node_t));
	node->last_pos = -1;
}

void newSecundaryIdx(database_t *db, secundary_t *secundary, int cod, id_type id, char *file_principal, char *file_list) {
	#ifdef DEBUG
		printf("Inserindo no secundario: %d => %d\n", cod, id);
	#endif // DEBUG
	// obtem a posicao do nó
	int pos_node = getNodePos(secundary, cod);
	if(pos_node == -1) {
		// cria um novo nó
		secundary->num_node++;
		secundary->nodes = realloc(secundary->nodes, secundary->num_node * sizeof(secundary_node_t));
		// obtem a nova posicao do nó
		pos_node = secundary->num_node - 1;
		zerarSecundaryNode(secundary->nodes + pos_node);
		secundary->nodes[pos_node].cod = cod;
	}
	secundary_node_t *node = secundary->nodes + pos_node;
	// aloca um novo espaço na lista para o novo id
	secundary->num_list++;
	secundary->pos_list = realloc(secundary->pos_list, secundary->num_list * sizeof(pos_list_t));
	// insere o ID
	pos_list_t *pos_list = secundary->pos_list + secundary->num_list - 1;
	pos_list->id = id;
	pos_list->next_pos = node->last_pos;
	node->last_pos = secundary->num_list - 1;
	// atualiza todo o arquivo de idx
	FILE *fd = fopen(file_principal, "w");
	fwrite(secundary->nodes, sizeof(secundary_node_t), secundary->num_node, fd);
	fclose(fd);
	// agora insere no arquivo de lista, no final
	fd = fopen(file_list, "a");
	fwrite(pos_list, sizeof(pos_list_t), 1, fd);
	fclose(fd);
}

void loadSecundaryIdx(database_t *db, secundary_t *secundary, char *file_principal, char *file_list) {
	// carregando o arquivo principal
	FILE *fd_principal = fopen(file_principal, "r");
	FILE *fd_list = fopen(file_list, "r");

	secundary->num_node = _file_size(fd_principal) / sizeof(secundary_node_t);
	secundary->num_list = _file_size(fd_list) / sizeof(pos_list_t);
	// aloca
	secundary->nodes = malloc(secundary->num_node * sizeof(secundary_node_t));
	secundary->pos_list = malloc(secundary->num_list * sizeof(pos_list_t));
	// carrega o principal
	fread(secundary->nodes, sizeof(secundary_node_t), secundary->num_node, fd_principal);
	// carrega a lista
	fread(secundary->pos_list, sizeof(pos_list_t), secundary->num_list, fd_list);

	fclose(fd_list);
	fclose(fd_principal);
}

void freeSecundaryIdx(secundary_t *secundary) {
	free(secundary->nodes);
	free(secundary->pos_list);
}

void closeDB(database_t *db) {
	// fecha todos os arquivos
	fecharArquivoDB(db);
	fecharArquivoIdx(db);
	fecharArquivoIdade(db);
	fecharArquivoGeneros(db);
	fecharArquivoGenerosTable(db);

	freeSecundaryIdx(&db->idx_idade);
	freeSecundaryIdx(&db->idx_genero);
	free(db->idx_id);
}