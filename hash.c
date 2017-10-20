#define _POSIX_C_SOURCE 200809L //strdup()
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "hash.h"
#define TAM_INI 50
#define FACTOR_CARGA_MAX 0.60
#define FACTOR_CARGA_MIN 0.10
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
// Devuelve true si pudo destruir todas las claves de la tabla hash.
// Pre: El hash fue creado.
// Post: Se liberaron todas las claves de la tabla hash.

bool claves_destruir(hash_t* hash){
	size_t pos = 0;
	while (pos < hash->tamanio){
		free((char*)hash->tabla_hash[pos].clave); //Libero las claves.
		pos++;
	}		
	return true;	
}
//Recibe un hash y una clave. Devuelve la posicion de la clave.
//Pre: El hash ha sido creado. Recibe una clave.
//Pos: Devuelve la posicion de la clave. Devuelve -1 en caso de no encontrase.
int encontrar_posicion(const hash_t *hash, const char *clave){
	size_t pos = funcion_hashing(clave,hash->tamanio);
	while(hash->tabla_hash[pos].estado != VACIO){
		if(hash->tabla_hash[pos].estado == OCUPADO){
			if(strcmp(hash->tabla_hash[pos].clave,clave) == 0){
				break;
			}
		}
		pos++;
		if(pos == hash->tamanio) pos = 0;
	}
	if(hash->tabla_hash[pos].estado==VACIO)
		return -1;
	return pos;
}

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

bool redimensionar_hash(hash_t* hash,size_t tamanio_nuevo){
	hash_campo_t* tabla_nueva = malloc(sizeof(hash_campo_t)*tamanio_nuevo);  //pide memoria para la tabla
	if(!tabla_nueva)
		return false;
	for (size_t j = 0; j < tamanio_nuevo; j++){
		tabla_nueva[j] = hash_campo_crear();
	}
	for(size_t i = 0; i < hash->tamanio; i++){
		if(hash->tabla_hash[i].estado == OCUPADO){
			size_t posicion = funcion_hashing(hash->tabla_hash[i].clave, tamanio_nuevo);
			if(tabla_nueva[posicion].estado == OCUPADO)
				posicion = funcion_perturbacion(tabla_nueva,tamanio_nuevo,posicion);
			tabla_nueva[posicion] = hash->tabla_hash[i];
		}
		else if(hash->tabla_hash[i].estado == BORRADO){
			hash->usados -= 1;
		}
	}
	free(hash->tabla_hash);
	hash->tabla_hash = tabla_nueva;
	hash->tamanio = tamanio_nuevo;
	return true;
}
/*
bool redimensionar_hash(hash_t*hash, size_t tamanio){
	hash_campo_t* tabla_nueva = calloc (tamanio, sizeof(hash_campo_t));
	if (!tabla_nueva) return false;
	//Creo los nodos de la nueva tabla hash 
	size_t pos_n = 0;
	while (pos_n < tamanio){
		tabla_nueva[pos_n] = hash_campo_crear();
		pos_n++;
	}
	size_t pos = 0;
	while(pos < hash->tamanio){
		if(hash->tabla_hash[pos].estado == OCUPADO){
			const char* clave = hash->tabla_hash[pos].clave;
			size_t pos_hash = funcion_hashing(clave, tamanio);
			while(tabla_nueva[pos_hash].estado != VACIO){
				if(tabla_nueva[pos_hash].estado == VACIO) break;
				pos_hash++;
				if(pos_hash == tamanio) pos_hash = 0;

			}
			//tabla_nueva[pos_hash] = hash->tabla_hash[pos];
			tabla_nueva[pos_hash].valor = dato;
			tabla_nueva[pos_hash].clave = strdup(clave);
			tabla_nueva[pos_hash].estado = OCUPADO;
		
		}
		pos++;
	}
	//Destruyo todas las  claves del viejo hash
	if (!claves_destruir(hash)){
		return false;
	}
	
	free(hash->tabla_hash);
	hash->tabla_hash = tabla_nueva;
	hash->tamanio = tamanio;
	return true;
}*/
/*
bool redimensionar_hash(hash_t*hash, size_t tamanio){
	hash_campo_t* tabla_nueva = calloc (tamanio, sizeof(hash_campo_t));
	if (!tabla_nueva) return false;
	//Creo los nodos de la nueva tabla hash 
	size_t pos_n = 0;
	while (pos_n < tamanio){
		tabla_nueva[pos_n] = hash_campo_crear();
		pos_n++;
	}
	hash_iter_t* iter = hash_iter_crear(hash);
	while (!hash_iter_al_final(iter)){
		const char* clave = hash_iter_ver_actual(iter);
		void* dato = hash_obtener(hash,clave);
		size_t pos_hash = funcion_hashing(clave, tamanio); 
		if (((tabla_nueva[pos_hash].estado == OCUPADO)) && (strcmp(tabla_nueva[pos_hash].clave, clave)!= 0)){ //Caso en que hay que buscar otro lugar libre
			while (pos_hash < tamanio){
				if (tabla_nueva[pos_hash].estado == VACIO) break; //Corto la iteración y me queda la posición.
				pos_hash++;
			}
		}
		tabla_nueva[pos_hash].valor = dato;
		tabla_nueva[pos_hash].clave = strdup(clave);
		tabla_nueva[pos_hash].estado = OCUPADO;
		hash_iter_avanzar(iter);
	}
	hash_iter_destruir(iter);
	//Destruyo todas las  claves del viejo hash
	if (!claves_destruir(hash)){
		return false;
	}
	
	free(hash->tabla_hash);
	hash->tabla_hash = tabla_nueva;
	hash->tamanio = tamanio;
	return true;
}
*/

/*Recibe un hash creado y un tamanio nuevo y redimensiona el tamanio actual del hash por
el numero pasado por parametro. Devuelve false en caso de error.*/
bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	if ((double)(hash_cantidad(hash)+1)/(double)hash->tamanio >= FACTOR_CARGA_MAX){
		if(!redimensionar_hash(hash, hash->tamanio*REDIMENSION))	return false;
	}
	size_t pos = funcion_hashing(clave, hash->tamanio);
	if (hash_pertenece(hash, clave)){ //Caso en el que hay que reemplazar valor
		while (hash->tabla_hash[pos].estado != VACIO && pos < hash->tamanio){
			if((hash->tabla_hash[pos].estado == OCUPADO) && (strcmp(hash->tabla_hash[pos].clave, clave) == 0)){ //La clave es la misma
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
		if (hash->tabla_hash[pos].estado == VACIO){
			(hash->tabla_hash[pos]).clave = strdup(clave);
			(hash->tabla_hash[pos]).estado = OCUPADO;
			(hash->tabla_hash[pos]).valor = dato;
			hash->usados++;
			return true;
		};
		pos++;
		if(pos == hash->tamanio) pos = 0;
	}
	return false;
}
void* hash_borrar(hash_t *hash, const char *clave){
	if ((1+hash_cantidad(hash)/hash->tamanio)<=FACTOR_CARGA_MIN){ 
		if(!redimensionar_hash(hash, hash->tamanio/REDIMENSION)) return NULL;
	}
	if (!hash_pertenece(hash,clave)) return NULL;
	hash_campo_t actual = hash->tabla_hash[encontrar_elemento(hash, clave)];
	actual.estado = BORRADO;
	free((char*)(actual).clave);
	(actual).clave = NULL;
	void* dato = actual.valor;
	actual.valor = NULL;
	hash->usados--;
	return dato;
}
	
/*
void* hash_borrar(hash_t *hash, const char *clave){
	if ((1+hash_cantidad(hash)/hash->tamanio)<=FACTOR_CARGA_MIN){ 
		if(!redimensionar_hash(hash, hash->tamanio/REDIMENSION)) return NULL;
	}
	if (!hash_pertenece(hash,clave)) return NULL;
	size_t pos = funcion_hashing(clave, hash->tamanio);	
	while (pos < hash->tamanio){
		if (hash->tabla_hash[pos].clave != NULL){ // Valido porque si clave es NULL, strcmp pincha.
			if (strcmp(hash->tabla_hash[pos].clave , clave) == 0 ) break; //Si la clave es la misma, entonces corto iteracion para quedarme con la posición.
		}
		pos++;	
	}
	hash->tabla_hash[pos].estado = BORRADO;
	free((char*)(hash->tabla_hash[pos]).clave);
	(hash->tabla_hash[pos]).clave = NULL;
	void* dato = (hash->tabla_hash[pos]).valor;
	(hash->tabla_hash[pos]).valor = NULL;
	hash->usados--;
	return dato;
}
*/
void *hash_obtener(const hash_t *hash, const char *clave){
	if (!hash_pertenece(hash,clave)) return NULL;
	hash_campo_t actual = hash->tabla_hash[encontrar_elemento(hash, clave)];
	return actual.valor;
}


/*void *hash_obtener(const hash_t *hash, const char *clave){
	size_t pos = funcion_hashing(clave, hash->tamanio);
	if (!hash_pertenece(hash,clave)) return NULL;
	while (pos < hash->tamanio){
		if (strcmp (hash->tabla_hash[pos].clave,clave) == 0){
				return hash->tabla_hash[pos].valor;
		}
		pos++;
	}
	return NULL;
}*/
bool hash_pertenece(const hash_t *hash, const char *clave){
	hash_campo_t actual = hash->tabla_hash[encontrar_elemento(hash, clave)];
	if(!actual) return false;
	return true;
}//ACA SIEMPRE DEVUELVE TRUE! CORREGIR!!!!

/*bool hash_pertenece(const hash_t *hash, const char *clave){
	size_t pos = funcion_hashing(clave, hash->tamanio);
	while (pos < hash->tamanio && (hash->tabla_hash[pos]).estado != VACIO){
		if ((hash->tabla_hash[pos]).estado == OCUPADO){
			if (strcmp((hash->tabla_hash[pos]).clave , clave) == 0)	return true;
		}
		pos++;
	}
	return false;
}*/


size_t hash_cantidad(const hash_t *hash){
	return hash->usados;
}

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
	iter->pos_actual = 0;
	while (iter->pos_actual < iter->hash->tamanio){
		if (iter->hash->tabla_hash[iter->pos_actual].estado == OCUPADO) break;
		iter->pos_actual++;
	}
	
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
	if (hash_iter_al_final(iter)) return NULL;
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
}
