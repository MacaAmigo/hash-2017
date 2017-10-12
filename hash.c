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

/*Recibe una posicion donde sucede una colision y resuelve dicha colision. Devuelve
una nueva posicion proximo a la colision.*/
size_t funcion_perturbacion(hash_t* hash, size_t i){
	while(hash->tabla_hash[i].estado==OCUPADO){
		i++;
		if(i==hash->tamanio)
			i=0;
	}
	return i;
}

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

/*Recibe un hash creado y un numero y multiplica el tamanio actual del hash por
el numero pasado por parametro. Devuelve false en caso de error.*/
bool redimensionar_hash(hast_t* hash,size_t n){
	size_t tamanio_nuevo=hash->tamanio*n;  //agranda el tamanio actual multiplicado x n
	hash_campo_t* tabla_nueva=malloc(sizeof(hash_campo_t)*n);  //pide memoria para la tabla
	if(!tabla_nueva)
		return false;
	for (size_t j=0;j<tamanio_nuevo;j++){
		tabla_nueva[i]=hash_campo_crear();
	}
	for(size_t i=0;i<hash->tamanio; i++){
		hash_campo_t actual=hash->tabla_hash[i];    //actual=nodo
		if(actual.estado==OCUPADO){                 //si el nodo esta ocupado
			size_t i = funcion_hashing(actual.clave, tamanio_nuevo); //rehashea
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
	size_t i = funcion_hashing(clave, hash->tamanio); 
	hash_campo_t actual=hash->tabla_hash[i];
	while(actual.estado!=VACIO && strcmp(actual.clave,clave)!=0){
		if(actual.clave!=NULL){
			if(strcmp(actual.clave,clave==0)
				break;
		}
		i++;
		if(i==hash->tamanio)
			i=0;
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

void *hash_borrar(hash_t *hash, const char *clave){
	if ((1+hash_cantidad(hash)/hash->tamanio) <= FACTOR_CARGA_MIN){ 
		if(!hash_redimensionar(hash, hash->tamanio/2)) return NULL;
	}
	if (!hash_pertenece(hash,clave)) return NULL;
	size_t pos = funcion_hashing(clave, hash->tamanio);	
	while (pos < hash->tamanio){
		if (hash->tabla_hash[pos]->clave != NULL){ // Valido porque si clave es NULL, strcmp pincha.
			if (strcmp(hash->tabla_hash[pos]->clave , clave) == 0 ) break; //Si la clave es la misma, entonces corto iteracion para quedarme con la posición.
		}
		pos++;	
	}
	hash->tabla_hash[pos]->estado = borrado;
	free((char*)hash->tabla_hash[pos]->clave);
	hash->tabla_hash[pos]->clave = NULL;
	void* dato = hash->tabla_hash[pos]->valor;
	hash->tabla_hash[pos]->valor = NULL;
	hash->usados--;
	return dato;
}

void *hash_obtener(const hash_t *hash, const char *clave){
	size_t i = funcion_hashing(clave, hash->tamanio); 
	hash_campo_t actual=hash->tabla_hash[i];
	while(actual.estado!=VACIO && strcmp(actual.clave,clave)!=0){
		i++;
		if(i==hash->tamanio)
			i=0;
		actual=hash->tabla_hash[i];
	}
	if(actual.estado==VACIO)
		return NULL;
	return actual.valor;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
	size_t pos = funcion_hashing(clave, hash->tamanio);
	if  (hash->tabla_hash[pos]->estado == vacio ) return false;
	while (pos < hash->tamanio && hash->tabla_hash[pos]->estado != vacio){
		if (hash->tabla_hash[pos]->estado == ocupado){
			if (strcmp(hash->tabla_hash[pos]->clave , clave) == 0) return true;
		}
		pos++;		
	}

	return false;
}
	

size_t hash_cantidad(const hash_t *hash){
	size_t cantidad=0;
	for(size_t i=0; i<tamanio; i++){
		hash_campo_t actual= hash->tabla_hash[i];
		if(actual.estado==OCUPADO)
			cantidad++;
	}
	return cantidad;
}

void hash_destruir(hash_t *hash){
	size_t pos = 0;
	while (pos < hash->tamanio){
		if (hash->destruir_dato){
			hash->destruir_dato(hash->tabla_hash[pos]->valor);
		}
		free((char*)hash->tabla_hash[pos]->clave); //Libero las claves.
		free(hash->tabla_hash[pos]);  //Libero cada nodo.
		pos++;
	}
	free(hash->tabla_hash); //libero el arreglo de punteros.
	free(hash); //Libero el hash.
}

/* Iterador del hash */

hash_iter_t *hash_iter_crear(const hash_t *hash){
	hash_iter_t iter=malloc(sizeof(hash_iter_t));
	if(!iter)
		return NULL;
	iter->hash=hash;
	size_t i=0;
	while(hash->tabla_hash[i].estado!=OCUPADO){
		i++;
	}
	iter->pos_actual=hash->tabla_hash[i];
	return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){
	if(i>=iter->hash->tamanio)
		return false;
	hash_campo_t actual=iter->hash->tabla_hash[iter->pos_actual];
	while(actual.estado!=OCUPADO && (iter->pos_actual)<(iter->hash->tamanio)){
		(iter->pos_actual)++;
		actual=(iter->hash->tabla_hash)[iter->pos_actual];
	}
	if((iter->pos_actual)<tamanio)
		return true;
	return false;
}

const char* hash_iter_ver_actual(const hash_iter_t *iter){
	return (iter->hash->tabla_hash)[iter->pos_actual].clave;
}

bool hash_iter_al_final(const hash_iter_t *iter){
	return (iter->pos_actual)>=(iter->hash->tamanio);
}

void hash_iter_destruir(hash_iter_t* iter){
	free(iter);
	return;
}
