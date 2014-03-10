//gcc -shared csmacdmaclayer.cc channel.o csmacdmacdownstreamqueue.o -lACE -lemane -lstdc++ -o libcsmacdmaclayer.so
#include "csmacdmaclayer.h"

#include "emane/emaneconfigurationrequirement.h"

#include "emaneutils/parameterconvert.h"
#include "emaneutils/spawnmemberfunc.h"

#include <ace/OS_NS_time.h>
#include <sstream>
//****************************************************
#include <fstream>
#include <iostream>
//****************************************************
namespace
{ //*************************
  class Channel;
  const EMANE::ConfigurationDefinition defs[] =
    {
      /* required, default, count, name,                       value,               type, description */
      {false,      true,    1,     "enablepromiscuousmode",     "off",              0,    "enable promiscuous mode"},
      {true,       true ,   1,     "datarate",                   "1M",              0,    "datarate bps"},
      {true,       true,    1,     "cwmax",                    "1024",              0,    "max competing windows"},
      {true,       true,    1,     "retrylimits",                "16",              0,    "the max retry time"},
      {true,       true,    1,     "prodelay",                  "100",              0,    "propogation delay (measured in the number of bits transmitted per second)"},
      {true,       false ,  1,     "channel",                      "",              0,    "channel id"},
      {0,0,0,0,0,0,0},
    };
      
      const int cwmin_=1;

      const char * pzLayerName = "CsmaCdMACLayer";
    //const ACE_UINT32 UPSTREAM_PACKET_CALLBACK = 0;
    //const ACE_UINT32 SEND_CALLBACK   = 1;
    //const ACE_UINT32 _CALLBACK   = 2;
  
}


//************************************************************************************
CSMACDMAC::MacLayer::MacLayer(EMANE::NEMId id, EMANE::PlatformServiceProvider *pPlatformService):
  MACLayerImplementor(id, pPlatformService),
  downstreamThread_(0),
  downstreamQueue_(pPlatformService_),
  bRunning_(false),
  u16TxSequenceNumber_(0),
  cond_(mutex_)
{     //std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
       switch(id){
              case 1:
                   fout.open("/tmp/lxc-node/6/1/var/log/debug.txt");
                    break;
              case 2:
                   fout.open("/tmp/lxc-node/6/2/var/log/debug.txt");
                    break;
              case 3:
                   fout.open("/tmp/lxc-node/6/3/var/log/debug.txt");
                    break;
              case 4:
                   fout.open("/tmp/lxc-node/6/4/var/log/debug.txt");
                    break;
              case 5:
                   fout.open("/tmp/lxc-node/6/5/var/log/debug.txt");
                   break;
              default:
                   break;
         }
      

   configRequirements_ = EMANE::loadConfigurationRequirements(defs);
   
  
}


CSMACDMAC::MacLayer::~MacLayer()
{
}



void 
CSMACDMAC::MacLayer::processUpstreamControl(const EMANE::ControlMessage &)
{
}



void 
CSMACDMAC::MacLayer::processDownstreamControl(const EMANE::ControlMessage & msg)
{
}

void 
CSMACDMAC::MacLayer::processUpstreamPacket(ACE_Time_Value tvStart,
                                           const EMANE::CommonMACHeader & hdr,
                                           EMANE::UpstreamPacket & pkt,
                                           const EMANE::ControlMessage & msg)
{
  ACE_Guard<ACE_Thread_Mutex> m(mutex_);

  if(hdr.getRegistrationId() != type_)
    {
      return;
    }

  // get packet info
  const EMANE::PacketInfo pinfo = pkt.getPacketInfo();
 //**********************************************************************
  if(bPromiscuousMode_ || (pinfo.destination_ == EMANE::NEM_BROADCAST_MAC_ADDRESS))
       { //fout<<"recv broad 1"<<pinfo.source_<<std::endl;
        sendUpstreamPacket(pkt);
        return;}

  if(pinfo.destination_ == id_)
       { //********************for debug when nem-2 is the receiver************************
         //if(id_==2)++count;

         // set timeout = delay + jitter (random)+ pkt duration
         ACE_Time_Value tvTimeOut;
         tvTimeOut.set(static_cast<double>(prodelay_/((double)u64DataRatebps_)));
         // if any dey is needed
         ACE_Time_Value timeOut=tvTimeOut+tvStart;
         ACE_Time_Value now=ACE_OS::gettimeofday();    
         if(tvLastrecv > now)
         timeOut+=tvLastrecv-now;

         tvLastrecv=timeOut;

         // create a copy of the pkt, the callback will take ownership of the pkt and itself
         const EMANE::UpstreamPacket *p = new EMANE::UpstreamPacket (pkt);

         // schedule the event id, pkt data, timeout (absolute time), one shot
         const long eventId = pPlatformService_->scheduleTimedEvent(0, p, timeOut);
             // event was scheduled
         if(eventId < 0)
          {delete p;
          }
          return;
       }
}

void 
CSMACDMAC::MacLayer::processUpstreamPacket(const EMANE::CommonMACHeader & hdr,
                                           EMANE::UpstreamPacket & pkt,
                                           const EMANE::ControlMessage & msg)
{   
}


void 
CSMACDMAC::MacLayer::processDownstreamPacket(EMANE::DownstreamPacket & pkt,
                                             const EMANE::ControlMessage &)
{
  // get current time
  const ACE_Time_Value tvCurrentTime = ACE_OS::gettimeofday();
 
  // get packet info
  const EMANE::PacketInfo pinfo = pkt.getPacketInfo();

  // get duration
  const ACE_Time_Value tvDuration = getDuration(u64DataRatebps_, pkt.length());

  // create downstream entry***************************************************
  CSMACDMAC::DownstreamQueueEntry entry(pkt,                       // pkt
                                        EMPTY_CONTROL_MESSAGE, // ctrl
                                        u16TxSequenceNumber_,      // sequence number
                                        tvCurrentTime,             // current time (SOT)
                                        tvDuration);               // duration

  // bump tx sequence number
  ++u16TxSequenceNumber_;

  // enqueue entry
  downstreamQueue_.enqueue(entry);
  //if(pinfo.destination_== EMANE::NEM_BROADCAST_MAC_ADDRESS)
    //fout<<"broad ";
  //fout<<"enqueue "<<tvCurrentTime<<std::endl;
}



void 
CSMACDMAC::MacLayer::initialize()
  throw(EMANE::InitializeException)
{
  
}



void 
CSMACDMAC::MacLayer::configure(const EMANE::ConfigurationItems &items)
  throw(EMANE::ConfigureException)
{ 
  Component::configure(items); 
}


void 
CSMACDMAC::MacLayer::start()
  throw(EMANE::StartException)
{
  EMANE::ConfigurationRequirements::const_iterator iter = configRequirements_.begin();

  try
    {
      for(;iter != configRequirements_.end(); ++iter)
        {
          if(iter->second.bPresent_)
            {
              if(iter->first == "enablepromiscuousmode")
                {
                  bPromiscuousMode_ = EMANEUtils::ParameterConvert(iter->second.item_.getValue()).toBool();
                }
              else if(iter->first == "datarate")
                {
                  u64DataRatebps_ = EMANEUtils::ParameterConvert(iter->second.item_.getValue()).toUINT64(1);
                }
              else if(iter->first == "cwmax")
                {
                  cwmax_= EMANEUtils::ParameterConvert(iter->second.item_.getValue()).toUINT16(1, 0xFFFF);
                }
              else if(iter->first == "retrylimits")
                {
                  retryLimits= EMANEUtils::ParameterConvert(iter->second.item_.getValue()).toUINT16(1, 0xFFFF);
                }
              else if(iter->first == "prodelay")
                {
                  prodelay_ = EMANEUtils::ParameterConvert(iter->second.item_.getValue()).toUINT16(1, 0xFFFF);
                  
                }
             else if(iter->first == "channel")
               { 
                 channelId = EMANEUtils::ParameterConvert(iter->second.item_.getValue()).toUINT16(1, 0xFFFF);
           
               }
              else if(iter->second.bRequired_)
                {
                  std::stringstream ssDescription;
                  ssDescription << pzLayerName << ": missing configuration item " << iter->first << std::ends;
                  throw EMANE::StartException(ssDescription.str());
                }
            }
        }
    }
   catch(EMANEUtils::ParameterConvert::ConversionException & exp)
    {
      std::stringstream sstream;
      sstream << pzLayerName << ": parameter " << iter->first << ": " << exp.what() << std::ends;
      throw EMANE::StartException(sstream.str());
    }

  // link channel
    switch(channelId){
              case 1:
                   channel_=ChannelSingleton1::instance();
                    break;
              case 2:
                   channel_=ChannelSingleton2::instance();
                    break;
              case 3:
                   channel_=ChannelSingleton3::instance();
                    break;
              case 4:
                   channel_=ChannelSingleton4::instance();
                    break;
              case 5:
                   channel_=ChannelSingleton5::instance();
                    break;
              case 6:
                   channel_=ChannelSingleton6::instance();
                    break;
              case 7:
                   channel_=ChannelSingleton7::instance();
                    break;
              case 8:
                   channel_=ChannelSingleton8::instance();
                    break;
              case 9:
                   channel_=ChannelSingleton9::instance();
                    break;
              case 10:
                   channel_=ChannelSingleton10::instance();
                    break;
              default:
                   break;
         }
   
  //*************************************************
  channel_->registerUsr(id_, this);

  //assign the value of some attributes
  ifs_.set(static_cast<double>(96/((double)u64DataRatebps_)));
  slotTime_.set(static_cast<double>(512/((double)u64DataRatebps_)));
  channel_->setdelay(prodelay_/((double)u64DataRatebps_));

  //initial some attibutes
  retry=0;
  tvLastrecv=ACE_Time_Value::zero;
  cw_=cwmin_;
  //count=0;
  // running
  bRunning_ = true;
  // start the downstream thread for queue processing
  EMANEUtils::spawn(*this, &MacLayer::recv, &downstreamThread_);
  
}



void 
CSMACDMAC::MacLayer::postStart()
{
}



void 
CSMACDMAC::MacLayer::stop()
  throw(EMANE::StopException)
{
  // set running flag to false
  bRunning_ = false;
  fout.close();
  switch(id_){
              case 1:
                   remove("tmp/lxc-node/4/1/var/log/debug.txt");
                    break;
              case 2:
                   remove("tmp/lxc-node/4/2/var/log/debug.txt");
                    break;
              case 3:
                   remove("tmp/lxc-node/4/3/var/log/debug.txt");
                    break;
              case 4:
                   remove("tmp/lxc-node/4/4/var/log/debug.txt");
                    break;
              case 5:
                   remove("tmp/lxc-node/4/5/var/log/debug.txt");
                   break;
              default:
                   break;
         }
//std::cout<<count<<"#"<<std::endl;

  // unblock the downstream queue
  downstreamQueue_.cancel();

  // check thread id
  if(downstreamThread_ != 0)
   {
      // join downstream thread
      ACE_OS::thr_join(downstreamThread_, NULL, NULL);

      // reset value
      downstreamThread_ = 0;
   }
   
}



void 
CSMACDMAC::MacLayer::destroy()
  throw()
{
}




void 
CSMACDMAC::MacLayer::processEvent(const EMANE::EventId &, const EMANE::EventObjectState &)
{
}




ACE_THR_FUNC_RETURN 
CSMACDMAC::MacLayer::recv()
{     if(bRunning_ == false)
        return 0;

      CSMACDMAC::DownstreamQueueEntry entry = downstreamQueue_.dequeue();
      ACE_Time_Value now=ACE_OS::gettimeofday();
 
//********************************broadcast**********************************
      if(entry.pkt_.getPacketInfo().destination_ == EMANE::NEM_BROADCAST_MAC_ADDRESS){
        //fout<<"broad send"<<now<<std::endl;
        channel_->send(now,entry.pkt_,id_);
        const long eventId = pPlatformService_->scheduleTimedEvent(3, NULL, now);
        }

//**********************************unicast************************************
      else{
      //fout<<"send"<<now<<std::endl;
      ACE_Guard<ACE_Thread_Mutex> m(mutex_);
      //if(id_!=2)++count;
      cw_=cwmin_;
      retry=0;
      CSMACDMAC::DownstreamQueueEntry *p = new CSMACDMAC::DownstreamQueueEntry (entry);
      // schedule the event id, pkt data, timeout (absolute time), one shot
      ACE_Time_Value timeOut=now+ifs_;
      
      // event was scheduled 
      const long eventId = pPlatformService_->scheduleTimedEvent(1, p, timeOut);
      //std::cout<<eventId<<" "<<id_<<" sched to process "<<timeOut.usec()<<std::endl;
      if(eventId < 0)
          {
            delete p;
            //break;
          }
      }  
  return 0;
}
//******************************************************************************************************
int 
CSMACDMAC::MacLayer::random(int maxI)
{ 
	srand((unsigned int)ACE_OS::gettimeofday().usec());
	int i=rand()%(maxI+1);
	return i;
}

ACE_Time_Value 
CSMACDMAC::MacLayer::getDuration(ACE_UINT64 u64DataRatebps, size_t lengthInBytes)
{
  // the result
  ACE_Time_Value tvResult = ACE_Time_Value::zero;

  // if data rate is greater than zero
  if (u64DataRatebps > 0)
   {
     // seconds and fraction of seconds
     const double dDuration = ((lengthInBytes * 8.0) / (double) u64DataRatebps);

     // set the result
     tvResult.set(dDuration);
   }

  // return result
  return tvResult;
}


void 
CSMACDMAC::MacLayer::processTimedEvent(ACE_UINT32 taskType, long eventId, const ACE_Time_Value & tvEventTime, const void *arg)
{ ACE_Time_Value now=tvEventTime;
  if(taskType==0){
    ACE_Guard<ACE_Thread_Mutex> m(mutex_);
    EMANE::UpstreamPacket * p = (EMANE::UpstreamPacket *) arg;
    if(p != NULL)
     {const EMANE::PacketInfo pinfo = p->getPacketInfo();
       fout<<p->getPacketInfo().source_<<" recv "<<now<<std::endl<<std::endl;
       // send packet upstream
       sendUpstreamPacket(*p);
       //************************************
       delete p;
     }
    return;
  }
  else if(taskType==1){
    ACE_Guard<ACE_Thread_Mutex> m(mutex_);
    //std::cout<<id_<<" start to process "<<tvEventTime.usec()<<std::endl;
    send(arg,now);
    return;
    }
  else if(taskType==2){
     ACE_Guard<ACE_Thread_Mutex> m(mutex_);
    // std::cout<<eventId<<" "<<id_<<" EOC "<<tvEventTime.usec()<<std::endl;
     EOContention(arg,now);
     return;
    }
  else if(taskType==3)
    { 
      if(downstreamThread_ != 0)
      {
        // join downstream thread
        ACE_OS::thr_join(downstreamThread_, NULL, NULL);

        // reset value
        downstreamThread_ = 0;
      }
      EMANEUtils::spawn(*this, &MacLayer::recv, &downstreamThread_);
      //recv();
      return;
    }
   
}
void 
CSMACDMAC::MacLayer::send(const void *arg,ACE_Time_Value now)
{      CSMACDMAC::DownstreamQueueEntry * p = (CSMACDMAC::DownstreamQueueEntry *) arg;
       //ACE_Time_Value now=ACE_OS::gettimeofday();
       fout<<id_<<" start to process seq:"<<p->seq_<<" retry:"<<retry<<" "<<now<<std::endl;
       ACE_Time_Value tvIdle=channel_->txstop()+ifs_;
        //channel is busy
        if(tvIdle>now )
         {
            const long eventId = pPlatformService_->scheduleTimedEvent(1, p, tvIdle-now+ACE_OS::gettimeofday());          
            if(eventId < 0)
                 {
                   delete p;
                 }
            return;
          }
           
        //channel is idle
        else
        { 
          p->tvSOT_=now;
          ////std::cout<<" send "<<tvEventTime.usec()<<std::endl;
          ACE_Time_Value tvEOC=channel_->contention(p->tvSOT_,id_)+ACE_OS::gettimeofday();
         
          
         ////std::cout<<" sched EOC "<<ACE_OS::gettimeofday().usec()<<std::endl;
          const long eventId = pPlatformService_->scheduleTimedEvent(2, p, tvEOC);
          if(eventId < 0)
                 {  
                   delete p;
                 }
               return;
         
        }
}

void 
CSMACDMAC::MacLayer::EOContention(const void *arg,ACE_Time_Value now){
CSMACDMAC::DownstreamQueueEntry * p = (CSMACDMAC::DownstreamQueueEntry *) arg;
if(channel_->collision()>1)
        {//*******************************************************for debug****************************
         ++retry;
         //retry
         if(retry<(retryLimits-1))
           {
	     int slot = random(cw_);
             cw_ = 2 * cw_;
             if(cw_>cwmax_)
               {  cw_=cwmax_;
               }
             //time  conversion, special compution "*" between ACE_Time_Value object.*********************
             ACE_Time_Value backoffTime;
             backoffTime.set(static_cast<double>(((slotTime_.sec() * 1000000) + slotTime_.usec())*slot/1.0e6));

             const ACE_Time_Value tvEOBackoff=backoffTime +ACE_OS::gettimeofday();
	       const long eventId = pPlatformService_->scheduleTimedEvent(1, p, tvEOBackoff);
               if(eventId < 0)
                 { 
                   delete p;
                 }
               return;
      
           }
          //drop without retrying.
          else
           { //std::cout<<id_<<" drop!!!!"<<std::endl;
             delete p;
             if(downstreamThread_ != 0)
               { // join downstream thread
                 ACE_OS::thr_join(downstreamThread_, NULL, NULL);

                 // reset value
                 downstreamThread_ = 0;
               }             
             EMANEUtils::spawn(*this, &MacLayer::recv, &downstreamThread_);
             return;
           }
        }
  else
    {
         ////std::cout<<p->pkt_.getPacketInfo().source_<<" send "<<tvEventTime.usec()<<std::endl;
         ACE_Time_Value tvEOT=p->tvDuration_-(now-p->tvSOT_)+ACE_OS::gettimeofday();

         ////std::cout<<" invork at the EOC delay"<<now.usec()<<std::endl;
         channel_->send(tvEOT,p->pkt_,id_);

         
         delete p;
          const long eventId = pPlatformService_->scheduleTimedEvent(3, NULL, tvEOT);
          if(eventId < 0)
            { 
               std::cout<<"wrong!"<<std::endl;
            }
          return;
        }
}
DECLARE_MAC_LAYER(CSMACDMAC::MacLayer);
