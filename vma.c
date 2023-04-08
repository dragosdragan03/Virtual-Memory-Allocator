#include "vma.h"

doubly_list_t *dll_create(unsigned int data_size)
{
	doubly_list_t *list = malloc(sizeof(doubly_list_t));
	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;
	return list;
}

dll_node_t *dll_get_nth_node(doubly_list_t *list, unsigned int n)
{
	dll_node_t *curr;
	if (list->size == 0)
		return 0;

	curr = list->head;
	for (int i = 0; i < n % list->size; i++)
		curr = curr->next;

	return curr;
}

dll_node_t *create_node(void *new_data, int data_size) // creez un nod
{
	dll_node_t *new_node = malloc(sizeof(dll_node_t)); // aloc diamic un nod
	new_node->data = malloc(data_size);                // aloca spatiu pentru tipul de data adaugat
	memcpy(new_node->data, new_data, data_size);       // copiaza in new_node, ce contine new_data cu size ul respectiv
	return new_node;
}

void dll_add_nth_node(doubly_list_t *list, unsigned int n, const void *data)
{
	dll_node_t *new_node = create_node((void *)data, list->data_size);

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

	dll_node_t *curr = list->head;
	for (uint64_t i = 0; i < n - 1; i++)
		curr = curr->next;

	new_node->next = curr->next;
	new_node->prev = curr;
	curr->next = new_node;
	new_node->next->prev = new_node;
	list->size++;
	return;
}

void dll_remove_nth_node(doubly_list_t *list, unsigned int n)
{
	if (list->size == 0)
		return;

	if (n == 0) {
		dll_node_t *aux = list->head;
		list->head = list->head->next;
		list->head->prev = list->head->prev->prev;
		list->head->prev->next = list->head;
		list->size--;
		free(aux->data);
		free(aux);
		return;
	}

	if (n >= list->size - 1)
		n = list->size - 1;

	dll_node_t *curr = list->head;
	for (int i = 0; i < n - 1; i++)
		curr = curr->next; // eu vreau sa sterg pe curr->next
	dll_node_t *aux = curr->next;

	curr->next->next->prev = curr;
	curr->next = curr->next->next;

	list->size--;

	free(aux->data);
	free(aux);
	return;
}

void dll_free(doubly_list_t **pp_list)
{
	doubly_list_t *list = *pp_list;

	if (list->size > 0) {
		dll_node_t *curr = list->head;
		dll_node_t *aux;
		for (int i = 0; i < list->size; i++) {
			aux = curr->next;
			free(curr->data);
			free(curr);
			curr = aux;
		}
	}
	free(*pp_list);
}

arena_t *alloc_arena(const uint64_t size) // aloc arena de blockuri cu parametrul size
{
	arena_t *arena = malloc(sizeof(arena_t));
	arena->arena_size = size;
	arena->alloc_list = dll_create(sizeof(block_t));
	arena->alloc_list->head = NULL;

	return arena;
}

void dealloc_arena(arena_t *arena)
{
	dll_node_t *cop;
	dll_node_t *minicurr;
	if (arena->alloc_list->size == 0) {
		dll_free(&arena->alloc_list);
		free(arena);
		return;
	}

	dll_node_t *curr = arena->alloc_list->head; // nod de parcurgere a blockurilor
	do { // parcurg blockurile
		block_t *block = (block_t *)curr->data;
		minicurr = ((doubly_list_t *)block->miniblock_list)->head;
		do {
			cop = minicurr->next;
			free(((miniblock_t *)minicurr->data)->rw_buffer);
			free((miniblock_t *)minicurr->data); // elibereaza miniblockul
			free(minicurr); // elibereaza nodul
			minicurr = cop;
		} while (minicurr != ((doubly_list_t *)block->miniblock_list)->head);
		cop = curr->next;
		free(((block_t *)curr->data)->miniblock_list); // elibereaza lista de miniblockuri
		free(curr->data); // elibereaza data blockului
		free(curr); // elibereaza blockul
		curr = cop;
	} while (curr != arena->alloc_list->head);
	free(arena->alloc_list); // elibereaza lista de blockuri
	free(arena); // elibereaza arena
}

int numar_nod(arena_t *arena, dll_node_t *nod)
{
	dll_node_t *curr = arena->alloc_list->head;
	int numar = 0;
	do {
		if (curr == nod)
			return numar;

		numar++;
		curr = curr->next;
	} while (curr != arena->alloc_list->head);

}

int numar_miniblock(arena_t *arena, dll_node_t *minicurr, dll_node_t *curr)
{
	block_t *block = (block_t *)curr->data;
	dll_node_t *nod = ((doubly_list_t *)block->miniblock_list)->head;

	int numar = 0;
	while (nod != minicurr) {
		numar++;
		nod = nod->next;
	}

	return numar;
}

dll_node_t *verificare_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	dll_node_t *curr;
	if (arena->alloc_list->size == 0)
		return NULL;

	uint64_t dimensiune = address + size;
	if (dimensiune <= ((block_t *)arena->alloc_list->head->data)->start_address) // verific daca e inaintea headului
		return NULL;

	curr = arena->alloc_list->head;

	do { // verific intre ce blockuri se afla adresa
		block_t *block = (block_t *)curr->data;
		block_t *block_next = (block_t *)curr->next->data;
		if (block->start_address + block->size <= address)
			if (curr->next == arena->alloc_list->head ||
				address + size <= block_next->start_address) {
				return curr;
			}
		curr = curr->next;
	} while (curr != arena->alloc_list->head);

	return NULL;

}

dll_node_t *verificare_miniblock(arena_t *arena, const uint64_t address)
{
	dll_node_t *minicurr;
	dll_node_t *curr = arena->alloc_list->head;

	do {
		block_t *block = (block_t *)curr->data;
		minicurr = ((doubly_list_t *)block->miniblock_list)->head;
		// headul listei de miniblockui, al blockului
		do {
			if (((miniblock_t *)minicurr->data)->start_address == address)
				return minicurr;

			minicurr = minicurr->next;
		} while (minicurr != ((doubly_list_t *)block->miniblock_list)->head);

		curr = curr->next;
	} while (curr != arena->alloc_list->head);

	return NULL; // nu a fost gasit miniblockul cu adresa respecitva
}

dll_node_t *verificare_miniblock_write(arena_t *arena, const uint64_t address,
	dll_node_t *curr)
{
	block_t *block1 = ((block_t *)curr->data);
	dll_node_t *minicurr = ((doubly_list_t *)block1->miniblock_list)->head; // headul listei de miniblockui, al blockului

	do {
		uint64_t adresa_inceput = ((miniblock_t *)minicurr->data)->start_address;
		uint64_t size = ((miniblock_t *)minicurr->data)->size;
		uint64_t adresa_final = adresa_inceput + size;

		if (adresa_inceput <= address && address < adresa_final)
			return minicurr;

		minicurr = minicurr->next;
	} while (minicurr != ((doubly_list_t *)block1->miniblock_list)->head);

	return NULL; // nu a fost gasit miniblockul cu adresa respecitva
}

dll_node_t *verificare_miniblock_mprotect(arena_t *arena, const uint64_t address,
	dll_node_t *curr)
{
	block_t *block1 = ((block_t *)curr->data);
	dll_node_t *minicurr = ((doubly_list_t *)block1->miniblock_list)->head; // headul listei de miniblockui, al blockului

	do {
		uint64_t adresa_inceput = ((miniblock_t *)minicurr->data)->start_address;

		if (adresa_inceput == address)
			return minicurr;

		minicurr = minicurr->next;
	} while (minicurr != ((doubly_list_t *)block1->miniblock_list)->head);

	return NULL; // nu a fost gasit miniblockul cu adresa respecitva
}

dll_node_t *verificare_block_write(arena_t *arena, const uint64_t address)
{
	dll_node_t *curr;
	if (arena->alloc_list->size == 0)
		return NULL;

	curr = arena->alloc_list->head;

	do { // verific intre ce blockuri se afla adresa
		block_t *block = (block_t *)curr->data;
		if (block->start_address <= address)
			if (address < block->start_address + block->size)
				return curr;

		curr = curr->next;
	} while (curr != arena->alloc_list->head);

	return NULL;
}

dll_node_t *nod_cu_miniblock(arena_t *arena, const uint64_t address)
{
	dll_node_t *curr = arena->alloc_list->head;

	do { // in ce block se afla miniblockul
		block_t *block = (block_t *)curr->data;
		if (block->start_address <= address &&
			address <= block->start_address + block->size)
			return curr;

		curr = curr->next;
	} while (curr != arena->alloc_list->head);
}

doubly_list_t *imbinare_liste(doubly_list_t *list1, doubly_list_t *list2)
{
	dll_node_t *aux = list1->head->prev;

	// for (int j = 0; j < list2->size; j++) {
	// 	dll_add_nth_node(list1, list1->size + j, curr->data);
	// 	curr = curr->next;
	// }

	// dll_free(&list2);

	list1->head->prev->next = list2->head;
	list1->head->prev = list2->head->prev;
	list2->head->prev->next = list1->head;
	list2->head->prev = aux;

	list1->size += list2->size;

	free(list2);

	return list1;
}

int adresa_valida(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (arena->alloc_list->size == 0)
		return 1;

	uint64_t dimensiune = address + size;
	if (dimensiune <= ((block_t *)arena->alloc_list->head->data)->start_address) // verific daca e inaintea headului
		return 1;

	dll_node_t *curr = arena->alloc_list->head;

	do { // verific intre ce blockuri se afla adresa
		block_t *block = (block_t *)curr->data;
		block_t *block_next = (block_t *)curr->next->data;
		if (block->start_address + block->size <= address)
			if (curr->next == arena->alloc_list->head ||
				address + size <= block_next->start_address)
				return 1;

		curr = curr->next;
	} while (curr != arena->alloc_list->head);

	return 0;
}

void alloc_block_alone(arena_t *arena, const uint64_t address,
	const uint64_t size, uint64_t adr_fin, uint64_t adr_inc, dll_node_t *curr)
{
	block_t *block = malloc(sizeof(block_t)); // am aloicat dinamic blockul ce urmeaza sa fie pus
	block->start_address = address; // am dat adrsa blockului
	block->size = size; // am pus size ul blockului

	block->miniblock_list = dll_create(sizeof(miniblock_t)); // aloc lista de miniblock
	miniblock_t *miniblock = malloc(sizeof(miniblock_t));
	miniblock->start_address = address; // start address ul miniblockului
	miniblock->size = size;
	miniblock->perm = 6;
	miniblock->rw_buffer = malloc(1);
	dll_add_nth_node(((doubly_list_t *)block->miniblock_list), 0, miniblock);

	free(miniblock);

	if (curr == NULL || adr_fin == 0)  // in cazul in care e primul block
		dll_add_nth_node(arena->alloc_list, 0, block);
	else
		dll_add_nth_node(arena->alloc_list, numar_nod(arena, curr) + 1, block);

	free(block);

	return;
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	// blockul de dinaintea adresei
	dll_node_t *curr = verificare_block(arena, address, size);
	uint64_t adresa_final;
	uint64_t adresa_inceput;

	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return;
	} else if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n");
		return;
	}
	if (!adresa_valida(arena, address, size)) {
		printf("This zone was already allocated.\n");
		return;
	}
	if (arena->alloc_list->size == 1) {
		if (address + size <= ((block_t *)arena->alloc_list->head->data)->start_address) {
			adresa_final = 0;
			adresa_inceput = ((block_t *)arena->alloc_list->head->data)->start_address;
		} else {
			adresa_final = ((block_t *)arena->alloc_list->head->data)->start_address + ((block_t *)arena->alloc_list->head->data)->size;
			adresa_inceput = arena->arena_size;
		}
		goto jump;
	} else if (!arena->alloc_list->size) {
		adresa_final = 0;
		adresa_inceput = arena->arena_size;
		goto jump;
	}

	if (!curr)
		adresa_final = 0;
	else
		adresa_final = ((block_t *)curr->data)->start_address + ((block_t *)curr->data)->size;

	if (!curr)
		adresa_inceput = ((block_t *)arena->alloc_list->head->data)->start_address;
	else if (curr == arena->alloc_list->head->prev)
		adresa_inceput = arena->arena_size;
	else
		adresa_inceput = ((block_t *)curr->next->data)->start_address;

jump:

	if (((adresa_final < address || adresa_final == 0) &&
		address + size < adresa_inceput) || arena->alloc_list->size == 0 || (adresa_final < address && adresa_inceput == arena->arena_size)) { // in cazul in care blockul este singur si trebuie alocat
		alloc_block_alone(arena, address, size, adresa_final, adresa_inceput, curr);
		return;
	}

	doubly_list_t *lista_miniblock = dll_create(sizeof(miniblock_t)); // creez lista pentru singurul miniblock
	miniblock_t *miniblock = malloc(sizeof(miniblock_t)); // creez miniblockul
	miniblock->size = size;
	miniblock->perm = 6;
	miniblock->rw_buffer = malloc(1);
	miniblock->start_address = address;
	dll_add_nth_node(lista_miniblock, 0, miniblock); // il adaug in lista
	free(miniblock);

	if ((adresa_final < address && address + size == adresa_inceput) || (adresa_final == address && adresa_final == 0)) // e lipit de blockul din dreapta
	{
		if (curr == NULL)
			curr = arena->alloc_list->head->prev;
		block_t *block = ((block_t *)curr->next->data);
		block->size = block->size + size;
		block->start_address = address;
		doubly_list_t *lista = imbinare_liste(lista_miniblock,
			(doubly_list_t *)block->miniblock_list);
		// ii atribui lista corecta blockului intreg
		block->miniblock_list = lista;

		return;
	}
	if (adresa_final == address && address + size == arena->arena_size) // e lipit de blockul din stanga
	{
		((block_t *)curr->data)->size = ((block_t *)curr->data)->size + size; // ii refac blockului din stanga size ul
		doubly_list_t *lista = imbinare_liste(((doubly_list_t *)((block_t *)curr->data)->miniblock_list), lista_miniblock);
		((block_t *)curr->data)->miniblock_list = lista; // ii atribui lista corecta blockului intreg
		return;
	}
	if (adresa_final == address && address + size == adresa_inceput && adresa_inceput != arena->arena_size) // in cazul in care blockul se afla intre alte 2 blockuri vecine si trebuiesc concatenate(cand am 2 granite)
	{

		((block_t *)curr->data)->size = ((block_t *)curr->data)->size + size + ((block_t *)curr->next->data)->size; // refac size ul intregului block
		doubly_list_t *lista = imbinare_liste(((doubly_list_t *)((block_t *)curr->data)->miniblock_list), lista_miniblock);
		doubly_list_t *lista_final = imbinare_liste(lista, ((doubly_list_t *)((block_t *)curr->next->data)->miniblock_list));
		((block_t *)curr->data)->miniblock_list = lista_final; // ii atribui lista corecta blockului intreg
		dll_remove_nth_node(arena->alloc_list, numar_nod(arena, curr->next));

		return;
	}
	if (adresa_final == address && address + size < adresa_inceput) // e lipit de blockul din stanga
	{
		((block_t *)curr->data)->size = ((block_t *)curr->data)->size + size; // ii refac blockului din stanga size ul
		doubly_list_t *lista = imbinare_liste(((doubly_list_t *)((block_t *)curr->data)->miniblock_list), lista_miniblock);
		((block_t *)curr->data)->miniblock_list = lista; // ii atribui lista corecta blockului intreg
		return;
	}
}

uint64_t dimensiune_stanga(arena_t *arena, dll_node_t *minib, dll_node_t *curr)
{
	uint64_t size = 0;
	block_t *block = (block_t *)curr->data;
	dll_node_t *nod = ((doubly_list_t *)block->miniblock_list)->head;

	do {
		size = size + ((miniblock_t *)nod->data)->size;
		nod = nod->next;
	} while (nod != minib);
	return size;
}

void free_block(arena_t *arena, const uint64_t address)
{
	if (!arena->alloc_list->size) {
		printf("Invalid address for free.\n");
		return;
	}
	dll_node_t *curr = nod_cu_miniblock(arena, address);
	dll_node_t *minicurr = verificare_miniblock(arena, address);
	if (minicurr) { // daca exista miniblockul
		block_t *block1 = (block_t *)curr->data;
		// sterg intregul block pentru ca e de size 1
		if (((doubly_list_t *)block1->miniblock_list)->size == 1) {
			free(((miniblock_t *)minicurr->data)->rw_buffer);
			// dealoc lista de miniblockuri + miniblockul
			dll_free(((doubly_list_t **)&block1->miniblock_list));
			if (arena->alloc_list->size == 1) {
				arena->alloc_list->head = NULL;
				arena->alloc_list->size = 0;
				free(curr->data);
				free(curr);
				return;
			}
			if (curr == arena->alloc_list->head)
				arena->alloc_list->head = curr->next;// reactualizez headul
			curr->prev->next = curr->next; // fac legaturile intre blockuri
			curr->next->prev = curr->prev;
			arena->alloc_list->size--;
			free(curr->data);
			free(curr);
			return;
		}
		// cazul in care sunt mai multe miniblockuri in lista
		miniblock_t *miniblock = (miniblock_t *)minicurr->data;
		doubly_list_t *list1 = (doubly_list_t *)block1->miniblock_list;
		dll_node_t *nod = (dll_node_t *)list1->head;
		miniblock_t *minib = (miniblock_t *)list1->head->prev->data;
		if (address == block1->start_address ||
			address == minib->start_address) { // daca e la inceput sau la final
			if (address == ((miniblock_t *)nod->data)->start_address)
				block1->start_address = address + miniblock->size;

			block1->size -= ((miniblock_t *)minicurr->data)->size;
			int nr_miniblock = numar_miniblock(arena, minicurr, curr);
			free(((miniblock_t *)minicurr->data)->rw_buffer);
			dll_remove_nth_node(list1, nr_miniblock);
			return;
		}
		// CAZUL IN CARE MINIBLICKUL MEU SE AFLA LA MIJLOC
		// fac un nou block pentru partea din dreapta a listei
		// iar pentru cea din stanga il pastrez pe acelasi si doar il updatez
		// dimensiunea listei din stanga de miniblockuri
		const uint64_t dim = dimensiune_stanga(arena, minicurr, curr);
		const uint64_t size = ((miniblock_t *)minicurr->data)->size;
		// fac un block nou pentru partea din dreapta
		block_t *block = malloc(sizeof(block_t));
		block->start_address = address + size;
		block->size = ((block_t *)curr->data)->size - dim - size;
		// aloc lista miniblockului
		block->miniblock_list = dll_create(sizeof(miniblock_t));
		doubly_list_t *lista_noua = (doubly_list_t *)block->miniblock_list;
		// il adaug in lista de blockuri
		dll_add_nth_node(arena->alloc_list, numar_nod(arena, curr) + 1, block);
		free(block);
		int nr_minib = numar_miniblock(arena, minicurr, curr);
		dll_node_t *aux = minicurr->next;
		for (int i = nr_minib + 1; i < list1->size; i++) {
			dll_add_nth_node(lista_noua, lista_noua->size, aux->data);
			aux = aux->next;
		}
		((block_t *)curr->data)->size = dim; // partea din stanga UPDATE
		// start_addresul nu se modifica pentru ca este acelasi block
		int i = nr_minib;
		free(((miniblock_t *)minicurr->data)->rw_buffer);
		do {
			dll_node_t *aux = minicurr->next;
			dll_remove_nth_node(list1, i);
			minicurr = aux;
		} while (minicurr != list1->head);
	} else {
		printf("Invalid address for free.\n");
	}
}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	// block ul in care se afl aminiblockul
	dll_node_t *curr = verificare_block_write(arena, address);
	if (!curr) {
		printf("Invalid address for read.\n");
		return;
	}

	// miniblockul unde vreau sa bag
	dll_node_t *minicurr = verificare_miniblock_write(arena, address, curr);

	uint64_t adresa_inceput_block = ((block_t *)curr->data)->start_address;
	uint64_t dimensiune = ((block_t *)curr->data)->size;
	// unde se termina adresa blockului ca sa stiu pana unde inserez
	uint64_t adr_final = adresa_inceput_block + dimensiune;
	uint64_t k = 0;
	uint64_t adresa_de_citit = address;

	// cat timp nu ajung la final si cat timp am ce sa pun din data
	while (adr_final > adresa_de_citit) {
		// de unde incepe miniblockul
		uint64_t adr_inceput = ((miniblock_t *)minicurr->data)->start_address;
		// cat are miniblockul
		uint64_t size_miniblock = ((miniblock_t *)minicurr->data)->size;
		// unde se termina miniblockul
		uint64_t adresa_final_miniblock = adr_inceput + size_miniblock;

		uint64_t permisiune = ((miniblock_t *)minicurr->data)->perm;
		// verific sa vad daca toate miniblockurile au permisiune de citire
		if (permisiune < 4) {
			printf("Invalid permissions for read.\n");
			return;
		}
		if (k == 0) {
			adresa_de_citit = adresa_final_miniblock;
			k++;
		} else {
			adresa_de_citit += size_miniblock;
		}
		minicurr = minicurr->next;
	}

	if (address + size > adr_final) {
		uint64_t numar_caractere = adr_final - address;
		printf("Warning: size was bigger than the block size. ");
		printf("Reading %ld characters.\n", numar_caractere);
	}

	adresa_de_citit = address;
	k = 0;
	minicurr = verificare_miniblock_write(arena, address, curr);
	// cat timp nu ajung la final si cat timp am ce sa citesc
	while (adr_final > adresa_de_citit && k != size) {
		// de unde incepe miniblockul
		uint64_t adr_inceput = ((miniblock_t *)minicurr->data)->start_address;
		// cat are miniblockul
		uint64_t size_miniblock = ((miniblock_t *)minicurr->data)->size;
		// unde se termina miniblockul
		uint64_t adresa_final_miniblock = adr_inceput + size_miniblock;

		for (uint64_t i = adresa_de_citit; i < adresa_final_miniblock; i++) {
			miniblock_t *minib = (miniblock_t *)minicurr->data;
			printf("%c", ((int8_t *)minib->rw_buffer)[i - adr_inceput]);
			k++;
			if (k == size) { // am bagat ot ce trebuia in block ul meu
				printf("\n");
				return;
			}
		}

		if (k == adresa_final_miniblock - address)
			adresa_de_citit = adresa_final_miniblock;
		else
			adresa_de_citit += size_miniblock;
		minicurr = minicurr->next;
	}
	printf("\n");
}

void
write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
	// block ul in care se afl aminiblockul
	dll_node_t *curr = verificare_block_write(arena, address);
	if (!curr) {
		printf("Invalid address for write.\n");
		return;
	}
	// miniblockul unde vreau sa scriu
	dll_node_t *minicurr = verificare_miniblock_write(arena, address, curr);

	uint64_t adresa_inceput_block = ((block_t *)curr->data)->start_address;
	uint64_t dimensiune = ((block_t *)curr->data)->size;
	uint64_t k = 0;
	// unde se termina adresa blockului ca sa stiu pana unde inserez
	uint64_t adr_final = adresa_inceput_block + dimensiune;
	// unde se termina adresa blockului ca sa stiu pana unde inserez
	uint64_t adr_de_inserat = address;

	// cat timp nu ajung la final si cat timp am ce sa pun din data
	while (adr_final > adr_de_inserat && adr_de_inserat < address + size) {
		// de unde incepe miniblockul
		uint64_t adr_inceput = ((miniblock_t *)minicurr->data)->start_address;
		// cat are miniblockul
		uint64_t size_miniblock = ((miniblock_t *)minicurr->data)->size;
		// unde se termina miniblockul
		uint64_t adresa_final_miniblock = adr_inceput + size_miniblock;

		int perm = ((miniblock_t *)minicurr->data)->perm;
		// verific sa vad daca toate miniblockurile au permisiuni de scriere
		if (perm != 6 && perm != 3 && perm != 2 && perm != 7) {
			printf("Invalid permissions for write.\n");
			return;
		}
		if (k == 0) {
			adr_de_inserat = adresa_final_miniblock;
			k++;
		} else {
			adr_de_inserat += size_miniblock;
		}
		minicurr = minicurr->next;
	}

	k = 0;
	adr_de_inserat = address;
	minicurr = verificare_miniblock_write(arena, address, curr);
	// cat timp nu ajung la final si cat timp am ce sa pun din data
	while (adr_final > adr_de_inserat && k != size) {
		// de unde incepe miniblockul
		uint64_t adr_inceput = ((miniblock_t *)minicurr->data)->start_address;
		// cat are miniblockul
		uint64_t size_miniblock = ((miniblock_t *)minicurr->data)->size;
		// unde se termina miniblockul
		uint64_t adresa_final_miniblock = adr_inceput + size_miniblock;

		free(((miniblock_t *)minicurr->data)->rw_buffer);
		((miniblock_t *)minicurr->data)->rw_buffer = malloc(size_miniblock *
			sizeof(int8_t));

		for (uint64_t i = adr_de_inserat; i < adresa_final_miniblock; i++) {
			if (k == size) // am bagat tot ce trebuia in block ul meu
				return;
			miniblock_t *minib = (miniblock_t *)minicurr->data;
			((int8_t *)minib->rw_buffer)[i - adr_inceput] = data[k];
			k++;
		}
		if (k == adresa_final_miniblock - address)
			adr_de_inserat = adresa_final_miniblock;
		else
			adr_de_inserat += size_miniblock;

		minicurr = minicurr->next;
	}

	if (address + size > adr_final) {
		uint64_t numar_caractere = adr_final - address;
		printf("Warning: size was bigger than the block size. ");
		printf("Writing %ld characters.\n", numar_caractere);
	}
}

void pmap(const arena_t *arena)
{
	printf("Total memory: 0x%lX bytes\n", arena->arena_size);

	uint64_t free_size = arena->arena_size;
	int numar_blockuri = 0;
	int numar_miniblockuri = 0;
	if (arena->alloc_list->size) {
		dll_node_t *curr = arena->alloc_list->head;
		dll_node_t *minicurr;
		do {
			free_size = free_size - ((block_t *)curr->data)->size;
			block_t *block1 = (block_t *)curr->data;
			doubly_list_t *list = (doubly_list_t *)block1->miniblock_list;
			minicurr = list->head;
			do {
				numar_miniblockuri++;
				minicurr = minicurr->next;
			} while (minicurr != list->head);
			numar_blockuri++;
			curr = curr->next;
		} while (curr != arena->alloc_list->head);
	} else {
		free_size = arena->arena_size;
		numar_blockuri = 0;
		numar_miniblockuri = 0;
	}

	printf("Free memory: 0x%lX bytes\n", free_size);
	printf("Number of allocated blocks: %d\n", numar_blockuri);
	printf("Number of allocated miniblocks: %d\n", numar_miniblockuri);

	if (arena->alloc_list->size) {
		dll_node_t *curr = arena->alloc_list->head;
		numar_blockuri = 1;
		do {
			// adresa de inceput a blockului
			uint64_t adr_inceput = ((block_t *)curr->data)->start_address;
			// size ul blockului
			size_t size = ((block_t *)(curr->data))->size;
			uint64_t adresa_final = adr_inceput + size;

			printf("\nBlock %d begin\n", numar_blockuri);
			printf("Zone: 0x%lX - 0x%lX\n", adr_inceput, adresa_final);

			block_t *block1 = (block_t *)curr->data;
			doubly_list_t *list = (doubly_list_t *)block1->miniblock_list;

			dll_node_t *minicurr = list->head;
			numar_miniblockuri = 1;
			do {
				printf("Miniblock %d:\t\t", numar_miniblockuri);
				adr_inceput = ((miniblock_t *)minicurr->data)->start_address;
				size = ((miniblock_t *)minicurr->data)->size;
				adresa_final = adr_inceput + size;
				printf("0x%lX\t\t-\t\t0x%lX\t", adr_inceput, adresa_final);
				uint8_t permisiune = ((miniblock_t *)minicurr->data)->perm;
				if (permisiune == 0)
					printf("\t| ---\n");
				if (permisiune == 1)
					printf("\t| --X\n");
				if (permisiune == 2)
					printf("\t| -W-\n");
				if (permisiune == 3)
					printf("\t| -WX\n");
				if (permisiune == 4)
					printf("\t| R--\n");
				if (permisiune == 5)
					printf("\t| R-X\n");
				if (permisiune == 6)
					printf("\t| RW-\n");
				if (permisiune == 7)
					printf("\t| RWX\n");
				numar_miniblockuri++;
				minicurr = minicurr->next;
			} while (minicurr != list->head);
			printf("Block %d end\n", numar_blockuri);
			numar_blockuri++;
			curr = curr->next;
		} while (curr != arena->alloc_list->head);
	}
}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	// block ul in care se afl aminiblockul
	dll_node_t *curr = verificare_block_write(arena, address);
	if (!curr) {
		printf("Invalid address for mprotect.\n");
		return;
	}

	// miniblockul unde vreau sa bag
	dll_node_t *minicurr = verificare_miniblock_mprotect(arena, address, curr);

	if (!minicurr) {
		printf("Invalid address for mprotect.\n");
		return;
	}

	((miniblock_t *)minicurr->data)->perm = 0;

	char *token = strtok(permission, " ");

jump:

	if (!token)
		return;

	if (!strncmp(token, "PROT_NONE", 9)) {
		((miniblock_t *)minicurr->data)->perm = 0;
		return;
	}
	if (!strncmp(token, "PROT_READ", 9))
		((miniblock_t *)minicurr->data)->perm += 4;

	if (!strncmp(token, "PROT_WRITE", 10))
		((miniblock_t *)minicurr->data)->perm += 2;

	if (!strncmp(token, "PROT_EXEC", 9))
		((miniblock_t *)minicurr->data)->perm += 1;

	token = strtok(NULL, " |");
	goto jump;
}
