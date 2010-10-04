
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_OBJECTDATAOSTREAM_H
#define EQNET_OBJECTDATAOSTREAM_H

#include <eq/net/dataOStream.h>   // base class
#include <eq/net/version.h>       // enum

namespace eq
{
namespace net
{
    class Object;

    /**
     * The DataOStream for object data.
     */
    class ObjectDataOStream : public DataOStream
    {
    public:
        ObjectDataOStream( const ObjectCM* cm )
                : _cm( cm ), _version( VERSION_NONE )
                , _sequence( 0 ) {}

        virtual ~ObjectDataOStream(){}
 
        void setVersion( const uint32_t version ) { _version = version; }
        uint32_t getVersion() const { return _version; }
        virtual void reset() { DataOStream::reset(); _sequence = 0; }

    protected:
        const ObjectCM* _cm;
        uint32_t _version;
        uint32_t _sequence;
    };
}
}
#endif //EQNET_OBJECTDATAOSTREAM_H
