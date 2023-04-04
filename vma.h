#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dll_node_t //structura pentru un nod
{
	void* data; /* Pentru ca datele stocate sa poata avea orice tip, folosim un
		pointer la void. */
	struct dll_node_t* prev, * next;
} dll_node_t;

typedef struct doubly_list_t //structura pentru lista
{
	dll_node_t* head;
	unsigned int data_size;
	unsigned int size; // cate noduri sunt
} doubly_list_t;

typedef struct {
	uint64_t start_address; // adresa de inceput a blocului
	size_t size; // size ul blockului
	void* miniblock_list; //pointer catre lista de miniblockuri
} block_t;

typedef struct {
	uint64_t start_address; // adresa de inceput a miniblockului
	size_t size; // size ul miniblockului
	uint8_t perm; // permissions (pentru a schimba permisiunile miniblockurilor)
	void* rw_buffer; // legat de permisiuni
} miniblock_t;

typedef struct { // o lista
	uint64_t arena_size; // size ul arenei
	doubly_list_t* alloc_list; // pointer catre lista dublu inlantuita
} arena_t;

doubly_list_t* dll_create(unsigned int data_size);

dll_node_t* dll_get_nth_node(doubly_list_t* list, unsigned int n);
dll_node_t* create_node(void* new_data, int data_size);
void dll_add_nth_node(doubly_list_t* list, unsigned int n, const void* data);
void dll_remove_nth_node(doubly_list_t* list, unsigned int n);
void dll_free(doubly_list_t** pp_list);

arena_t* alloc_arena(const uint64_t size);
void dealloc_arena(arena_t* arena);

void alloc_block(arena_t* arena, const uint64_t address, const uint64_t size);
void free_block(arena_t* arena, const uint64_t address);

void read(arena_t* arena, uint64_t address, uint64_t size);
void write(arena_t* arena, const uint64_t address, const uint64_t size, int8_t* data);
void pmap(const arena_t* arena);
void mprotect(arena_t* arena, uint64_t address, int8_t* permission);