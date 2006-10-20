
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include "connection.h"
#include "packets.h"
#include "session.h"

using namespace eqNet;
using namespace eqBase;
using namespace std;

void Barrier::_construct()
{
    registerCommand( CMD_BARRIER_ENTER, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Barrier::_cmdEnter ));
    registerCommand( CMD_BARRIER_ENTER_REPLY, this, 
                reinterpret_cast<CommandFcn>(&eqNet::Barrier::_cmdEnterReply ));
}

Barrier::Barrier( eqBase::RefPtr<Node> master, const uint32_t height )
        : Object( TYPE_BARRIER, CMD_BARRIER_ALL ),
          _master( master )
{
    _data.master = master->getNodeID();
    _data.height = height;
    _construct();

    EQINFO << "New barrier of height " << _data.height << endl;
}

Barrier::Barrier( const void* instanceData )
        : Object( TYPE_BARRIER, CMD_BARRIER_ALL ),
          _data( *(Data*)instanceData ) 
{
    _construct();
    EQINFO << "Barrier of height " << _data.height << " instanciated" << endl;
}

void Barrier::init( const void* data, const uint64_t dataSize )
{
    _master = eqNet::Node::getLocalNode()->connect( _data.master, 
                                                    getSession()->getServer( ));
}

void Barrier::enter()
{
    EQASSERT( _data.height > 1 );
    EQASSERT( _master.isValid( ));
    EQVERB << "enter barrier of height " << _data.height << endl;
    EQASSERT( getSession( ));

    BarrierEnterPacket packet;
    packet.version     = getVersion();
    packet.requestorID = getInstanceID();
    send( _master, packet );
    
    _leaveNotify.wait();
    EQVERB << "slave left barrier of height " << _data.height << endl;
}

CommandResult Barrier::_cmdEnter( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _thread );
    EQASSERT( _master == eqNet::Node::getLocalNode( ));

    BarrierEnterPacket* packet = (BarrierEnterPacket*)pkg;
    EQVERB << "Handle barrier enter " << packet << endl;
    if( packet->version > getVersion( ))
        return COMMAND_REDISPATCH;
    
    EQASSERT( packet->version == getVersion( ));

    _enteredBarriers.push_back( EnteredBarrier( node, packet->requestorID ));

    if( _enteredBarriers.size() < _data.height )
        return COMMAND_DISCARD;

    EQASSERT( _enteredBarriers.size() == _data.height );
    EQVERB << "Barrier reached" << endl;

    BarrierEnterReplyPacket reply;
    reply.sessionID  = getSession()->getID();
    reply.objectID   = getID();

    for( vector<EnteredBarrier>::iterator iter = _enteredBarriers.begin();
         iter != _enteredBarriers.end(); ++iter )
    {
        reply.instanceID = iter->instanceID;
        if( iter->node->isLocal( ))
            _leaveNotify.post(); // OPT
        else
            iter->node->send( reply );
    }

    _enteredBarriers.clear();
    return COMMAND_DISCARD;
}

CommandResult Barrier::_cmdEnterReply( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _thread );
    _leaveNotify.post();
    return COMMAND_HANDLED;
}
