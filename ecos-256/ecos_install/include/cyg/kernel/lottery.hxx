#ifndef CYGONCE_KERNEL_LOTTERY_HXX
#define CYGONCE_KERNEL_LOTTERY_HXX

//==========================================================================
//
//      lottery.hxx
//
//      Lottery scheduler class declarations
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-09-10
// Purpose:     Define lottery scheduler implementation
// Description: The classes defined here are used as base classes
//              by the common classes that define schedulers and thread
//              things. A lottery scheduler provides each thread with a
//              share of the processor based on the number of tickets that
//              it owns.
// Usage:       Included according to configuration by
//              <cyg/kernel/sched.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>

// -------------------------------------------------------------------------
// Customize the scheduler

#define CYGIMP_THREAD_PRIORITY  1       // Threads have changable priorities

#define CYG_THREAD_MIN_PRIORITY 1
#define CYG_THREAD_MAX_PRIORITY 0x7FFFFFFF

// set default scheduling info value for thread constructors.
#define CYG_SCHED_DEFAULT_INFO  CYG_THREAD_MAX_PRIORITY

#error Lottery Scheduler not yet complete, do not use!!!

// -------------------------------------------------------------------------
// Thread queue implementation.
// This class provides the (scheduler specific) implementation of the
// thread queue class.

class Cyg_ThreadQueue_Implementation
{
    friend class Cyg_Scheduler_Implementation;
    friend class Cyg_SchedThread_Implementation;
    
    Cyg_Thread *queue;

protected:

    // API used by Cyg_ThreadQueue

                                        // Add thread to queue
    void                enqueue(Cyg_Thread *thread);

                                        // return first thread on queue
    Cyg_Thread          *highpri();

                                        // remove first thread on queue    
    Cyg_Thread          *dequeue();

                                        // remove specified thread from queue    
    void                remove(Cyg_Thread *thread);

                                        // test if queue is empty
    cyg_bool            empty();

    void                rotate();       // Rotate the queue
};

inline cyg_bool Cyg_ThreadQueue_Implementation::empty()
{
    return queue == NULL;
}

// -------------------------------------------------------------------------
// This class contains the implementation details of the scheduler, and
// provides a standard API for accessing it.

class Cyg_Scheduler_Implementation
    : public Cyg_Scheduler_Base
{
    friend class Cyg_ThreadQueue_Implementation;
    friend class Cyg_SchedThread_Implementation;

    // All runnable threads are kept on a single run queue
    // in MRU order.
    Cyg_ThreadQueue_Implementation     run_queue;

    cyg_uint32  rand_seed;

    cyg_int32   total_tickets;
    
protected:

    Cyg_Scheduler_Implementation();     // Constructor
    
    // The following functions provide the scheduler implementation
    // interface to the Cyg_Scheduler class. These are protected
    // so that only the scheduler can call them.
    
    // choose a new thread
    Cyg_Thread  *schedule();

    // make thread schedulable
    void        add_thread(Cyg_Thread *thread);

    // make thread un-schedulable
    void        rem_thread(Cyg_Thread *thread);

    // register thread with scheduler
    void        register_thread(Cyg_Thread *thread);

    // deregister thread
    void        deregister_thread(Cyg_Thread *thread);
    
    // Test the given priority for uniqueness
    cyg_bool    unique( cyg_priority priority);

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE

    // If timeslicing is enbled, define a scheduler
    // entry point to do timeslicing. This will be
    // called from the RTC DSR.

protected:
    
    static cyg_count32         timeslice_count;
    
public:    
    void timeslice();

    static void reset_timeslice_count();
    
#endif
    
    
};

// -------------------------------------------------------------------------
// Cyg_Scheduler_Implementation inlines

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE

inline void Cyg_Scheduler_Implementation::reset_timeslice_count()
{
    timeslice_count = CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS;
}

#endif

// -------------------------------------------------------------------------
// Scheduler thread implementation.
// This class provides the implementation of the scheduler specific parts
// of each thread.

class Cyg_SchedThread_Implementation
{
    friend class Cyg_Scheduler_Implementation;
    friend class Cyg_ThreadQueue_Implementation;

    Cyg_Thread *next;                   // next thread in queue
    Cyg_Thread *prev;                   // previous thread in queue
        
    void insert( Cyg_Thread *thread );  // Insert thread in front of this

    void remove();                      // remove this from queue
    
protected:

    cyg_priority        priority;       // current thread priority == tickets held

    cyg_priority        compensation_tickets;   // sleep compensation
    
    Cyg_SchedThread_Implementation(CYG_ADDRWORD sched_info);

    void yield();                       // Yield CPU to next thread

};

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_LOTTERY_HXX
// EOF lottery.hxx
