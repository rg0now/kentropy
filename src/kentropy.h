/*
 * kentropy.h
 *
 *  Created on: Jan 9, 2014
 *      Author: andras
 */

#ifndef KENTROPY_H_
#define KENTROPY_H_

#include <stdint.h>
#include "cnht.h"

typedef struct
{
	uint32_t num;
	cnht *tbl;
} store;

typedef struct chll
{
	unsigned char c;
	uint32_t num;
	struct chll *next;
} chll;

#endif /* KENTROPY_H_ */
