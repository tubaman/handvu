#include "sdlut.h"

/* I like booleans */
typedef int bool;
#define true 1
#define false 0


/*
 * Function pointers for storing callbacks
 */


bool sdlut_redisplay_necessary;

void (*sdlut_active_function_ptr)   (SDL_ActiveEvent*)		= NULL;
void (*sdlut_display_function_ptr)  (void)			= NULL;
void (*sdlut_idle_function_ptr)     (void)			= NULL;
void (*sdlut_keyboard_function_ptr) (SDL_KeyboardEvent*)	= NULL;
void (*sdlut_motion_function_ptr)   (SDL_MouseMotionEvent*)	= NULL;
void (*sdlut_mouse_function_ptr)    (SDL_MouseButtonEvent*)	= NULL;
void (*sdlut_resize_function_ptr)   (SDL_ResizeEvent*)		= NULL;

void (*sdlut_glut_keyboard_function_ptr) (unsigned char key, 
         int x, int y) = NULL;
void (*sdlut_glut_mouse_function_ptr)    (int button, int state, 
         int x, int y) = NULL;
void (*sdlut_glut_motion_function_ptr)   (int x, int y) = NULL;
void (*sdlut_glut_reshape_function_ptr)  (int w, int h) = NULL;


/*
 * Prototypes of SDL->GLUT event adapter functions
 */


static void sdlut_glut_keyboard_adapter (SDL_KeyboardEvent*);
static void sdlut_glut_mouse_adapter    (SDL_MouseButtonEvent*);
static void sdlut_glut_motion_adapter   (SDL_MouseMotionEvent*);
static void sdlut_glut_reshape_adapter  (SDL_ResizeEvent*);


/*
 * SDL system initialization and window manipulation functions
 */


void sdlutInit ()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("Couldn't init SDL: %s\n", SDL_GetError());
    return;
  }

#if 0
  /* disable key repeating */
  if ((flags & SDLUT_REPEAT) == SDLUT_REPEAT)
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, 
                        SDL_DEFAULT_REPEAT_INTERVAL);
  else
    SDL_EnableKeyRepeat(0, 0);

  /* grab all mouse and keyboard input to window */
  if ((flags & SDLUT_GRAB) == SDLUT_GRAB)
    SDL_WM_GrabInput(SDL_GRAB_ON);
  else
    SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
}


void sdlutInitDisplayMode (int flags)
{
  if ((flags & SDLUT_RGB) == SDLUT_RGB)
  {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 4);
  }

  if ((flags & SDLUT_ALPHA) == SDLUT_ALPHA)
  {
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 4);
  }

  if ((flags & SDLUT_DEPTH) == SDLUT_DEPTH)
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 4);

  if ((flags & SDLUT_STENCIL) == SDLUT_STENCIL)
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 4);

  if ((flags & SDLUT_DOUBLE) == SDLUT_DOUBLE)
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  if ((flags & SDLUT_FSAA) == SDLUT_FSAA)
  {
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  }
}


void sdlutCreateWindow (const char* title, int w, int h, int bpp, int flags)
{
  if (SDL_SetVideoMode(w,h,bpp,SDL_OPENGL|flags) == NULL)
  {
    printf("Couldn't set video mode: %s\n", SDL_GetError());
    return;
  }

  /* set the window title and icon name */
  SDL_WM_SetCaption(title, title);
}


void sdlutDestroyWindow ()
{
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();
}


/*
 * Ah, the main event loop.  Exits when a QUIT event is triggered
 */


void sdlutMainLoop ()
{
  SDL_Event event;
  bool      done = false;

  sdlut_redisplay_necessary = true;

  while (!done)
  {
    if (sdlut_redisplay_necessary && sdlut_display_function_ptr)
    {
      sdlut_display_function_ptr();
      sdlut_redisplay_necessary = false;
    }

    if (SDL_PollEvent(&event))
    {
      do
      {
        switch (event.type)
        {
        case SDL_ACTIVEEVENT:
          if (sdlut_active_function_ptr)
            sdlut_active_function_ptr((SDL_ActiveEvent*) &event);
          break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
          if (sdlut_keyboard_function_ptr)
            sdlut_keyboard_function_ptr((SDL_KeyboardEvent*) &event);
          break;

        case SDL_MOUSEMOTION:
          if (sdlut_motion_function_ptr)
            sdlut_motion_function_ptr((SDL_MouseMotionEvent*) &event);
          break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
          if (sdlut_mouse_function_ptr)
            sdlut_mouse_function_ptr((SDL_MouseButtonEvent*) &event);
          break;

        case SDL_QUIT:
          done = true;
          break;

        case SDL_VIDEORESIZE:
          if (sdlut_resize_function_ptr)
            sdlut_resize_function_ptr((SDL_ResizeEvent*) &event);
          break;

        case SDL_VIDEOEXPOSE:
          sdlut_redisplay_necessary = true;
          break;

        default:
          break;
        }
      } while (SDL_PollEvent(&event));
    }
    else
    {
      if (sdlut_idle_function_ptr)
        sdlut_idle_function_ptr();
    }
  }
}


void sdlutPostRedisplay ()
{
  sdlut_redisplay_necessary = true;
}


void sdlutSwapBuffers ()
{
  SDL_GL_SwapBuffers();
}


/*
 * SDL-style event callbacks
 */


void sdlutActiveFunc (void (*func)(SDL_ActiveEvent*))
{
  sdlut_active_function_ptr = func;
}


void sdlutDisplayFunc (void (*func)(void))
{
  sdlut_display_function_ptr = func;
}


void sdlutIdleFunc (void (*func)(void))
{
  sdlut_idle_function_ptr = func;
}


void sdlutKeyboardFunc (void (*func)(SDL_KeyboardEvent*))
{
  sdlut_keyboard_function_ptr = func;
}


void sdlutMotionFunc (void (*func)(SDL_MouseMotionEvent*))
{
  sdlut_motion_function_ptr = func;
}


void sdlutMouseFunc (void (*func)(SDL_MouseButtonEvent*))
{
  sdlut_mouse_function_ptr = func;
}


void sdlutResizeFunc (void (*func)(SDL_ResizeEvent*))
{
  sdlut_resize_function_ptr = func;
}


/*
 *  GLUT-style event callbacks
 */


void sdlutGlutDisplayFunc (void (*func)(void))
{
  sdlut_display_function_ptr = func;
}


void sdlutGlutIdleFunc (void (*func)(void))
{
  sdlut_idle_function_ptr = func;
}


void sdlutGlutKeyboardFunc (void (*func)(unsigned char key, int x, int y))
{
  sdlut_glut_keyboard_function_ptr = func;

  if (sdlut_glut_keyboard_function_ptr)
    sdlut_keyboard_function_ptr = sdlut_glut_keyboard_adapter;
  else
    sdlut_keyboard_function_ptr = NULL;
}


void sdlutGlutMouseFunc (void (*func)(int button, int state, int x, int y))
{
  sdlut_glut_mouse_function_ptr = func;

  if (sdlut_glut_mouse_function_ptr)
    sdlut_mouse_function_ptr = sdlut_glut_mouse_adapter;
  else
    sdlut_mouse_function_ptr = NULL;
}


void sdlutGlutMotionFunc (void (*func)(int x, int y))
{
  sdlut_glut_motion_function_ptr = func;

  if (sdlut_glut_motion_function_ptr)
    sdlut_motion_function_ptr = sdlut_glut_motion_adapter;
  else
    sdlut_motion_function_ptr = NULL;
}


void sdlutGlutReshapeFunc (void (*func)(int w, int h))
{
  sdlut_glut_reshape_function_ptr = func;

  if (sdlut_glut_reshape_function_ptr)
    sdlut_resize_function_ptr = sdlut_glut_reshape_adapter;
  else
    sdlut_resize_function_ptr = NULL;
}


/*
 * Adapter functions to support SDL->GLUT event translation
 */


/* for use in sdlut_glut_keyboard_adapter, to grab the mouse loc */
static int x, y;

static void sdlut_glut_keyboard_adapter (SDL_KeyboardEvent* e)
{
  if (e->state == SDL_RELEASED && e->keysym.sym < 128)
  {
    SDL_GetMouseState(&x, &y);
    sdlut_glut_keyboard_function_ptr(e->keysym.sym, x, y);
  }
}


/* SDL -> GLUT mappings

   SDL_LEFT_BUTTON   = 1  ->  GLUT_LEFT_BUTTON   = 0
   SDL_MIDDLE_BUTTON = 2  ->  GLUT_MIDDLE_BUTTON = 1
   SDL_RIGHT_BUTTON  = 3  ->  GLUT_RIGHT_BUTTON  = 2

   SDL_PRESESD       = 0  ->  GLUT_DOWN = 1
   SDL_RELEASED      = 1  ->  GLUT_UP   = 0
*/
static int ButtonTrans[4] = { 0, 0, 1, 2 };
static int StateTrans[2]  = { 1, 0 };

static void sdlut_glut_mouse_adapter (SDL_MouseButtonEvent* e)
{
  sdlut_glut_mouse_function_ptr(ButtonTrans[e->button], 
                                StateTrans[e->state], 
                                e->x, e->y);
}


static void sdlut_glut_motion_adapter (SDL_MouseMotionEvent* e)
{
  sdlut_glut_motion_function_ptr(e->x, e->y);
}


static void sdlut_glut_reshape_adapter (SDL_ResizeEvent* e)
{
  sdlut_glut_reshape_function_ptr(e->w, e->h);
}


static char shiftmap[128] = {
	0,	/* NUL */
	1,	/* SOH */
	2,	/* STX */
	3,	/* ETX */
	4,	/* EOT */
	5,	/* ENQ */
	6,	/* ACK */
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	' ',	/* SPACE */
	'1',	/* ! */
	'\'',	/* " */
	'3',	/* # */
	'4',	/* $ */
	'5',	/* % */
	'7',	/* & */
	'\"',	/* ' */
	'9',	/* ( */
	'0',	/* ) */
	'8',	/* * */
	'=',	/* + */
	'<',	/* , */
	'_',	/* - */
	'>',	/* . */
	'?',	/* / */
	')',	/* 0 */
	'!',	/* 1 */
	'@',	/* 2 */
	'#',	/* 3 */
	'$',	/* 4 */
	'%',	/* 5 */
	'^',	/* 6 */
	'&',	/* 7 */
	'*',	/* 8 */
	'(',	/* 9 */
	';',	/* : */
	':',	/* ; */
	',',	/* < */
	'+',	/* = */
	'.',	/* > */
	'/',	/* ? */
	'2',	/* @ */
	'a',	/* A */
	'b',	/* B */
	'c',	/* C */
	'd',	/* D */
	'e',	/* E */
	'f',	/* F */
	'g',	/* G */
	'h',	/* H */
	'i',	/* I */
	'j',	/* J */
	'k',	/* K */
	'l',	/* L */
	'm',	/* M */
	'n',	/* N */
	'o',	/* O */
	'p',	/* P */
	'q',	/* Q */
	'r',	/* R */
	's',	/* S */
	't',	/* T */
	'u',	/* U */
	'v',	/* V */
	'w',	/* W */
	'x',	/* X */
	'y',	/* Y */
	'z',	/* Z */
	'{',	/* [ */
	'|',	/* \ */
	'}',	/* ] */
	'6',	/* ^ */
	'-',	/* _ */
	'~',	/* ` */
	'A',	/* a */
	'B',	/* b */
	'C',	/* c */
	'D',	/* d */
	'E',	/* e */
	'F',	/* f */
	'G',	/* g */
	'H',	/* h */
	'I',	/* i */
	'J',	/* j */
	'K',	/* k */
	'L',	/* l */
	'M',	/* m */
	'N',	/* n */
	'O',	/* o */
	'P',	/* p */
	'Q',	/* q */
	'R',	/* r */
	'S',	/* s */
	'T',	/* t */
	'U',	/* u */
	'V',	/* v */
	'W',	/* w */
	'X',	/* x */
	'Y',	/* y */
	'Z',	/* z */
	'[',	/* { */
	'\\',	/* | */
	']',	/* } */
	'`',	/* ~ */
	127	/* DEL */
};


unsigned char sdlutShiftASCII (unsigned char c)
{
  if (c >= 128 )
    return c;

  return shiftmap[c];
}


unsigned int lastTicks = 0;


unsigned int sdlutGetElapsed ()
{
  unsigned int thisTicks = SDL_GetTicks();
  unsigned int elapsed;

  if( lastTicks )
    elapsed = thisTicks - lastTicks;
  else
    elapsed = 0;

  lastTicks = thisTicks;

  return elapsed;
}
