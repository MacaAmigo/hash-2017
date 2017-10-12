#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "hash.h"
#define TAM_INI 50
#define FACTOR_CARGA_MAX 0.7
#define FACTOR_CARGA_MIN 0.3
#define AUMENTO_TAM 2


typedef enum estados {
	VACIO,
	OCUPADO,
	BORRADO
}estado_t;

typedef struct hash_campo{
	const char* clave;
	void* valor;
	estado_t estado;
} hash_campo_t;

struct hash{
	campo_t* tabla_hash;
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

hash_campo_t hash_campo_crear(){
	hash_campo_t campo;
	campo.clave = NULL;
	campo.valor = NULL;
	campo.estado = VACIO;
	return campo;
}

size_t funcion_hash(const char* clave);

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
	hash_t* hash = malloc(sizeof(hash_t));
	if (!hash) return NULL;
	hash->tabla_hash = malloc(TAM_INI*sizeof(campo_t));
	if (!hash->tabla_hash){
		free(hash);
		return NULL;
	}
	hash->tamanio = TAM_INI;
	hash->usados = 0;
	hash->destruir_dato = destruir_dato;
	for (int i = 0; i < hash->tamanio; i++){
		hash->tabla_hash[i] = hash_campo_crear();
	}
	return hash;
}

/*Recibe una posicion donde sucede una colision y resuelve dicha colision. Devuelve
una nueva posicion proximo a la colision.*/
size_t funcion_perturbacion(hash_t* hash, size_t i){
	while(hash->tabla_hash[i].estado!=VACIO){
		i++;
	}
	return i;
}

/*Recibe un hash creado y un numero y multiplica el tamanio actual del hash por
el numero pasado por parametro. Devuelve false en caso de error.*/
bool redimensionar_hash(hast_t* hash,size_t n){
	size_t tamanio_nuevo=hash->tamanio*n;
	hash_campo_t* tabla_nueva=malloc(sizeof(hash_campo_t)*n);
	if(!tabla_nueva)
		return false;
	for(size_t i=0;i<hash->tamanio; i++){
		hash_campo_t actual=hash->tabla_hash[i];
		if(actual.estado==OCUPADO){
			size_t i=funcion_hash(actual.clave)%tamanio_nuevo;
			if(tabla_nueva[i].estado==OCUPADO)
				i=funcion_perturbacion(i);
			tabla_nueva[i]=actual;
		}
		else if(actual.estado==BORRADO){
			hash->usados -=1;
		}
	}
	free(hash->tabla_hash);
	hash->tabla_hash=tabla_nueva;
	hash->tamanio=tamanio_nuevo;
	return true;

}
bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	size_t i=funcion_hash(clave)%(hash->tamanio);
	hash_campo_t actual=hash->tabla_hash[i];
	while(actual.estado!=VACIO && strcmp(actual.clave,clave)!=0){
		i++;
		actual=hash->tabla_hash[i];
	}
	if(actual.estado==VACIO){
		double factor_carga_futuro=(hash->tamanio)/(hash->ocupados+1);
		if(factor_carga_futuro>FACTOR_CARGA_MAX){
			if(!redimensionar_hash(hash,AUMENTO_TAM))
			return false;
		}
		hash->tabla_hash[i].clave=strdup(clave);
		hash->ocupados++;
	}
	hash->tabla_hash[i].valor=dato;
	return true;
}


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
