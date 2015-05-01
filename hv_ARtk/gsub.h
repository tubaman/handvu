#ifndef AR_GSUB_H
#define AR_GSUB_H
#ifdef __cplusplus
extern "C" {
#endif


/* this is a modded gsub to use sdlut in place of glut.  w00t! --sjd */


#include <AR/config.h>
#include <AR/param.h>
#include <AR/ar.h>

extern int  argDrawMode;
extern int  argTexmapMode;

void argInit( ARParam *cparam, double zoom, /*int fullFlag,*/ int xwin, int ywin, int hmd_flag );
void argInitContext( int fullFlag, int fsaaFlag );
void argLoadHMDparam( ARParam *lparam, ARParam *rparam );
void argCleanup( void );
void argSwapBuffers( void );

void argMainLoop( void (*mouseFunc)(int button, int state, int x, int y),
                  void (*keyFunc)(unsigned char key, int x, int y),
                  void (*mainFunc)(void) );


void argDrawMode2D( void );
void argDraw2dLeft( void );
void argDraw2dRight( void );
void argDrawMode3D( void );
void argDraw3dLeft( void );
void argDraw3dRight( void );
void argDraw3dCamera( int xwin, int ywin );
void argMult3dCamera( int xwin, int ywin );


void argConvGlparad( double para[3][4], double gl_para[16] );
void argConvGlparaf( double para[3][4], float gl_para[16] );
void argConvGLcpara( ARParam *param, double gnear, double gfar, double m[16] );

void argDispImage( ARUint8 *image, int xwin, int ywin );
void argDispHalfImage( ARUint8 *image, int xwin, int ywin );

void argDrawSquare( double vertex[4][2], int xwin, int ywin );
void argLineSeg( double x1, double y1, double x2, double y2, int xwin, int ywin );
void argLineSegHMD( double x1, double y1, double x2, double y2 );

void argInqSetting( int *hmdMode, 
                    int *gMiniXnum2, int *gMiniYnum2,
                    void (**mouseFunc)(int button, int state, int x, int y),
                    void (**keyFunc)(unsigned char key, int x, int y),
                    void (**mainFunc)(void) );

#ifdef __cplusplus
}
#endif
#endif
