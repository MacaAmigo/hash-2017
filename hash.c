#define _POSIX_C_SOURCE 200809L //strdup()
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "hash.h"
#define TAM_INI 50
#define FACTOR_CARGA_MAX 0.7
#define FACTOR_CARGA_MIN 0.3
#define REDIMENSION 2


typedef enum estados {
	VACIO,
	OCUPADO,
	BORRADO
}estado_t;

typedef struct hash_campo{
	char* clave;
	void* valor;
	estado_t estado;
} hash_campo_t;

struct hash{
	hash_campo_t* tabla_hash;
	size_t usados; //Elementos ocupados.
	size_t tamanio; //Largo de la tabla de hash
	hash_destruir_dato_t destruir_dato;
};
struct hash_iter{
	const hash_t* hash;
	size_t  pos_actual;
};

// Crea un nodo.
// Post: devuelve un nuevo nodo con clave y valor en NULL y estado vacío.

hash_campo_t hash_campo_crear(){
	hash_campo_t campo;
	campo.clave = NULL;
	campo.valor = NULL;
	campo.estado = VACIO;
	return campo;
}


//Función de hash
//Fowler–Noll–Vo hash function
//Función de hash que devuelve una posición de una tabla hash.
//Pre: Recibe una clave no vacía, y el largo del arreglo.
//Devuelve una posición.
//http://codereview.stackexchange.com/questions/85556/simple-string-hashing-algorithm-implementation
size_t funcion_hashing(const char *clave, size_t largo_primo){
	size_t hash = 0x811c9dc5;
	while (*clave){
		hash ^= (unsigned char) *clave++;
		hash *= 0x01000193;
	}
	return (hash % largo_primo);
}
/*
//Recibe una posicion donde sucede una colision y resuelve dicha colision. Devuelve
//una nueva posicion proximo a la colision.
size_t funcion_perturbacion(hash_campo_t* tabla_hash, size_t tamanio, size_t i){
	while(tabla_hash[i].estado==OCUPADO){
		i++;
		if(i==tamanio)
			i=0;
	}
	return i;
}
*/
hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
	hash_t* hash = malloc(sizeof(hash_t));
	if (!hash) return NULL;
	hash->tabla_hash = malloc(TAM_INI*sizeof(hash_campo_t));
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

/*Recibe un hash creado y un tamanio nuevo y redimensiona el tamanio actual del hash por
el numero pasado por parametro. Devuelve false en caso de error.*/
bool redimensionar_hash(hash_t* hash,size_t tamanio_nuevo){
	hash_campo_t* tabla_nueva = calloc (tamanio_nuevo, sizeof(hash_campo_t));  //pide memoria para la tabla
	if (! tabla_nueva) return false;

	/* Crea los nodos de la nueva tabla*/
	for (size_t j = 0; j < tamanio_nuevo; j++){
		tabla_nueva[j] = hash_campo_crear();
	}

	/*Itero el viejo hash para obtener el par clave-valor*/
	for (size_t i = 0; i < hash->tamanio; i++){
		hash_campo_t actual = hash->tabla_hash[i];
		const char* clave = actual.clave;
		void* dato = hash_obtener(hash,clave);
		size_t posicion = funcion_hashing(clave, tamanio_nuevo);
		if ((tabla_nueva[posicion].estado == OCUPADO) && strcmp(tabla_nueva[posicion].clave, clave)!= 0){ //Caso en que hay que buscar otro lugar libre
				while(posicion < tamanio_nuevo){
					if (tabla_nueva[posicion].estado == VACIO) break; //Corto la iteración y me queda la posición.
					posicion++;
				}
		}
		tabla_nueva[posicion].valor = dato;
		tabla_nueva[posicion].clave = strdup(clave);
		tabla_nueva[posicion].estado = OCUPADO ;

	}
	 /*Libero la vieja tabla hash*/
	free (hash->tabla_hash);
	hash->tabla_hash = tabla_nueva;
	hash->tamanio = tamanio_nuevo;
	return true;
}
/*
bool redimensionar_hash(hash_t* hash,size_t tamanio_nuevo){
	hash_campo_t* tabla_nueva=malloc(sizeof(hash_campo_t)*tamanio_nuevo);  //pide memoria para la tabla
	if(!tabla_nueva)
		return false;
	for (size_t j=0;j<tamanio_nuevo;j++){
		tabla_nueva[j]=hash_campo_crear();
	}
	for(size_t i=0;i<hash->tamanio; i++){
		hash_campo_t actual=hash->tabla_hash[i];
		if(actual.estado==OCUPADO){
			size_t posicion = funcion_hashing(actual.clave, tamanio_nuevo);
			if(tabla_nueva[posicion].estado==OCUPADO)
				posicion=funcion_perturbacion(tabla_nueva,tamanio_nuevo,posicion);
			tabla_nueva[posicion]=actual;
		}
		else if(actual.estado==BORRADO){
			hash->usados -=1;
		}
	}
	free(hash->tabla_hash);
	hash->tabla_hash=tabla_nueva;
	hash->tamanio=tamanio_nuevo;
	return true;
}*/
bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	if ((double long)(hash_cantidad(hash)+1)/(double long)hash->tamanio >= FACTOR_CARGA_MAX){
		if(!redimensionar_hash(hash, hash->tamanio*REDIMENSION))		return false;
	}
	size_t pos = funcion_hashing(clave, hash->tamanio);
	hash_campo_t actual = hash->tabla_hash[pos];
	if (hash_pertenece(hash, clave)){ //Caso en el que hay que reemplazar valor
		while (actual.estado != VACIO && pos < hash->tamanio){
			if((actual.estado == OCUPADO) && (strcmp(actual.clave, clave) == 0)){ //La clave es la misma
				if (hash->destruir_dato){
					hash->destruir_dato(hash->tabla_hash[pos].valor);  //Destruyo el valor.
				}
				hash->tabla_hash[pos].valor = dato;
				return true;
			}
			pos++;
		}
	}
	while (pos < hash->tamanio){ //Caso en el que puedo guardar o debo buscar otra posición
		if (actual.estado == VACIO){
			(hash->tabla_hash[pos]).clave = strdup(clave);
			(hash->tabla_hash[pos]).estado = OCUPADO;
			(hash->tabla_hash[pos]).valor = dato;
			hash->usados++;
			return true;
		}
		pos++;
	}
	return false;
}

void* hash_borrar(hash_t *hash, const char *clave){
	if (((double long)(hash->usados-1)/(double long)hash->tamanio) < FACTOR_CARGA_MIN && hash->tamanio>TAM_INI){
		if(!redimensionar_hash(hash, hash->tamanio/REDIMENSION))
			return NULL;
	}
	size_t pos = funcion_hashing(clave, hash->tamanio);
	while ((hash->tabla_hash[pos]).estado != VACIO){
		if ((hash->tabla_hash[pos]).clave != NULL){
			if (strcmp((hash->tabla_hash[pos]).clave , clave) == 0 )
				break;
		}
		pos++;
		if(pos == hash->tamanio)
			pos=0;
	}
	if((hash->tabla_hash[pos]).estado == VACIO)
		return NULL;
	(hash->tabla_hash[pos]).estado = BORRADO;
	free((char*)(hash->tabla_hash[pos]).clave);
	(hash->tabla_hash[pos]).clave = NULL;
	void* dato = (hash->tabla_hash[pos]).valor;
	(hash->tabla_hash[pos]).valor = NULL;
	hash->usados--;
	return dato;
}

void *hash_obtener(const hash_t *hash, const char *clave){
	size_t pos = funcion_hashing(clave, hash->tamanio);
	if (!hash_pertenece(hash,clave)) return NULL;
	hash_campo_t actual = hash->tabla_hash[pos];
	while (pos < hash->tamanio){
		if (strcmp (actual.clave,clave) == 0){
				return actual.valor;
		}
		pos++;
	}
	return NULL;
}
bool hash_pertenece(const hash_t *hash, const char *clave){
	size_t pos = funcion_hashing(clave, hash->tamanio);
	while (pos < hash->tamanio && (hash->tabla_hash[pos]).estado != VACIO){
		if ((hash->tabla_hash[pos]).estado == OCUPADO){
			if (strcmp((hash->tabla_hash[pos]).clave , clave) == 0)	return true;
		}
		pos++;
	}
	return false;
}


size_t hash_cantidad(const hash_t *hash){
	return hash->usados;
}/*	size_t cantidad=0;
	for(size_t i=0; i<hash->tamanio; i++){
		hash_campo_t actual= hash->tabla_hash[i];
		if(actual.estado==OCUPADO)
			cantidad++;
	}
	return cantidad;
}
*/
void hash_destruir(hash_t *hash){
	size_t pos = 0;
	while (pos < hash->tamanio){
		if((hash->tabla_hash[pos]).estado==OCUPADO){
			if (hash->destruir_dato)
				hash->destruir_dato((hash->tabla_hash[pos]).valor);
			free((hash->tabla_hash[pos]).clave);//Libero las claves.
		}
		pos++;
	}
	free(hash->tabla_hash); //libero el arreglo.
	free(hash); //Libero el hash.
}

/* Iterador del hash */

hash_iter_t *hash_iter_crear(const hash_t *hash){
	hash_iter_t* iter = malloc(sizeof(hash_iter_t));
	if(!iter)  return NULL;
	iter->hash = hash;
	if (hash_cantidad(hash) == 0) {
		iter->pos_actual = 0;
		return iter;
	}
	size_t pos = 0;
	while ((hash->tabla_hash[pos]).estado != OCUPADO){
		if (pos == hash->tamanio)
			break;
		pos++;
	}
	iter->pos_actual = pos;
	return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){
	if (hash_iter_al_final(iter)) return false;
	iter->pos_actual++;
	while (iter->pos_actual < iter->hash->tamanio){
		if (iter->hash->tabla_hash[iter->pos_actual].estado == OCUPADO)  break;
		iter->pos_actual++;
	}
	return true;
}

const char* hash_iter_ver_actual(const hash_iter_t *iter){
	return (iter->hash->tabla_hash)[iter->pos_actual].clave;
}

bool hash_iter_al_final(const hash_iter_t *iter){
	if (hash_cantidad(iter->hash) == 0) return true;
	size_t aux = iter->pos_actual;
	while (aux < iter->hash->tamanio){
		if ((iter->hash->tabla_hash[aux].estado == OCUPADO)){
			return false;
		}
		aux++;
	}
	return true;
}

void hash_iter_destruir(hash_iter_t* iter){
	free(iter);
return;
}
