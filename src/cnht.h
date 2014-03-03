/*
 * cnht.h
 *
 *  Created on: Jan 9, 2014
 *      Author: andras
 */

#ifndef CNHT_H_
#define CNHT_H_

#include <stdint.h>

#define CNHT_BUCKETS 8

typedef struct cnht_entry
{
	char c;
	uint32_t num;
	struct cnht_entry *next;

} cnht_entry;

typedef struct
{
	cnht_entry **entries;

} cnht;

cnht *cnhtCreate();
void cnhtSetOrInc(cnht *tbl, char c, uint32_t val);
void cnhtDestroy(cnht *tbl);

#endif /* CNHT_H_ */
