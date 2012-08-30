
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

#include "objectOCommand.h"


namespace co
{

namespace detail
{

class ObjectOCommand
{
};

}

ObjectOCommand::ObjectOCommand( const Connections& receivers,
                                const uint32_t cmd, const uint32_t type,
                                const UUID& id, const uint32_t instanceID )
    : NodeOCommand( receivers, cmd, type )
    , _impl( new detail::ObjectOCommand )
{
    *this << id << instanceID;
}

ObjectOCommand::~ObjectOCommand()
{
    delete _impl;
}

size_t ObjectOCommand::getSize()
{
    return NodeOCommand::getSize() + sizeof( UUID ) + sizeof( uint32_t );
}

}
