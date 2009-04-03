
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQBASE_LOG_H
#define EQBASE_LOG_H

#include <eq/base/base.h>
#include <eq/base/clock.h>

#include <assert.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <time.h>

#ifdef WIN32_API
#  include <process.h>
#  define getpid _getpid
#endif

namespace eq
{
namespace base
{
    /** The logging levels. */
    enum LogLevel
    {
        LOG_ERROR = 1,
        LOG_WARN,
        LOG_INFO,
        LOG_VERB,
        LOG_ALL
    };

    /** The logging topics. */
    enum LogTopic
    {
        LOG_CUSTOM = 0x10,       // 16
        LOG_ANY    = 0xfffu      // Does not include user-level events.
    };

    /** The string buffer used for logging. */
    class LogBuffer : public std::streambuf
    {
    public:
        LogBuffer( std::ostream& stream )
                : _line(0), _indent(0), _blocked(0), _noHeader(0), 
                  _newLine(true), _stream(stream)
            {}
        
        void indent() { ++_indent; }
        void exdent() { --_indent; }

        void disableFlush() { ++_blocked; } // use counted variable to allow
        void enableFlush()                  //   nested enable/disable calls
            { 
                assert( _blocked && "Too many enableFlush on log stream" );
                --_blocked;
                if( !_blocked ) 
                    pubsync();
            }

        void disableHeader() { ++_noHeader; } // use counted variable to allow
        void enableHeader()  { --_noHeader; } //   nested enable/disable calls

#ifdef WIN32
        void setLogInfo( const char* subdir, const char* file, const int line )
            { _file = file; _line = line; } // SUBDIR not needed on WIN32
#else
        void setLogInfo( const char* subdir, const char* file, const int line )
            { _file = std::string( subdir ) + '/' + file; _line = line; }
#endif

    protected:
        virtual int_type overflow( LogBuffer::int_type c );
        
        virtual int sync() 
            {
                if( !_blocked )
                {
                    const std::string& string = _stringStream.str();
                    _stream.write( string.c_str(), string.length( ));
                    _stringStream.str( "" );
                }
                _newLine = true;
                return 0;
            }

    private:
        LogBuffer( const LogBuffer& );
        LogBuffer& operator = ( const LogBuffer& );

        /** The current file logging. */
        std::string _file;

        /** The current line logging. */
        int _line;

        /** Clock for time stamps */
        static Clock _clock;

        /** The current indentation level. */
        int _indent;

        /** Flush reference counter. */
        int _blocked;

        /** The header disable counter. */
        int _noHeader;

        /** The flag that a new line has started. */
        bool _newLine;

        /** The temporary buffer. */
        std::ostringstream _stringStream;

        /** The wrapped ostream. */
        std::ostream& _stream;
    };

    /** The logging class */
    class Log : public std::ostream
    {
    public:

        Log() : std::ostream( &_logBuffer ), _logBuffer( getOutput( )){}
        virtual ~Log() { _logBuffer.pubsync(); }

        void indent() { _logBuffer.indent(); }
        void exdent() { _logBuffer.exdent(); }
        void disableFlush() { _logBuffer.disableFlush(); }
        void enableFlush()  { _logBuffer.enableFlush();  }
        void forceFlush()  { _logBuffer.pubsync();  }
        void disableHeader() { _logBuffer.disableHeader(); }
        void enableHeader()  { _logBuffer.enableHeader();  }

        /** The current log level. */
        static EQ_EXPORT int level;

        /** The current log topics. */
        static EQ_EXPORT unsigned topics;

        /** The per-thread logger. */
        static EQ_EXPORT Log& instance( const char* subdir, const char* file,
                                        const int line );

        /** Exit the log instance for the current thread. */
        static EQ_EXPORT void exit();

        /** The string representation of the current log level. */
        static std::string& getLogLevelString();

        /** Change the output stream */
        static EQ_EXPORT void setOutput( std::ostream& stream );

        void notifyPerThreadDelete() { delete this; }

        /** Get the current output stream */
        static std::ostream& getOutput ();

    private:
        LogBuffer _logBuffer; 

        Log( const Log& );
        Log& operator = ( const Log& );

        void setLogInfo( const char* subdir, const char* file, const int line )
            { _logBuffer.setLogInfo( subdir, file, line ); }

    };

    /** The ostream indent manipulator. */
    EQ_EXPORT std::ostream& indent( std::ostream& os );
    /** The ostream exdent manipulator. */
    EQ_EXPORT std::ostream& exdent( std::ostream& os );
    /** The ostream flush disable manipulator. */
    EQ_EXPORT std::ostream& disableFlush( std::ostream& os );
    /** The ostream flush enable manipulator. */
    EQ_EXPORT std::ostream& enableFlush( std::ostream& os );
    /** The ostream forced flush manipulator. */
    EQ_EXPORT std::ostream& forceFlush( std::ostream& os );
    /** The ostream header disable manipulator. */
    EQ_EXPORT std::ostream& disableHeader( std::ostream& os );
    /** The ostream header enable manipulator. */
    EQ_EXPORT std::ostream& enableHeader( std::ostream& os );

#ifdef WIN32
    inline std::string getErrorString( const DWORD error )
    {
        char text[512] = "";
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, text, 511, 0 );
        const size_t length = strlen( text );
        if( length>2 && text[length-2] == '\r' )
            text[length-2] = '\0';
        return std::string( text );
    }
    inline std::string getLastErrorString()
    {
         return getErrorString( GetLastError( ));
    }
#endif
}
}

#ifndef SUBDIR
#  define SUBDIR ""
#endif

#define EQERROR (eq::base::Log::level >= eq::base::LOG_ERROR) &&    \
    eq::base::Log::instance( SUBDIR, __FILE__, __LINE__ )
#define EQWARN  (eq::base::Log::level >= eq::base::LOG_WARN)  &&    \
    eq::base::Log::instance( SUBDIR, __FILE__, __LINE__ )
#define EQINFO  (eq::base::Log::level >= eq::base::LOG_INFO)  &&    \
    eq::base::Log::instance( SUBDIR, __FILE__, __LINE__ )
#define EQVERB  (eq::base::Log::level >= eq::base::LOG_VERB)  &&    \
    eq::base::Log::instance( SUBDIR, __FILE__, __LINE__ )
#define EQLOG(topic)  (eq::base::Log::topics & (topic))  &&  \
    eq::base::Log::instance( SUBDIR, __FILE__, __LINE__ )

#endif //EQBASE_LOG_H
