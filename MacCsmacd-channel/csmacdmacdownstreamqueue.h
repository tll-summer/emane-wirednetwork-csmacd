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

#ifndef CSMACDMAC_DOWNSTREAMQUEUE_HEADER_
#define CSMACDMAC_DOWNSTREAMQUEUE_HEADER_

#include "emane/emanephylayerimpl.h"
#include "emane/emaneplatformserviceprovider.h"
#include "emane/emanestatisticunsignedinteger32.h"

#include <queue>

#include <ace/Time_Value.h>
#include <ace/Thread_Mutex.h>
#include <ace/Condition_T.h>
#include <ace/Task.h>

namespace CSMACDMAC {
 /**
  * Downstream queue entry definition
  *
  * @brief CSMACD MAC downstream queue entry definition
  */
 struct DownstreamQueueEntry 
 {
   EMANE::DownstreamPacket pkt_;                         // packet payload
   EMANE::ControlMessage   ctrl_;                        // packet ctrl info
   ACE_UINT16              seq_;                         // packet sequence number
   ACE_Time_Value          tvSOT_;                       // packet start-of-transmission
   ACE_Time_Value          tvDuration_;                  // packet transmission duration


  /**
   *
   * @brief  initializer
   *
   * @param pkt        reference to the downstream packet
   * @param ctrl       reference to the control message
   * @param seq        sequence number
   * @param tvSOT      start of transmission
   * @param tvDuration duration of transmision
   */

   DownstreamQueueEntry(const EMANE::DownstreamPacket & pkt, 
                        const EMANE::ControlMessage & ctrl, 
                        ACE_UINT16 seq, 
                        const ACE_Time_Value & tvSOT,
                        const ACE_Time_Value & tvDuration) :
       pkt_(pkt),
       ctrl_(ctrl),
       seq_(seq),
       tvSOT_(tvSOT),
       tvDuration_(tvDuration)
   { }
  
  /**
   *
   * @brief  default constructor
   *
   */
DownstreamQueueEntry() :
     pkt_(EMANE::PacketInfo(), 0, 0),
     ctrl_(0,0,0,0),
     seq_(0),
     tvSOT_(ACE_Time_Value::zero),
     tvDuration_(ACE_Time_Value::zero)
   { }
 };


 typedef std::queue<DownstreamQueueEntry> DownstreamPacketQueue;

 /**
  * @class DownstreamQueue
  *
  * @brief Provides a queue implementation for the CSMACD Mac layer.
  */
 class DownstreamQueue
 {
 public:

  /**
   * @brief Constructor
   *
   * @param pPlatformService reference to the platformservice provider
   *
   */ 
  DownstreamQueue(EMANE::PlatformServiceProvider * pPlatformService);

  /**
   *
   * @brief Destructor
   *
   */
  virtual ~DownstreamQueue();

  /**
   * 
   * @brief Returns the current size of the queue
   *
   * @retval size_t current size of the queue
   *
   */
  size_t total();

  /**
   * 
   * @brief removes an element from the queue
   *
   * @return entry the pop'd entry
   *
   */
  DownstreamQueueEntry dequeue();

  /**
   * 
   * @brief Adds an element to the queue
   *
   * @param entry the entry to be added to the queue
   *
   */
  void enqueue(DownstreamQueueEntry &entry);

  /**
   * 
   * @brief Returns a reference to the element to be pop'd next
   *
   * @return entry the entry to be pop'd next
   *
   */
  const DownstreamQueueEntry & peek();

  /**
   * 
   * @brief Cancels the dequeing loop
   *
   */
  void cancel();

 private:
  /**
   * 
   * Reference to the platfrom service
   *
   */
  EMANE::PlatformServiceProvider * pPlatformService_;


  /**
   * 
   * Interrupt variable
   *
   */
  bool bCancel_;

  /**
   * 
   * Actual Queue
   *
   */
  DownstreamPacketQueue queue_;

  /**
   * 
   * Maximum allowable size
   *
   */
  size_t maxQueueSize_;

  /**
   * 
   * Synchronization mechanism
   *
   */
  ACE_Thread_Mutex mutex_;

  /**
   * 
   * Synchronization mechanism
   *
   */
  ACE_Condition<ACE_Thread_Mutex> cond_;

  /**
   * 
   * Stat counters
   *
   */
  EMANE::StatisticUnsignedInteger32 numPacketsEnqued_;
  EMANE::StatisticUnsignedInteger32 numBytesEnqued_;
  EMANE::StatisticUnsignedInteger32 numPacketsDequed_;
  EMANE::StatisticUnsignedInteger32 numPacketsOverFlow_;
  EMANE::StatisticUnsignedInteger32 numHighWaterMark_;
 };
}

#endif //CSMACDMAC_DOWNSTREAMQUEUE_HEADER_
