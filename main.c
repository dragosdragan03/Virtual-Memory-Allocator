#include "vma.h"

int main(void)
{
	char sir[100];
	char token[100];

	while (scanf("%s", token)) { // citesc cat timp sunt randuri de citit
		arena_t *arena;

		if (strcmp(token, "ALLOC_ARENA") == 0) {
			uint64_t dimensiune;
			scanf("%lu", &dimensiune);
			arena = alloc_arena(dimensiune);
			continue;
		}
		if (strcmp(token, "DEALLOC_ARENA") == 0) {
			dealloc_arena(arena);
			break;
		}
		if (strcmp(token, "ALLOC_BLOCK") == 0) {
			uint64_t adresa;
			scanf("%ld", &adresa);
			uint64_t dimensiune;
			scanf("%ld", &dimensiune);
			if (adresa >= arena->arena_size) {
				printf("The allocated address is outside the size of arena\n");
				continue;
			} else if (adresa + dimensiune > arena->arena_size) {
				printf("The end address is past the size of the arena\n");
				continue;
			}
			alloc_block(arena, adresa, dimensiune);
			continue;
		}
		if (strcmp(token, "FREE_BLOCK") == 0) {
			uint64_t adresa;
			scanf("%ld", &adresa);
			free_block(arena, adresa);
			continue;
		}
		if (strcmp(token, "READ") == 0) {
			uint64_t adresa;
			scanf("%ld", &adresa);
			uint64_t dimensiune;
			scanf("%ld", &dimensiune);
			read(arena, adresa, dimensiune);
			continue;
		}
		if (strcmp(token, "WRITE") == 0) {
			uint64_t adresa;
			uint64_t dimensiune;
			scanf("%ld", &adresa);
			scanf("%ld", &dimensiune);
			int8_t *data = malloc(dimensiune * sizeof(int8_t));
			char gol = getc(stdin);
			for (int i = 0; i < dimensiune; i++)
				data[i] = getc(stdin);

			write(arena, adresa, dimensiune, data);
			free(data);

			continue;
		}
		if (strcmp(token, "PMAP") == 0) {
			pmap(arena);
			continue;
		}
		if (strcmp(token, "MPROTECT") == 0) {
			uint64_t adresa;
			scanf("%ld", &adresa);
			int8_t permisiune[100];
			fgets(permisiune, 100, stdin);
			mprotect(arena, adresa, permisiune);
			continue;
		}
		printf("Invalid command. Please try again.\n");
	}

	return 0;
}
