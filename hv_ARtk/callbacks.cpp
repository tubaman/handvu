#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <AR/ar.h>
#include <AR/arMulti.h>
#include <AR/video.h>
#include <float.h>
#include "gsub.h"
#include "sdlut.h"
#include "main.h"
#include "callbacks.h"
#include "drawing.h"
#include "hv_ARtk_demo.h"

using namespace std;


#define KMOD_ALL	( KMOD_CTRL | KMOD_ALT | KMOD_SHIFT | KMOD_META )

static const int SELECT_BUF_SIZE = 1024 * 10;

//#define PROX_THRESH 135.0

static const int MAX_NAME_STACK = 32;
static const unsigned int NAME_END = UINT_MAX;

static const int MOUSE_SQR_DIST_THRESH = 16;
static const unsigned int MOUSE_TICKS_THRESH = 500;

int DrawMode = RENDER_MODE;

texture_info videoTex;

static bool multiVisible = false;
static bool mouse_moved = false;
int mouse_x, mouse_y;

bool show_markers = true;
extern bool fixed_world;
extern bool take_snapshots;
extern bool fullsize_snapshots;

unsigned char *framePixels = NULL;

static GLUquadric *quadric = gluNewQuadric();

static void selectDraw();
bool render_select( int x, int y, int* selected,  
		d3* selected_win, d3* selected_world);
bool process_selection ( GLint hits, GLuint *buffer, int x, int y,
			 int* selected,  
			 d3* selected_win);
void initObjs();

//-------------------------------------------------------------------------
// regular event callback functions
//-------------------------------------------------------------------------


void displayFunc()
{
  if (mouse_moved) {
    mouse_moved = false;
    // this in turn might call selectDraw()
    hv_pointer_update(HV_PTR_SHOW, HV_PTR_NO_BUTTON_ACTION, 
		      mouse_x, mouse_y);
  }

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  ARUint8 *dataPtr = NULL;

  arVideoCapNext();
  dataPtr = (ARUint8 *)arVideoGetImage();

  if( dataPtr == NULL )
    return;

  hv_process_frame((char*)dataPtr, xsize, ysize);

  if (take_snapshots && !fullsize_snapshots) {
    hv_save_frame((char*) dataPtr, xsize, ysize, fullsize_snapshots);
  }
  
  makeTexture( dataPtr, 640, 480, &videoTex );

  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  argDrawMode2D();
  if( videopassthrough )
    argDispImage( dataPtr, 0, 0 );

  if (fixed_world) {
    // show whereever it was last
    hv_set_anchor_transform(true, NULL);

  }
   
 // find makers
  ARMarkerInfo *found;
  int found_num;
  
  // detect markers in the scene
  if( arDetectMarker( dataPtr, threshold, &found, &found_num ) < 0 ) {

    fprintf( stderr, "ERROR: arDetectMarker\n" );
    fflush( stderr );
    cleanup();
    exit( 1 );
  }
  
  if (!fixed_world) {
    // process the detected floater markers - there is only the
    // anchor marker in fixed_world
    current_marker = NULL;
    for( vector< Marker * >::iterator i = floater_markers.begin() ;
	 i != floater_markers.end() ; ++i )
    {
      int k = -1;
      for( int j = 0 ; j < found_num ; ++j )
	if( found[ j ].id == ( *i )->id )
	  k = j;
      
      if( k != -1 ) {

	arGetTransMat( &found[ k ], ( *i )->center, 
		       ( *i )->width, ( *i )->trans );
	
	double gl_para[16];
	argConvGlparad((*i)->trans, gl_para);
	hv_set_floater_transform((*i)->id, true, gl_para);

      }
      else
      {
	hv_set_floater_transform((*i)->id, false, NULL);
      }
    }
  }

  
  // process the detected anchored markers
  double multiError = arMultiGetTransMat( found, found_num, multi_markers );
  multiVisible = ( multiError >= 0.0 && multiError < 100.0 );
  
  if( multiVisible ) {

    double gl_para[16];
    argConvGlparad(multi_markers->trans, gl_para);

    if (fixed_world) {
      hv_set_floater_transform(0, true, gl_para);
    } else {
      hv_set_anchor_transform(true, gl_para);
    }
  }


  argDrawMode3D();
  argDraw3dCamera( 0, 0 );  
  
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_NORMALIZE );
  glEnable( GL_LIGHTING );
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_CULL_FACE );

  glMatrixMode( GL_MODELVIEW );

  hv_draw(true);

  // draw markers
  if( show_markers ) {
    if( multiVisible )
      drawMulti( multi_markers );

    for( vector< Marker * >::iterator i = floater_markers.begin() ;
	 i != floater_markers.end() ; ++i ) {
      if( ( *i )->visible ) {
	glColor4d(0, 0, 0, 0.4);
	drawMarker( *i );
      }
    }
  }
  /*
    if (floater_visible[i]) {
      glPushMatrix();
        glMultMatrixd(&floaterpositions[ i ][0]);
	glDisable(GL_LIGHTING);
	drawBullseye(40, 3);
	glEnable(GL_LIGHTING);
      glPopMatrix();
    }
  }
}
  */


  if (take_snapshots && fullsize_snapshots) {
    if (!framePixels) {
      fprintf( stderr, "error: framePixels not set\n");
      exit(-1);
    }
    glReadBuffer( GL_BACK );
    glReadPixels( 0, 0, xsize, ysize, GL_BGR, GL_UNSIGNED_BYTE, 
		  framePixels );

    hv_save_frame((char*) framePixels, xsize, ysize, 
		  fullsize_snapshots);
  }

  sdlutSwapBuffers();
}

/* turn off all displays, reset the anchored object to a nice
 * default location
 */
void ResetWorld()
{
  double gl_para[] = 
    {7.275137e-03, 6.592478e-01, -7.518906e-01, 0.000000e+00, 9.997167e-01, -2.183459e-02, -9.471231e-03, 0.000000e+00, -2.266111e-02, -7.516087e-01, -6.592199e-01, 0.000000e+00, 4.982944e+01, -3.182761e+01, 1.200848e+03, 1.000000e+00};
  hv_set_anchor_transform(true, gl_para);

  for( vector< Marker * >::iterator i = floater_markers.begin() ;
       i != floater_markers.end() ; ++i ) {
    hv_set_floater_transform((*i)->id, false, NULL);
  }
  initObjs();
}

void keyboardFunc( SDL_KeyboardEvent *e )
{
  switch( e->keysym.sym )
  {
  case SDLK_ESCAPE:
  case 'q':
    if( e->type == SDL_KEYUP )
    {
      quit();
    }
    return;
    break;

  case 'm':
    if ( e->type == SDL_KEYDOWN ) {
      show_markers = !show_markers;
      char buf[256];
      sprintf( buf, "INFO: %sshowing markers", 
	       show_markers ? "" : "not " );
      hv_show_message(buf);
    }
    break;

  case 'e':
    if ( e->type == SDL_KEYDOWN ) {
      hv_toggle_adjust_exposure();
    }
    break;

  case 'f':
    if ( e->type == SDL_KEYDOWN ) {
      fixed_world = !fixed_world;
      char buf[256];
      sprintf( buf, "INFO: fixed world is now %s", 
	       fixed_world ? "on" : "off" );
      hv_show_message(buf);
    }
    break;

  case 'w':
    if ( e->type == SDL_KEYDOWN ) {
      ResetWorld();
      hv_show_message("INFO: world reset");
    }
    break;

  case 's':
    if( e->type == SDL_KEYDOWN )
      {
	take_snapshots = !take_snapshots;
	char buf[256];
	sprintf( buf, "INFO: %staking %ssnapshots", 
		 take_snapshots ? "" : "not ",
		 fullsize_snapshots ? "fullsize " : "");
	hv_show_message(buf);
      }
    break;

  case '=':
    if (e->type == SDL_KEYDOWN) {// && e->keysym.mod & KMOD_SHIFT) {
      threshold = min(255, threshold+10);
      char buf[256];
      sprintf(buf, "INFO: threshold now %d", threshold);
      hv_show_message(buf);
    }
    break;

  case '-':
    if (e->type == SDL_KEYDOWN) {
      threshold = max(0, threshold-10);
      char buf[256];
      sprintf(buf, "INFO: threshold now %d", threshold);
      hv_show_message(buf);
    }
    break;

  default:
    if (e->type == SDL_KEYDOWN && e->keysym.mod==0) {
      // hand to HandVu
      hv_key_pressed(e->keysym.sym);
      break;
    }
  }
}


void mouseFunc( SDL_MouseButtonEvent *e )
{
  if( e->state == SDL_PRESSED )
  {
    if (e->button==1) {  // left button
      hv_pointer_update(HV_PTR_SHOW, HV_PTR_LBUTTON_PRESSED, 
			e->x, e->y);
    } else if (e->button==3 ) {  // right button
      hv_pointer_update(HV_PTR_SHOW, HV_PTR_RBUTTON_PRESSED, 
			e->x, e->y);
    }
  }
  else if( e->state == SDL_RELEASED ) { 
    hv_pointer_update(HV_PTR_SHOW, HV_PTR_BUTTON_RELEASED, 
		      e->x, e->y);
  } 
}


void motionFunc( SDL_MouseMotionEvent *e )
{
  mouse_moved = true;
  mouse_x = e->x;
  mouse_y = e->y;
  // pointer update happens in displayFunc to avoid a 
  // large lag between the two and too many updates
}


void idleFunc()
{
  sdlutPostRedisplay();
}


//-------------------------------------------------------------------------
// picking helper functions
//-------------------------------------------------------------------------


static void selectDraw()
{
  argDrawMode3D();
  argMult3dCamera( 0, 0 );  
  
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );
  glDisable( GL_LIGHTING );
  glDisable( GL_NORMALIZE );
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_CULL_FACE );

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();

  DrawMode = SELECT_MODE;
  //server->draw();
  hv_draw(false);

  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
}


/*
 * do selection render pass.  call process_selection to determine
 * results of selection.  return true if hit, false if miss.
 */

bool render_select( int x, int y, int* selected,  
		   d3* selected_win)
{
  GLuint selectBuffer[ SELECT_BUF_SIZE ];
  GLint hits;

  glSelectBuffer( SELECT_BUF_SIZE, selectBuffer );
  glRenderMode( GL_SELECT );

  glInitNames();
  glPushName(0);

  /*
   * set up pick matrix as 5x5 window around cursor
   */
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix( ( GLdouble )x, ( GLdouble )( viewport[ 3 ] - y ),
                 5.0, 5.0, viewport );
  glMatrixMode( GL_MODELVIEW );

  *selected = -1;        
  selectDraw();

  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glFlush();

  /*
   * check selection buffer for hits
   */
  hits = glRenderMode( GL_RENDER );
  return process_selection( hits, selectBuffer, x, y,
			    selected, selected_win);
}


/*
 * once hits are recorded from selection pass, process selection
 * buffer to determine what was selected
 */
bool process_selection ( GLint hits, GLuint *buffer, int x, int y,
			 int* selected,  
			 d3* selected_win)
{
  float min_depth = FLT_MAX;

  GLuint *ptr = buffer;
  GLint names;
  float depth;
  unsigned int name = 0, tempname = 0;

  if( hits == -1 )
  {
    fprintf( stderr, "WARNING: selection namestack overflow\n" );
    return false;
  }

  for( int i = 0 ; i < hits ; ++i )
  {
    names = *ptr;                          ++ptr;
    depth = ( float )*ptr / 0xffffffff;    ++ptr;  ++ptr;
/*
 * note - redbook says to divide by 0x7fffffff but that clearly wasn't
 * working, as reported depth was twice actual value.  so, changing
 * to 0xffffffff fixes the problem.
 */

    if( names != 1 )
    {
      fprintf( stderr, "WARNING: unexpected name length %d\n", names );
      for( int j = 0 ; j < names ; ++j )
        ++ptr;
    }
    else
    {
      tempname = *ptr; ++ptr;
    }

    /*
     * after extracting depth and name of hit record, store if it's
     * the closest to the camera
     */
    if( depth < min_depth )
    {
      min_depth = depth;
      name = tempname;
    }
  }

  /*
   * 0 is the default name - a miss
   */
  if( name == 0 )
    return false;

  /*
   * id of teapot is name - 1 (starts at index 0)
   * store selected coordinates
   */
  *selected = name - 1;
  selected_win->x = x;
  selected_win->y = viewport[ 3 ] - y - 1;
  selected_win->z = min_depth;

  return true;
}


int nextpowerof2 ( int x )
{
  if( x <= 2 ) return 2;
  if( x <= 4 ) return 4;
  if( x <= 8 ) return 8;
  if( x <= 16 ) return 16;
  if( x <= 32 ) return 32;
  if( x <= 64 ) return 64;
  if( x <= 128 ) return 128;
  if( x <= 256 ) return 256;
  if( x <= 512 ) return 512;
  if( x <= 1024 ) return 1024;
  if( x <= 2048 ) return 2048;
  return 4096;
}

void makeTexture ( unsigned char *pixels, int width, int height,
                   texture_info *tex )
{
  if( ! pixels )
  {
    fprintf( stderr, "null pixels!\n" );
    return;
  }

  if( tex->id == 0 )
  {
    tex->iw = width;
    tex->ih = height;

    tex->tw = nextpowerof2( width );
    tex->th = nextpowerof2( height );

    tex->s = tex->iw / ( float )tex->tw;
    tex->t = tex->ih / ( float )tex->th;

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    unsigned char *zeros = new unsigned char[ tex->tw * tex->th * 3 ];
    memset( zeros, 0, tex->tw * tex->th * 3 );

    glGenTextures( 1, &tex->id );
    glBindTexture( GL_TEXTURE_2D, tex->id );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex->tw, tex->th, 0, GL_RGB, 
                  GL_UNSIGNED_BYTE, zeros );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex->iw, tex->ih, GL_RGB, 
                     GL_UNSIGNED_BYTE, pixels );
    glBindTexture( GL_TEXTURE_2D, 0 );

    delete[] zeros;
  }
  else
  {
    glBindTexture( GL_TEXTURE_2D, tex->id );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex->iw, tex->ih, GL_RGB, 
                     GL_UNSIGNED_BYTE, pixels );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }
}


