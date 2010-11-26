
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQBASE_THREAD_H
#define EQBASE_THREAD_H

#include <eq/base/base.h>     // EQBASE_API definition
#include <eq/base/debug.h>    // debug macros in thread-safety checks
#include <eq/base/lock.h>     // member
#include <eq/base/monitor.h>  // member
#include <eq/base/threadID.h> // member

#include <vector>
#include <typeinfo>
#include <sstream>

namespace eq
{
namespace base
{
    class ExecutionListener;

    /** An utility class to execute code in a separate execution thread. */
    class Thread 
    {
    public:
        /** Construct a new thread. @version 1.0 */
        EQBASE_API Thread();

        /** Destruct the thread. @version 1.0 */
        EQBASE_API virtual ~Thread();

        /** 
         * Start the thread.
         *
         * All thread state listeners will be notified from the new thread,
         * after the thread was initialized successfully.
         * 
         * @return <code>true</code> if the thread was launched and initialized
         *         successfully, <code>false</code> otherwise.
         * @sa init(), run(), addListener()
         * @version 1.0
         */
        EQBASE_API bool start();

        /** 
         * The init function for the child thread.
         *
         * The parent thread will not be unlocked before this function has been
         * executed. If the thread initialization fails, that is, this method
         * does return false, the thread will be stopped and the start() method
         * will return false.
         * 
         * @return the success value of the thread initialization.
         * @version 1.0
         */
        virtual bool init(){ return true; }

        /** 
         * The entry function for the child thread.
         *
         * This method should contain the main execution routine for the thread
         * and is called after a successful init().
         * 
         * @version 1.0
         */
        virtual void run() = 0;

        /** 
         * Exit the child thread immediately.
         * 
         * This function does not return. It is only to be called from the child
         * thread. The thread listeners will be notified.
         *
         * @version 1.0
         */
        EQBASE_API virtual void exit();

        /** 
         * Cancel (stop) the child thread.
         *
         * This function is not to be called from the child thread.
         * @version 1.0
         */
        EQBASE_API void cancel();

        /** 
         * Wait for the exit of the child thread.
         *
         * @return true if the thread was joined, false otherwise.
         * @version 1.0
         */
        EQBASE_API bool join();

        /** 
         * Return if the thread is stopped.
         * 
         * Note that the thread may be neither running nor stopped if it is
         * currently starting or stopping.
         *
         * @return true if the thread is stopped, false if not.
         * @version 1.0
         */
        bool isStopped() const { return ( _state == STATE_STOPPED ); }

        /** 
         * Return if the thread is running.
         * 
         * Note that the thread may be neither running nor stopped if it is
         * currently starting or stopping.
         *
         * @return true if the thread is running, false if not.
         * @version 1.0
         */
        bool isRunning() const { return ( _state == STATE_RUNNING ); }

        /** 
         * Returns if this thread object is the current (calling) thread.
         * 
         * @return true if the current thread has is the same thread as this
         *         thread, false if not.
         * @version 1.0
         */
        EQBASE_API bool isCurrent() const;

        /** 
         * Add a new thread state listener.
         * 
         * @param listener the listener.
         * @version 1.0
         */
        EQBASE_API static void addListener( ExecutionListener* listener );

        /** 
         * Remove a thread state listener.
         * 
         * @param listener the listener.
         * @version 1.0
         */
        EQBASE_API static bool removeListener( ExecutionListener* listener );

        /** Remove all registered listeners, used at exit. @version 1.0 */
        EQBASE_API static void removeAllListeners();

        /** @return a unique identifier for the calling thread. @version 1.0 */
        EQBASE_API static ThreadID getSelfThreadID();

        /** @internal */
        EQBASE_API static void yield();

        /** @internal */
        static void pinCurrentThread();

        /** @internal */
        EQBASE_API static void setDebugName( const std::string& name );

    private:
        ThreadID _id;

        /** The current state of this thread. */
        enum State
        {
            STATE_STOPPED,
            STATE_STARTING, // start() in progress
            STATE_RUNNING,
            STATE_STOPPING  // child no longer active, join() not yet called
        };

        Monitor< State > _state;

        static void* runChild( void* arg );
        void        _runChild();

        void _installCleanupHandler();

        static void _notifyStarted();
        static void _notifyStopping();
        friend void _notifyStopping( void* ); //!< @internal

        friend std::ostream& operator << ( std::ostream& os, const Thread* );
    };

    /** Print the thread to the given output stream. */
    std::ostream& operator << ( std::ostream& os, const Thread* thread );

// thread-safety checks
// These checks are for development purposes, to check that certain objects are
// properly used within the framework. Leaving them enabled during application
// development may cause false positives, e.g., when threadsafety is ensured
// outside of the objects by the application.

#ifndef NDEBUG
#  define EQ_CHECK_THREADSAFETY
#endif

/** Declare a thread id variable to be used for thread-safety checks. */
#define EQ_TS_VAR( NAME )                        \
    struct NAME ## Struct                                   \
    {                                                       \
        NAME ## Struct ()                                   \
            : extMutex( false ), inRegion( false )          \
            {}                                              \
        mutable eq::base::ThreadID id;                      \
        bool extMutex;                                      \
        mutable bool inRegion;                              \
    } NAME;                                                 \

#ifdef EQ_CHECK_THREADSAFETY
#  define EQ_TS_THREAD( NAME )                                          \
    {                                                                   \
        if( NAME.id == eq::base::ThreadID::ZERO )                       \
        {                                                               \
            NAME.id = eq::base::Thread::getSelfThreadID();              \
            EQVERB << "Functions for " << #NAME                         \
                   << " locked to this thread" << std::endl;            \
        }                                                               \
        if( !NAME.extMutex && NAME.id != eq::base::Thread::getSelfThreadID( )) \
        {                                                               \
            EQERROR << "Threadsafety check for " << #NAME               \
                    << " failed on object of type "                     \
                    << eq::base::className( this ) << ", thread "       \
                    << eq::base::Thread::getSelfThreadID() << " != "    \
                    << NAME.id << std::endl;                            \
            EQABORT( "Non-threadsave code called from two threads" );   \
        }                                                               \
    }

#  define EQ_TS_NOT_THREAD( NAME )                                      \
    {                                                                   \
        if( !NAME.extMutex && NAME.id != eq::base::ThreadID::ZERO )     \
        {                                                               \
            if( NAME.id ==  eq::base::Thread::getSelfThreadID( ))       \
            {                                                           \
                EQERROR << "Threadsafety check for not " << #NAME       \
                        << " failed on object of type "                 \
                        << eq::base::className( this ) << std::endl;    \
                EQABORT( "Code called from wrong thread" );             \
            }                                                           \
        }                                                               \
    }

    /** @internal */
    template< typename T > class ScopedThreadCheck : public NonCopyable
    {
    public:
        explicit ScopedThreadCheck( const T& data )
                : _data( data )
            {
                EQASSERTINFO( !data.inRegion,
                              "Another thread already in critical region" );
                data.inRegion = true;
            }
        ~ScopedThreadCheck() 
            {
                EQASSERTINFO( _data.inRegion,
                              "Another thread was in critical region" );
                _data.inRegion = false;
            }
    private:
        const T& _data;
    };

# define EQ_TS_SCOPED( NAME ) \
    eq::base::ScopedThreadCheck< NAME ## Struct > scoped ## NAME ## Check(NAME);

#else
#  define EQ_TS_THREAD( NAME ) {}
#  define EQ_TS_NOT_THREAD( NAME ) {}
#  define EQ_TS_SCOPED( NAME ) {}
#endif

}
}
#endif //EQBASE_THREAD_H
