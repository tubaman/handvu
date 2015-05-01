typedef struct _Marker_t Marker;

#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <vector>
#include <AR/arMulti.h>
#include <GL/gl.h>

#define DEBUG 1


struct _Marker_t 
{
  std::string	name;
  int		id;
  bool		visible;
  bool		was_visible;
  int		near_id;
  double	width;
  double	center[2];
  double	trans[3][4];
};


#ifdef USE_CG
#  include <Cg/cgGL.h>
extern CGcontext cgContext;
#endif

extern std::vector< Marker * > floater_markers;
extern ARMultiMarkerInfoT *multi_markers;
extern Marker *current_marker;

extern int xsize, ysize; 
extern int threshold;

extern int videopassthrough;
extern int fullscreen;
extern int grabinput;
extern char *videofile;
extern char *videoout;

extern GLdouble projectionMatrix[16];
extern GLint viewport[4];
extern double aspectRatio;


void cleanup();
void quit();
void reload();

Marker * findMarker( const char * );

#endif /* MAIN_H */
