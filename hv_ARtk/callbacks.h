#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "sdlut.h"
//#include "util.h"


enum { RENDER_MODE, SELECT_MODE };
extern int DrawMode;

typedef struct
{
	GLuint		id;
	int		iw;  // image width
	int		ih;
	int		tw;  // texture width (padded!)
	int		th;
	float		s;   // max s coord
	float		t;   // max t coord
} texture_info;

void makeTexture ( unsigned char *pixels, int width, int height, 
                   texture_info *tex );


extern texture_info videoTex;
extern unsigned char *framePixels;


void displayFunc();
void idleFunc();
void keyboardFunc( SDL_KeyboardEvent * );
void mouseFunc( SDL_MouseButtonEvent * ); 
void motionFunc( SDL_MouseMotionEvent * );

/*
void startMenuFunc( int, int, void * );
void windowMenuFunc( int, int, void * );
*/
/*
void snappedDisplayFunc();
void snappedKeyboardFunc( SDL_KeyboardEvent * );
void snappedMouseFunc( int, int, int, int ); 
void snappedMotionFunc( int, int );
*/

#endif /* CALLBACKS_H */
