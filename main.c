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

int main(int argc, char *argv[]) {
	#ifdef DEBUG
		registro_t reg_new = {1, "Nome", 20, 'M', "Rock@Pagode@MPB", 1};
		database_t db;
		initDB(&db);
		generosStrToCod(&db, reg_new.generos);
		generosCodToStr(&db, reg_new.generos);
		printf("%s\n", reg_new.generos);
		closeDB(&db);
	#endif // DEBUG
	return 0;
}