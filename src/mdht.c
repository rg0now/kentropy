/*
 * Majdan Dynamic Hash Table
 * 
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 * 
 * Report bugs to <majdan.andras@gmail.com>
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "mdht.h"

static mdht_entry *mdhtGetOrAddLL(
		mdht *ht,
		uint32_t numbuckets,
		mdht_entry **entry,
		uint32_t key,
		void *data,
		size_t len,
		void *store,
		void **storeret,
		MDHT_LL_RESULT *res);

static mdht_entry *mdhtRehashLL(mdht *ht, mdht_entry *entry, uint32_t newbuckets);
static int sameData(const char *data1, const char *data2, size_t len);
static void mdhtDestroyTable(
		mdht_entry_table *tbl, void (*funcStoreDestroy)(void *st));

/**
 * Creates a Majdan Dynamic Hash Table
 *
 * @param numBuckets   Buckets number in each table
 * @param maxLL        Max length of linked lists
 * @param funcHash     Hash function to use
 * @param funcRnd      Random function to use
 *
 * You have to seed your random function!
 *
 * @return The hash table
 *
 */
mdht *mdhtCreate(
		uint32_t numBuckets,
		uint32_t maxLL,
		uint32_t (*funcHash)(const void *data, size_t len, uint32_t iv),
		uint32_t (*funcRnd)())
{
	mdht *ht = (mdht*)malloc( sizeof(mdht) );
	ht->maxLL = maxLL;
	ht->funcHash = funcHash;
	ht->funcRnd = funcRnd;

	mdht_entry_table *tbl =
			(mdht_entry_table*)malloc( sizeof(mdht_entry_table) );

	tbl->numBuckets = numBuckets;
	tbl->hash_iv = funcRnd();
	tbl->entries = (mdht_entry**)malloc( sizeof(mdht_entry*) * numBuckets);

	mdht_entry **entry;
	entry = tbl->entries;

	while(numBuckets--)
		*(entry++)=NULL;

	ht->table = tbl;

	return ht;
}

mdht_entry *mdhtGetOrAdd(
		mdht *ht,
		void *data,
		size_t len,
		void *store,
		void **storeret,
		MDTH_GETORADD_RESULT *res)
{
	mdht_entry_table *tbl = ht->table;
	mdht_entry **entry;

	mdht_entry *entryres;
	MDHT_LL_RESULT llres;

	uint32_t key;

	while(1)
	{
		key = ht->funcHash(data, len, tbl->hash_iv);
		entry = &(tbl->entries[key % tbl->numBuckets]);

		llres = MDHT_LL_REPEAT;
		while(llres == MDHT_LL_REPEAT)
			entryres = mdhtGetOrAddLL(ht, tbl->numBuckets, entry, key, data,
							len, store, storeret, &llres);

		if(llres == MDHT_LL_NEXT)
			tbl = (mdht_entry_table*)entryres->data;
		else
		{
			*res = (MDTH_GETORADD_RESULT)llres;
			break;
		}
	}

	return entryres;
}

mdht_entry *mdhtGetOrAddLL(
		mdht *ht,
		uint32_t numbuckets,
		mdht_entry **entry,
		uint32_t key,
		void *data,
		size_t len,
		void *store,
		void **storeret,
		MDHT_LL_RESULT *res)
{
	uint32_t curLL = ht->maxLL;

	mdht_entry **currEntryPlace;
	mdht_entry *currEntry;
	mdht_entry_normal *currEntryNormal;

	currEntryPlace = entry;

	while(curLL--)
	{
		if(*currEntryPlace == NULL)
		{
			*currEntryPlace = (mdht_entry*)malloc( sizeof(mdht_entry) );
			currEntry = *currEntryPlace;

			currEntry->type = MDHT_ENTRY_NORMAL;
			currEntry->next = NULL;
			currEntry->data = (void*)malloc( sizeof(mdht_entry_normal) );

			currEntryNormal = (mdht_entry_normal*)currEntry->data;

			currEntryNormal->key = key;
			currEntryNormal->data = data;
			currEntryNormal->len = len;
			currEntryNormal->store = store;

			*res = MDHT_LL_NEW;
			return currEntry;
		}
		else
		{
			currEntry = *currEntryPlace;

			if(currEntry->type == MDHT_ENTRY_TABLE)
			{
				*res = MDHT_LL_NEXT;
				return currEntry;
			}
			else
			{
				// Normal entry
				currEntryNormal = (mdht_entry_normal*)currEntry->data;
				if( (currEntryNormal->key == key) &&
					(currEntryNormal->len == len))
				{
					if( sameData(currEntryNormal->data, data, len) )
					{
						*res = MDHT_LL_FOUND;
						*storeret = currEntryNormal->store;
						return currEntry;
					}
				}
				currEntryPlace = &(currEntry->next);
			}
		}
	}

	// Rehash linked list
	*entry = mdhtRehashLL(ht, *entry, numbuckets);
	*res = MDHT_LL_REPEAT;
	return NULL;

}

int sameData(const char *data1, const char *data2, size_t len)
{
	while(len--)
		if( *data1++ != *data2++ ) return 0;

	return 1;
}

mdht_entry *mdhtRehashLL(mdht *ht, mdht_entry *entry, uint32_t newbuckets)
{
	mdht_entry *newentry = (mdht_entry*)malloc( sizeof(mdht_entry) );
	newentry->type = MDHT_ENTRY_TABLE;
	newentry->next = NULL;
	newentry->data = (void*)malloc( sizeof(mdht_entry_table) );

	mdht_entry_table *tbl;
	tbl = (mdht_entry_table*)newentry->data;

	tbl->numBuckets = newbuckets;
	tbl->hash_iv = ht->funcRnd();
	tbl->entries = (mdht_entry**)malloc( sizeof(mdht_entry*) * newbuckets);
	mdht_entry **nullentry;
	nullentry = tbl->entries;

	uint32_t buckets = newbuckets;

	while(buckets--)
		*(nullentry++)=NULL;

	uint32_t key;

	mdht_entry *currEntry = entry;
	mdht_entry_normal *currEntryNormal;

	mdht_entry **tblentry;

	while(currEntry != NULL)
	{
		currEntryNormal = (mdht_entry_normal*)currEntry->data;
		key = ht->funcHash(
				currEntryNormal->data,
				currEntryNormal->len,
				tbl->hash_iv);
		currEntryNormal->key = key;
		tblentry = &(tbl->entries[key % newbuckets]);

		while(*tblentry != NULL)
			tblentry = &((*tblentry)->next);

		*tblentry = currEntry;
		currEntry = currEntry->next;
		(*tblentry)->next = NULL;
	}

	return newentry;
}

void printSpaces(uint32_t spaces)
{
	while(spaces--)
		printf(" ");
}

void mdhtPrintTable(mdht_entry_table *tbl, uint32_t spaces)
{
	uint32_t numBuckets = tbl->numBuckets;
	mdht_entry_normal *entry;
	mdht_entry *tmp;

	printf("buckets=%"PRIu32" iv=%"PRIu32" \n", numBuckets, tbl->hash_iv);
	printSpaces(spaces);
	printf("\\\n");
	printSpaces(spaces);
	printf(" \\\n");

	for(uint32_t i=0; i < numBuckets; i++)
	{
		if(tbl->entries[i] != NULL)
		{
			if(tbl->entries[i]->type == MDHT_ENTRY_TABLE)
			{
				printSpaces(spaces);
				printf("|--[%"PRIu32"] table ", i);
				mdhtPrintTable(
						(mdht_entry_table *)tbl->entries[i]->data, spaces+2);
			}
			else
			{
				printSpaces(spaces);
				printf("|--");
				entry = (mdht_entry_normal*)tbl->entries[i]->data;
				printf("[%"PRIu32"] entry\n", i);
				printSpaces(spaces);
				printf("  -> key=%"PRIu32"\n", entry->key);
				tmp = tbl->entries[i]->next;
				while(tmp != NULL)
				{
					entry = (mdht_entry_normal*)tmp->data;
					printSpaces(spaces);
					printf("  -> key=%"PRIu32"\n", entry->key);
					tmp = tmp->next;
				}

			}
		}
	}

}

void mdhtPrint(mdht *ht)
{
	mdht_entry_table *tbl = ht->table;
	uint32_t spaces = 0;

	mdhtPrintTable(tbl, spaces);
}

void mdhtDestroy(mdht *ht, void (*funcStoreDestroy)(void *st))
{
	mdhtDestroyTable(ht->table, funcStoreDestroy);
	free(ht->table);
	free(ht);
}

void mdhtDestroyTable(mdht_entry_table *tbl, void (*funcStoreDestroy)(void *st))
{
	mdht_entry *entry, *tmp;
	mdht_entry_normal *entryNormal;
	mdht_entry_table *entryTable;

	for(uint32_t i=0; i < tbl->numBuckets; i++)
	{
		if(tbl->entries[i] != NULL)
		{
			if(tbl->entries[i]->type == MDHT_ENTRY_TABLE)
			{
				entryTable = (mdht_entry_table*)tbl->entries[i]->data;
				mdhtDestroyTable(entryTable, funcStoreDestroy);
				free(entryTable);
				free(tbl->entries[i]);
			}
			else
			{
				entry = tbl->entries[i];
				while(entry != NULL)
				{
					entryNormal = (mdht_entry_normal*)entry->data;

					if(entryNormal->store != NULL)
						funcStoreDestroy(entryNormal->store);

					tmp = entry->next;
					free(entryNormal);
					free(entry);
					entry = tmp;
				}
			}
		}
	}
	free(tbl->entries);
}
