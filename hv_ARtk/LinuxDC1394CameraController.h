#include <dc1394_control.h>
#include "Common.h"
#include "HandVu.h"


#if 0
class LinuxDC1394CameraController : public CameraController {
 public:
  LinuxDC1394CameraController();

 public:
  virtual double GetCurrentExposure();        // [0..1]
  // true if change has an effect, false if step is too small
  virtual bool SetExposure(double exposure);  // [0..1]
  virtual bool SetCameraAutoExposure(bool enable=true);
  virtual bool CanAdjustExposure();

 private:
  raw1394handle_t           m_handle;
  nodeid_t                  m_node;
  bool                      m_can_control_exposure;
  bool                      m_can_control_gain;
  int                       m_min_exposure;
  int                       m_max_exposure;
  int                       m_min_gain;
  int                       m_max_gain;
  unsigned int              m_last_exposure;
  bool                      m_initialized;

};

#endif
