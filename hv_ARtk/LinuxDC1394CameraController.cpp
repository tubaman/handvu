

#include "LinuxDC1394CameraController.h"

#if 0

LinuxDC1394CameraController::LinuxDC1394CameraController()
  : m_handle(NULL),
    m_can_control_exposure(false),
    m_can_control_gain(false),
    m_initialized(false)
{
  m_initialized = true;

  // get handle to dc1394 interface
  m_handle = dc1394_create_handle(0);
  if (m_handle==NULL) {
    fprintf(stderr, "can't create handle\n");
    return;
  }

  // get the first camera on the interface
  
  int numCameras;
  int showCameras = 0; // don't print info
  nodeid_t* nodes =
    dc1394_get_camera_nodes(m_handle, &numCameras, showCameras);
  if (numCameras==-1 || nodes==NULL) {
    fprintf(stderr, "can't get cameras\n");
    return;
  }
  m_node = nodes[0];

  // init might not be necessary
  int success = dc1394_init_camera(m_handle, m_node);
  if (success!=DC1394_SUCCESS) {
    fprintf(stderr, "can not init camera\n");  
    return;
  }

  // debug stuff
  //  dc1394_camerainfo cinfo;
  //  dc1394_print_camera_info(&cinfo);
 
  // query for manual exposure availability
  dc1394_feature_info info;
  info.feature_id = FEATURE_EXPOSURE;
  dc1394_get_camera_feature(m_handle, m_node, &info);
  //  dc1394_print_feature(&info);
  if (info.manual_capable != DC1394_TRUE) {
    fprintf(stderr, "can not control exposure\n");
    return;
  }

  m_min_exposure = info.min;
  m_max_exposure = info.max;
  if (m_min_exposure >= m_max_exposure) {
    fprintf(stderr, "no or wrong exposure range");
    return;
  }

  m_can_control_exposure = true;

  // query for manual gain availability
  info.feature_id = FEATURE_GAIN;
  dc1394_get_camera_feature(m_handle, m_node, &info);
  if (info.manual_capable != DC1394_TRUE) {
    fprintf(stderr, "can not control gain\n");
    return;
  }

  m_min_gain = info.min;
  m_max_gain = info.max;
  if (m_min_gain >= m_max_gain) {
    fprintf(stderr, "no or wrong exposure range");
    return;
  }

  m_can_control_gain = true;
}


// [0..1]
double LinuxDC1394CameraController::GetCurrentExposure()   
{
  unsigned int curr_exposure = 0;
  int success = dc1394_get_exposure(m_handle, m_node, &curr_exposure);
  if (success!=DC1394_SUCCESS) {
    fprintf(stderr, "get_exposure returned %d\n", success);
    return -1;
  }
  m_last_exposure = curr_exposure;
  return (curr_exposure-m_min_exposure) / 
    (m_max_exposure-m_min_exposure);
}


// true if change has an effect, false if step is too small
// [0..1]
bool LinuxDC1394CameraController::SetExposure(double exposure)  
{
  printf("exposure %f\n", exposure);
  unsigned int new_exposure = (unsigned int)
    exposure*(m_max_exposure-m_min_exposure)-m_min_exposure;
  if (new_exposure==m_last_exposure) {
    return false;
  }

  int success = 
    dc1394_set_exposure(m_handle, m_node, new_exposure);
  if (success!=DC1394_SUCCESS) {
    fprintf(stderr, "set_exposure returned %d\n", success);
    return false;
  }
  m_last_exposure = new_exposure;

  unsigned int new_gain = 
    (unsigned int) exposure*(m_max_gain-m_min_gain)-m_min_gain;
  success = 
    dc1394_set_gain(m_handle, m_node, new_gain);
  if (success!=DC1394_SUCCESS) {
    fprintf(stderr, "set_gain returned %d\n", success);
    return false;
  }

  return true;
}


bool LinuxDC1394CameraController::SetCameraAutoExposure(bool enable)
{
  if (enable) {
    // disable software control, enable camera auto control

    // test if camera auto exposure control is on
    dc1394bool_t is_on = DC1394_FALSE;
    int success = 
      dc1394_is_feature_auto(m_handle, m_node,
			     FEATURE_EXPOSURE, &is_on);
    if (success!=DC1394_SUCCESS) {
      fprintf(stderr, "error querying auto exposure\n");
      return false;
    }

    if (is_on==DC1394_FALSE) {
      // printf("auto exposure control is not active\n");
      success = 
	dc1394_auto_on_off(m_handle, m_node, 
			   FEATURE_EXPOSURE, DC1394_TRUE);
      if (success!=DC1394_SUCCESS) {
	fprintf(stderr, "can not turn on camera auto exposure\n");
	return false;
      }
    }

    if (m_can_control_gain) {
      // turn on auto-gain, too

      // test if camera auto gain control is on
      is_on = DC1394_FALSE;
      success = 
	dc1394_is_feature_auto(m_handle, m_node,
			       FEATURE_GAIN, &is_on);
      if (success!=DC1394_SUCCESS) {
	fprintf(stderr, "error querying auto gain\n");
	return false;
      }

      if (is_on==DC1394_FALSE) {
	success = 
	  dc1394_auto_on_off(m_handle, m_node, 
			     FEATURE_GAIN, DC1394_TRUE);
	if (success!=DC1394_SUCCESS) {
	  fprintf(stderr, "can not turn on camera auto gain\n");
	  // ignore this
	  // return false;
	}
      }
    }

  } else {
    // test if camera auto exposure control is on
    dc1394bool_t is_on = DC1394_TRUE;
    int success = 
      dc1394_is_feature_auto(m_handle, m_node,
			     FEATURE_EXPOSURE, &is_on);
    if (success!=DC1394_SUCCESS) {
      fprintf(stderr, "error querying auto exposure\n");
      return false;
    }

    if (is_on==DC1394_TRUE) {
      // printf("auto exposure control is active\n");
      success = 
	dc1394_auto_on_off(m_handle, m_node, 
			   FEATURE_EXPOSURE, DC1394_FALSE);
      if (success!=DC1394_SUCCESS) {
	fprintf(stderr, "can not turn off camera auto exposure\n");
	return false;
      }
    }
   
    // set m_last_exposure
    double val = GetCurrentExposure();
    if (val==-1) {
      return false;
    }
  }
  
  return true;
}


bool LinuxDC1394CameraController::CanAdjustExposure()
{
  return m_can_control_exposure;
}

#endif
