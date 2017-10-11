#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "hash.h"

enum estados {
	vacio, 
	ocupado, 
	borrado
};

typedef struct nodo{
	const char* clave;
	void* valor;
	enum estados estado;
}nodo_t;

struct hash{
	nodo_t** tabla_hash;
	size_t usados; //Elementos ocupados.
	size_t tamanio; //Largo de la tabla de hash
	hash_destruir_dato_t destruir_dato;
};
struct hash_iter{
	const hash_t* hash;
	size_t  pos_actual;
};

// Crea un nodo.
// Post: devuelve un nuevo nodo con clave y valor en NULL y estado vací.

nodo_t* nodo_crear(){
	nodo_t* nodo = malloc(sizeof(nodo_t));
	if (nodo == NULL) return NULL;

	nodo->clave = NULL;
	nodo->valor = NULL;
	nodo->estado = vacio;
	return nodo;
}


hash_t *hash_crear(hash_destruir_dato_t destruir_dato);
bool hash_guardar(hash_t *hash, const char *clave, void *dato);
void *hash_borrar(hash_t *hash, const char *clave);
void *hash_obtener(const hash_t *hash, const char *clave);
bool hash_pertenece(const hash_t *hash, const char *clave);
size_t hash_cantidad(const hash_t *hash);
void hash_destruir(hash_t *hash);

/* Iterador del hash */

hash_iter_t *hash_iter_crear(const hash_t *hash);
bool hash_iter_avanzar(hash_iter_t *iter);
const char *hash_iter_ver_actual(const hash_iter_t *iter);
bool hash_iter_al_final(const hash_iter_t *iter);
void hash_iter_destruir(hash_iter_t* iter);
