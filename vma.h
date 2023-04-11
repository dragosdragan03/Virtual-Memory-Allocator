#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//structura pentru un nod
typedef struct dll_node_t {
	void *data;
	struct dll_node_t *prev, *next;
} dll_node_t;

//structura pentru lista
typedef struct doubly_list_t {
	dll_node_t *head;
	unsigned int data_size;
	unsigned int size; // cate noduri sunt
} doubly_list_t;

typedef struct {
	uint64_t start_address; // adresa de inceput a blocului
	size_t size; // size ul blockului
	void *miniblock_list; // pointer catre lista de miniblockuri
} block_t;

typedef struct {
	uint64_t start_address; // adresa de inceput a miniblockului
	size_t size; // size ul miniblockului
	uint8_t perm; // pentru a schimba permisiunile miniblockurilor
	void *rw_buffer; // legat de permisiuni
} miniblock_t;

typedef struct { // o lista
	uint64_t arena_size; // size ul arenei
	doubly_list_t *alloc_list; // pointer catre lista dublu inlantuita
} arena_t;

doubly_list_t *dll_create(unsigned int data_size);

dll_node_t *dll_get_nth_node(doubly_list_t *list, int n);
dll_node_t *create_node(void *new_data, int data_size);
void dll_add_nth_node(doubly_list_t *list, unsigned int n, const void *data);
void dll_remove_nth_node(doubly_list_t *list, unsigned int n);
void dll_free(doubly_list_t **pp_list);

arena_t *alloc_arena(const uint64_t size);
void dealloc_arena(arena_t *arena);

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);
void free_block(arena_t *arena, const uint64_t address);

void read(arena_t *arena, uint64_t address, uint64_t size);
void write(arena_t *arena, const uint64_t address
	, const uint64_t size, int8_t *data);
void pmap(const arena_t *arena);
void mprotect(arena_t *arena, uint64_t address, int8_t *permission);

int numar_nod(arena_t *arena, dll_node_t *nod);
int numar_miniblock(dll_node_t *minicurr, dll_node_t *curr);
dll_node_t *verificare_block(arena_t *arena, const uint64_t address
	, const uint64_t size);
dll_node_t *verificare_miniblock(arena_t *arena, const uint64_t address);
dll_node_t *verificare_adresa_miniblock(const uint64_t address
	, dll_node_t *curr);
dll_node_t *verificare_miniblock_mprotect(const uint64_t address
	, dll_node_t *curr);
dll_node_t *verificare_adresa_block(arena_t *arena, const uint64_t address);
dll_node_t *nod_cu_miniblock(arena_t *arena, const uint64_t address);
doubly_list_t *imbinare_liste(doubly_list_t *list1, doubly_list_t *list2);
int adresa_valida(arena_t *arena, const uint64_t address, const uint64_t size);
uint64_t adresa_final(dll_node_t *curr);
uint64_t adresa_inceput(arena_t *arena, dll_node_t *curr);
void alloc_block_alone(arena_t *arena, const uint64_t address
	, const uint64_t size, uint64_t adr_fin, dll_node_t *curr);
void alloc_block_mijloc(arena_t *arena, const uint64_t size
	, doubly_list_t *lista_miniblock, dll_node_t *curr);
void alloc_block_dreapta(arena_t *arena, const uint64_t address
	, const uint64_t size, doubly_list_t *lista_miniblock, dll_node_t *curr);
uint64_t dimensiune_stanga(dll_node_t *minib, dll_node_t *curr);
