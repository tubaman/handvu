#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <AR/param.h>
#include <AR/ar.h>
#include <AR/arMulti.h>
#include <AR/video.h>
#include "gsub.h"
#include "sdlut.h"
//#include "util.h"
#include "callbacks.h"
//#include "video.h"
//#include "image.h"
#include "hv_ARtk_demo.h"
#include <sys/stat.h>
#include <errno.h>

using namespace std;

void EXIT_WITH_ERROR ( char *msg );

//-------------------------------------------------------------------------
// global variables
//-------------------------------------------------------------------------


char *programName = "arwin";

static char *single_filename  = "data/single.dat";
static char *multi_filename   = "data/ARmulti.dat";
//static char *multi_filename   = "data/multi.dat";
static char *cparam_filename  = "data/camera_para.dat";
//static char *startup_filename = "data/startup.dat";

#ifdef USE_CG
CGcontext cgContext;
#endif

#ifdef V4L
static char *vconf = "";
#else
static char *vconf = "-mode=640x480_YUV411 -rate=30";
#endif

vector< Marker * > floater_markers;
ARMultiMarkerInfoT *multi_markers;
Marker *current_marker = NULL;

ARParam cparam;
int threshold = 150;

int xsize, ysize;
int windowx, windowy;

double aspectRatio;
GLint viewport[ 4 ];
GLdouble projectionMatrix[ 16 ];

int videopassthrough = 1;
int fullscreen = 0;
int grabinput = 0;
char *displaystr = ":0";
bool fsaa = true;

bool take_snapshots = false;
bool fullsize_snapshots = false;
bool fixed_world = false;
bool reset_world_once = true;
char* conductor_fname = "../config/ARtk.conductor";
char* save_fname_root = NULL;
extern int hv_pointer_dist;


//-------------------------------------------------------------------------
// function prototypes
//-------------------------------------------------------------------------


int main( int, char ** );
static void parseArgs( int, char ** );
static void usage();
static void init();
static void contextInit();
static void displayInit();
static void readFloaters( const char * );
//static void readStartupConfig( const char * );

// dragging
void initObjs();
void ResetWorld();

#ifdef USE_CG
static void cgErrorCallback();
#endif


//-------------------------------------------------------------------------
// startup and init functions
//-------------------------------------------------------------------------


int main( int argc, char **argv )
{
  parseArgs( argc, argv );

  init();

  sdlutDisplayFunc( displayInit );
  sdlutMainLoop();

  cleanup();
  return 0;
}


void parseArgs( int argc, char **argv )
{
  int i = 1;
  while( i < argc )
  {
    if( strncmp( argv[i], "--fullscreen", 13 ) == 0 )
    {
      fullscreen = 1;
      ++i;
    }
    else if( strncmp( argv[i], "--nofullscreen", 15 ) == 0 )
    {
      fullscreen = 0;
      ++i;
    }
    else if( strncmp( argv[i], "--grabinput", 12 ) == 0 )
    {
      grabinput = 1;
      ++i;
    }
    else if( strncmp( argv[i], "--nograbinput", 14 ) == 0 )
    {
      grabinput = 0;
      ++i;
    }
    else if( strncmp( argv[i], "--videopassthrough", 19 ) == 0 )
    {
      videopassthrough = 1;
      ++i;
    }
    else if( strncmp( argv[i], "--novideopassthrough", 19 ) == 0 )
    {
      videopassthrough = 0;
      ++i;
    }
    else if( strncmp( argv[i], "--display", 10 ) == 0 )
    {
      ++i;
      displaystr = argv[i];
      ++i;
    }
    else if( strncmp( argv[i], "--conductor", 12 ) == 0 )
    {
      ++i;
      conductor_fname = argv[i];
      ++i;
    }
    else if( strncmp( argv[i], "--pointerdist", 14 ) == 0 )
    {
      ++i;
      hv_pointer_dist = atoi( argv[i] );
      ++i;
    }
    else if( strncmp( argv[i], "--snapshots", 14 ) == 0 )
    {
      ++i;
      take_snapshots = true;
    }
    else if( strncmp( argv[i], "--fullsnaps", 14 ) == 0 )
    {
      ++i;
      fullsize_snapshots = true;
    }
    else if( strncmp( argv[i], "--fixedworld", 13 ) == 0 )
    {
      ++i;
      fixed_world = true;
      reset_world_once = true;
    }
    else if( strncmp( argv[i], "--threshold", 12 ) == 0 )
    {
      ++i;
      threshold = atoi( argv[i] );
      ++i;
    }
    else if( strncmp( argv[i], "--nofsaa", 9 ) == 0 )
    {
      ++i;
      fsaa = false;
    }
    else
    {
      fprintf( stderr, "ERROR: unrecognized option: %s\n", argv[i] );
      fflush( stderr );

      usage();
      exit( 1 );
    }
  }
}


void usage()
{
  fprintf( stderr, "usage: ./hv_ARtk [ --fullscreen ] [ --conductor filename ]" );
  fprintf( stderr, " [ --display :<id> ] [ --nofsaa ] [ --fixedword ] [ --pointerdist n ] [ --threshod n ]\n" );
  fflush( stderr );
}


void init()
{
  ARParam wparam;

  if( arVideoOpen( vconf ) < 0 )
    EXIT_WITH_ERROR( "arVdeoOpen" );
  
  if( arVideoInqSize( &xsize, &ysize ) < 0 )
    EXIT_WITH_ERROR( "arVideoInqSize" );

  if( arParamLoad( cparam_filename, 1, &wparam ) < 0 )
    EXIT_WITH_ERROR( "arParamLoad" );

  arParamChangeSize( &wparam, xsize, ysize, &cparam );
  arInitCparam( &cparam );

  // load floaters
  readFloaters( single_filename );
  if( floater_markers.size() == 0 )
    EXIT_WITH_ERROR( "readSingles" );

  // load space registers
  multi_markers = arMultiReadConfigFile( multi_filename );
  if( multi_markers == NULL )
    EXIT_WITH_ERROR( "arMultiReadConfigFile" );

  // for take_snapshots, which can be turned on dynamically
  framePixels = new unsigned char[ xsize * ysize * 3 ];

  contextInit();

#ifdef USE_CG
  // initialize Cg context
  cgSetErrorCallback( cgErrorCallback );
  cgContext = cgCreateContext();
#endif

//  readStartupConfig( startup_filename );
  
  //
  // directory to save snapshots in
  //
  if (save_fname_root==NULL) {
    const char *home = getenv("HOME");
    if (home==NULL) {
      save_fname_root = (char*) alloca(20);
      sprintf(save_fname_root, "/tmp/hv_pics/");
    } else {
      save_fname_root = (char*) alloca(strlen(home)+20);
      sprintf(save_fname_root, "%s/hv_pics", home);
    }
  }
  // create it if it doesn't exist
  struct stat st;
  int error = stat(save_fname_root, &st);
#ifdef HAVE_STAT_EMPTY_STRING_BUG
  if (strlen(save_fname_root)==0) {
    error = -1;
    errno = ENOENT;
  }
#endif
 if (error!=0 && errno==ENOENT || !S_ISDIR(st.st_mode)) {
#if defined(HAVE_MKDIR)
    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    error = mkdir(save_fname_root, mode);
    if (error) {
      fprintf(stderr, "can not create directory %s\n", save_fname_root);
      exit(-1);
    }
    fprintf(stderr, "created directory for snapshots: %s\n", save_fname_root);
#else
    fprintf(stderr, "please create the following directory: %s\n", 
	    save_fname_root);
    exit(-1);
#endif
  }

  hv_init(conductor_fname, save_fname_root, xsize, ysize);
  initObjs();
  if (reset_world_once) {
    ResetWorld();
  }
}


void contextInit()
{
  argInit( &cparam, 1.0, 0, 0, 0 );
  argInitContext( fullscreen, fsaa );

  SDL_EnableKeyRepeat( 250, 10 );

  if( grabinput )
    SDL_WM_GrabInput( SDL_GRAB_ON );

  // two-sided lighting, use both sides of the polgyons
  glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );

  // use infinite viewer for specular lighting - looks good, speedy
  glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE );
}


void displayInit()
{
  arVideoCapStart();

  arUtilSleep( 500 );

  argDrawMode2D();
  argDrawMode3D();
  argDraw3dCamera( 0, 0 );
  
  glGetIntegerv( GL_VIEWPORT, viewport );
  glGetDoublev( GL_PROJECTION_MATRIX, projectionMatrix );

  glClearColor( 0.0, 0.0, 0.0, 0.0 );
  glClear( GL_COLOR_BUFFER_BIT );
  sdlutSwapBuffers();
  glClear( GL_COLOR_BUFFER_BIT );
  sdlutSwapBuffers();

  sdlutKeyboardFunc( keyboardFunc );
  sdlutMouseFunc( mouseFunc );
  sdlutMotionFunc( motionFunc );
  sdlutIdleFunc( idleFunc );
  sdlutDisplayFunc( displayFunc );

  if( fsaa )
  {
    glEnable( GL_MULTISAMPLE_ARB );
    glHint( GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST );
  }
}


//-------------------------------------------------------------------------
// cleanup functions
//-------------------------------------------------------------------------


void cleanup()
{
  for( vector< Marker * >::iterator i = floater_markers.begin() ;
       i != floater_markers.end() ; ++i )
  {
    arFreePatt( ( *i )->id );
    delete *i;
  }

  if( multi_markers )
  {
    if( multi_markers->marker )
      free( multi_markers->marker );

    free( multi_markers );
  }

  // destroy cg context?

  arVideoCapStop();
  arVideoClose();

  argCleanup();
  sdlutDestroyWindow();
}


void quit()
{
  cleanup();
  exit( 0 );
}


void reload()
{
  EXIT_WITH_ERROR( "reload" );
}


//-------------------------------------------------------------------------
// marker functions
//-------------------------------------------------------------------------


Marker * findMarker( const char *patt )
{
  for( vector< Marker * >::iterator i = floater_markers.begin() ;
       i != floater_markers.end() ; ++i )
  {
    if( ( *i )->name == string( patt ) )
      return *i;
  }

  return NULL;
}


#ifdef USE_CG

void cgErrorCallback ()
{
  CGerror error = cgGetError();

  if( error ) 
  {
    const char *listing = cgGetLastListing( cgContext );
    fprintf( stderr, "--------------------------------------------------------------------------\n" );
    fprintf( stderr, "ERROR: Cg error\n" );
    fprintf( stderr, "%s\n\n", cgGetErrorString( error ) );
    fprintf( stderr, "%s\n", listing );
    fprintf( stderr, "--------------------------------------------------------------------------\n" );
    exit( 1 );
  }
}

#endif // USE_CG


//-------------------------------------------------------------------------
// config file functions
//-------------------------------------------------------------------------

char * getNextLine( char *line, int size, FILE *fp )
{
  char *retval;

  while( 1 )
  {
    retval = fgets( line, size, fp );

    if( retval == NULL )
      return NULL;

    if( line[0] != '\n' && line[0] != '#' )
      return retval;
  }
}

void readFloaters( const char *filename )
{
#undef CLOSE_AND_RETURN
#define CLOSE_AND_RETURN { floater_markers.clear(); fclose( fp ); return; }

  FILE *fp = fopen( filename, "r" );
  if( fp == NULL )
    CLOSE_AND_RETURN;

  char pattern_filename[1024];
  char line[1024];
  getNextLine( line, 1024, fp );

  int num;
  if( sscanf( line, "%d", &num ) != 1 )
    CLOSE_AND_RETURN;

  if( num < 1 )
    CLOSE_AND_RETURN;

  floater_markers.clear();
  floater_markers.reserve( num );

  for( int i = 0 ; i < num ; ++i )
  {
    Marker *m = new Marker();
    floater_markers.push_back( m );

    getNextLine( line, 1024, fp );
    if( sscanf( line, "%s", pattern_filename ) != 1 )
      CLOSE_AND_RETURN;

    string temp = pattern_filename;
    m->name = temp.substr( temp.find_last_of( '.' ) + 1 );

    m->id = arLoadPatt( pattern_filename );
    if( m->id < 0 )
      CLOSE_AND_RETURN;

    m->visible = false;

    getNextLine( line, 1024, fp );
    if( sscanf( line, "%lf", &m->width ) != 1 )
      CLOSE_AND_RETURN;

    getNextLine( line, 1024, fp );
    if( sscanf( line, "%lf %lf", &m->center[0], &m->center[1] ) != 2 )
      CLOSE_AND_RETURN;
  }

  fclose( fp );
}


inline void EXIT_WITH_ERROR ( char *msg )
{
  fprintf( stderr, "ERROR: %s\n", msg );
  fflush( stderr );
  exit( 1 );
}


