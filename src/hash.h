/*
 * hash.h
 *
 *  Created on: Jan 9, 2014
 *      Author: andras
 */

#ifndef HASH_H_
#define HASH_H_

#include <stdint.h>

uint32_t murmur3(const void *data, size_t nbytes, uint32_t iv);

#endif /* HASH_H_ */
