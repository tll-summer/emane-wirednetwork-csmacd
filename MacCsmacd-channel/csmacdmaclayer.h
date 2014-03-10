#ifndef CSMACDMAC_MACLAYER_HEADER_
#define CSMACDMAC_MACLAYER_HEADER_

#include "emane/emanemaclayerimpl.h"
#include "emane/emanemactypes.h"
#include "emane/emanestatisticunsignedinteger32.h"

#include "csmacdmacdownstreamqueue.h"
//***************************************************
#include "channel.h"

#include <fstream>
#include <ace/Time_Value.h>
//***************************************************
#include <ace/Thread_Mutex.h>
#include <ace/Condition_T.h>
#include <ace/Task.h>
namespace CSMACDMAC {

/**
 *
 * @class MacLayer
 *
 * @brief Implementation of the rf pipe mac layer.
 *
 */

 class MacLayer : public EMANE::MACLayerImplementor
 {
 public:
   MacLayer(EMANE::NEMId id, EMANE::PlatformServiceProvider *pPlatformService);
  ~MacLayer();

  void initialize()
    throw(EMANE::InitializeException);
  void configure(const EMANE::ConfigurationItems & items)
    throw(EMANE::ConfigureException);
  void start()
    throw(EMANE::StartException);
  void postStart();
  void stop()
    throw(EMANE::StopException);
  void destroy()
    throw();

  void processUpstreamControl(const EMANE::ControlMessage &msg);
  void processUpstreamPacket(ACE_Time_Value tvStart,
                             const EMANE::CommonMACHeader & hdr,
                             EMANE::UpstreamPacket & pkt,
                             const EMANE::ControlMessage &msg);
  void processUpstreamPacket(const EMANE::CommonMACHeader & hdr,
                             EMANE::UpstreamPacket & pkt,
                             const EMANE::ControlMessage &msg);
 
  void processDownstreamControl(const EMANE::ControlMessage &msg);
  void processDownstreamPacket(EMANE::DownstreamPacket &pkt,const EMANE::ControlMessage &msg);

  void processEvent(const EMANE::EventId &eventId,const EMANE::EventObjectState &state);
  void processTimedEvent(ACE_UINT32 taskType, long eventId, const ACE_Time_Value &tv, const void *arg);


 private:
  static const EMANE::RegistrationId type_ = EMANE::REGISTERED_EMANE_MAC_CSMACD;
// functions  
  ACE_THR_FUNC_RETURN recv();  
  int random(int i);
  void send(const void *arg,ACE_Time_Value t);
  void EOContention(const void *arg,ACE_Time_Value t);
  ACE_Time_Value getDuration(ACE_UINT64 u64DataRatebps, size_t lengthInBytes);

  // config items
  bool            bPromiscuousMode_;
  ACE_UINT64      u64DataRatebps_;
  ACE_UINT16 cwmax_;
  ACE_UINT16 retryLimits;
  ACE_UINT16 prodelay_;
  ACE_UINT16 channelId;

  //fix attributes
  ACE_Time_Value ifs_;
  ACE_Time_Value slotTime_;

  bool bRunning_;
  ACE_thread_t downstreamThread_;

  CSMACDMAC::DownstreamQueue downstreamQueue_;
  ACE_UINT16 u16TxSequenceNumber_;

  Channel* channel_;
  int cw_;
  int retry;
  ACE_Time_Value tvLastrecv;
  
  
  ACE_Thread_Mutex mutex_;
  ACE_Condition<ACE_Thread_Mutex> cond_;


//***for debug **************************************
  //int count;
  std::ofstream fout;  
 };
}

#endif //CSMACDMAC_MACLAYER_HEADER_
