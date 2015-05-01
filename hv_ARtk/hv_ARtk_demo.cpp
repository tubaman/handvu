#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
#include <vector>
#include <float.h>
#include "hv_ARtk_demo.h"
#include <math.h>

bool render_select( int x, int y, int* selected,  
		    d3* selected_win);
//void unproject(double winx, double winy, double winz,
//	       double* objx, double* objy, double* objz);
void drawBullseye( double radius, int rings );


/*
 * store OpenGL state matrices
 */
extern GLint viewport[ 4 ];

// for unproject
GLdouble modelview[ 16 ];
GLdouble projection[ 16 ];
GLdouble selected_view[16];


/*
 * UI state
 */
bool dragging = false;
bool rotating = false;
bool hovering = false;
bool show_ptr = false;
 
/*
 * information about selected point - id of object,
 * window coordinates and coordinates relative to the multi-marker
 * (anchor)
 */
int selected = -1;
d3 selected_win;
d3 selected_rel;
d3 selected_world; // relative to modelview

double anchor[16];
bool anchor_visible = false;
std::vector<bool> floater_visible;
typedef std::vector<double> dvec;
typedef std::vector<dvec> dmat;
dmat floaterpositions;

const unsigned int IDX_ANCHORED_LIGHT = 1;
const unsigned int IDX_FLOATER_LIGHT = 3;

//----------------------------------------------------------------------


/*
 * information about a set of objects
 */
std::vector< d3 > positions;
std::vector< d3 > rotations;
std::vector< d3 > colors;

// the first num_anchored objects are positioned relative to
// the "anchor" (multiMarker), the others on one floater each
unsigned int num_anchored = 0; 

d3 ptr_col;

int ptr_x, ptr_y;
int hv_pointer_dist = 120;

/*
 * assign positions and colors of objects and lights
 */
void initObjs ()
{
  positions.clear(); 
  rotations.clear();
  colors.clear();

  // anchored object
  d3 p;
  p.x = 50; p.y = -50; p.z = 30;
  positions.push_back( p );

  // object representing the anchored light
  p.x = 0; p.y = 200; p.z = 80;
  positions.push_back( p );

  num_anchored = positions.size();

  // floating object
  p.x = 70; p.y = -80; p.z = 30;
  positions.push_back( p );

  // object representing the floating light
  p.x = 0; p.y = 0; p.z = 40;
  positions.push_back( p );

  positions.push_back( p );
  positions.push_back( p );
  positions.push_back( p );
  positions.push_back( p );
  positions.push_back( p );
  positions.push_back( p );

  // we're being lazy - these are too many vector entries for
  // floaters, but that way we don't have to translate indices
  floaterpositions.reserve(positions.size());
  floater_visible.reserve(positions.size());

  //  srand((unsigned)time(NULL));
  for( unsigned int i = 0 ; i < positions.size() ; ++i )
  {
    d3 c;
    c.x = (float)rand() / ( float )RAND_MAX;
    c.y = (float)rand() / ( float )RAND_MAX;
    c.z = (float)rand() / ( float )RAND_MAX;

    colors.push_back( c );

    d3 r;
    r.x = r.y = r.z = 0;
    rotations.push_back( r );

    dvec d16( 16 );
    floaterpositions.push_back( d16 );
    floater_visible.push_back(false);
  }

  // pointer
  ptr_col.x = 0;
  ptr_col.y = 0;
  ptr_col.z = 255;
}


/*
 * draw teapots (with names for selection)
 */
void drawObjs ()
{
  glMatrixMode( GL_MODELVIEW );

  // for unproject
  glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
  glGetDoublev( GL_PROJECTION_MATRIX, projection );
  
  float spec[] = {1, 1, 1, 1};
  float shine[] = {70};
  glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
  glMaterialfv(GL_FRONT, GL_SHININESS, shine);

  if (anchor_visible) {
    glPushMatrix();
    glMultMatrixd(anchor);
    
    for( unsigned int i = 0 ; i < num_anchored ; ++i )
    {
      glPushMatrix();
        if (selected==(int)i) {
	  glGetDoublev( GL_MODELVIEW_MATRIX, selected_view );
	}
	float col[] = {colors[ i ].x, colors[ i ].y, colors[ i ].z, 1};
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);

	glLoadName( i + 1 );
	if (i==IDX_ANCHORED_LIGHT) {
	  // direct lamp shade towards teapot
	  glTranslated( positions[i].x, positions[i].y, positions[i].z );

#if 0
	  double upx = 0, upy = 1, upz = 1;
	  // todo : should check for singularity here
	  GLdouble curr[16];
	  glGetDoublev( GL_MODELVIEW_MATRIX, curr );
	  double w0x, w0y, w0z;
	  {  // world coordinates for object 0
	    double x = positions[0].x;
	    double y = positions[0].y;
	    double z = positions[0].z;
	    w0x = curr[0]*x + curr[4]*y + curr[8]*z + curr[12]*1;
	    w0y = curr[1]*x + curr[5]*y + curr[9]*z + curr[13]*1;
	    w0z = curr[2]*x + curr[6]*y + curr[10]*z + curr[14]*1;
	  }
	  //w0x = curr[13]+positions[0].x;
	  //w0y = curr[14]+positions[0].y;
	  //w0z = curr[15]+positions[0].z;
	  //glLoadIdentity();
	  double wLx = curr[13]+positions[i].x;
	  double wLy = curr[14]+positions[i].y;
	  double wLz = curr[15]+positions[i].z;
	  {  // world coordinates for object i
	    double x = positions[i].x;
	    double y = positions[i].y;
	    double z = positions[i].z;
	    wLx = curr[0]*x + curr[4]*y + curr[8]*z + curr[12]*1;
	    wLy = curr[1]*x + curr[5]*y + curr[9]*z + curr[13]*1;
	    wLz = curr[2]*x + curr[6]*y + curr[10]*z + curr[14]*1;
	  }
	  /*
	  gluLookAt(wLx, wLy, wLz,
		    w0x, w0y, w0z,
		    upx, upy, upz);
		    		    -positions[0].x, 
		    -positions[0].y, 
		    -positions[0].z, 
		    upx, upy, upz);
	  	  gluLookAt(positions[IDX_ANCHORED_LIGHT].x, 
		    positions[IDX_ANCHORED_LIGHT].y, 
		    positions[IDX_ANCHORED_LIGHT].z,
		    positions[0].x, positions[0].y, positions[0].z,
		    upx, upy, upz);*/
#endif
	  double cx, cy, cz;
	  double angle;
	  {
	    double x = positions[0].x-positions[i].x;
	    double y = positions[0].y-positions[i].y;
	    double z = positions[0].z-positions[i].z;
	    double a = 0; //positions[i].x;
	    double b = 0; //positions[i].y;
	    double c = 1; //positions[i].z;
	    cx = y*c - z*b;
	    cy = z*a - x*c;
	    cz = x*b - y*a;
	    double len0 = sqrt(x*x+y*y+z*z);
	    double leni = sqrt(a*a+b*b+c*c);
	    double dot = (a*x + b*y + c*z)/len0/leni;
	    angle = acos(dot)*180/M_PI;
	  }
	  glRotated(angle, cx, cy, cz);
	  //	  glutSolidTeapot(40);
	  glutSolidSphere(40, 60, 60);
	} else {
	  glTranslated( positions[i].x, positions[i].y, positions[i].z );
	  glRotated(90+rotations[i].x, 1, 0, 0);
	  glRotated(90+rotations[i].y, 0, 1, 0);
	  glRotated(rotations[i].z, 0, 0, 1);
	  glutSolidTeapot(70);
	  //	  glutSolidSphere(50, 100, 100);
	}
      glPopMatrix();
    }
    glPopMatrix();
  }

  for( unsigned int i = num_anchored ; i < positions.size() ; ++i )
  {
    if (floater_visible[i]) {
      glPushMatrix();
        glMultMatrixd(&floaterpositions[ i ][0]);
	if (selected==(int)i) {
	  glGetDoublev( GL_MODELVIEW_MATRIX, selected_view );
	}
	float col[] = {colors[ i ].x, colors[ i ].y, colors[ i ].z, 1};
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);

	glLoadName( i + 1 );
	glTranslated( positions[i].x, positions[i].y, positions[i].z );
	if (i==IDX_FLOATER_LIGHT) {
	  glutWireSphere(20, 10, 10);
	} else {
	  glRotated(90+rotations[i].x, 1, 0, 0);
	  glRotated(90+rotations[i].y, 0, 1, 0);
	  glRotated(rotations[i].z, 0, 0, 1);
	  glutSolidTeapot(60);
	}
      glPopMatrix();
    }
  }
}

void drawPointer()
{
  if (!show_ptr) return;

  glDisable(GL_LIGHTING);

  d3 ptr_win;
  ptr_win.x = ptr_x;
  ptr_win.y = viewport[ 3 ] - ptr_y - 1;
  ptr_win.z = .1;

  /*
   * calculate new world coordinates
   */
  d3 ptr_start;
  gluUnProject(ptr_win.x, ptr_win.y, ptr_win.z,
	       modelview, projection, viewport,
	       &ptr_start.x, &ptr_start.y, &ptr_start.z );

  d3 ptr_end;
  if (true || selected==-1) {
    gluUnProject(ptr_win.x, ptr_win.y-hv_pointer_dist, ptr_win.z,
		 modelview, projection, viewport,
		 &ptr_end.x, &ptr_end.y, &ptr_end.z );
  } else {
    ptr_end = selected_world;
  }

  glColor4d( ptr_col.x, ptr_col.y, ptr_col.z, 0.5 );
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();

  glTranslated( ptr_start.x, ptr_start.y, ptr_start.z );

  glBegin( GL_QUADS );
    const double w = 1;
    double x = ptr_end.x-ptr_start.x;
    double y = ptr_end.y-ptr_start.y;
    double z = ptr_end.z-ptr_start.z;
    glVertex3d(w, 0, 0);
    glVertex3d(x+w, -y/2, z/2);
    glVertex3d(x-w, -y/2, z/2);
    glVertex3d(-w, 0, 0);
  
    if (dragging || hovering) {
      // draw first links of claws
      glVertex3d(x+w, -y/2, z/2);
      glVertex3d(x+w*2, -y/1.5, z/1.5);
      glVertex3d(x+w, -y/1.5, z/1.5);
      glVertex3d(x, -y/2, z/2);

      glVertex3d(x-w, -y/2, z/2);
      glVertex3d(x-w*2, -y/1.5, z/1.5);
      glVertex3d(x-w, -y/1.5, z/1.5);
      glVertex3d(x, -y/2, z/2);
    }

    if (dragging) {
      // draw closed claws
      glVertex3d(x+w*2, -y/1.5, z/1.5);
      glVertex3d(x+w, -y, z);
      glVertex3d(x, -y, z);
      glVertex3d(x+w, -y/1.5, z/1.5);

      glVertex3d(x-w*2, -y/1.5, z/1.5);
      glVertex3d(x-w, -y, z);
      glVertex3d(x, -y, z);
      glVertex3d(x-w, -y/1.5, z/1.5);

    } else if (hovering) {
      // draw open claws
      glVertex3d(x+w*2, -y/1.5, z/1.5);
      glVertex3d(x+w*2, -y, z);
      glVertex3d(x+w, -y, z);
      glVertex3d(x+w, -y/1.5, z/1.5);

      glVertex3d(x-w*2, -y/1.5, z/1.5);
      glVertex3d(x-w*2, -y, z);
      glVertex3d(x-w, -y, z);
      glVertex3d(x-w, -y/1.5, z/1.5);

    } else if (rotating) {
      // draw little points in 3 directions
      glVertex3d(x+w, -y/2, z/2);
      glVertex3d(x, -y, z);
      glVertex3d(x, -y, z);
      glVertex3d(x-w, -y/2, z/2);

      glVertex3d(x, -y/2, z/2);
      glVertex3d(x+y/2, -y, z);
      glVertex3d(x+y/2, -y, z);
      glVertex3d(x, -y/2-w, z/2);

      glVertex3d(x, -y/2, z/2);
      glVertex3d(x-y/2, -y, z);
      glVertex3d(x-y/2, -y, z);
      glVertex3d(x, -y/2-w, z/2);

    } else {
      // draw pointy top
      glVertex3d(x+w, -y/2, z/2);
      glVertex3d(x, -y, z);
      glVertex3d(x, -y, z);
      glVertex3d(x-w, -y/2, z/2);

    }
  glEnd();

  glDisable(GL_BLEND);
  glPopMatrix();
}


void drawLights() {

  glMatrixMode(GL_MODELVIEW);

  // "ambient" light from behind
  GLenum use_light = GL_LIGHT0;
  {
    glEnable(use_light);
    float diffuse[] = {.8, .8, .8, .8};
    float specular[] = {0, 0, 0, 1};
    float ambient[] = {0.2, 0.2, 0.2, 1};
    glLightfv(use_light, GL_DIFFUSE, diffuse);
    glLightfv(use_light, GL_SPECULAR, specular);
    glLightfv(use_light, GL_AMBIENT, ambient);
    //    float pos[4] = {0, 0, -1, 0};
    //    glLightfv(use_light, GL_POSITION, pos);
  }  

  use_light = GL_LIGHT1;
  if (anchor_visible) {
    glPushMatrix();
    {
      glEnable(use_light);
      float diffuse[] = {1, 1, 1, 1};
      float specular[] = {1, 1, 1, 1};
      float ambient[] = {0, 0, 0, 1};
      glLightfv(use_light, GL_DIFFUSE, diffuse);
      glLightfv(use_light, GL_SPECULAR, specular);
      glLightfv(use_light, GL_AMBIENT, ambient);

      glMultMatrixd(anchor);
      glTranslated(positions[IDX_ANCHORED_LIGHT].x,
		   positions[IDX_ANCHORED_LIGHT].y,
		   positions[IDX_ANCHORED_LIGHT].z);
      float pos[4] = {0, 0, 0, 1};
      glLightfv(use_light, GL_POSITION, pos);
      //    glLightf(use_light, GL_CONSTANT_ATTENUATION, 2.0);
      //    glLightf(use_light, GL_LINEAR_ATTENUATION, 2.0);
      //    glLightf(use_light, GL_QUADRATIC_ATTENUATION, 2.0);
      float dir[3];
      dir[0] = positions[0].x-positions[IDX_ANCHORED_LIGHT].x;
      dir[1] = positions[0].y-positions[IDX_ANCHORED_LIGHT].y;
      dir[2] = positions[0].z-positions[IDX_ANCHORED_LIGHT].z;
      glLightfv(use_light, GL_SPOT_DIRECTION, dir);
      glLightf(use_light, GL_SPOT_CUTOFF, 45);
      //    glLightf(use_light, GL_SPOT_EXPONENT, 3);
      //    glRotatef(dir);
    }
    glPopMatrix();
  } else {
    glDisable(use_light);
  }

  use_light = GL_LIGHT2;
  if (floater_visible[IDX_FLOATER_LIGHT]) {
    glPushMatrix();
    {
      glEnable(use_light);
      float diffuse[] = {.5, .5, 1, 1};
      float specular[] = {.5, .5, 1, 1};
      float ambient[] = {0, 0, 0, 1};
      glLightfv(use_light, GL_DIFFUSE, diffuse);
      glLightfv(use_light, GL_SPECULAR, specular);
      glLightfv(use_light, GL_AMBIENT, ambient);

      glMultMatrixd(&floaterpositions[IDX_FLOATER_LIGHT][0]);
      glTranslated( positions[IDX_FLOATER_LIGHT].x, 
		    positions[IDX_FLOATER_LIGHT].y, 
		    positions[IDX_FLOATER_LIGHT].z );
      float pos[4] = {0, 0, 0, 1};
      glLightfv(use_light, GL_POSITION, pos);
      float dir[3];
      dir[0] = positions[2].x-positions[IDX_FLOATER_LIGHT].x;
      dir[1] = positions[2].y-positions[IDX_FLOATER_LIGHT].y;
      dir[2] = positions[2].z-positions[IDX_FLOATER_LIGHT].z;
      glLightfv(use_light, GL_SPOT_DIRECTION, dir);
      //      glLightf(use_light, GL_SPOT_CUTOFF, 45);
    }
    glPopMatrix();
  } else {
    glDisable(use_light);
  }
}

void hv_draw(bool rendering)
{
  drawObjs();
  if (rendering) {
    drawLights();
    drawPointer();
  }
}

void hv_set_anchor_transform(bool visible, double m[16])
{
  anchor_visible = visible;
  if (visible && m!=NULL) {
    memcpy(anchor, m, 16*sizeof(double));
    //for (int i = 0; i<16; i++) printf("%e, ", anchor[i]); printf("\n");
  }
}

void hv_set_floater_transform(int id, bool visible, double m[16])
{
  id += num_anchored;
  if (id>=(int) floater_visible.size()) {
    fprintf(stderr, "invalid floater id: %d\n", id);
    exit(-1);
  }
  floater_visible[id] = visible;
  if (visible && m!=NULL) {
    memcpy(&floaterpositions[id][0], m, 16*sizeof(double));
  }
}



//----------------------------------------------------------------------


static const int SELECT_BUF_SIZE = 1024 * 10;


void drag( int x, int y )
{
  /*
   * new window coords are x,y plus old z
   */
  d3 new_win = selected_win;
  new_win.x = x;
  new_win.y = viewport[ 3 ] - y - 1;

  /*
   * calculate new world coordinates
   */
  d3 new_world;
  gluUnProject(new_win.x, new_win.y, new_win.z,
	       selected_view, projection, viewport,	       
	       &new_world.x, &new_world.y, &new_world.z );

  /*
   * delta vector is new world minus old world
   */
  d3 offset;
  offset.x = new_world.x - selected_rel.x;
  offset.y = new_world.y - selected_rel.y;
  offset.z = new_world.z - selected_rel.z;

  /*
   * update selected teapot position with delta vector
   */
  positions[ selected ].x += offset.x;
  positions[ selected ].y += offset.y;
  positions[ selected ].z += offset.z;

  selected_win   = new_win;
  selected_rel = new_world;
}

void rotate( int x, int y )
{
  /*
   * new window coords are x,y plus old z
   */
  rotations[ selected ].x = x - selected_win.x;
  rotations[ selected ].y = (viewport[3]-y-1) - selected_win.y;
}

void hv_pointer_update(bool show, int button, int x, int y)
{
  ptr_x = x;
  ptr_y = y;

  show_ptr = show;
  if (!show) {
    dragging = false;
    rotating = false;
    selected = -1;
    return;
  }

  if (button == HV_PTR_LBUTTON_PRESSED) {
    if (!dragging) {
      if( render_select( x, y-hv_pointer_dist, &selected, &selected_win ) ) 
      {
	gluUnProject(selected_win.x, selected_win.y, selected_win.z,
		     selected_view, projection, viewport,
		     &selected_rel.x, &selected_rel.y, &selected_rel.z);
	
	gluUnProject(selected_win.x, selected_win.y-hv_pointer_dist, selected_win.z,
		     modelview, projection, viewport,
		     &selected_world.x, &selected_world.y, &selected_world.z);
	
	dragging = true;
	rotating = false;
      }

    } else {
      drag( x, y-hv_pointer_dist );
    }

  } else if (button == HV_PTR_RBUTTON_PRESSED) {
    if (!rotating) {
      if( render_select( x, y-hv_pointer_dist, &selected, &selected_win ) ) 
      {
	gluUnProject(selected_win.x, selected_win.y, selected_win.z,
		     selected_view, projection, viewport,
		     &selected_rel.x, &selected_rel.y, &selected_rel.z);
	
	gluUnProject(selected_win.x, selected_win.y-hv_pointer_dist, selected_win.z,
		     modelview, projection, viewport,
		     &selected_world.x, &selected_world.y, &selected_world.z);
	
	rotating = true;
	dragging = false; 
     }

    } else {
      rotate( x, y-hv_pointer_dist );
    }

  } else if (button == HV_PTR_BUTTON_RELEASED) {
    dragging = false;
    rotating = false;
    selected = -1;

  } else if (button == HV_PTR_NO_BUTTON_ACTION) {
    if (dragging) {
      drag( x, y-hv_pointer_dist );
    }
    if (rotating) {
      rotate( x, y-hv_pointer_dist );
    }

  } else {
    fprintf(stderr, "wrong 'button' value in hv_pointer_update\n");
    exit(-1);
  }

  //  printf("selected: %d\n", selected);

  if (!dragging && !rotating && 
      render_select( x, y-hv_pointer_dist, &selected, &selected_win ) ) 
  {
    gluUnProject(selected_win.x, selected_win.y, selected_win.z,
		 selected_view, projection, viewport,
		 &selected_rel.x, &selected_rel.y, &selected_rel.z);
    
    gluUnProject(selected_win.x, selected_win.y-hv_pointer_dist, selected_win.z,
		 modelview, projection, viewport,
		 &selected_world.x, &selected_world.y, &selected_world.z);
	
    hovering = true;
  } else {
    hovering = false;
  }
}


void ChangeObjectColor()
{
  if (selected!=-1) {
    colors[selected].x = (float)rand() / ( float )RAND_MAX;
    colors[selected].y = (float)rand() / ( float )RAND_MAX;
    colors[selected].z = (float)rand() / ( float )RAND_MAX;
  }
}

