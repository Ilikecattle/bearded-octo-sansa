/*
 * Env.h
 *
 *  Created on: 2013-01-19
 *      Author: Danny
 */

#ifndef ENV_H_
#define ENV_H_

#include "Object.h"

struct Env {
	struct Object** o;
	int size;
	int coord[2];
	alt_up_pixel_buffer_dma_dev* pixel_buffer;
};

struct Env* initEnv(alt_up_pixel_buffer_dma_dev*);
void addToEnv(struct Env*, struct Object*);
void removeFromEnv(struct Env*, struct Object*);
#endif /* ENV_H_ */
