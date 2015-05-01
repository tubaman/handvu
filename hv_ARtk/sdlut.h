#ifndef __SDLUT_H__
#define __SDLUT_H__

#include <SDL/SDL.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * GLUT-like interface to SDL, cause GLUT is nice, but SDL is better.
 * Check http://www.libsdl.org/ for shitty documentation. --sjd
 */


#if 0
/* flags to sdlutInit */
#define SDLUT_GRAB	0x01	/* grab all mouse / key events */
#define SDLUT_REPEAT	0x02	/* enable key-repeating */
#endif

/* flags to sdlutInitDisplayMode */
#define SDLUT_DOUBLE	0x01	/* request double-buffering */
#define SDLUT_RGB	0x02	/* request RGB */
#define SDLUT_DEPTH	0x04	/* request depth-buffering */
#define SDLUT_STENCIL	0x08	/* request stencil-buffering */
#define SDLUT_ALPHA	0x10	/* request alpha channel */
#define SDLUT_FSAA	0x20	/* full screen anti aliasing */
#define SDLUT_RGBA	0x12	/* request four channels */

/* flags to sdlutCreateWindow */
#define SDLUT_FULLSCREEN SDL_FULLSCREEN	/* request full-screen mode */


/* must be called first!  initializes SDL */
void sdlutInit ();

/* call with flags to set the appropriate OpenGL attributes */
void sdlutInitDisplayMode (int flags);

/* creates the SDL window - only one allowed! */
void sdlutCreateWindow (const char* title, int width, int height, 
                        int bpp, int flags);

/* destroys the window and the SDL instance */
void sdlutDestroyWindow ();


/* register callbacks that take SDL-style events */
void sdlutActiveFunc   (void (*func)(SDL_ActiveEvent* activeEvent));
void sdlutDisplayFunc  (void (*func)(void));
void sdlutIdleFunc     (void (*func)(void));
void sdlutKeyboardFunc (void (*func)(SDL_KeyboardEvent* keyEvent));
void sdlutMotionFunc   (void (*func)(SDL_MouseMotionEvent* mouseMotionEvent));
void sdlutMouseFunc    (void (*func)(SDL_MouseButtonEvent* mouseButtonEvent));
void sdlutResizeFunc   (void (*func)(SDL_ResizeEvent* resizeEvent));


/* register callbacks that take GLUT-style events */
void sdlutGlutDisplayFunc  (void (*func)(void));
void sdlutGlutIdleFunc     (void (*func)(void));
void sdlutGlutKeyboardFunc (void (*func)(unsigned char key, int x, int y));
void sdlutGlutMouseFunc    (void (*func)(int button, int state, int x, int y));
void sdlutGlutMotionFunc   (void (*func)(int x, int y));
void sdlutGlutReshapeFunc  (void (*func)(int w, int h));


/* after init and registration, enter the main loop to handle events.  
   returns when a quit event is generated */
void sdlutMainLoop ();

/* used like glutPostRedisplay, to signal the screen needs refreshing */
void sdlutPostRedisplay ();

/* when double-buffering, swaps front and back buffers */
void sdlutSwapBuffers ();

/* hack way to shift an ascii keycode, since SDL doesn't. bah. */
unsigned char sdlutShiftASCII (unsigned char c);

/* ticks since last called (0 for first call) */
unsigned int sdlutGetElapsed ();


#ifdef __cplusplus
}
#endif

#endif /* __SDLUT_H__ */
