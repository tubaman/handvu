
#include <cv.h>
#include "Common.h"
#include "HandVu.h"
//#include "LinuxDC1394CameraController.h"
#include "hv_ARtk_demo.h"
#include <time.h>


static IplImage* m_pImage = NULL;
static int m_cxImage = -1;
static int m_cyImage = -1;
static bool m_is_initialized = false;
static bool m_img_bottom_up = false;
//static LinuxDC1394CameraController* m_pCamController = NULL;
static CvFont m_font;
static string m_msg = "";
static clock_t m_save_frame_after;
static clock_t m_show_until;
static clock_t m_permit_next_color_change_at;

bool Initialize(int img_width, int img_height, int iPixelSize,
		string conductor_fname,
		string save_fname_root);
void Draw(IplImage* pImage);
void DrawMessage();
void ChangeObjectColor();


void hv_init(const char* conductor_fname, 
	     const char* save_fname_root,
	     int width, int height)
{
  if (!m_is_initialized) {
    string cf = "", sf = "";
    if (conductor_fname) {
      cf = conductor_fname;
    }
    if (save_fname_root) {
      sf = save_fname_root;
    }
    bool success = Initialize(width, height, 8, cf, sf);
    if (!success) {
      fprintf(stderr, "could not initialize\n");
      exit(-1);
    }
  }
}


void hv_process_frame(char* pData, int width, int height)
{
  if (!m_is_initialized) {
    fprintf(stderr, "hv_process_frame called before initialization\n");
    exit(-1);

  } else { 
    // test if change from initialization
    bool img_bottom_up = true;
    if (height<0) {
      // norm for RGB images
      img_bottom_up = false;
      height = -height;
    }
    if (width!=m_cxImage || height!=m_cyImage || 
	img_bottom_up!=m_img_bottom_up) {
      fprintf(stderr, "hv_process_frame: format changed since init!\n");
      exit(-1);
    }
  }

  m_pImage->imageData = (char*) pData;
  m_pImage->imageSize = width*height;

  hvState state;

  // ------- main library call ---------
  hvAction action = 
    hvProcessFrame(m_pImage);
  // -------
  
  if (action==HV_DROP_FRAME) {
    // HandVu recommends dropping the frame entirely
    //    VERBOSE0(3, "HandVuFilter: dropping frame");
  } else if (action==HV_SKIP_FRAME) {
    //    VERBOSE0(3, "HandVuFilter: supposed to skip frame");
    // HandVu recommends displaying the frame, but not doing any further
    // processing on it - keep going
  } else if (action==HV_PROCESS_FRAME) {
    //    VERBOSE0(3, "HandVuFilter: processed frame");
    // full processing was done and is recommended for following steps;
    // keep going
  } else {
    fprintf(stderr, "unknown HVAction\n");
    exit(-1);
  }
  
  hvGetState(0, state);
  
  //
  // use HandVu state as "mouse" input
  //
  int button_action = HV_PTR_NO_BUTTON_ACTION;
  static int was_tracked = false; 
  // in order to not always turn off the pointer again and again
  if (state.recognized) {
    if (state.posture=="Lback") {
      button_action = HV_PTR_LBUTTON_PRESSED;
    } else if (state.posture=="open") {
      button_action = HV_PTR_BUTTON_RELEASED;
    } else if (state.posture=="sidepoint") {
      button_action = HV_PTR_RBUTTON_PRESSED;
    } else if (state.posture=="Lpalm") {
      if (m_permit_next_color_change_at<clock()) {
	ChangeObjectColor();
	m_permit_next_color_change_at = clock() + 1*CLOCKS_PER_SEC;
      }
    }
  }
  if (state.tracked) {
    int x = (int) ((m_cxImage-1)*state.center_xpos);
    int y = (int) ((m_cyImage-1)*state.center_ypos);
    hv_pointer_update(HV_PTR_SHOW, button_action, x, y);
    was_tracked = true;
    
  } else if (was_tracked) {
    hv_pointer_update(HV_PTR_NO_SHOW, button_action, -1, -1);
    was_tracked = false;
  }

#if 0
  int apertureSize = 3;
  double lowThresh = 50;
  double highThresh = 150;
  static IplImage* pGray = NULL;
  static IplImage* pEdges = NULL;
  static IplImage* pAngles = NULL;
  if (pEdges==NULL) {
    pGray = cvCreateImage(cvSize(m_pImage->width, m_pImage->height), IPL_DEPTH_8U, 1);
    pEdges = cvCreateImage(cvSize(m_pImage->width, m_pImage->height), IPL_DEPTH_8U, 1);
    pAngles = cvCreateImage(cvSize(m_pImage->width, m_pImage->height), IPL_DEPTH_8U, 1);
  }
  cvCvtColor(m_pImage, pGray, CV_BGR2GRAY);
  cvCanny(pGray, pEdges, lowThresh, highThresh, apertureSize);
  
  unsigned char* pPix = (unsigned char*) pEdges->imageData;
  CvPixelPosition8u position;
  CV_INIT_PIXEL_POS(position, 
                    (unsigned char*)(m_pImage->imageData),
                    m_pImage->widthStep, 
                    cvSize(m_pImage->width, m_pImage->height), 0, 0,
                    m_pImage->origin);
  for (int y=0; y<m_pImage->height/2; y++) {
    CV_MOVE_TO(position, 0, y, 3);
    for (int x=0; x<m_pImage->width; x++) {
      unsigned char* curr = CV_GET_CURRENT(position, 3);
      curr[1] = *pPix;
      curr[0] = *pPix;
      curr[2] = *pPix++;
      CV_MOVE_RIGHT(position, 3);
    }
  }
#endif
  // Draw(m_pImage);
  
  DrawMessage();
}

void hv_save_frame(char* pData, int width, int height, bool fullimg)
{
  if (!m_is_initialized) {
    fprintf(stderr, "not initialized\n");
    exit(-1);
  }

  if (!fullimg) {
    if (clock()<=m_save_frame_after) {
      return;
    }
    m_save_frame_after = clock()+CLOCKS_PER_SEC/2;
  }
  

  if (hvGetOverlayLevel()>0) {
    static bool have_warned = false;
    if (!have_warned) {
      hv_show_message("WARNING: NOT SAVING IMAGES (overlay > 0)");
      have_warned = true;
    }
    return;
  }

  { 
    // test if change from initialization
    bool img_bottom_up = true;
    if (height<0) {
      // norm for RGB images
      img_bottom_up = false;
      height = -height;
    }
    if (width!=m_cxImage || height!=m_cyImage || 
	img_bottom_up!=m_img_bottom_up) {
      fprintf(stderr, "hv_process_frame: format changed since init!\n");
      exit(-1);
    }
  }

  m_pImage->imageData = (char*) pData;
  m_pImage->imageSize = width*height;

  string f("");
  if (fullimg) {
    hvSaveImageArea(m_pImage, 0, 0, m_cxImage, m_cyImage, f);
  } else {
    // hack for ARtk_demo
    m_pImage->channelSeq[0] = 'R';
    m_pImage->channelSeq[1] = 'G';
    m_pImage->channelSeq[2] = 'B';
    hvSaveScannedArea(m_pImage, f);
  }
}


void Draw(IplImage* pImage) 
{
  cvRectangle(pImage, cvPoint(5, 100),
	      cvPoint(15, 200), CV_RGB(255,255,0), 3);
  cvCircle(pImage, cvPoint(50, 80), 10, CV_RGB(0,255,0), CV_FILLED);
}

extern int g_verbose;
extern FILE* g_ostream;

bool Initialize(int img_width, int img_height, int iPixelSize,
		string conductor_fname, 
		string save_fname_root)
{
  if (m_is_initialized) {
    return false;
  }

  m_cxImage = img_width;
  if (img_height<0) {
    // norm for RGB images
    m_img_bottom_up = false;
    m_cyImage = -img_height;
  } else {
    m_img_bottom_up = true;
    m_cyImage = img_height;
  }

  if (m_pImage!=NULL) {
    fprintf(stderr, "m_pImage had already been initialized?!?\n");
  }
  m_pImage = 
    cvCreateImageHeader(cvSize(m_cxImage, m_cyImage), IPL_DEPTH_8U, 3);

  g_verbose = 3;
  g_ostream = fopen("HVout.txt", "aw+");

  string version;
  hvGetVersion(version, 3);
  printf("%s\n", version.c_str());
  hvInitialize(m_cxImage, m_cyImage);
  
  hvLoadConductor(conductor_fname);
  hvSetSaveFilenameRoot(save_fname_root);
  
  hvSetOverlayLevel(0);
  hvStartRecognition();
  
  cvInitFont( &m_font, CV_FONT_VECTOR0, 0.5f /* hscale */, 
	      0.5f /* vscale */, 0.1f /*italic_scale */, 
	      1 /* thickness */);

  m_save_frame_after = clock();
  //  m_show_until;
  m_permit_next_color_change_at = clock();

  m_is_initialized = true;

  return true;
}



void hv_key_pressed(char c)
{
  if (!m_is_initialized) {
    return;
  }

  int overlay;
  switch (c) {
  case '0':
  case '1':
  case '2':
  case '3':
    overlay = c-'0';
    hvSetOverlayLevel(overlay);
    break;
    
  case 'r':
    hvStopRecognition();
    hvStartRecognition();
    break;
    
  case 'u':
    if (hvCanCorrectDistortion()) {
      bool enabled = hvIsCorrectingDistortion();
      hvCorrectDistortion(!enabled);
    }
    break;
    
  default:
    break;
  }
}

void hv_show_message(const char* msg)
{
  if (msg) {
    // reset timer
    m_show_until = clock() + 2*CLOCKS_PER_SEC;
    // set new message
    m_msg = string(msg);
    fprintf(stderr, "%s\n", msg);
  } else {
    m_msg = "";
  }
}

void DrawMessage()
{
  if (m_msg=="" || m_show_until<clock()) {
    m_msg = "";
    return;
  }

  CvSize textsize;
  int underline;
  cvGetTextSize( m_msg.c_str(), &m_font, &textsize, &underline );

  int x = 10;
  int y = m_pImage->height-textsize.height-20;

  CvPoint pos = cvPoint(x, y);
  cvPutText(m_pImage, m_msg.c_str(), pos, &m_font, CV_RGB(0, 255, 0));
}

void hv_toggle_adjust_exposure()
{
  if (hvCanAdjustExposure()) {
    bool on = hvIsAdjustingExposure();
    on = !on;
    hvSetAdjustExposure(on);
    if (on) {
      hv_show_message("HandVu is adjusting exposure");
    } else {
      hv_show_message("camera is adjusting exposure");
    }
  } else {
    hv_show_message("cannot toggle exposure control");
  }
}
