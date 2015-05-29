#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "misc.h"
#include "registro.h"

/**
 * converte de registro_t para uma string
 * @param  reg    previamente inicializado
 * @param  buffer string que será alterada
 * @return        strlen(buffer)
 */
uint registroToBuffer(registro_t *reg, char *buffer) {
	return snprintf(buffer, REGSIZE, "%d|%s|%d|%c|%s|%d|",
		reg->id, reg->nome, reg->idade, reg->sexo, reg->generos, reg->tu);
}

/**
 * de buffer para registro_t
 * @param buffer string com o buffer do tipo ID|Nome|...
 * @param reg    nao necessariamente inicializado
 */
void bufferToReg(char *buffer, registro_t *reg) {
	reg->id = atoi(buffer);
	if(reg->id == 0) {
		// nesse ponto, o programa sabe que o registro foi removido
		// pq o primeiro char de id é *
		return ;
	}
	// procura o '|'
	while(*buffer != '|') buffer++;
	buffer++; // agora vale o primeiro char de nome
	// posicao da primeira letra do nome
	char *start = buffer;
	while(*buffer != '|') buffer++;
	// se eu quiser dar um malloc
	// reg->nome = malloc(buffer - start + 1);
	reg->nome[buffer - start] = '\0';
	memcpy(reg->nome, start, buffer-start);
	buffer++; // agora vale o primeiro char de idade
	reg->idade = atoi(buffer);
	while(*buffer != '|') buffer++;
	buffer++; // agora vale o primeiro char de sexo
	reg->sexo = *buffer;
	buffer+=2;
	start = buffer;
	while(*buffer != '|') buffer++;
	// caso eu queira um malloc
	// reg->generos = malloc(buffer - start + 1);
	reg->generos[buffer - start] = '\0';
	memcpy(reg->generos, start, buffer-start);
	buffer++;
	reg->tu = atoi(buffer);
}

/**
 * criar um novo registro
 * @param  db  previamente inicializada
 * @param  reg conteúdo do registro novo
 * @return     retorna a posição do primeiro byte do registro ou EOF caso erro
 */
offset_t novoRegistro(database_t *db, registro_t *reg) {
	// opcao 1
	#ifdef DEBUG
		printf("Inserindo um novo registro, Nome: '%s'\n", reg->nome);
	#endif // DEBUG
	// atualiza o arquivo principal
	if(pesquisarRegistro(db, reg->id) != EOF) {
		// isto indica que o registro já existe
		#ifdef DEBUG
			printf("Registro ID: '%d' já existe!\n", reg->id);
		#endif // DEBUG
		return EOF;
	}
	// volta a flag para zero
	setFlag(db, 0);
	FILE *fd = abrirArquivoDB(db, "a");
	// offset indica o valor do primeiro caracter no arquivo
	offset_t offset = ftell(fd) + 1;
	char buffer[REGSIZE];
	uint8_t buffer_size;
	buffer_size = registroToBuffer(reg, buffer);
	fputc(buffer_size, fd);
	fwrite(buffer, sizeof(char), buffer_size, fd);
	fecharArquivoDB(db);
	// atualiza o registro secundario idade
	newSecundaryIdx(db, &db->idx_idade, reg->idade, reg->id, IDADEFILENAME);
	// atualiza o registro secundario de generos
	int i = 0;
	while(reg->generos[i]) {
		newSecundaryIdx(db, &db->idx_genero, reg->generos[i], reg->id, GENEROSFILENAME);
		i++;
	}
	// atualiza o indice primario
	db->num_id++;
	db->idx_id = realloc(db->idx_id, db->num_id * sizeof(idx_id_t));
	idx_id_t *idx_id = db->idx_id + db->num_id - 1;
	idx_id->id = reg->id;
	idx_id->offset = offset;
	// abre o arquivo de idx principal
	// insere no final
	fd = abrirArquivoIdx(db, "a");
	fwrite(idx_id, sizeof(idx_id_t), 1, fd);
	fecharArquivoIdx(db);
}

/**
 * lê um registro do arquivo
 * o cursor do arquivo deve estar apontado para a posição anterior do primeiro byte do registro
 * @param  db  previamente inicializada
 * @param  reg nao necessariamente inicializado, será alterada
 * @return     a pos~ição do primeiro byte do registro ou EOF caso erro
 */
offset_t lerRegistro(database_t *db, registro_t *reg) {
	FILE *fd = db->file_db;
	int regsize = fgetc(fd);
	if(regsize == EOF) {
		// não há mais registros no arquivos
		return EOF;
	}
	// retorna a posição do primeiro caractere do registro
	offset_t result = ftell(fd);
	char *buffer = malloc(regsize+1); // mais um para comportar o \0
	buffer[regsize] = 0;
	fread(buffer, sizeof(char), regsize, fd);
	bufferToReg(buffer, reg);
	free(buffer);
	return result;
}

/**
 * pesquisa o registro com base nos índices carregados em memória sequencialmente
 * @param  db previamente inicializada
 * @param  id valor do id procurado
 * @return    retorna a posicao do arquivo do primeiro byte do id
 */
offset_t pesquisarRegistro(database_t *db, id_type id) {
	// opcao 3
	if(db->ordenado == 0) {
		// pesquisa sequencial
		int i;
		for(i=0; i<db->num_id; i++) {
			if(db->idx_id[i].id == id) {
				return db->idx_id[i].offset;
			}
		}
		// nao encontrado	
		return -1;
	} else {
		// pesquisa binária
		#ifdef DEBUG
			printf("Iniciando a pesquisa binária, procurando por ID: '%d'\n", id);
		#endif // DEBUG
		int beg = 0;
		int end = db->num_id-1;
		while(beg <= end) {
			int mid = (beg + end) / 2;
			#ifdef DEBUG
				printf("Testando o ID: '%d'\n", db->idx_id[mid].id);
			#endif // DEBUG
			if(db->idx_id[mid].id == id) {
				// encontrou
				return db->idx_id[mid].offset;
			} else if(id < db->idx_id[mid].id) {
				// está para a esquerda
				end = mid-1;
			} else if(id > db->idx_id[mid].id) {
				// está para a direita
				beg = mid+1;
			}
		}
		// caso se não encontrar
		return -1;
	}
}

/**
 * remove o registro tanto em memória, quanto no arquivo principal e nos de índex
 * @param  db previamente inicializada
 * @param  id do registro que será removido
 * @return    true caso seja verdadeiro
 */
bool removerRegistro(database_t *db, id_type id) {
	// opcao 2
	// ordena antes de remover
	setFlag(db, 1);
	int i;
	offset_t pos = pesquisarRegistro(db, id);
	if(pos == EOF) {
		return false;
	}
	// volta a flag para zero
	FILE *fd = abrirArquivoDB(db, "r+");
	fseek(fd, pos, SEEK_SET);
	// insere o * indicando q o registro está apagado
	fputc('*', fd);

	//Remove nos arquivos de índex (memória e arquivo)
	abrirArquivoIdx(db, "r+b");
	abrirArquivoGeneros(db, "r+b");
	abrirArquivoIdade(db, "r+b");

	for(i = 0; i < db->num_id; i++) {
		if (db->idx_id[i].id == id) {
			//Apaga na memória
			db->idx_id[i].id = 0; 

			//Apaga no arquivo de índex primário
			fseek(db->file_id, (i - 1) * sizeof(idx_id_t), SEEK_SET);
			fwrite(db->idx_id + i, sizeof(idx_id_t), 1, db->file_id);
			break;
		}
	}

	//Remove o arquivo de índex secundário de gênero (memória e arquivo)
	for (i = 0; i < db->idx_genero.num_node; i++) {
		if(db->idx_genero.nodes[i].id == id) {	
			//Apaga na memória
			db->idx_genero.nodes[i].id = 0;

			//Apaga no arquivo de gênero
			fseek(db->file_generos, (i - 1) * sizeof(secundary_node_t), SEEK_SET);
			fwrite(db->idx_genero.nodes + i, sizeof(secundary_node_t), 1, db->file_generos);
		}
	}

	//Remove o arquivo de índex secundário de idade (memória e arquivo)
	for (i = 0; i < db->idx_idade.num_node; i++) {
		if(db->idx_idade.nodes[i].id == id) {	
			//Apaga na memória
			db->idx_idade.nodes[i].id = 0;

			//Apaga no arquivo de idade
			fseek(db->file_idade, (i - 1) * sizeof(secundary_node_t), SEEK_SET);
			fwrite(db->idx_idade.nodes + i, sizeof(secundary_node_t), 1, db->file_idade);
		}
	}


	fecharArquivoIdx(db);
	fecharArquivoGeneros(db);
	fecharArquivoIdade(db);
	fecharArquivoDB(db);

	// diz que a flag está desordenada
	setFlag(db, 0);
	return true;
}

/**
 * pergunta se o registro reg curte o genero
 * @param  reg    previamente inicializada
 * @param  genero genero a ser pesquisado
 * @return        retorna verdadeiro se o registro curte o genero
 */
bool regCurteGenero(registro_t *reg, genero_t genero) {
	int i = 0;
	// varre todos os generos q o usuario curte
	while(reg->generos[i]) {
		if(reg->generos[i] == genero) {
			return true;
		}
		i++;
	}
	return false;
}

/*
Função que monta o conjunto com as pessoas que estão numa determinada faixa etária
Variáveis:
	db- banco de dados em memória
	ini- idade inicial
	fim- idade final
	conj_pessoas- conjunto de pessoas a ser montado
	i- contador
	parametro- usado para ver se foi encontrada pelo menos uma pessoa que atende aos requisitos

Retorno:
	Ponteiro para um conjunto de pessoas (seus idx) - Se encontrada pelo menos uma pessoa que atende aos requisitos
	NULL- Não foi encontrado ninguém 
*/
id_type* monta_conjuntoPopIdad(database_t *db, idade_t ini, idade_t fim) {
	id_type *conj_pessoas;
	int i, parametro;

	parametro = 0;
	conj_pessoas = calloc(db->num_id, sizeof(id_type)); //Inicializa todos com 0
	for(i = 0; i < db->idx_idade.num_node; i++) {
		if(db->idx_idade.nodes[i].cod >= ini && db->idx_idade.nodes[i].cod <= fim) {
			conj_pessoas[i] = db->idx_idade.nodes[i].id;
			parametro++;
		}
	}
	if(parametro == 0) { //Verifica se realmente foram encontradas pessoas na faixa etária
		free(conj_pessoas);
		return NULL;
	} else {
		return conj_pessoas;
	}
}

/**
 * precisa de free
 * pesquisa os generos mais populares por idade
 * @param  db  previamente inicializada
 * @param  ini idade mínima
 * @param  fim idade máxima
 * @return     os 10 gêneros mais populares para a faixa de idade
 * vetor do tipo [1, 2, 3, 4, 0, 0, 0, ...] ou tem 10 generos, ou acaba em 0
 */
genero_t *generosPopularesIdade(database_t *db, idade_t ini, idade_t fim) {
	// opcao 6
	genero_t *result = calloc(10, sizeof(genero_t));
	if(ini > fim) {
		#ifdef DEBUG
			printf("Digitaram as idades erradas\n");
		#endif //DEBUG
		return result; //Vazio
	}
	// vetor com a quantidade de pessoas que escuta determinado genero
	int escutam[GENSIZE] = {0};
	//Monta o conjunto de pessoas na faixa etária dada
	id_type *conj_pessoas;
	conj_pessoas = monta_conjuntoPopIdad(db, ini, fim);
	if(conj_pessoas == NULL) {
		#ifdef DEBUG
			printf("Não foram encontradas pessoas nesta faixa de idade\n");
		#endif //DEBUG
		return result; //Os requisitos não foram encontrados entre os usuários - vetor de gêneros vazio
	} else {
		//Coloca os gêneros que as pessoas do conjunto montado escutam no vetor escutam
		fill_escutam(db, conj_pessoas, escutam);
		free(conj_pessoas);
	}
	// define que os dez maiores sao os dez primeiros
	int i;
	for(i=0; i<10; i++) {
		result[i] = i+1;
	}
	// posicao do genero menos curtido em result
	int menos_curtido = 0;
	for(i=1; i<3; i++) {
		if(escutam[i] < escutam[menos_curtido]) {
			menos_curtido = i;
		}
	}
	// varre todos os registro até encontrar os maiores
	i = 11;
	while(i < GENSIZE) {
		// se ele é mais curtido do que o menor
		if(escutam[i] > escutam[menos_curtido]) {
			result[menos_curtido] = i;
		}
		menos_curtido = 0;
		// reencontra o menos curtido
		int j;
		for(j=1; j<10; j++) {
			if(escutam[j] < escutam[menos_curtido]) {
				menos_curtido = j;
			}
		}
		i++;
	}
	return result;
}

/**
 * precisa de free
 * pesquisa os usuários que gostam de um genero e estão dentro da faixa de idade
 * @param  db        previamente inicializada
 * @param  genero    genero a ser pesquisado
 * @param  ini idade minima
 * @param  fim idade máxima
 * @return           lista de IDs das pessoas que gostam do genero e tem idade entre ini-fim
 * lista do tipo [id1, id2, 0]
 * o último elemento é sempre zero
 */
id_type *usariosPorGenero(database_t *db, genero_t genero, idade_t ini, idade_t fim) {
	// opcao 7
	id_type *conj_pessoas, *result;
	int result_size = 0;
	int i, j;

	if(ini > fim) { //Verifica se a faixa etária foi digitada corretamente
		#ifdef DEBUG
			printf("As idades foram digitadas erradas\n");
		#endif //DEBUG
			result = calloc(1, sizeof(id_type));
			return result;
	}
	//Primeiramente monta um conjunto de todas as pessoas que atendem aos requisitos com o maior tamanho possível
	conj_pessoas = calloc(db->num_id, sizeof(id_type));
	for (i = 0; i < db->num_id; i++) {
		if (db->idx_idade.nodes[i].cod >= ini && db->idx_idade.nodes[i].cod <= fim) { //Verifica se a pessoa está na faixa etária
			for (j = 0; j < db->idx_genero.num_node; j++) {
				if (db->idx_genero.nodes[j].id == db->idx_idade.nodes[i].id && db->idx_genero.nodes[j].cod == genero) { //Verifica se a pessoa curte o gênero
					conj_pessoas[i] = db->idx_idade.nodes[i].id;
					result_size++; //Incrementa o tamanho que o resultado terá
					break;
				}
			}
		}
	}

	//Monta o vetor a ser retornado com o tamanho necessário
	result = malloc((result_size + 1) * sizeof(id_type));
	j = 0;
	for (i = 0; i < db->num_id; i++) {
		if(conj_pessoas[i] != 0) {
			result[j] = conj_pessoas[i];
			j++;
		}
	}
	result[j] = 0; //Coloca um zero no final do vetor
	free(conj_pessoas);
	return result;
}

/*
Função que passa um registro do arquivo para a memória através do índex
Váriaveis:
	db- Banco de dados presente em memória
	id- id da pessoa em questão
	reg- registro completo correpondente à pessoa
	db_file- arquivo correspondente ao banco de dados
	buffer- buffer do registro da pessoa em questão
	reg_size- tamanho do registro
*/
void idToRegistro(database_t *db, id_type id, registro_t *reg) {
	FILE *db_file;

	//Lê o registro em questão de acordo com o offset do arquivo de índex primário
	db_file = abrirArquivoDB(db, "r");
	fseek(db_file, pesquisarRegistro(db, id)-1, SEEK_SET);
	lerRegistro(db, reg);
	fecharArquivoDB(db);
}

/*
Função que verifica se uma pessoa curte um determinado gênero pelo seu id
Variáveis:
	db- Banco de dados presente em memória
	id- id da pessoa em questão
	genero- código do gênero procurado
Retorno:
	true- A pessoa curte o gênero
	false- A pessoa não curte o gênero
*/
bool idCurteGenero(database_t *db, id_type id, genero_t genero) {
	int i;

	for (i = 0; i < db->idx_genero.num_node; i++) {
		if(db->idx_genero.nodes[i].cod == genero && db->idx_genero.nodes[i].id == id) { //A pessoa curte o gênero
			return true;
		}
	}
	return false;
}

/*
Função que verifica se uma dada pessoa curte todos os gêneros necessários
Variáveis:
	db- Banco de dados presente em memória
	id- id da pessoa em questão
	generos- conjunto de gêneros não vazio
	i- contador
	reg- registro completo correpondente à pessoa

Retorno:
	false- a pessoa não curte pelo menos um dos gêneros no conjunto
	true- a pessoa curte todos os gêneros do conjunto
*/
bool pessoaCurteGeral(database_t *db, id_type id, genero_t *generos) {
	int i;

	i = 0;
	while (generos[i]) {
		if(!idCurteGenero(db, id, generos[i])) { //Pessoa não curte pelo menos um dos gêneros
			return false;
		}
		i++;
	}
	return true;
}

/*
Função que monta o conjunto com as pessoas que gostam simultâneamente de um certo conjunto de gêneros não vazio
Variáveis:
	db- banco de dados em memória
	gêneros- conjunto de gêneros
	conj_pessoas- conjunto de pessoas a ser montado
	i- contador
	parametro- usado para ver se foi encontrada pelo menos uma pessoa que atende aos requisitos

Retorno:
	Ponteiro para um conjunto de pessoas (seus idx) - Se encontrada pelo menos uma pessoa que atende aos requisitos
	NULL- Não foi encontrado ninguém 
*/
id_type* monta_conjuntoGeneros(database_t *db, genero_t *generos) {
	id_type *conj_pessoas;
	int i, parametro;

	parametro = 0;
	conj_pessoas = calloc(db->num_id, sizeof(id_type)); //Inicializa todos com 0
	for(i = 0; i < db->num_id; i++) {
		if(pessoaCurteGeral(db, db->idx_id[i].id, generos)) {
			conj_pessoas[i] = db->idx_id[i].id;
			parametro++;
		}
	}
	if(parametro == 0) { //Verifica se realmente foram encontradas pessoas que gosta de todos os gêneros
		free(conj_pessoas);
		return NULL;
	} else {
		return conj_pessoas;
	}
}

/*
Função que preenche o vetor escutam de acordo com o gosto das pessoas do conjunto
Variáveis:
	db- banco de dados em memória
	conj_pessoas- conjunto de pessoas a ser montado
	escutam- vetor com os valores que indicam a quantidade de pessoas no conjunto que gostam de cada tipo de música
	i, j- contadores
	reg- registro completo correpondente à pessoa
*/
void fill_escutam(database_t *db, id_type *conj_pessoas, int *escutam) {
	int i, j;

	for (i = 0; i < db->num_id; i++) {
		if(conj_pessoas[i] != 0) {
			for(j = 0; j < db->idx_genero.num_node; j++) {
				if (db->idx_genero.nodes[j].id == conj_pessoas[i]) {
					escutam[db->idx_genero.nodes[j].cod]++;
				}
			}
		}
	}
}

/**
 * precisa de free
 * os 3 generos mais populares entres as pessoas que curtem os generos do parametro
 * @param  db      previamente inicializada
 * @param  generos generos deve ser algo como [1, 2, 3, 4, 0], o ultimo valor é sempre 0
 * @return         é algo como [1, 2, 3, 0], DEVE LER ATÉ O 0, não necessariamente tem 3 elementos
 	conj_pessoas- conjunto das pessoas que escutam os gêneros procurados
 */
genero_t *generosPopularesGenero(database_t *db, genero_t *generos) {
	// opcao 4
	// mto parecida com a opcao 5
	genero_t *result = calloc(4, sizeof(genero_t));
	if(generos[0] == 0) {
		// vetor de generos esta vazio
		return result;
	}
	// vetor com a quantidade de pessoas que escuta determinado genero
	int escutam[GENSIZE] = {0};
	registro_t reg;
	//Monta o conjunto que contém as pessoas que escutam os gêneros
	id_type *conj_pessoas;
	conj_pessoas = monta_conjuntoGeneros(db, generos);
	#ifdef DEBUG
		int k;
		printf("Pessoas que curtem os generos: ");
		for(k = 0; k < db->num_id; k++) {
			if(conj_pessoas[k] != 0) {
				printf("%d ", conj_pessoas[k]);
			}
		}
	#endif //DEBUG
	if(conj_pessoas == NULL) {
		#ifdef DEBUG
			printf("Não foram encontradas pessoas que gostam simultaneamente dos generos\n");
		#endif //DEBUG
		return result; //Os requisitos não foram encontrados entre os usuários - vetor de gêneros vazio
	} else {
		//Coloca os gêneros que as pessoas do conjunto montado escutam no vetor escutam
		fill_escutam(db, conj_pessoas, escutam);
		free(conj_pessoas);
	}

	// define que os 3 maiores sao os 3 primeiros
	int i;
	for(i=0; i<3; i++) {
		result[i] = i+1;
	}
	// posicao do genero menos curtido em result
	int menos_curtido = 0;
	for(i=1; i<=3; i++) {
		if(escutam[i] < escutam[menos_curtido]) {
			menos_curtido = i;
		}
	}
	// varre todos os registro até encontrar os maiores
	i = 4;
	while(i < GENSIZE) {
		// se ele é mais curtido do que o menor
		if(escutam[i] > escutam[menos_curtido]) {
			result[menos_curtido] = i;
		}
		menos_curtido = 0;
		// reencontra o menos curtido
		int j;
		for(j=1; j<=3; j++) {
			if(escutam[j] < escutam[menos_curtido]) {
				menos_curtido = j;
			}
		}
		i++;
	}
	// altera o vetor resultado
	for(i=1; i<=3; i++) {
		result[i-1] = escutam[i];
	}
	return result;
}

/**
 * precisa de free
 * os 10 usuarios mais jovem de TU que curtem generos
 * se há somente 5 usuários o retorno é 1, 2, 3, 4, 5, 0, 0, 0, ...
 * generos deve ser algo como [1, 2, 3, 4, 0], o ultimo valor é sempre 0
 * ler até o 0 ou 10 elementos
 */
id_type *usuariosMaisJovems(database_t *db, genero_t *generos, tu_t tu) {
	// opcao 5
	idade_t idades[10];
	id_type *result = calloc(10, sizeof(id_type));
	FILE *fd = abrirArquivoDB(db, "r");
	if(_file_size(fd) == 0) {
		// o arquivo esta vazio
		return result;
	}
	fecharArquivoDB(db);
	int i = 0;
	// preenche as 10 primeiras idades
	int num_id = db->num_id;
	while(i < 10 || i < num_id) {
		idades[i] = db->idx_idade.nodes[i].cod;
		result[i] = db->idx_idade.nodes[i].id;
		i++;
	}
	// varre os demais registros
	while(i < num_id) {
		int j;
		// testa as 10 idades, se alguma é menor do que a q está sendo analisada
		for(j=0; j<10; j++) {
			if(db->idx_idade.nodes[i].cod < idades[j]) {
				idades[j] = db->idx_idade.nodes[i].cod;
				result[j] = db->idx_idade.nodes[i].id;
				break;
			}
		}
		i++;
	}
	return result;
}

/**
 * converte uma string como 'Rock' para seu código
 * @param  db  previamente inicializada
 * @param  str sem arrobas
 * @return     o valor do cod da string
 */
genero_t generoStrToCod(database_t *db, char *str) {
	#ifdef DEBUG
		printf("Testando genero '%s'\n", str);
	#endif // DEBUG
	genero_table_t *genero_table = &db->genero_table;
	int i;
	for(i=0; i<genero_table->num_node; i++) {
		if(strcmp(genero_table->nodes[i].str, str) == 0) {
			// encontrado
			return genero_table->nodes[i].cod;
		}
	}
	// novo cod
	int cod = i+1;
	// se nao for encontrado, cria um novo
	genero_table->num_node++;
	genero_table->nodes = realloc(genero_table->nodes, genero_table->num_node * sizeof(genero_node_t));
	genero_node_t *node = genero_table->nodes + genero_table->num_node - 1;
	strcpy(node->str, str);
	// o novo valor passa a ser i
	node->cod = cod;
	// atualiza o arquivo de generos
	FILE *fd = fopen(GENEROSTABLEFILENAME, "a");
	// escrita binária
	// fwrite(node, sizeof(genero_node_t), 1, fd);
	// escrita para usuario
	fprintf(fd, "%s=%d\n", node->str, node->cod);
	fclose(fd);
	return cod;
}

/**
 * converte a string: 'Pagode@Rock@Metal' para seus devidos codigos
 * @param db  previamente inicializada
 * @param str será alterada
 */
void generosStrToCod(database_t *db, char *str) {
	// conta a quantidade de arrobas
	#ifdef DEBUG
		printf("Convertendo a string para cod '%s'", str);
	#endif // DEBUG
	// caso especial unknown
	if(strcmp(str, "unknown") == 0) {
		#ifdef DEBUG
			puts("");
		#endif // DEBUG
		str[0] = 0; // o conteudo da str
		return ;
	}
	int i = 0;
	int num_generos = 0;
	while(str[i]) {
		if(str[i] == '@') {
			num_generos++;
			// ja define o @ como \0 para começar uma nova procura
			str[i] = '\0';
		}
		i++;
	}
	num_generos++;
	#ifdef DEBUG
		printf(", com %d generos\n", num_generos);
	#endif // DEBUG
	// ja sei qntos elementos tem
	// string com o proximo genero
	char *genero = str;
	// começa a varrer todos os generos
	for(i=0; i<num_generos; i++) {
		str[i] = generoStrToCod(db, genero);
		// avança para o proximo genero
		// encontra o \0
		while(*genero) genero++;
		// avança mais um
		genero++;
	}
	str[num_generos] = '\0';
	#ifdef DEBUG
		printfVerticaly(str);
	#endif // DEBUG
}

/**
 * Converte Cod para uma string
 * @param  db  inicializado previamente
 * @param  cod código do genero
 * @param  str string para ser copiado
 * @return     strlen(str)
 */
uint generoCodToStr(database_t *db, genero_t cod, char *str) {
	genero_table_t *table = &db->genero_table;
	int i;
	for(i=0; i<table->num_node; i++) {
		// procura na tabela todos os códigos
		if(table->nodes[i].cod == cod) {
			// genero encontrado
			strcpy(str, table->nodes[i].str);
			return strlen(table->nodes[i].str);
		}
	}
	return 0;
}

/**
 * converte de código para string com arrobas
 * @param db  inicializado previamente
 * @param str string com códigos dos generos
 */
void generosCodToStr(database_t *db, char *str) {
	// caso especial unknown
	if(str[0] == 0) {
		strcpy(str, "unknown");
		return ;
	}
	// buffer para armazenar temporarimente str
	char buffer[GENSIZE];
	strcpy(buffer, str);
	// varre todos os generos do usuário
	int i = 0;
	while(buffer[i]) {
		// anda com str ao msmo tempo que copia
		str += generoCodToStr(db, buffer[i], str);
		*str = '@';
		str++;
		i++;
	}
	str--;
	*str = '\0';
}