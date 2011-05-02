//
// Urho3D Engine
// Copyright (c) 2008-2011 Lasse ��rni
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "Precompiled.h"
#include "Log.h"
#include "NetworkEvents.h"
#include "Peer.h"
#include "StringUtils.h"

#include <enet/enet.h>

#include "DebugNew.h"

static const int MAX_IPADDRESS = 32;

OBJECTTYPESTATIC(Peer);

Peer::Peer(Context* context, ENetPeer* peer, PeerType type) :
    Object(context),
    peer_(peer),
    type_(type),
    connectionState_(CS_CONNECTING),
    port_(0),
    simulatedPacketLoss_(0.0f),
    simulatedLatency_(0),
    halfSimulatedLatency_(0),
    userData_(0)
{
}

Peer::~Peer()
{
    FlushPackets();
    OnDisconnect();
}

void Peer::Send(const VectorBuffer& packet, unsigned char channel, bool reliable, bool inOrder)
{
    if ((!peer_) || (!packet.GetSize()))
        return;
    
    if ((simulatedPacketLoss_ > 0.0f) && (!reliable))
    {
        if (Random() < simulatedPacketLoss_)
            return;
    }
    
    ENetPacket* enetPacket = enet_packet_create(packet.GetData(), packet.GetSize(), reliable ? ENET_PACKET_FLAG_RELIABLE :
        (inOrder ? 0 : ENET_PACKET_FLAG_UNSEQUENCED));
    
    if (!simulatedLatency_)
        enet_peer_send(peer_, channel, enetPacket);
    else
    {
        QueuedPacket packet;
        packet.packet_ = enetPacket;
        packet.channel_ = channel;
        sentPackets_.push_back(packet);
    }
}

void Peer::Send(const void* data, unsigned size, unsigned char channel, bool reliable, bool inOrder)
{
    if ((!peer_) || (!data) || (!size))
        return;
    
    if ((simulatedPacketLoss_ != 0.0f) && (!reliable))
    {
        if (Random() < simulatedPacketLoss_)
            return;
    }
    
    ENetPacket* enetPacket = enet_packet_create(data, size, reliable ? ENET_PACKET_FLAG_RELIABLE : (inOrder ? 0 :
        ENET_PACKET_FLAG_UNSEQUENCED));
    
    if (!simulatedLatency_)
        enet_peer_send(peer_, channel, enetPacket);
    else
    {
        QueuedPacket packet;
        packet.packet_ = enetPacket;
        packet.channel_ = channel;
        sentPackets_.push_back(packet);
    }
}

bool Peer::Receive(VectorBuffer& packet, unsigned char channel)
{
    bool received = false;
    bool reliable = false;
    
    if (channel == CHANNEL_ANY)
    {
        for (std::map<unsigned char, std::list<QueuedPacket> >::iterator i = packets_.begin(); i != packets_.end(); ++i)
        {
            std::list<QueuedPacket>& packetList = i->second;
            if ((!packetList.empty()) && (packetList.front().timer_.GetMSec(false) >= halfSimulatedLatency_))
            {
                ENetPacket* enetPacket = packetList.front().packet_;
                reliable = (enetPacket->flags & ENET_PACKET_FLAG_RELIABLE) != 0;
                packetList.pop_front();
                
                packet.SetData(enetPacket->data, enetPacket->dataLength);
                enet_packet_destroy(enetPacket);
                received = true;
                break;
            }
        }
    }
    else
    {
        std::list<QueuedPacket>& packetList = packets_[channel];
        if ((!packetList.empty()) && (packetList.front().timer_.GetMSec(false) >= halfSimulatedLatency_))
        {
            ENetPacket* enetPacket = packetList.front().packet_;
            reliable = (enetPacket->flags & ENET_PACKET_FLAG_RELIABLE) != 0;
            packetList.pop_front();
            
            packet.SetData(enetPacket->data, enetPacket->dataLength);
            enet_packet_destroy(enetPacket);
            received = true;
        }
    }
    
    if ((received) && (!reliable) && (simulatedPacketLoss_ > 0.0f))
    {
        if (Random() < simulatedPacketLoss_)
        {
            packet.Clear();
            received = false;
        }
    }
    
    return received;
}

void Peer::Update()
{
    // Check send timer of packets with simulated latency and send as necessary
    for (std::list<QueuedPacket>::iterator i = sentPackets_.begin(); i != sentPackets_.end();)
    {
        if (i->timer_.GetMSec(false) >= halfSimulatedLatency_)
        {
            enet_peer_send(peer_, i->channel_, i->packet_);
            i = sentPackets_.erase(i);
        }
        else
            ++i;
    }
}

void Peer::FlushPackets()
{
    for (std::map<unsigned char, std::list<QueuedPacket> >::iterator i = packets_.begin(); i != packets_.end(); ++i)
    {
        std::list<QueuedPacket>& packetList = i->second;
        while (!packetList.empty())
        {
            ENetPacket* enetPacket = packetList.front().packet_;
            packetList.pop_front();
            enet_packet_destroy(enetPacket);
        }
    }
    while (!sentPackets_.empty())
    {
        ENetPacket* enetPacket = sentPackets_.front().packet_;
        sentPackets_.pop_front();
        enet_packet_destroy(enetPacket);
    }
}

void Peer::Disconnect()
{
    if ((peer_) && (connectionState_ > CS_DISCONNECTING))
    {
        connectionState_ = CS_DISCONNECTING;
        enet_peer_disconnect(peer_, 0);
    }
}

void Peer::ForceDisconnect()
{
    OnDisconnect();
}

void Peer::SetSimulatedPacketLoss(float loss)
{
    simulatedPacketLoss_ = Clamp(loss, 0.0f, 1.0f);
}

void Peer::SetSimulatedLatency(unsigned latency)
{
    simulatedLatency_ = latency;
    halfSimulatedLatency_ = latency >> 1;
}

void Peer::OnConnect()
{
    if (peer_)
    {
        char ipBuffer[MAX_IPADDRESS];
        enet_address_get_host_ip(&peer_->address, ipBuffer, MAX_IPADDRESS);
        address_ = std::string(ipBuffer);
        port_ = peer_->address.port;
        connectionState_ = CS_CONNECTED;
        
        // Send event
        using namespace PeerConnected;
        
        VariantMap eventData;
        eventData[P_PEER] = (void*)this;
        SendEvent(E_PEERCONNECTED, eventData);
        
        LOGINFO(address_ + ":" + ToString(port_) + " connected");
    }
}

void Peer::OnDisconnect()
{
    if (peer_)
    {
        enet_peer_reset(peer_);
        connectionState_ = CS_DISCONNECTED;
        peer_ = 0;
        
        // Send event
        using namespace PeerDisconnected;
        
        VariantMap eventData;
        eventData[P_PEER] = (void*)this;
        SendEvent(E_PEERDISCONNECTED, eventData);
        
        if (!address_.empty())
            LOGINFO(address_ + ":" + ToString(port_) + " disconnected");
        else
            LOGINFO("Disconnected");
    }
}
