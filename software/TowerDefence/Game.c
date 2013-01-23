/*
 * Game.c
 * The main models of tower defence game developed by group 5 in EECE 381 class.
 *
 */
#include "Game.h"
int swap;
/*helper function to calculate the distance between two points */
float distance(int x1, int y1, int x2, int y2) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	return sqrt((float)(dx*dx+dy*dy));
}

/*
 * System clock that run the animation for all objects in the environment
 */
alt_u32 my_alarm_callback (void* paras)
{
	int i;
	for(i = 0; i < ((struct Env*)paras)->size; i++) {
		(*(((struct Env*)paras)->o+i))->animate(((struct Env*)paras)->pixel_buffer, &((*(((struct Env*)paras)->o+i))->currImg),
					(*(((struct Env*)paras)->o+i))->x, (*(((struct Env*)paras)->o+i))->y, (*(((struct Env*)paras)->o+i))->scale, 1);

	}
	if(start == 0)
		backgroundAnimation(((struct Env*)paras)->pixel_buffer, ((struct Env*)paras)->coord);

	return alt_ticks_per_second()/10; //100ms
}
/*
 * this function clear the screen and set up pixel buffers for graphics
 */
alt_up_pixel_buffer_dma_dev* initVGA() {
	// Use the name of your pixel buffer DMA core
	alt_up_pixel_buffer_dma_dev* pixel_buffer =alt_up_pixel_buffer_dma_open_dev("/dev/pixel_buffer_dma_0");

	  // Set the background buffer address – Although we don’t use thebackground,
	  // they only provide a function to change the background buffer address, so
	  // we must set that, and then swap it to the foreground.
	  unsigned int pixel_buffer_addr1 = PIXEL_BUFFER_BASE;
	  unsigned int pixel_buffer_addr2 = PIXEL_BUFFER_BASE + (320*240*2);

	  alt_up_pixel_buffer_dma_change_back_buffer_address(pixel_buffer,pixel_buffer_addr1);
	  // Swap background and foreground buffers
	  alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
	  // Wait for the swap to complete
	  while(alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));

	  alt_up_pixel_buffer_dma_change_back_buffer_address(pixel_buffer,pixel_buffer_addr2);
	  // Clear the screen
	  alt_up_pixel_buffer_dma_clear_screen(pixel_buffer, 0);
	  alt_up_pixel_buffer_dma_clear_screen(pixel_buffer, 1);

	 // alt_up_pixel_buffer_dma_draw_line(pixel_buffer, 0, 0, 320, 240, 0xFFFF,1);
	   //Swap background and foreground buffers
	  	  alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
	  	   //Wait for the swap to complete
	  	  while(alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));
	/*  	alt_up_pixel_buffer_dma_draw_line(pixel_buffer, 320, 0, 0, 240, 0xFFFF,0);
	  	alt_up_pixel_buffer_dma_draw_line(pixel_buffer, 0, 0, 320, 240, 0xFFFF,1);

	  	alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
	  while(alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));

	  alt_up_pixel_buffer_dma_draw_line(pixel_buffer, 330, 0, 0, 230, 0xFFFF,1);
	  alt_up_pixel_buffer_dma_draw_line(pixel_buffer, 0, 100, 100, 100, 0xFFFF,0);
  	  alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
		  	  while(alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));
		  	  */
	  //while(1);
	  	  return pixel_buffer;
}

bool loadSDCard(alt_up_sd_card_dev* device) {
	if((device = alt_up_sd_card_open_dev("/dev/SD_Card")) != NULL) {
		if (alt_up_sd_card_is_Present()) {
			printf("Card connected.\n");
			if (alt_up_sd_card_is_FAT16()) {
				printf("FAT16 file system detected.\n");
				return true;
			} else {
				printf("Unknown file system.\n");
			}
		}
	}
	return false;
}

/*
 * Load bitmap image from SD card.  Fixed problem with color coding
 * file name is required to be upper-case and bitmap pixels has to be even size x even size
 */
bool loadSDImage(char* filename, int** destination) {
	int i, j, bytes = 0, offset = 0, size = 0, byte = 0;
	int a, b;
	int file_pointer = alt_up_sd_card_fopen(filename, false);
	if(file_pointer < 0) {
		alt_up_sd_card_fclose(file_pointer); //close the file
		return false;
	}
printf("Loading %s\n", filename);
	//Start reading the bitmap header
	while(bytes < 10) {
		alt_up_sd_card_read(file_pointer);
		bytes++;
	}
	offset = alt_up_sd_card_read(file_pointer);
	while(bytes < offset-1){
		if(bytes == 21) {
			size = alt_up_sd_card_read(file_pointer);
			//printf("size: %d\n", size);
			*destination = (int*)malloc(size*size*sizeof(int));
		} //else if(bytes == 27) {
			//printf("bits: %d\n", alt_up_sd_card_read(file_pointer));
		//} else if(bytes == 29) {
			//printf("compression: %d\n", alt_up_sd_card_read(file_pointer));
		//}
		else
			alt_up_sd_card_read(file_pointer);
		bytes++;
	}
	//Start reading the pixel data
	for(j = size-1; j >= 0; j--) {
		for(i = 0; i < size; i++) {
			a = alt_up_sd_card_read(file_pointer);
			b = alt_up_sd_card_read(file_pointer);
			if(a < 0 || b < 0) {
				printf("%s invalid at pixel[%d, %d]!\n", filename, i, j);
				alt_up_sd_card_fclose(file_pointer);
				return false;
			}
			byte = getColor555(b*256+a);
			*(*destination + j*size+i) = byte;
		}//printf("\n");
	}//*(*destination + i-1) = 0x0000; //for some reason, the last byte it reads is 0xFFFF

	alt_up_sd_card_fclose(file_pointer);
printf("Done!\n");
	return true;
}
/*
 * An abstract function that allows to load all images with same prefix
 *	index is the number of characters that are prefix
 */
struct animation* loadSDImageSeq(char* filename, int index, int size) {
	int i = 0;
	swap = 0;
	char temp[10],buffer[10];
	char* prefix = (char*)malloc(sizeof(char)*(index));
	prefix[index] = '\0';
	int* ptr;

	strncpy(prefix, filename, index);
	struct animation* a;
	struct animation* b;
	while(i < size) {
		strcpy(temp, prefix);
		sprintf(buffer, "%d.BMP", i);
		if(i < 10)
			strncat(temp, buffer, 5);
		else
			strncat(temp, buffer, 6);
		if(start == 1)
			alt_up_char_buffer_string(char_buffer, temp, 48, 30); //show reading file name because we are in loading page
		while(!loadSDImage(temp, &ptr)){
			printf("Loading File Error: %s\n", temp);
		}
		if(i == 0) {
			a = initAnimation(ptr, 1);
			imageInDisk(a);
		}
		else {
			b = initAnimation(ptr, 0);
			imageInDisk(b);
			addImage(a, b);
		}
		i++;
	}
	free(prefix);

	printf("loaded %s %s\n", prefix, temp);
	return a;
}
/*
 * Draw some static images on the background before the game is starting
 */
void backgroundImage(alt_up_pixel_buffer_dma_dev* pixel_buffer) {

	int* pic;
	while(!loadSDImage("E.BMP", &pic));
	draw(pixel_buffer, 210, 200, (int*)pic, 64);
	free(pic);
}
/*
 * check the collision event between each objects in the environment
 */
void checkCollision(struct Env* e) {
	int i, j;
	for(i = 0; i < e->size; i++) {
		for(j = 0; j < e->size; j++) {
			if(i != j) {
				if(distance(e->o[i]->center_x, e->o[i]->center_y, e->o[j]->center_x, e->o[j]->center_y) <= e->o[i]->scale) {
					//e->o[i]->collide(e->o[i], e->o[j]);
					printf("collision!\n");
				}
			}
		}
	}
}
/*
 * Helper method that will print the string on the display
 */
void displayString(char* string, int x, int y) {
	alt_up_char_buffer_clear(char_buffer);
	alt_up_char_buffer_string(char_buffer, string, x, y);
}
/*
 * Main Game Loop
 */
int main()
{
	alt_up_pixel_buffer_dma_dev* pixel_buffer = initVGA();
	alt_alarm alarm;

	char_buffer  = alt_up_char_buffer_open_dev("/dev/char_drawer");
	alt_up_char_buffer_init(char_buffer);

	alt_up_sd_card_dev *device_reference = NULL;
	int frame = 25;
	struct Env* p = initEnv(pixel_buffer);

	/*
	 * Loading Screen if SD card is not presented
	 */
	struct animation* b = initAnimation((int*)pacman01, 1);
	addImage(b, initAnimation((int*)pacman02, 0));
	addImage(b, initAnimation((int*)pacman03, 0));
	addImage(b, initAnimation((int*)pacman04, 0));
	struct Object *face = initObject(100, 130, 10, b);
	addToEnv(p, face);

	alt_alarm_start (&alarm,alt_ticks_per_second(),my_alarm_callback,(void*)p);

	while(!loadSDCard(device_reference)) {
		displayString("Please insert the SD card to start", frame, 30);
		frame++;
		setXY(face, face->x+2, face->y);
		if(face->x >250) face->x = 0;
		if(frame > 60) frame = -5;
		usleep(300000);
	}alt_up_char_buffer_clear(char_buffer);
	removeFromEnv(p, face);
	struct GameInfo* info = initGameInfo();

	alt_alarm_stop(&alarm); //stop the timer for loading files and pages
/*
 * Main Menu Page with animation; since menu is not implemented yet, 300 loops for now
 */

	struct animation* alien0 = loadSDImageSeq("A100.BMP", 2, 2); //2 images where first 2 characters are prefix
	struct Object* alien01 = initObject(50, 50, 10, alien0);
	imageInDisk(alien0);
	addToEnv(p, alien01);

	alt_up_char_buffer_string(char_buffer, "Loading ", 40, 30);
	struct animation* mainAnimation = loadSDImageSeq("MA0.BMP", 2, 59);
	imageInDisk(mainAnimation);
	struct Object* mainpage = initObject(80, 60, 140, mainAnimation);
	addToEnv(p, mainpage);

	alt_up_char_buffer_clear(char_buffer);
	alt_up_char_buffer_string(char_buffer, "Start", 50, 28);
	alt_up_char_buffer_string(char_buffer, "Save", 50, 32);
	alt_up_char_buffer_string(char_buffer, "High Score", 50, 36);
	int i = 0;
	draw(pixel_buffer, mainpage->x, mainpage->y, mainpage->currImg->image,140);
	mainpage->currImg = mainpage->currImg->next;
	swap = 1;
	draw(pixel_buffer, mainpage->x, mainpage->y, mainpage->currImg->image,140);
	mainpage->currImg = mainpage->currImg->next;
	while(i <100){
		//Swap background and foreground buffers
	  	alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
	  	//Wait for the swap to complete
	  	while(alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));

	  	draw(pixel_buffer, mainpage->x, mainpage->y, mainpage->currImg->image,140);
	  	mainpage->currImg = mainpage->currImg->next;
		i++;
	}
	removeFromEnv(p, mainpage); //remove the main page
	alt_up_char_buffer_clear(char_buffer); //remove menu characters
	start = 0;
	swap = 0; //start using only one buffer

	backgroundImage(pixel_buffer);
	//restart the timer
	alt_alarm_start (&alarm,alt_ticks_per_second(),my_alarm_callback,(void*)p);
/**
 * start from here, all objects and animations that been created for test purpose
 */
  struct animation* a = loadSDImageSeq("A000.BMP", 2, 15);
  struct Object *tower = initObject(100, 100, 10, a);
  addToEnv(p, tower);


  struct animation* c = initAnimation((int*)explode11, 1);
  addImage(c, initAnimation((int*)explode12, 0));
  addImage(c, initAnimation((int*)explode13, 0));
  addImage(c, initAnimation((int*)explode14, 0));
  struct Object *ani = initObject(150, 50, 10, c);
  addToEnv(p, ani);

  struct animation* explode = initAnimation((int*)explode01, 1);
  addImage(explode, initAnimation((int*)explode02, 0));
  addImage(explode, initAnimation((int*)explode03, 0));
  addImage(explode, initAnimation((int*)explode04, 0));
  addImage(explode, initAnimation((int*)explode05, 0));
  struct Object *explosion = initObject(60, 150, 10, explode);
  addToEnv(p, explosion);


	struct animation* starAnimation = initAnimation((int*)star, 1);
	struct Object* star = initObject(100, 150, 1, starAnimation);
	addToEnv(p, star);

	struct animation* bossAnimate = loadSDImageSeq("BO00.BMP", 2, 28);
	struct Object* star1 = initObject(10, 150, 20, bossAnimate);
	addToEnv(p, star1);

	struct Frame* gamePanel = initGamePanel(250, 0, 320, 240, info, pixel_buffer);
	gamePanel->update = updateGamePanel;
	writeMSG(gamePanel, "Warning!! Pacman is approaching!!!");
  //draw the panel background

  //again, testing images and objects
  int* ship0, ship1, ship2, ship3, ship4, ship5, ship6, ship7, ship8, ship9;
  loadSDImage("S1.BMP", &ship0);
  loadSDImage("S1_B.BMP", &ship1);
  loadSDImage("S2.BMP", &ship2);
  loadSDImage("S2_B.BMP", &ship9);
  loadSDImage("S3.BMP", &ship3);
  loadSDImage("S3_B.BMP", &ship4);
  loadSDImage("S4.BMP", &ship5);
  loadSDImage("S4_B.BMP", &ship6);
  loadSDImage("S5.BMP", &ship7);
  loadSDImage("S5_B.BMP", &ship8);
  draw(pixel_buffer, 20, 10, ship0, 10);
  draw(pixel_buffer, 30, 10, ship1, 20);
  draw(pixel_buffer, 50, 10, ship2, 10);
  draw(pixel_buffer, 60, 10, ship9, 20);
  draw(pixel_buffer, 80, 10, ship3, 10);
  draw(pixel_buffer, 90, 10, ship4, 20);
  draw(pixel_buffer, 110, 10, ship5, 10);
  draw(pixel_buffer, 120, 10, ship6, 20);
  draw(pixel_buffer, 140, 10, ship7, 10);
  draw(pixel_buffer, 150, 10, ship8, 20);
  /*
   * testing end here
   */

  //this line starts the animation for all objects in the environment
  //alt_alarm_start (&alarm,alt_ticks_per_second(),my_alarm_callback,(void*)p);
  int k = 0; //testing frame variable

  while(1) {
	  //background stars
	  setXY(star, rand()%220, rand()%220);

	  gamePanel->update(gamePanel); //update game panel

	  checkCollision(p); //a major function that check each collision happen between each object

	  //Game Processing
	  setXY(face, face->x+1, face->y);
	  if(face->x > 240) face->x = 10;
	  if(k == 300) {
		  removeFromEnv(p, tower);
		  writeMSG(gamePanel, "UFO hass disappeared!");
	  }
	  info->score++;

	  //nothing running right now..thus slowing down the game speed a bit
	  usleep(100000);
	  k++;
  }
  return 0;
}


