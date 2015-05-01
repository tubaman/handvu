#ifndef DRAWING_H
#define DRAWING_H

#include <AR/arMulti.h>
#include "main.h"
//#include "app3dint.h"


void drawBullseye( double radius, int rings );
void drawMulti( ARMultiMarkerInfoT * );
void drawMarker( Marker * );

void enableLights();
//void drawApp3d( app3dint *app, double trans[][4] );
//void selectDrawApp3d( app3dint *app, double trans[][4] );
//void drawNearness( double posA[3], double posB[3] );



#endif /* DRAWING_H */
