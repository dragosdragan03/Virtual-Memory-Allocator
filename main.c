#include "vma.h"

int main(void)
{
    char sir[100];

    while (1) { // citesc cat timp sunt randuri de citit
        arena_t* arena;
        char token[100];
        scanf("%s", token);
        if (strcmp(token, "ALLOC_ARENA") == 0) {
            int dimensiune;
            scanf("%d", &dimensiune);
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
            int8_t* data = malloc(dimensiune * sizeof(int8_t));
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
            fgets(sir, 100, stdin);
            char* perm = strtok(sir, " ");
            int8_t* permisiune;
            while (perm) {
                strcpy(permisiune, perm);
                if (perm != "|")
                    mprotect(arena, adresa, permisiune);
                perm = strtok(NULL, " ");
            }

            continue;
        }
        printf("Invalid command.Please try again.\n");
    }

    return 0;
}