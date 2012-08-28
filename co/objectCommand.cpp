
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "objectCommand.h"

#include "buffer.h"


namespace co
{

namespace detail
{

class ObjectCommand
{
public:
    ObjectCommand()
    {}

    ObjectCommand( const ObjectCommand& rhs )
        : objectID( rhs.objectID )
        , instanceID( rhs.instanceID )
    {}

    void operator=( const ObjectCommand& rhs )
    {
        objectID = rhs.objectID;
        instanceID = rhs.instanceID;
    }

    UUID objectID;
    uint32_t instanceID;
};

}

ObjectCommand::ObjectCommand( BufferPtr buffer )
    : Command( buffer )
    , _impl( new detail::ObjectCommand )
{
    if( buffer )
        *this >> _impl->objectID >> _impl->instanceID;
}

ObjectCommand::ObjectCommand( const ObjectCommand& rhs )
    : Command( rhs )
    , _impl( new detail::ObjectCommand( *rhs._impl ))
{
    if( getBuffer( ))
        *this >> _impl->objectID >> _impl->instanceID;
}

ObjectCommand& ObjectCommand::operator = ( const ObjectCommand& rhs )
{
    if( this != &rhs )
        *_impl = *rhs._impl;
    return *this;
}

ObjectCommand::~ObjectCommand()
{
    delete _impl;
}

const UUID& ObjectCommand::getObjectID() const
{
    return _impl->objectID;
}

uint32_t ObjectCommand::getInstanceID() const
{
    return _impl->instanceID;
}

}