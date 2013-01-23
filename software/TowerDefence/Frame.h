/*
 * Frame.h
 *
 *  Created on: 2013-01-20
 *      Author: Danny
 */

#ifndef FRAME_H_
#define FRAME_H_
#include "Game.h"
#include "GameInfo.h"
struct Frame {
	int ltx, lty, rbx, rby;
	int statesLine;
	struct Object* super;
	struct Frame** elements;
	struct GameInfo* game;
	char* str;
	void (*update)(struct Frame*);
};

struct Frame* initFrame(int, int, int, int, struct GameInfo*);
struct Frame* initGamePanel(int, int, int, int, struct GameInfo*, alt_up_pixel_buffer_dma_dev*);
struct Frame* initScoreBoard(int, int, int, int, struct GameInfo*, alt_up_pixel_buffer_dma_dev*);
struct Frame* initCtrlPanel(int, int, int, int, struct GameInfo*, alt_up_pixel_buffer_dma_dev*);
struct Frame* initMsgBoard(int, int, int, int, struct GameInfo*, alt_up_pixel_buffer_dma_dev*);
void drawFrame(struct Frame*, int);
void updateScoreFrame( struct Frame* f);
void updateMSG(struct Frame*, char*);
void updateGamePanel(struct Frame*);
void updateScoreBoard( struct Frame*);
void updateCtrlPanel(struct Frame*);
void updateMsgBoard(struct Frame*);
void writeMSG(struct Frame*, char*);
#endif /* FRAME_H_ */
