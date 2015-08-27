#ifndef BEBOP_H
#define BEBOP_H

#define BEBOP_ERR_STR_SZ  150

#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

extern "C"
{
  #include "libARSAL/ARSAL.h"
  #include "libARController/ARController.h"
  #include "libARDiscovery/ARDiscovery.h"
}

#include "bebop_autonomy/bebop_video_decoder.h"


// Debug
#include <iostream>
#include <fstream>
#include <sys/syscall.h>
#include <sys/types.h>

namespace bebop_autonomy
{

namespace util
{
inline long int GetLWPId()
{
  return syscall(SYS_gettid);
}

}  // namespace util

class Bebop
{
private:
  static const char* LOG_TAG;
  boost::atomic<bool> is_connected_;
  ARDISCOVERY_Device_t* device_ptr_;
  ARCONTROLLER_Device_t* device_controller_ptr_;
  eARCONTROLLER_ERROR error_;
  eARCONTROLLER_DEVICE_STATE device_state_;
  ARSAL_Sem_t state_sem_;
  VideoDecoder video_decoder_;

  // sync
  mutable boost::condition_variable frame_avail_cond_;
  mutable boost::mutex frame_avail_mutex_;
  mutable bool is_frame_avail_;

  static void StateChangedCallback(eARCONTROLLER_DEVICE_STATE new_state, eARCONTROLLER_ERROR error, void *bebop_void_ptr);
  static void CommandReceivedCallback(eARCONTROLLER_DICTIONARY_KEY cmd_key, ARCONTROLLER_DICTIONARY_ELEMENT_t* element_dict_ptr, void* bebop_void_ptr);
  static void FrameReceivedCallback(ARCONTROLLER_Frame_t *frame, void *bebop_void_ptr_);

  void Cleanup();

  void ThrowOnInternalError(const std::string& message = std::string());
  void ThrowOnCtrlError(const eARCONTROLLER_ERROR& error, const std::string& message = std::string());

public:

  inline ARSAL_Sem_t* GetStateSemPtr() {return &state_sem_;}
  inline const ARCONTROLLER_Device_t* GetControllerCstPtr() const {return device_controller_ptr_;}

  // Make this atomic
  inline bool IsConnected() const {return is_connected_;}

  Bebop(ARSAL_Print_Callback_t custom_print_callback = 0);
  ~Bebop();

  void Connect();
  bool Disconnect();

  void Takeoff();
  void Land();

  // -1..1
  void Move(const double& roll, const double& pitch, const double& gaz_speed, const double& yaw_speed);
  void MoveCamera(const double& tilt, const double& pan);

  // This function is blocking and runs in the caller's thread's context
  // which is different from FrameReceivedCallback's context
  bool GetFrontCameraFrame(std::vector<uint8_t>& buffer, uint32_t &width, uint32_t &height) const;
  uint32_t GetFrontCameraFrameWidth() const {return video_decoder_.GetFrameWidth();}
  uint32_t GetFrontCameraFrameHeight() const {return video_decoder_.GetFrameHeight();}

  // Debug
//  std::ofstream out_file;
};

}  // namespace bebop_autonomy


#endif  // BEBOP_H
