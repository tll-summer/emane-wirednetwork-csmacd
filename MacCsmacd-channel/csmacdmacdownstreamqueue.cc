/*
 * Copyright (c) 2010 - DRS CenGen, LLC, Columbia, Maryland
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of DRS CenGen, LLC nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The copyright holder hereby grants to the U.S. Government a copyright
 * license to use this computer software/computer software documentation
 * that is of the same scope as the rights set forth in the definition of
 * "unlimited rights" found in DFARS 252.227-7014(a)(15)(June 1995).
 */


#include "csmacdmacdownstreamqueue.h"

namespace 
{
  const ACE_UINT16 QUEUE_SIZE_DEFAULT = 0xFF;
}

CSMACDMAC::DownstreamQueue::DownstreamQueue(EMANE::PlatformServiceProvider * pPlatformService)
  : pPlatformService_(pPlatformService),
    bCancel_(false), 
    maxQueueSize_(QUEUE_SIZE_DEFAULT),
    cond_(mutex_)
{
  pPlatformService_->registerStatistic("numPacketsEnqued",   &numPacketsEnqued_);
  pPlatformService_->registerStatistic("numBytesEnqued",     &numBytesEnqued_);
  pPlatformService_->registerStatistic("numPacketsDequed",   &numPacketsDequed_);
  pPlatformService_->registerStatistic("numPacketsOverflow", &numPacketsOverFlow_);
  pPlatformService_->registerStatistic("numHighWaterMark",   &numHighWaterMark_);
}


CSMACDMAC::DownstreamQueue::~DownstreamQueue() 
{  
  // unregister statistics container
  pPlatformService_->unregisterStatistic("numPacketsEnqued");
  pPlatformService_->unregisterStatistic("numBytesEnqued");
  pPlatformService_->unregisterStatistic("numPacketsDequed");
  pPlatformService_->unregisterStatistic("numPacketsOverflow");
  pPlatformService_->unregisterStatistic("numHighWaterMark");
} 

size_t 
CSMACDMAC::DownstreamQueue::total() 
{ 
   ACE_Guard<ACE_Thread_Mutex> m(mutex_);

   // return queue size
   return queue_.size();
}



CSMACDMAC::DownstreamQueueEntry 
CSMACDMAC::DownstreamQueue::dequeue()
{ 
   ACE_Guard<ACE_Thread_Mutex> m(mutex_);

   // wait while queue is empty and not canceled
   while(queue_.empty() && !bCancel_) 
     {
       cond_.wait();
     }

   // woke up canceled
   if (bCancel_)
     {
       return CSMACDMAC::DownstreamQueueEntry();
     }

   // get entry
   CSMACDMAC::DownstreamQueueEntry entry = queue_.front();
 
   // pop entry 
   queue_.pop();
   
   // bump packets dequeued
   ++numPacketsDequed_;

   // return entry
   return entry;
}


void 
CSMACDMAC::DownstreamQueue::enqueue(CSMACDMAC::DownstreamQueueEntry &entry) 
{ 
   ACE_Guard<ACE_Thread_Mutex> m(mutex_);

   // check for queue overflow
   while(queue_.size() >= maxQueueSize_) 
     {
       // bump overflow count
       ++numPacketsOverFlow_; 

       // pop entry
       queue_.pop();
     }

   // push entry 
   queue_.push(entry);

   // bump enque pkt count
   ++numPacketsEnqued_;

   // bump enque byte count
   numBytesEnqued_ += entry.pkt_.length();

   // bump high water mark
   if(queue_.size() > numHighWaterMark_.getValue()) 
     {
       numHighWaterMark_ = queue_.size();
     }

   // signal entry added
   cond_.signal();
}



const CSMACDMAC::DownstreamQueueEntry & 
CSMACDMAC::DownstreamQueue::peek()
{ 
   ACE_Guard<ACE_Thread_Mutex> m(mutex_);

   // return entry
   return queue_.front();
}



void 
CSMACDMAC::DownstreamQueue::cancel()
{
  ACE_Guard<ACE_Thread_Mutex> m(mutex_);
 
  // set cancel flag 
  bCancel_ = true;

  // signal entry added
  cond_.signal();
}
