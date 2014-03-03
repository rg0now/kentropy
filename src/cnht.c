/*
 * Character Number Hash Table
 * 
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 * 
 * Report bugs to <majdan.andras@gmail.com>
 */

#include <stdlib.h>
#include <stdint.h>

#include "cnht.h"

cnht *cnhtCreate()
{
	cnht *tbl = (cnht*)malloc( sizeof(cnht) );
	tbl->entries = (cnht_entry**)calloc( CNHT_BUCKETS, sizeof(cnht_entry*) );

	return tbl;
}

void cnhtSetOrInc(cnht *tbl, char c, uint32_t val)
{
	uint8_t idx = (uint8_t)c % CNHT_BUCKETS;
	cnht_entry *entry, *oldentry;

	if(tbl->entries[idx] == NULL)
	{
		tbl->entries[idx] = (cnht_entry*)malloc( sizeof(cnht_entry) );
		tbl->entries[idx]->c = c;
		tbl->entries[idx]->num = val;
		tbl->entries[idx]->next = NULL;
		return;
	}
	else
	{
		entry = tbl->entries[idx]->next;
		while(entry != NULL)
		{
			if( entry->c == c )
			{
				entry->num++;
				return;
			}
			entry = entry->next;
		}
		oldentry = tbl->entries[idx];
		tbl->entries[idx] = (cnht_entry*)malloc( sizeof(cnht_entry) );
		tbl->entries[idx]->next = oldentry;
		tbl->entries[idx]->c = c;
		tbl->entries[idx]->num = val;
	}

	return;
}

void cnhtDestroy(cnht* tbl)
{
	cnht_entry *tofree;
	cnht_entry *tmp;

	if(tbl == NULL) return;

    for (uint32_t i = 0; i < CNHT_BUCKETS; i++)
    {
        tofree = tbl->entries[i];
        while (tofree != NULL) {
            tmp = tofree->next;
            free(tofree);
            tofree = tmp;
        }
    }

	free(tbl->entries);
	free(tbl);

	return;
}
