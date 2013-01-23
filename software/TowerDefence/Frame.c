/*
 * Frame.c
 *
 *  Created on: 2013-01-20
 *      Author: Danny
 */

#include "Frame.h"

struct Frame* initFrame(int x1, int y1, int x2, int y2, struct GameInfo* g) {
	struct Frame* f = (struct Frame*)malloc(sizeof(struct Frame));
	f->super = initObject(x1, y1, x2-x1, NULL);
	f->super->class = panel;
	f->ltx = x1;
	f->lty = y1;
	f->rbx = x2;
	f->rby = y2;
	f->statesLine = 0;
	f->game = g;
	f->update = NULL;
	f->elements = NULL;
	f->str = NULL;
	return f;
}

struct Frame* initGamePanel(int x1, int y1, int x2, int y2, struct GameInfo* g, alt_up_pixel_buffer_dma_dev* pixel_buffer) {

	alt_up_pixel_buffer_dma_draw_box(pixel_buffer, 250, 0, 320, 240, 0x7BEF, 0);
	alt_up_pixel_buffer_dma_draw_box(pixel_buffer, 253, 3, 317, 237, 0xBDF7, 0);
	struct Frame* f = initFrame(x1, y1, x2, y2, g);
	f->elements = (struct Frame**)malloc(sizeof(struct Frame*)*3);
	f->elements[0] = initScoreBoard(255, 5, 315, 20, g, pixel_buffer);
	f->elements[0]->update = updateScoreFrame;
	f->elements[1] = initCtrlPanel(0, 0, 0, 0, g, pixel_buffer);
	f->elements[1]->update = updateCtrlPanel;
	f->elements[2] = initMsgBoard(255, 180, 315, 235, g, pixel_buffer);
	f->elements[2]->update = updateMsgBoard;
	return f;
}

struct Frame* initScoreBoard(int x1, int y1, int x2, int y2, struct GameInfo* g, alt_up_pixel_buffer_dma_dev* pixel_buffer) {
	alt_up_pixel_buffer_dma_draw_box(pixel_buffer, x1, y1, x2, y2, getColor(0x11, 0x00, 0x22), 0);
	struct Frame* f = initFrame(x1, y1, x2, y2, g);
	f->elements = (struct Frame**)malloc(sizeof(struct Frame*)*3);
	return f;
}

struct Frame* initCtrlPanel(int x1, int y1, int x2, int y2, struct GameInfo* g, alt_up_pixel_buffer_dma_dev* pixel_buffer) {

	struct Frame* f = initFrame(x1, y1, x2, y2, g);
	f->elements = (struct Frame**)malloc(sizeof(struct Frame*)*3);
	return f;
}

struct Frame* initMsgBoard(int x1, int y1, int x2, int y2, struct GameInfo* g, alt_up_pixel_buffer_dma_dev* pixel_buffer) {
	alt_up_pixel_buffer_dma_draw_box(pixel_buffer, x1, y1, x2, y2, getColor(0x25, 0x38, 0x3C), 0);
	struct Frame* f = initFrame(x1, y1, x2, y2, g);
	f->str = (char*)malloc(2*sizeof(char));
	return f;
}
/*
 * draw the main panel and update all frames in the panel
 */
void updateGamePanel(struct Frame* f) {
	  int i;
	  for(i = 0; i < 3; i ++) {
		  f->elements[i]->update(f->elements[i]);
	  }
}
void updateCtrlPanel(struct Frame* f) {

}
void updateMsgBoard(struct Frame* f) {
	//updateMSG(f, f->str);
}

/*Update the score board with current score*/
void updateScoreFrame(struct Frame* f) {
	char string[12], currency[14];
	sprintf(string, "Score %d", f->game->score);
	sprintf(currency, "$$ -> %d", f->game->currency);
	alt_up_char_buffer_string(char_buffer, string, 65, 2);
	alt_up_char_buffer_string(char_buffer, currency, 65, 4);
}

void writeMSG(struct Frame* f, char* str) {
	updateMSG(f->elements[2], str);
}
/*Update the status board with message; will warp words to next line*/
void updateMSG(struct Frame* f, char* msg) {
	int i = 0, isPrint = 1, len1 = 0, len2 = 0;

	if(msg != NULL) {
		free(f->str);
		f->str = (char*)malloc(sizeof(char)*strlen(msg));
		strcpy(f->str, msg);
		if(strcmp(msg, " ") == 0) { //clear the screen message if it is space character
			while(f->statesLine > 0) {
				alt_up_char_buffer_string(char_buffer, "                 ", 64, 46+f->statesLine);
				f->statesLine--;
			}
		} else {
			char* string[30];
			char string1[15], string2[15];
			while((string[i] = strsep(&msg, " "))!=NULL) {
				strcpy(string1, string[i-1]);
				if(isPrint == 0) {
					len1 = strlen(string[i-1]);
					len2 = strlen(string[i]);
					if(len1+len2 < 15) {
						strcpy(string2, string[i]);
						strncat(string1, " ", 1);
						strncat(string1, string2, len2);
						isPrint = 1;
					}
					alt_up_char_buffer_string(char_buffer, string1, 64, 46+f->statesLine);
					f->statesLine++;
				} else isPrint = 0;
				i++;
			}alt_up_char_buffer_string(char_buffer, string[i-1], 64, 46+f->statesLine);
		}
	}f->statesLine+=2; //Next msg can show after an empty line
}
