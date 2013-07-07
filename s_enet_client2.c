#include <stdlib.h>
#include <stdio.h>

#include "enet/enet.h"
#include "s_enet_internal.h"
#include "s_enet.h"

struct client2_t {
    ENetHost* e_host;
    ENetEvent e_event;

    //size_t peers;

    int reliable;
    int send_immediatly;

    char* peer_ip;

    SECln2OnConnect on_connect;
    SECln2OnRecv on_recv;
    SECln2OnDisconnect on_disconnect;

    void* user_data;
};


client2_t*  SENET_API   SECln2_Create (const size_t max_peers, const size_t channels, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth)
{
    client2_t* cln = malloc(sizeof(client2_t));
    if (cln == NULL) {
        return NULL;
    }

    cln->e_host = enet_host_create(NULL, max_peers, channels, max_in_bandwidth, max_out_bandwidth);
    if (cln->e_host == NULL) {
        free(cln);
        return NULL;
    }

    //cln->peers = 0;

    cln->reliable = 1;
    cln->send_immediatly = 0;

    cln->on_connect = NULL;
    cln->on_recv = NULL;
    cln->on_disconnect = NULL;

    cln->user_data = NULL;

    return cln;
}

void        SENET_API   SECln2_Destroy (client2_t* cln)
{
    enet_host_destroy(cln->e_host);
    free(cln);
}


peer_t*     SENET_API   SECln2_Connect (client2_t* cln, const char* host, const uint16 port, const uint32 connect_data)
{
    ENetAddress addr;
    enet_address_set_host(&addr, host);
    addr.port = port;

    ENetPeer* peer = enet_host_connect(cln->e_host, &addr, cln->e_host->channelLimit, connect_data);
    return (peer_t*)peer;
}

void        SENET_API   SECln2_PeerDisconnect (client2_t* cln, peer_t* peer, const int force, const uint32 disconnect_data)
{
    if (force != 0) {
        enet_peer_reset((ENetPeer*)peer);
    } else {
        if (cln->send_immediatly != 0) {
            enet_peer_disconnect((ENetPeer*)peer, disconnect_data);
            enet_host_flush(cln->e_host);
        } else {
            enet_peer_disconnect_later((ENetPeer*)peer, disconnect_data);
        }
    }
}

void        SENET_API   SECln2_PeersDisconnectAll (client2_t* cln, const int force, const uint32 disconnect_data)
{
    int i = 0;
    if (force != 0) {
        for (i = 0; i < cln->e_host->peerCount; i++) {
            enet_peer_reset(&cln->e_host->peers[i]);
        }
    } else {
        if (cln->send_immediatly != 0) {
            for (i = 0; i < cln->e_host->peerCount; i++) {
                enet_peer_disconnect(&cln->e_host->peers[i], disconnect_data);
            }
            enet_host_flush(cln->e_host);
        } else {
            for (i = 0; i < cln->e_host->peerCount; i++) {
                enet_peer_disconnect_later(&cln->e_host->peers[i], disconnect_data);
            }
        }
    }
}

int         SENET_API   SECln2_PeerIsConnected (client2_t* cln, peer_t* peer)
{
    if (((ENetPeer*)peer)->state == ENET_PEER_STATE_CONNECTED) { return 1; }
    return 0;
}


void        SENET_API   SECln2_SetLimits (client2_t* cln, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth)
{
    enet_host_bandwidth_limit(cln->e_host, max_in_bandwidth, max_out_bandwidth);
}

void        SENET_API   SECln2_SetReliability (client2_t* cln, const int reliable)
{
    cln->reliable = reliable;
}

void        SENET_API   SECln2_SetSendImmediatly (client2_t* cln, const int send_immediatly)
{
    cln->send_immediatly = send_immediatly;
}


void        SENET_API   SECln2_SetCallbacks (client2_t* cln, SECln2OnConnect on_connect_proc, SECln2OnDisconnect on_disconnect_proc, SECln2OnRecv on_recv_proc)
{
    cln->on_connect = on_connect_proc;
    cln->on_recv = on_recv_proc;
    cln->on_disconnect = on_disconnect_proc;
}


void        SENET_API   SECln2_SetUserData (client2_t* cln, void* user_data)
{
    cln->user_data = user_data;
}

void*       SENET_API   SECln2_GetUserData (client2_t* cln)
{
    return cln->user_data;
}


size_t      SENET_API   SECln2_PeersCount (client2_t* cln)
{
    //return cln->peers;
    return cln->e_host->connectedPeers;
}

size_t      SENET_API   SECln2_PeersGetAll (client2_t* cln, peer_t* peers[], size_t array_size)
{
    //if (cln->peers == 0) { return 0; }
    if (cln->e_host->connectedPeers == 0) { return 0; }

    int i = 0, added = 0;

    do {
        if (cln->e_host->peers[i].state == ENET_PEER_STATE_CONNECTED) {
            peers[added] = (peer_t*)&cln->e_host->peers[i];
            added++;
        }
        i++;
    } while (i < cln->e_host->peerCount && added < array_size);

    return added;
}

void        SENET_API   SECln2_PeersIterate (client2_t* cln, SECln2IterateProc proc, void* user_data)
{
    //if (cln->peers == 0) { return; }
    if (cln->e_host->connectedPeers == 0) { return; }

    int i = 0;
    for (i = 0; i < cln->e_host->peerCount; i++) {
        if (cln->e_host->peers[i].state == ENET_PEER_STATE_CONNECTED) {
            proc(cln, (peer_t*)&cln->e_host->peers[i], user_data);
        }
    }
}


char*       SENET_API   SECln2_PeerGetHostIP (client2_t* cln, peer_t* peer)
{
    if (cln->peer_ip == NULL) { cln->peer_ip = malloc(40 * sizeof(char)); }
    if (cln->peer_ip == NULL) { return NULL; }

    enet_address_get_host_ip(&((ENetPeer*)peer)->address, cln->peer_ip, 40);
    return cln->peer_ip;
}

uint16      SENET_API   SECln2_PeerGetPort (client2_t* cln, peer_t* peer)
{
    return ((ENetPeer*)peer)->address.port;
}


void        SENET_API   SECln2_PeerSetUserData (client2_t* cln, peer_t* peer, void* user_data)
{
    ((ENetPeer*)peer)->data = user_data;
}

void*       SENET_API   SECln2_PeerGetUserData (client2_t* cln, peer_t* peer)
{
    return ((ENetPeer*)peer)->data;
}


int         SENET_API   SECln2_Send (client2_t* cln, peer_t* peer, const size_t channel, const uint8* data, const size_t data_len, const int alloc)
{
    unsigned int flags = 0;
    if (cln->reliable != 0) {
        flags += ENET_PACKET_FLAG_RELIABLE;
    }
    if (alloc == 0) {
        flags += ENET_PACKET_FLAG_NO_ALLOCATE;
    }

    ENetPacket* packet = enet_packet_create(data, data_len, flags);
    if (packet == NULL) {
        return -1;
    }

    int ret = enet_peer_send((ENetPeer*)peer, channel, packet);

    if (cln->send_immediatly != 0) {
        enet_host_flush(cln->e_host);
    }

    if (ret == 0) { return 1; }
    return 0;
}

void        SENET_API   SECln2_Broadcast (client2_t* cln, const size_t channel, const uint8* data, const size_t data_len, const int alloc)
{
    //if (cln->peers == 0) { return; }
    if (cln->e_host->connectedPeers == 0) { return; }

    unsigned int flags = 0;
    if (cln->reliable != 0) {
        flags += ENET_PACKET_FLAG_RELIABLE;
    }
    if (alloc == 0) {
        flags += ENET_PACKET_FLAG_NO_ALLOCATE;
    }

    ENetPacket* packet = enet_packet_create(data, data_len, flags);
    if (packet == NULL) {
        return;
    }

    enet_host_broadcast(cln->e_host, channel, packet);

    if (cln->send_immediatly != 0) {
        enet_host_flush(cln->e_host);
    }
}


void        SENET_API   SECln2_Process (client2_t* cln, const uint32 wait_time)
{
    if (enet_host_service(cln->e_host, &cln->e_event, wait_time) > 0) {
        switch (cln->e_event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                #ifdef SENET_DEBUG
                debug("ENET_EVENT_TYPE_RECEIVE\n");
                #endif
                if (cln->on_recv != NULL) {
                    cln->on_recv(cln, (peer_t*)cln->e_event.peer, cln->e_event.channelID, cln->e_event.packet->data, cln->e_event.packet->dataLength);
                }
                enet_packet_destroy(cln->e_event.packet);
            break;
            case ENET_EVENT_TYPE_DISCONNECT:
                #ifdef SENET_DEBUG
                debug("ENET_EVENT_TYPE_DISCONNECT\n");
                #endif
                if (cln->on_disconnect != NULL) {
                    cln->on_disconnect(cln, (peer_t*)cln->e_event.peer, cln->e_event.data);
                }
                enet_peer_reset(cln->e_event.peer);
                //cln->peers--;
            break;
            case ENET_EVENT_TYPE_CONNECT:
                #ifdef SENET_DEBUG
                debug("ENET_EVENT_TYPE_CONNECT\n");
                #endif
                //cln->peers++;
                if (cln->on_connect != NULL) {
                    cln->on_connect(cln, (peer_t*)cln->e_event.peer, cln->e_event.data);
                }
            break;
            case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}

