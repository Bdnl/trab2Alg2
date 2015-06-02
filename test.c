/**
 * Leonardo Guarnieri de Bastiani  8910434
 * Guilherme José Acra             7150306
 * Ricardo Chagas                  8957242
 */

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

#ifdef TEST
/**
 * arquivo de teste para as funções do programa
 */
int main(int argc, char *argv[]) {
	database_t db;
	initDB(&db);
	genero_t generos_para_teste[] = "Rock";
	generosStrToCod(&db, generos_para_teste);
	generosPopularesGenero(&db, generos_para_teste);
	usariosPorGenero(&db, generos_para_teste[0], 10, 20);
	usuariosMaisJovems(&db, generos_para_teste, 1);
	closeDB(&db);
	return 0;
}

#endif // TEST