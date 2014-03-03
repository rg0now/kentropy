/*
 * mdht.h
 *
 *  Created on: Jan 8, 2014
 *      Author: andras
 */

#ifndef MDHT_H_
#define MDHT_H_

#include <stdint.h>

typedef enum {
	MDHT_GET_SUCCESS=0,
	MDHT_ADD_SUCCESS=1
} MDTH_GETORADD_RESULT;

typedef enum {
	MDHT_ENTRY_NORMAL=0,
	MDHT_ENTRY_TABLE=1
} MDHT_ENTRY_TYPE;

typedef enum {
	MDHT_LL_FOUND=0,
	MDHT_LL_NEW=1,
	MDHT_LL_NEXT=2,
	MDHT_LL_REPEAT=3
} MDHT_LL_RESULT;

typedef struct
{
	uint32_t key;
	void *data;
	uint32_t len;
	void *store;

} mdht_entry_normal;

typedef struct mdht_entry
{
	MDHT_ENTRY_TYPE type;
	void *data;

	struct mdht_entry *next;

} mdht_entry;

typedef struct
{
	uint32_t numBuckets;
	uint32_t hash_iv;
	mdht_entry **entries;

} mdht_entry_table;

typedef struct
{
	uint32_t maxLL;

	uint32_t (*funcHash)(const void *data, size_t len, uint32_t iv);
	uint32_t (*funcRnd)();

	mdht_entry_table* table;

} mdht;

mdht *mdhtCreate(
		uint32_t numBuckets,
		uint32_t maxLL,
		uint32_t (*funcHash)(const void *data, size_t len, uint32_t iv),
		uint32_t (*funcRnd)());

mdht_entry *mdhtGetOrAdd(
		mdht *ht,
		void *data,
		size_t len,
		void *store,
		void **storeret,
		MDTH_GETORADD_RESULT *res);

void mdhtDestroy(mdht *ht, void (*funcStoreDestroy)(void *store));

void mdhtPrint(mdht *ht);

#endif /* MDHT_H_ */
