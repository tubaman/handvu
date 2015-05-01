#include "drawing.h"
#include <GL/glut.h>
#include <math.h>
#include "gsub.h"
//#include "util.h"


static GLUquadric *quadric = gluNewQuadric();


void drawBullseye( double radius, int rings )
{
  int divs = ( rings * 2 ) - 1;
  double r = radius / (double) divs;

  gluQuadricDrawStyle( quadric, GLU_FILL );
  gluQuadricOrientation( quadric, GLU_OUTSIDE );
  gluQuadricNormals( quadric, GLU_SMOOTH );

  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_BLEND );

  glPushMatrix();
  glTranslatef( 0.0, 0.0, -0.5 );
  for( int i = 0 ; i < rings ; ++i )
    gluDisk( quadric, i * 2 * r, ( i * 2 + 1 ) * r, 16, 1 );
  // north marker
  glBegin(GL_QUADS);
    double w = radius/10;
    double f = radius;
    double t = radius*1.2;
    glVertex3d(w, f, 0);
    glVertex3d(w, t, 0);
    glVertex3d(-w, t, 0);
    glVertex3d(-w, f, 0);
  glEnd();
  glPopMatrix();

  glDisable( GL_BLEND );
}


void drawMulti( ARMultiMarkerInfoT *m )
{
  double multiglmatrix[16];
  double glmatrix[16];
  argConvGlparad( m->trans, multiglmatrix );

  argDrawMode3D();
  argDraw3dCamera( 0, 0 );
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_LIGHTING );
  glDisable( GL_NORMALIZE );

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glMultMatrixd( multiglmatrix );

  for( int i = 0 ; i < m->marker_num ; ++i )
  {
    double w = m->marker[i].width / 2.0;

    if( m->marker[i].visible >= 0 )
      glColor4f( 1.0, 1.0, 1.0, 0.3 );
    else
      glColor4f( 1.0, 1.0, 0.0, 0.1 );

    argConvGlparad( m->marker[i].trans, glmatrix );
    glPushMatrix();
    glMultMatrixd( glmatrix );

    drawBullseye( w * 1.5, 3 );
/*
    glBegin( GL_QUADS );
      glVertex3f( m->marker[i].center[0] - w, 
                  m->marker[i].center[1] - w,
                  0.0
                );
      glVertex3f( m->marker[i].center[0] + w, 
                  m->marker[i].center[1] - w,
                  0.0
                );
      glVertex3f( m->marker[i].center[0] + w, 
                  m->marker[i].center[1] + w,
                  0.0
                );
      glVertex3f( m->marker[i].center[0] - w, 
                  m->marker[i].center[1] + w,
                  0.0
                );
    glEnd();
*/

    glPopMatrix();
  }

  glPopMatrix();
}


void drawMarker( Marker *m )
{
  double w = m->width / 2.0;
  double glmatrix[16];
  argConvGlparad( m->trans, glmatrix );

  argDrawMode3D();
  argDraw3dCamera( 0, 0 );
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_LIGHTING );
  glDisable( GL_NORMALIZE );

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();   
  glMultMatrixd( glmatrix );

  glColor4f( 1.0, 0.0, 0.0, 0.5 );
  drawBullseye( w * 1.5, 3 );

/*
  double b = 0;

  glBegin( GL_QUADS );
    glVertex3f( m->center[0] - w - b, m->center[1] - w - b, -0.1 );
    glVertex3f( m->center[0] + w + b, m->center[1] - w - b, -0.1 );
    glVertex3f( m->center[0] + w + b, m->center[1] + w + b, -0.1 );
    glVertex3f( m->center[0] - w - b, m->center[1] + w + b, -0.1 );
  glEnd();
*/
      
  glPopMatrix();
}

/*
void selectDrawApp3d( app3dint *app, double trans[][4] )
{
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();

  argDrawMode3D();
  argMult3dCamera( 0, 0 );

  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );
  glDisable( GL_LIGHTING );
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_NORMALIZE );
  glDisable( GL_CULL_FACE );

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();

  app->drawFunc( trans );

  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();

  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
} 
*/

void enableLights()
{
  GLfloat ambient[4] = { 0.2, 0.2, 0.2, 1.0 };
  glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient );
  
  GLfloat light0[2][4] = { { 10.0, 10.0, -5.0,  0.0 },
                           {  0.7,  0.7,  0.7,  1.0 } };
  glLightfv( GL_LIGHT0, GL_POSITION, light0[0] );
  glLightfv( GL_LIGHT0, GL_DIFFUSE,  light0[1] );
  glLightfv( GL_LIGHT0, GL_SPECULAR, light0[1] );
  glEnable( GL_LIGHT0 );
  
  GLfloat light1[2][4] = { { -10.0, -10.0, -10.0,  0.0 },
                           {   0.7,   0.7,   0.7,  1.0 } };
  glLightfv( GL_LIGHT1, GL_POSITION, light1[0] );
  glLightfv( GL_LIGHT1, GL_DIFFUSE,  light1[1] );
  glLightfv( GL_LIGHT1, GL_SPECULAR, light1[1] );
  glEnable( GL_LIGHT1 ); 
  
  GLfloat light2[2][4] = { { -10.0,  10.0,  0.0,  0.0 },
                           {   0.6,   0.5,  0.3,  1.0 } };
  glLightfv( GL_LIGHT2, GL_POSITION, light2[0] );
  glLightfv( GL_LIGHT2, GL_DIFFUSE,  light2[1] );
  glLightfv( GL_LIGHT2, GL_SPECULAR, light2[1] );
  glEnable( GL_LIGHT2 );
  
  GLfloat light3[2][4] = { {  10.0, -10.0,  0.0,  0.0 },
                           {   0.4,   0.5,  0.6,  1.0 } };
  glLightfv( GL_LIGHT3, GL_POSITION, light3[0] );
  glLightfv( GL_LIGHT3, GL_DIFFUSE,  light3[1] );
  glLightfv( GL_LIGHT3, GL_SPECULAR, light3[1] );
  glEnable( GL_LIGHT3 );
} 

/*
void drawApp3d( app3dint *app, double trans[][4] )
{
  argDrawMode3D();
  argDraw3dCamera( 0, 0 );
  
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_LIGHTING );
  glEnable( GL_NORMALIZE );
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_CULL_FACE );
  
  enableLights();
  
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  
  app->drawFunc( trans );
  
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
}


static void calcNearnessTransform( double posA[3], double posB[3], 
                                   double offset, double *dist )
{   
  double delta[3];
  delta[0] = posA[0] - posB[0];
  delta[1] = posA[1] - posB[1];
  delta[2] = posA[2] - posB[2];
  *dist = normalize( delta );  
    
  double midpoint[3];
  midpoint[0] = ( posA[0] + posB[0] ) / 2.0;
  midpoint[1] = ( posA[1] + posB[1] ) / 2.0;
  midpoint[2] = ( posA[2] + posB[2] ) / 2.0;
    
  double Xvec[3] = { 1.0, 0.0, 0.0 };
  double Yvec[3] = { 0.0, 1.0, 0.0 };
  double Zvec[3] = { 0.0, 0.0, 1.0 };
  
  double normal[3];   
  crossprod( delta, Yvec, normal );
  normalize( normal );
    
  double rotvec[3];
  crossprod( normal, Zvec, rotvec );
  
  double NdotZ = dotprod( normal, Zvec );
  double angle = - acos( NdotZ ) * 180.0 / M_PI;
    
  double plane[3];
  crossprod( normal, delta, plane );
  normalize( plane );
    
  double sign;
  if( dotprod( delta, Xvec ) < 0 )
    sign = 1;
  else
    sign = -1;

  double theta;
  if( dotprod( plane, Xvec ) < 0 )
    theta = sign * - acos( dotprod( plane, Yvec ) ) * 180.0 / M_PI;
  else
    theta = sign * acos( dotprod( plane, Yvec ) ) * 180.0 / M_PI;

  glTranslatef( plane[0] * offset, plane[1] * offset, plane[2] * offset );  
  glTranslatef( midpoint[0], midpoint[1], midpoint[2] );
  glRotatef( angle, rotvec[0], rotvec[1], rotvec[2] );
  glRotatef( theta, 0.0, 0.0, 1.0 );
} 


void drawNearness( double posA[3], double posB[3] )
{
  GLfloat color[4] = {  0.2,  0.9,  0.4,  1.0 };
                
  glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT,   color );
  glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE,   color );
  glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR,  color );
  glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION,  glBlack );
  glMaterialf ( GL_FRONT_AND_BACK, GL_SHININESS, 0.5 * 128.0 );
                
  gluQuadricDrawStyle( quadric, GLU_FILL );
  gluQuadricOrientation( quadric, GLU_OUTSIDE );
  gluQuadricNormals( quadric, GLU_SMOOTH );
    
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_NORMALIZE );
  glEnable( GL_LIGHTING );
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_CULL_FACE );
  
  argDrawMode3D();
  argDraw3dCamera( 0, 0 );
  
  enableLights();

  glMatrixMode( GL_MODELVIEW );  
  glPushMatrix();

  double diameter;
  calcNearnessTransform( posA, posB, -25.0, &diameter );
  
  GLdouble clipplane[4] = { 0.0, -1.0, 0.0, 0.0 };
  glClipPlane( GL_CLIP_PLANE0, clipplane );
  glEnable( GL_CLIP_PLANE0 );
  
  glutSolidTorus( 5.0, diameter / 2.0, 16, 24 );
 
  glDisable( GL_CLIP_PLANE0 );

  glPushMatrix();
    glTranslatef( diameter / 2.0, 0.0, 0.0 );
    glRotatef( -90.0, 1.0, 0.0, 0.0 );
    gluCylinder( quadric, 10.0, 0.0, 25.0, 16, 1 );
    gluDisk( quadric, 0, 10.0, 16, 1 );
  glPopMatrix();
  
  glPushMatrix();
    glTranslatef( -diameter / 2.0, 0.0, 0.0 );
    glRotatef( -90.0, 1.0, 0.0, 0.0 );
    gluCylinder( quadric, 10.0, 0.0, 25.0, 16, 1 );
    gluDisk( quadric, 0, 10.0, 16, 1 );
  glPopMatrix();
   
  glPopMatrix();  
} 
*/
