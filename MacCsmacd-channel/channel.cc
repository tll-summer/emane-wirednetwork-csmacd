//gcc -c channel.cc -lemane -lACE -lstdc++
#include "channel.h"
#include <iostream>
#include <fstream>
//***************************************
CSMACDMAC::Channel::Channel(){
	  //delay_.set(static_cast<double>(100/1.0e6));
 	  txstop_=ACE_Time_Value::zero;
 	  cwstop_=ACE_Time_Value::zero;
 	  numtx_=0;
	  }
void 
CSMACDMAC::Channel::setdelay(double prodelay)
		{
	  delay_.set(prodelay);
	  }
//*************************************************************************************	  
CSMACDMAC::Channel::~Channel(){
      ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(rwmutex_);
			
	for(MACUserMap::const_iterator iter = macUserMap_.begin(); iter != macUserMap_.end(); ++iter)
       { 
   	unregisterUsr(iter->first);
       }
}

//******************************************	  
bool 
CSMACDMAC::Channel::registerUsr(EMANE::NEMId id, EMANE::MACLayerImplementor * usr)
    {
     ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(rwmutex_);
    
    std::pair<MACUserMap::iterator, bool> ret;
  
     if(macUserMap_.insert(std::make_pair(id,usr)).second == false)
      {
        return false;
      }
     return true;
    }

//**********************************************************
bool 
CSMACDMAC::Channel::unregisterUsr(EMANE::NEMId id)
{
     ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(rwmutex_);

     if(macUserMap_.erase(id) == 0)
      {
        return false;
      }
     return true;
}

ACE_Time_Value 
CSMACDMAC::Channel::contention(ACE_Time_Value startoftrans,int id)
{           ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(rwmutex_cwstop_numtx);

 	    if(startoftrans> cwstop_){
 		    cwstop_ = startoftrans+ delay_;
 		    numtx_ = 0;
 		  }
 	    numtx_++;
 	    return cwstop_-startoftrans;
}

int 
CSMACDMAC::Channel::collision()
{ ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(rwmutex_cwstop_numtx);
       return numtx_ ;
}

//********************************************************************
void 
CSMACDMAC::Channel::send(ACE_Time_Value tvEOT,EMANE::DownstreamPacket &pkt_,EMANE::NEMId id)
{ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(rwmutex_txstop);  
 txstop_ = tvEOT;
 //std::cout<<ACE_OS::gettimeofday().usec()<<" "<<id<<" send "<<tvEOT.usec()<<std::endl;
 //ACE_Read_Guard<ACE_RW_Thread_Mutex> guard(rwmutex_);
 for(MACUserMap::const_iterator iter = macUserMap_.begin(); iter != macUserMap_.end(); ++iter)
 {
   if(iter->first ==id)
   {continue;}
   //wait for a propo time,then recv
   //0x0004 is the CSMACD_MAC defined in emanemactypes.h
   
   iter->second->recv(tvEOT,EMANE::CommonMACHeader(0x0006),EMANE::UpstreamPacket(pkt_.getPacketInfo(),pkt_.get(),pkt_.length())); 
}
 return ;
}
	
ACE_Time_Value 
CSMACDMAC::Channel::txstop()
{  ACE_Read_Guard<ACE_RW_Thread_Mutex> guard(rwmutex_txstop);
    return txstop_;
}

CSMACDMAC::Channel1::Channel1():Channel::Channel(){}
CSMACDMAC::Channel2::Channel2():Channel::Channel(){}
CSMACDMAC::Channel3::Channel3():Channel::Channel(){}
CSMACDMAC::Channel4::Channel4():Channel::Channel(){}
CSMACDMAC::Channel5::Channel5():Channel::Channel(){}
CSMACDMAC::Channel6::Channel6():Channel::Channel(){}
CSMACDMAC::Channel7::Channel7():Channel::Channel(){}
CSMACDMAC::Channel8::Channel8():Channel::Channel(){}
CSMACDMAC::Channel9::Channel9():Channel::Channel(){}
CSMACDMAC::Channel10::Channel10():Channel::Channel(){}
		
