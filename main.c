#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "misc.h"
#include "database.h"

/**
 * Títulos das páginas
 * @param subtitulo:	string com o subtítulo da página
*/
void titulo(char * subtitulo){
    system("clear");
    //system("cls");
    printf("**GERENCIADOR DE USUÁRIOS**\n");
    if(strcmp(subtitulo, "_")){
        printf("%s\n", subtitulo);
    }
    printf("======================================================\n\n");
}

/**
 * Testa se a string contem apenas números (positivos)
 * @param eh_número:	ponteiro para a string a ser testada
*/
int teste_numero(char *eh_numero){
	int i = 0;
	while(eh_numero[i] != '\0'){
		if(eh_numero[i] < 48 || eh_numero[i] > 57){ //48 = '0'	57 = '9'
			return 0;
		}
		i++;
	}
	return 1;
}

/**
 * Testa se o ID é válido
 * @param eh_valido:	ponteiro para a string a ser testada
*/
int teste_id(char *eh_valido){
	return teste_numero(eh_valido);
}

/**
 * Testa se a idade é válida
 * @param eh_valido:	ponteiro para a string a ser testada
*/
int teste_idade(char *eh_valido){
	int idade;

	//Verifica se é número
	if(!teste_numero(eh_valido))	
		return 0;

	//Verifica se está dentro de uma faixa aceitável para idades
	idade = atoi(eh_valido);
	if(idade > 120)	
		return 0;

	return 1;
}

/**
 * Testa se o sexo é válido
 * @param eh_valido:	ponteiro para a string a ser testada
*/
int teste_sexo(char *eh_valido){
	if((eh_valido[0] == 'M' || eh_valido[0] == 'F') && eh_valido[1] == '\0')
		return 1;
	return 0; 
}

/**
 * Testa se o tipo de usuário é válido
 * @param eh_valido:	ponteiro para a string a ser testada
*/
int teste_tu(char *eh_valido){
	if((eh_valido[0] == '1' || eh_valido[0] == '2' || eh_valido[0] == '3') && eh_valido[1] == '\0')
		return 1;
	return 0;
}

/**
 * Menu "Inserir usuário"
 * @param db:	ponteiro para database
*/
void menu_1(database_t *db){
	registro_t new_reg;	//Novo registro
	char eh_valido[15];	//Recebe variáveis que precisa testar a validade
	char ok = 1; 		//Se =1 ao final da função, todos os dados estão dentro do padrão esperado 

	titulo("Inserir usuário");

	printf("Insira os dados:\n");
		
	//ID
	_scanf_s(eh_valido, 15);
	if(teste_id(eh_valido))
		new_reg.id = atoi(eh_valido);
	else{
		printf("ID em formato inválido.\n");
		ok = 0;
	}
	
	//Nome
	_scanf_s(new_reg.nome, NOMESIZE);

	//Idade
	_scanf_s(eh_valido, 10);
	if(teste_idade(eh_valido))
		new_reg.idade = atoi(eh_valido);
	else{
		printf("Idade em formato inválido.\n");
		ok = 0;
	}
	
	//Sexo
	_scanf_s(eh_valido, 10);
	if(teste_sexo(eh_valido))
		new_reg.sexo = eh_valido[0];
	else{
		printf("Sexo em formato inválido.\n");
		ok = 0;
	}
	
	//Gêneros
	_scanf_s(new_reg.generos, GENSIZE);
	if(new_reg.generos[0] == '\0')
		strcpy(new_reg.generos, "unknown");
	
	//Tipo de usuário
	_scanf_s(eh_valido, 10);
	if(teste_tu(eh_valido))
		new_reg.tu = atoi(eh_valido);
	else{
		printf("Tipo do usuário em formato inválido.\n");
		ok = 0;
	}

	//Confere se algum campo era inválido
	if(!ok){
		printf("Falha: Campo(s) invalido(s)\n");
		system_pause();
		return;
	}

	//Confere se o ID já existe
	if(pesquisarRegistro(db, new_reg.id) == -1){
		//Transforma a string dos gêneros em um vetor de códigos
		//Adiciona, se necessário, o gênero na tabela de gêneros
		generosStrToCod(db, new_reg.generos);
		//Adiciona o registro
		novoRegistro(db, &new_reg);		
		printf("Operacao realizada com sucesso\n");
		system_pause();
		return;
	}

	printf("Falha: ID ja cadastrado\n");
	system_pause();
	return;
}

/**
 * Menu "Remover usuário"
 * @param db:	ponteiro para database
*/
void menu_2(database_t *db){
	char eh_valido[15];	//Recebe o ID para testar a validade
	id_type id; 		//ID já validada para a remoção

	titulo("Remover usuário");

	//Ler e validar ID
	printf("ID:");
	_scanf_s(eh_valido, 15);
	if(!teste_id(eh_valido)){
		printf("Falha: ID em formato invalido\n");
		system_pause();
		return;
	}
	
	id = atoi(eh_valido);

	//Remover usuário
	if(removerRegistro(db, id))
		printf("Operacao realizada com sucesso\n");
	else
		printf("Falha: ID nao encontrado\n");

	system_pause();
	return;	
}

/**
 * Menu "Pesquisar usuário"
 * @param db:	ponteiro para database
*/
void menu_3(database_t *db){
	char eh_valido[15];		//Recebe o ID para testar a validade
	id_type id; 			//ID já validada para a remoção
	offset_t offset_pesq;	//Offset do registro pesquisado
	registro_t reg_pesq; 	//Registro que foi pesquisado

	titulo("Pesquisar usuário");

	//Ler e validar ID
	printf("ID:");
	_scanf_s(eh_valido, 15);
	if(!teste_id(eh_valido)){
		printf("Falha: ID em formato invalido\n");
		system_pause();
		return;
	}
	
	id = atoi(eh_valido);

	//Pesquisar usuario e verificar se o mesmo existe
	offset_pesq = pesquisarRegistro(db, id);
	if(offset_pesq == -1){
		printf("Falha: ID nao encontrado\n");
		system_pause();
		return;
	}

	printf("%ld\n", offset_pesq);

	//Deslocar db para a posição offset_pesq, 
	//ler o registro e imprimir os dados
	abrirArquivoDB(db, "r");
	fseek((db->file_db), (offset_pesq - 1), SEEK_SET);
	offset_pesq = lerRegistro(db, &reg_pesq);
	fecharArquivoDB(db);

	printf("\n%d\n", reg_pesq.id);
	printf("%s\n", reg_pesq.nome);
	printf("%d\n", reg_pesq.idade);
	printf("%c\n", reg_pesq.sexo);
	generosCodToStr(db, reg_pesq.generos);	//Transforma o vetor de códigos na string dos gêneros
	printf("%s\n", reg_pesq.generos);
	printf("%d\n", reg_pesq.tu);

	system_pause();
	return;
}

/**
 * Menu "Buscar gostos musicais semelhantes"
 * @param db:	ponteiro para database
*/
void menu_4(database_t *db){
	genero_t generos_pesq[GENSIZE];	//Lista de gêneros a serem pesquisados
	genero_t *generos_cod_result; 	//Lista de códigos de gêneros resultados da pesquisa
	char generos_result[GENSIZE]; 	//String com os gêneros resultados da pesquisa

	titulo("Buscar gostos musicais semelhantes");

	//Lê a string de gêneros
	printf("Gêneros:");
	_scanf_s(generos_pesq, GENSIZE);
	if(generos_pesq[0] == '\0')			
		return;

	//Transforma a string dos gêneros em um vetor de códigos
	generosStrToCod(db, generos_pesq);

	//É feita a busca
	generos_cod_result = generosPopularesGenero(db, generos_pesq);

	//A imformação é passada de generos_cod_result para generos_result
	strcpy(generos_result, generos_cod_result);
	free(generos_cod_result);	//Malloc foi feito na função generosPopularesGenero
	
	//Transforma o vetor de códigos na string dos gêneros e imprime
	generosCodToStr(db, generos_result);
	
	//Imprime uma gênero por linha
	printf("\nResultado:\n");
	int i=0;
	while(generos_result[i]){
		if(generos_result[i] != '@')
			putchar(generos_result[i]);
		else
			putchar('\n');
		i++;
	}
	putchar('\n');

	system_pause();
	return;
}

/**
 * Menu "Buscar usuários mais jovens"
 * @param db:	ponteiro para database
*/
void menu_5(database_t *db){
	genero_t generos_pesq[GENSIZE];	//Lista de gêneros a serem pesquisados
	char eh_valido[15];				//Tipo do usuário que precisa ser validado
	tu_t tu_pesq;					//Tipo de usuário válido a ser pesquisado
	id_type *id_result; 			//Ponteiro para o vetor de IDs resultados da pesquisa

	titulo("Buscar usuários mais jovens");

	//Lê a string de gêneros
	printf("Gêneros:");
	_scanf_s(generos_pesq, GENSIZE);
	if(generos_pesq[0] == '\0')			
		return;

	//Lê o tipo de usuário
	printf("Tipo de usuário:");
	_scanf_s(eh_valido, 10);
	if(teste_tu(eh_valido))
		tu_pesq = atoi(eh_valido);
	else{
		printf("Falha: Tipo do usuário em formato inválido.\n");
		system_pause();
		return;
	}

	//Transforma a string dos gêneros em um vetor de códigos
	generosStrToCod(db, generos_pesq);

	//Busca os usuários mais jovens
	id_result = usuariosMaisJovems(db, generos_pesq, tu_pesq);

	//Imprime os IDs
	printf("\nIDs:\n");
	int i = 0;
	while(id_result[i]){
		printf("%d\n", id_result[i]);
		i++;
	}

	free(id_result); //Malloc feito na função usuariosMaisJovens
	system_pause();
	return;
}

/**
 * Menu "Buscar gêneros mais populares"
 * @param db:	ponteiro para database
*/
void menu_6(database_t *db){
	char eh_valido1[10], eh_valido2[10];	//Recebe as idades para testar a validade
	idade_t idade_min, idade_max; 			//Idades mínima e máxima para a busca
	genero_t *generos_cod_result; 			//Lista de códigos de gêneros resultados da pesquisa
	char generos_result[GENSIZE]; 			//String com os gêneros resultados da pesquisa

	titulo("Buscar gêneros mais populares");

	//Lê e valida as idades mínima e máxima
	printf("Idade mínima:");
	_scanf_s(eh_valido1, 10);
	printf("Idade máxima:");
	_scanf_s(eh_valido2, 10);

	if(teste_idade(eh_valido1) && teste_idade(eh_valido2)){
		idade_min = atoi(eh_valido1);
		idade_max = atoi(eh_valido2);
	}else{
		printf("Falha: Idade em formato inválido.\n");
		system_pause();
		return;
	}

	//Busca
	generos_cod_result = generosPopularesIdade(db, idade_min, idade_max);

	//A imformação é passada de generos_cod_result para generos_result
	strcpy(generos_result, generos_cod_result);
	free(generos_cod_result);	//Malloc foi feito na função generosPopularesIdade
	
	//Transforma o vetor de códigos na string dos gêneros e imprime
	generosCodToStr(db, generos_result);
	
	//Imprime uma gênero por linha
	printf("\nResultado:\n");
	int i=0;
	while(generos_result[i]){
		if(generos_result[i] != '@')
			putchar(generos_result[i]);
		else
			putchar('\n');
		i++;
	}
	putchar('\n');

	system_pause();

	return;
}

/**
 * Menu "Buscar usuários por idade e gênero"
 * @param db:	ponteiro para database
*/
void menu_7(database_t *db){
	char eh_valido1[10], eh_valido2[10];	//Recebe as idades para testar a validade
	idade_t idade_min, idade_max; 			//Idades mínima e máxima para a busca
	genero_t genero_pesq[GENSIZE];			//Gênero a ser pesquisado
	id_type *id_result; 					//Ponteiro para o vetor de IDs resultados da pesquisa
	
	titulo("Buscar usuários por idade e gênero");

	//Lê e valida as idades mínima e máxima
	printf("Idade mínima:");
	_scanf_s(eh_valido1, 10);
	printf("Idade máxima:");
	_scanf_s(eh_valido2, 10);

	if(teste_idade(eh_valido1) && teste_idade(eh_valido2)){
		idade_min = atoi(eh_valido1);
		idade_max = atoi(eh_valido2);
	}else{
		printf("Falha: Idade em formato inválido.\n");
		system_pause();
		return;
	}

	//Lê a string de gêneros
	printf("Gêneros:");
	_scanf_s(genero_pesq, GENSIZE);
	if(genero_pesq[0] == '\0')			
		return;

	//Transforma a string dos gêneros em um vetor de códigos
	generosStrToCod(db, genero_pesq);

	//Busca
	//Obs: É usada apenas a primeira posição do vetor genero_pesq
	id_result = usariosPorGenero(db, (*genero_pesq), idade_min, idade_max);	

	//Imprime os IDs
	printf("\nIDs:\n");
	int i = 0;
	while(id_result[i]){
		printf("%d\n", id_result[i]);
		i++;
	}

	free(id_result); //Malloc feito na função usuariosPorGenero
	system_pause();
	return;
}

/**
 * Menu principal do programa, no qual o usuário pode escolher a operação a ser realizada
 * @param db:	ponteiro para database
 * @return: 	0 caso o usuário deseja sair do programa
 				1 caso contrário	
*/
int menu_principal(database_t *db){
	char opcao;

	titulo("_");

	//Opções do menu
	printf("1. Inserir usuário\n");
	printf("2. Remover usuário\n");
	printf("3. Pesquisar usuário\n");
	printf("4. Buscar gostos musicais semelhantes\n");
	printf("5. Buscar usuários mais jovens\n");
	printf("6. Buscar gêneros mais populares\n");
	printf("7. Buscar usuários por idade e gênero\n");
	printf("8. Fechar o programa\n");

	//opcao recebe a escolha do usuário
	opcao = _getchar();
	
	//Vai para a operação desejada
	switch(opcao){
		case '1':
			menu_1(db);
			return 1;
		case '2':
			menu_2(db);
			return 1;
		case '3':
			menu_3(db);
			return 1;
		case '4':
			menu_4(db);
			return 1;
		case '5':
			menu_5(db);
			return 1;
		case '6':
			menu_6(db);
			return 1;
		case '7':
			menu_7(db);
			return 1;
		case '8':		//Ao retornar 0, sai do while na main
			return 0;
		default:		//Se o caracter for inválido, volta ao menu principal
			return 1;
	}
}

int main(void){
	//Declara e inicia database 
	database_t db;
	initDB(&db);
	
	//Loop do Menu Principal
	while(menu_principal(&db));

	//Encerra database, liberando todas as estruturas
	closeDB(&db);

	return 0;
}