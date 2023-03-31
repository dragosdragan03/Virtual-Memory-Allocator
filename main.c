#include "vma.h"

int main(void)
{
    char sir[100];

    while (fgets(sir, 100, stdin)) { // citesc cat timp sunt randuri de citit
        // char sir1[50];
        // strcpy(sir1, sir);
        arena_t *arena;
        char *token = strtok(sir, " ");
        if (strcmp(token, "ALLOC_ARENA") == 0) {
            token = strtok(NULL, " "); // stochez in token dimensiunea fisierului
            token[strlen(token) - 1] = '\0';
            int dimensiune = atoi(token);
            arena = alloc_arena(dimensiune);
            continue;
        }
        if (strcmp(token, "DEALLOC_ARENA\n") == 0) {
            dealloc_arena(arena);
            break;
        }
        if (strcmp(token, "ALLOC_BLOCK") == 0) {
            token = strtok(NULL, " ");
            const uint64_t adresa = atoi(token);
            token = strtok(NULL, " ");
            token[strlen(token) - 1] = '\0';
            const uint64_t dimensiune = atoi(token);
            alloc_block(arena, adresa, dimensiune);
            continue;
        }
        if (strcmp(token, "FREE_BLOCK") == 0) {
            token = strtok(NULL, " ");
            token[strlen(token) - 1] = '\0';
            const uint64_t adresa = atoi(token);
            free_block(arena, adresa);
            continue;
        }
        if (strcmp(token, "READ") == 0) {
            token = strtok(NULL, " ");
            const uint64_t adresa = atoi(token);
            token = strtok(NULL, " ");
            token[strlen(token) - 1] = '\0';
            const uint64_t dimensiune = atoi(token);
            read(arena, adresa, dimensiune);
            continue;
        }
        if (strcmp(token, "WRITE") == 0) {
            token = strtok(NULL, " ");
            const uint64_t adresa = atoi(token);
            token = strtok(NULL, " ");
            const uint64_t dimensiune = atoi(token);
            token = strtok(NULL, " ");
            token[strlen(token) - 1] = '\0';
            int8_t *data = token;
            write(arena, adresa, dimensiune, data);
            continue;
        }
        if (strcmp(token, "PMAP\n") == 0) {
            pmap(arena);
            continue;
        }
        if (strcmp(token, "MPROTECT") == 0) {
            token = strtok(NULL, " ");
            const uint64_t adresa = atoi(token);
            token = strtok(NULL, " ");
            token[strlen(token) - 1] = '\0';
            int8_t *permisiune = token;
            mprotect(arena, adresa, permisiune);
            continue;
        }
        printf("Invalid command\n");
    }

    return 0;
}