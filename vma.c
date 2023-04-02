#include "vma.h"

doubly_linked_list_t* dll_create(unsigned int data_size)
{
	doubly_linked_list_t* list = malloc(sizeof(doubly_linked_list_t));
	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;
	return list;
}

dll_node_t* dll_get_nth_node(doubly_linked_list_t* list, unsigned int n)
{
	dll_node_t* curr;
	if (list->size == 0)
		return 0;

	curr = list->head;
	for (int i = 0; i < n % list->size; i++)
		curr = curr->next;

	return curr;
}

dll_node_t* create_node(void* new_data, int data_size) // creez un nod
{
	dll_node_t* new_node = malloc(sizeof(dll_node_t)); // aloc diamic un nod
	new_node->data = malloc(data_size);                // aloca spatiu pentru tipul de data adaugat
	memcpy(new_node->data, new_data, data_size);       // copiaza in new_node, ce contine new_data cu size ul respectiv
	return new_node;
}

void dll_add_nth_node(doubly_linked_list_t* list, unsigned int n, const void* data)
{
	dll_node_t* new_node = create_node((void*)data, list->data_size);

	if (list->size == 0) {
		list->head = new_node;
		new_node->next = new_node;
		new_node->prev = new_node;
		list->size++;
		return;
	}

	if (n == 0) {
		new_node->next = list->head;
		new_node->prev = list->head->prev;

		list->head->prev->next = new_node;
		list->head->prev = new_node;

		list->head = new_node;
		list->size++;
		return;
	}

	if (n > list->size)
		n = list->size;

	dll_node_t* curr = list->head;
	for (int i = 0; i < n - 1; i++)
		curr = curr->next;

	new_node->next = curr->next;
	new_node->prev = curr;
	curr->next = new_node;
	new_node->next->prev = new_node;
	list->size++;
	return;
}

dll_node_t* dll_remove_nth_node(doubly_linked_list_t* list, unsigned int n)
{
	if (list->size == 0)
		return 0;

	if (n == 0) {
		dll_node_t* aux = list->head;
		list->head = list->head->next;
		list->head->prev = list->head->prev->prev;
		list->head->prev = list->head;
		list->size--;
		return aux;
	}

	if (n >= list->size - 1)
		n = list->size - 1;

	dll_node_t* curr = list->head;
	for (int i = 0; i < n - 1; i++)
		curr = curr->next;
	dll_node_t* aux = curr->next;

	curr->next->next->prev = curr;
	curr->next = curr->next->next;

	list->size--;
	return aux;
}

void dll_free(doubly_linked_list_t** pp_list)
{
	doubly_linked_list_t* list = *pp_list;

	if (list->size > 0) {
		dll_node_t* curr = list->head;
		dll_node_t* aux;
		for (int i = 0; i < list->size; i++) {
			aux = curr->next;
			free(curr->data);
			free(curr);
			curr = aux;
		}
	}
	free(*pp_list);
}

dll_node_t* verificare_block(arena_t* arena, const uint64_t address, const uint64_t size)
{
	dll_node_t* curr;
	if (arena->alloc_list->head == NULL)
		return NULL;

	if (address + size <= ((block_t*)arena->alloc_list->head)->start_address) // verific daca e inaintea headului
		return arena->alloc_list->head->prev;

	curr = arena->alloc_list->head;

	do { // verific intre ce blockuri se afla adresa
		if (((block_t*)curr->data)->start_address + ((block_t*)curr->data)->size <= address)
			if (curr->next == curr || address + size <= ((block_t*)curr->next->data)->start_address) {
				return curr;
			}
		curr = curr->next;
	} while (curr == arena->alloc_list->head);

	return NULL;
}

int numar_nod(arena_t* arena, dll_node_t* nod)
{
	dll_node_t* curr = arena->alloc_list->head;
	int numar = 0;
	do {
		if (curr == nod) {
			// printf("%d", numar);
			return numar;
		}
		numar++;
		curr = curr->next;
	} while (curr != arena->alloc_list->head);

}

dll_node_t* verificare_miniblock(arena_t* arena, const uint64_t address)
{
	dll_node_t* minicurr;
	dll_node_t* curr = arena->alloc_list->head;

	do {
		minicurr = ((doubly_linked_list_t*)((block_t*)(curr->data))->miniblock_list)->head; // headul listei de miniblockui, al blockului

		do {
			if (((miniblock_t*)minicurr->data)->start_address == address)
				return minicurr;

			minicurr = minicurr->next;
		} while (minicurr != ((doubly_linked_list_t*)((block_t*)(curr->data))->miniblock_list)->head);

		curr = curr->next;
	} while (curr != arena->alloc_list->head);
	return NULL; // nu a fost gasit miniblockul cu adresa respecitva
}

arena_t* alloc_arena(const uint64_t size) // aloc arena de blockuri cu parametrul size
{
	arena_t* arena = malloc(sizeof(arena_t));
	arena->arena_size = size;
	arena->alloc_list = dll_create(sizeof(block_t));
	arena->alloc_list->head = NULL;

	return arena;
}

void dealloc_arena(arena_t* arena)
{
	dll_node_t* cop;
	dll_node_t* minicurr;
	dll_node_t* curr = arena->alloc_list->head; // nod de parcurgere a blockurilor
	do { // parcurg blockurile
		minicurr = ((doubly_linked_list_t*)(((block_t*)curr->data)->miniblock_list))->head;
		do {
			cop = minicurr->next;
			free(((miniblock_t*)minicurr->data)->rw_buffer);
			free((miniblock_t*)minicurr->data); // elibereaza miniblockul
			free(minicurr); // elibereaza nodul
			minicurr = cop;
		} while (minicurr != ((doubly_linked_list_t*)((block_t*)(curr->data))->miniblock_list)->head);
		cop = curr->next;
		free(((block_t*)curr->data)->miniblock_list); // elibereaza lista de miniblockuri
		free(curr->data); // elibereaza data blockului
		free(curr); // elibereaza blockul
		curr = cop;
	} while (curr != arena->alloc_list->head);
	free(arena->alloc_list); // elibereaza lista de blockuri
	free(arena); // elibereaza arena
}

doubly_linked_list_t* imbinare_liste(doubly_linked_list_t* list1, doubly_linked_list_t* list2)
{
	dll_node_t* curr = list1->head;
	doubly_linked_list_t* list3 = dll_create(sizeof(miniblock_t));

	for (int i = 0; i < list1->size; i++) {
		dll_add_nth_node(list3, i, curr->data);
		curr = curr->next;
	}
	curr = list2->head;
	for (int j = 0; j < list2->size; j++) {
		dll_add_nth_node(list3, list1->size + j, curr->data);
		curr = curr->next;
	}

	dll_free(&list1);
	dll_free(&list2);

	return list3;

}

void alloc_block(arena_t* arena, const uint64_t address, const uint64_t size)
{
	dll_node_t* curr = verificare_block(arena, address, size); // blockul de dinaintea adresei
	// dll_node_t* verificare = curr->next;

	int adresa_final;
	int adresa_inceput;

	if (curr == NULL || (address + size <= ((block_t*)curr->next->data)->start_address)) {
		adresa_final = 0;
		printf("1 ");
	}
	else {
		adresa_final = ((block_t*)curr->data)->start_address + ((block_t*)curr->data)->size;
		printf("2 ");
	}

	if (curr == NULL || (curr->next == arena->alloc_list->head))
		adresa_inceput = arena->arena_size - size;
	else
		adresa_inceput = ((block_t*)curr->next->data)->start_address;

	if (arena->alloc_list->size == 1) {
		if (address + size <= ((block_t*)curr->next->data)->start_address) // sa vad daca e in stanga primului nod 
			adresa_inceput = ((block_t*)curr->next->data)->start_address;
	}

	printf("%d %d\n", adresa_final, adresa_inceput);

	if ((adresa_final < address || adresa_final == 0) && address + size < adresa_inceput) // in cazul in care blockul este singur si trebuie alocat
	{
		block_t* block = malloc(sizeof(block_t)); // am aloicat dinamic blockul ce urmeaza sa fie pus
		// dll_node_t* nod_block = create_node(block, sizeof(block));
		block->start_address = address; // am dat adrsa blockului
		block->size = size; // am pus size ul blockului

		block->miniblock_list = dll_create(sizeof(miniblock_t)); // aloc lista de miniblock
		miniblock_t* miniblock = malloc(sizeof(miniblock_t));
		miniblock->start_address = address; // start address ul miniblockului
		miniblock->size = size;
		miniblock->perm = 6;
		miniblock->rw_buffer = malloc(size * sizeof(char));
		dll_add_nth_node(((doubly_linked_list_t*)block->miniblock_list), 0, miniblock);
		// miniblock->start_address = address;

		if (curr == NULL || adresa_final == 0)  // in cazul in care e primul block
			dll_add_nth_node(arena->alloc_list, 0, block);
		else
			dll_add_nth_node(arena->alloc_list, numar_nod(arena, curr) + 1, block);

		// printf("%lu %lu", block->start_address, block->size);
		// printf(" %d", ((doubly_linked_list_t*)block->miniblock_list)->size);

		return;
	}

	doubly_linked_list_t* lista_miniblock = dll_create(sizeof(miniblock_t)); // creez lista pentru singurul miniblock
	miniblock_t* miniblock = malloc(sizeof(miniblock_t)); // creez miniblockul
	miniblock->size = size;
	miniblock->perm = 6;
	miniblock->rw_buffer = malloc(size * sizeof(char));
	miniblock->start_address = address;
	dll_add_nth_node(lista_miniblock, 0, miniblock); // il adaug in lista

	if (adresa_final == address && address + size == adresa_inceput) // in cazul in care blockul se afla intre alte 2 blockuri vecine si trebuiesc concatenate(cand am 2 granite)
	{
		((block_t*)curr->data)->size = ((block_t*)curr->data)->size + size + ((block_t*)curr->next->data)->size; // refac size ul intregului block

		doubly_linked_list_t* list = imbinare_liste(((doubly_linked_list_t*)((block_t*)curr->data)->miniblock_list), lista_miniblock);
		doubly_linked_list_t* list_final = imbinare_liste(list, ((doubly_linked_list_t*)((block_t*)curr->next->data)->miniblock_list));

		((block_t*)curr->data)->miniblock_list = list_final; // ii atribui lista corecta blockului intreg
		dll_node_t* cop = curr->next; // am fct o copie blockului de dupa

		curr->next = curr->next->next; // am refacut legaturile
		curr->next->prev = curr;
		arena->alloc_list--;
		free(cop->next->data); // am sters al 3 lea block
		free(cop->next);
		free(cop->data); // am sters al doilea block
		free(cop);
		return;
	}
	else if (adresa_final == address && address + size < adresa_inceput) // e lipit de blockul din stanga
	{
		((block_t*)curr->data)->size = ((block_t*)curr->data)->size + size; // ii refac blockului din stanga size ul

		doubly_linked_list_t* list = imbinare_liste(((doubly_linked_list_t*)((block_t*)curr->data)->miniblock_list), lista_miniblock);

		((block_t*)curr->data)->miniblock_list = list; // ii atribui lista corecta blockului intreg

		return;
		// n am ce legaturi sa fac ca doar adaug o lista de un miniblock la primul block
	}
	else if (adresa_final < address && address + size == adresa_inceput) // e lipit de blockul din dreapta
	{
		printf("bla");
		((block_t*)curr->next->data)->size = ((block_t*)curr->next->data)->size + size;
		((block_t*)curr->next->data)->start_address = address;
		doubly_linked_list_t* list = imbinare_liste(lista_miniblock, (doubly_linked_list_t*)((block_t*)curr->next->data)->miniblock_list);

		((block_t*)curr->next->data)->miniblock_list = list; // ii atribui lista corecta blockului intreg

		return;
	}

}

void free_block(arena_t* arena, const uint64_t address) // trebuie sa dealoc un minibloc
{
	// dll_node_t* minicurr = verificare_miniblock(arena, address);

	// if (curr->data) { // daca exista blockul
	// 	if (((doubly_linked_list_t*)(((block_t*)curr->data)->miniblock_list))->size == 1) { // sterg intregul block pentru ca e de size 1
	// 		dll_free(&((doubly_linked_list_t*)(((block_t*)curr->data)->miniblock_list))); // dealoc lista de miniblockuri (unicul miniblock)
	// 		// free(((doubly_linked_list_t *)(((block_t *)curr->data)->miniblock_list))->head->data);

	// 		if (curr = arena->alloc_list->head) // fac curr next ca fiind head
	// 			curr->next = arena->alloc_list->head;

	// 		curr->prev->next = curr->next; // fac legaturile intre blockuri
	// 		curr->next->prev = curr->prev;
	// 		free(curr->data);
	// 		free(curr);
	// 		return;
	// 	}

	// 	block_t* block1 = (block_t*)curr->data; // castez curr de tip block
	// 	doubly_linked_list_t* list1 = (doubly_linked_list_t*)((block1)->miniblock_list); // castez block1 de tip dll
	// 	dll_node_t* nod = (dll_node_t*)list1->head; // castez lista de tip nod dintr un miniblock
	// 	if (adress == ((miniblock_t*)nod->data)->start_address) { // daca e la inceput
	// 		nod->prev->next = nod->next;
	// 		nod->next->prev = nod->prev;
	// 		list1->head = nod->next;
	// 		free(nod->data);
	// 		free(nod);
	// 		return;
	// 	}
	// 	else if (adress == ((miniblock_t*)(nod)->prev->data)->start_address) { // daca e la final
	// 		nod->prev->next = nod->next;
	// 		nod->next->prev = nod->prev;
	// 		free(nod->data);
	// 		free(nod);
	// 		return;
	// 	}
	// }

	// trebuie sa mai verificam daca este la mijlocul listei pentru a forma alte 2 blockuri
}

void read(arena_t* arena, uint64_t address, uint64_t size)
{

}

void write(arena_t* arena, const uint64_t address, const uint64_t size, int8_t* data)
{

}

void pmap(const arena_t* arena)
{
	printf("Total memory: %lu\n", arena->arena_size);

	uint64_t free_size = arena->arena_size;
	int numar_blockuri = 0;
	int numar_miniblockuri = 0;
	dll_node_t* curr = arena->alloc_list->head;
	dll_node_t* minicurr;
	do {
		free_size = free_size - ((block_t*)curr->data)->size;
		minicurr = ((doubly_linked_list_t*)((block_t*)curr->data)->miniblock_list)->head;
		do {
			numar_miniblockuri++;
			minicurr = minicurr->next;
		} while (minicurr != ((doubly_linked_list_t*)((block_t*)(curr->data))->miniblock_list)->head);
		numar_blockuri++;
		curr = curr->next;
	} while (curr != arena->alloc_list->head);

	printf("Free size: %lu\n", free_size);
	printf("Number of allocated blocks: %d\n", numar_blockuri);
	printf("Number of allocated miniblocks: %d\n", numar_miniblockuri);

	curr = arena->alloc_list->head;
	numar_blockuri = 1;
	do {
		uint64_t adresa_inceput = ((block_t*)curr->data)->start_address; // adresa de inceput a blockului
		size_t size = ((block_t*)(curr->data))->size; // size ul blockului
		uint64_t adresa_final = adresa_inceput + size;

		printf("Block %d begin\n", numar_blockuri);
		printf("Zone: %lu - %lu\n", adresa_inceput, adresa_final);

		minicurr = ((doubly_linked_list_t*)((block_t*)curr->data)->miniblock_list)->head;
		numar_miniblockuri = 1;
		do {
			printf("Miniblock %d:", numar_miniblockuri);
			adresa_inceput = ((miniblock_t*)minicurr->data)->start_address;
			size = ((miniblock_t*)minicurr->data)->size;
			adresa_final = adresa_inceput + size;
			printf("\t\t%lu\t\t-\t\t%lu\t\t|", adresa_inceput, adresa_final);
			uint8_t permisiune = ((miniblock_t*)minicurr->data)->perm;
			if (permisiune == 0)
				printf(" ---\n");
			if (permisiune == 1)
				printf(" --X\n");
			if (permisiune == 2)
				printf(" -W-\n");
			if (permisiune == 3)
				printf(" -WX\n");
			if (permisiune == 4)
				printf(" R--\n");
			if (permisiune == 5)
				printf(" R-X\n");
			if (permisiune == 6)
				printf(" RW-\n");
			if (permisiune == 7)
				printf(" RWX\n");

			numar_miniblockuri++;
			minicurr = minicurr->next;
		} while (minicurr != ((doubly_linked_list_t*)((block_t*)(curr->data))->miniblock_list)->head);
		printf("Block %d end\n", numar_blockuri);
		numar_blockuri++;
		curr = curr->next;
	} while (curr != arena->alloc_list->head);

}

void mprotect(arena_t* arena, uint64_t address, int8_t* permission)
{

}