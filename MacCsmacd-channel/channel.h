#ifndef CSMACDMAC_CHANNEL_HEADER_
#define CSMACDMAC_CHANNEL_HEADER_

#include <ace/Time_Value.h>
#include <ace/Null_Mutex.h>
#include <ace/Singleton.h>
#include <map>
#include "emane/emanemaclayerimpl.h"
#include "emane/emanemactypes.h"
#include "emane/emanedownstreampacket.h"
#include "emane/emaneupstreampacket.h"
namespace CSMACDMAC {
//**********************************
        //class EMANE::MACLayerImplementor;

	class Channel{
	public:
                Channel();
		~Channel();
                //****************************************************
                bool registerUsr(EMANE::NEMId id, EMANE::MACLayerImplementor * usr);
                bool unregisterUsr(EMANE::NEMId id);
		
                void setdelay(double prodelay);
		ACE_Time_Value contention(ACE_Time_Value startoftrans,int id);
		int collision();
		void send(ACE_Time_Value txtime,EMANE::DownstreamPacket &pkt_,EMANE::NEMId id);
	
		ACE_Time_Value txstop();
                
		
	private:
 	  ACE_Time_Value delay_;//propagation time, all the time between nodes in this channel are the same .
 	  ACE_Time_Value txstop_;
 	  ACE_Time_Value cwstop_;
 	  int numtx_;
          typedef std::map<EMANE::NEMId,EMANE::MACLayerImplementor *> MACUserMap;
          MACUserMap macUserMap_;
          ACE_RW_Thread_Mutex rwmutex_cwstop_numtx;
          ACE_RW_Thread_Mutex rwmutex_txstop;
          ACE_RW_Thread_Mutex rwmutex_;
        };

        class Channel1 : public Channel{
        public:
          Channel1();
        };
        typedef ACE_Singleton <Channel1, ACE_Null_Mutex > ChannelSingleton1;

        class Channel2 : public Channel{
        public:
          Channel2();
        };
 	typedef ACE_Singleton <Channel2, ACE_Null_Mutex > ChannelSingleton2;

        class Channel3 : public Channel{
        public:
          Channel3();
        };
        typedef ACE_Singleton <Channel3, ACE_Null_Mutex > ChannelSingleton3;

        class Channel4 : public Channel{
        public:
          Channel4();
        };
 	typedef ACE_Singleton <Channel4, ACE_Null_Mutex > ChannelSingleton4;

        class Channel5 : public Channel{
        public:
          Channel5();
        };
        typedef ACE_Singleton <Channel5, ACE_Null_Mutex > ChannelSingleton5;

        class Channel6 : public Channel{
        public:
          Channel6();
        };
 	typedef ACE_Singleton <Channel6, ACE_Null_Mutex > ChannelSingleton6;

        class Channel7 : public Channel{
        public:
          Channel7();
        };
        typedef ACE_Singleton <Channel7, ACE_Null_Mutex > ChannelSingleton7;

        class Channel8 : public Channel{
        public:
          Channel8();
        };
 	typedef ACE_Singleton <Channel8, ACE_Null_Mutex > ChannelSingleton8;

        class Channel9 : public Channel{
        public:
          Channel9();
        };
        typedef ACE_Singleton <Channel9, ACE_Null_Mutex > ChannelSingleton9;

        class Channel10 : public Channel{
        public:
          Channel10();
        };
 	typedef ACE_Singleton <Channel10, ACE_Null_Mutex > ChannelSingleton10;
 }
#endif	
  

 	  
		
  
  
