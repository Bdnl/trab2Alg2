#ifndef __REGISTRO_H__
#define __REGISTRO_H__

#include "database.h"

uint registroToBuffer(registro_t *reg, char *buffer);

// de buffer para registro_t
void bufferToReg(char *buffer, registro_t *reg);

// retorna EOF caso erro
offset_t novoRegistro(database_t *db, registro_t *reg);

// retorna EOF quando acaba o arquivo
offset_t lerRegistro(database_t *db, registro_t *reg);

// retorna a posicao do arquivo do primeiro byte do id
offset_t pesquisarRegistro(database_t *db, id_type id);

// retorna verdadeiro se foi um sucesso
bool removerRegistro(database_t *db, id_type id);

// retorna verdadeiro se o registro curte o genero
bool regCurteGenero(registro_t *reg, genero_t genero);

/**
 * precisa de free
 * retorna os 10 gêneros mais populares para a faixa de idade
 * vetor do tipo [1, 2, 3, 4, 0, 0, 0, ...] ou tem 10 generos, ou acaba em 0
 */
genero_t *generosPopularesIdade(database_t *db, idade_t ini, idade_t fim);

/**
 * precisa de free
 * lista de IDs das pessoas que gostam do genero e tem idade entre ini-fim
 * lista do tipo [id1, id2, 0]
 * o último elemento é sempre zero
 */
id_type *usariosPorGenero(database_t *db, genero_t genero, idade_t idade_ini, idade_t idade_fim);

/**
 * precisa de free
 * os 3 generos mais populares entres as pessoas que curtem os generos do parametro
 * generos deve ser algo como [1, 2, 3, 4, 0], o ultimo valor é sempre 0
 * o retorno é algo como [1, 2, 0], ler até o zero ou 3 elementos
 */
genero_t *generosPopularesGenero(database_t *db, genero_t *generos);

/**
 * precisa de free
 * os 10 usuarios mais jovem de TU que curtem generos
 * se há somente 5 usuários o retorno é 1, 2, 3, 4, 5, 0, 0, 0, ...
 * ler até o 0 ou 10 elementos
 */
id_type *usuariosMaisJovems(database_t *db, genero_t *generos, tu_t tu);

#endif