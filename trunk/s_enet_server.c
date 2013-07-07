#include <stdlib.h>
#include <stdio.h>

#include "enet/enet.h"
#include "s_enet_internal.h"
#include "s_enet.h"

struct server_t {
    ENetAddress e_addr;
    ENetHost* e_host;
    ENetEvent e_event;

    size_t max_peers, channels;
    uint32 max_in, max_out;

    //size_t peers;

    char* peer_ip;

    int reliable;
    int send_immediatly;

    SESrvOnConnect on_connect;
    SESrvOnDisconnect on_disconnect;
    SESrvOnRecv on_recv;

    void* user_data;
};


/** \brief Create a server
 *
 * The server will not yet accept incoming connections until you start it (SESrv_Start())
 *
 * \param port[in] Listening port
 * \param max_peers[in] Maximum peers that the server will accept (the maximum is 4095)
 * \param channels[in] Channels count (the maximum is 255)
 * \param max_in_bandwidth[in] Maximum downstream bandwidth (bytes/s) - 0 means unlimited
 * \param max_out_bandwidth[in] Maximum upstream bandwidth (bytes/s) - 0 means unlimited
 *
 * \return Server pointer, or 0 if failure
 *
 */
server_t*   SENET_API   SESrv_Create (const uint16 port, const size_t max_peers, const size_t channels, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth)
{
    server_t* srv = malloc(sizeof(server_t));
    if (srv == NULL) {
        return NULL;
    }

    srv->e_addr.host = ENET_HOST_ANY;
    srv->e_addr.port = port;

    srv->e_host = NULL;

    srv->max_peers = max_peers;
    srv->channels = channels;
    srv->max_in = max_in_bandwidth;
    srv->max_out = max_out_bandwidth;

    //srv->peers = 0;

    srv->peer_ip = NULL;

    srv->reliable = 1;
    srv->send_immediatly = 0;

    srv->on_connect = NULL;
    srv->on_disconnect = NULL;
    srv->on_recv = NULL;

    srv->user_data = NULL;

    return srv;
}

/** \brief Destroy a server
 *
 * This function doesn't send disconnect notification to connected peers, nor calls
 * the user callbacks to notify of disconnection. To do so, you should call SESrv_DisconnectAll()
 * or SESrv_Stop() and wait for the disconnect notifications until SESrv_PeersCount() returns 0
 *
 * \param srv[in] Server pointer
 *
 */
void        SENET_API   SESrv_Destroy (server_t* srv)
{
    if (srv->e_host != NULL) { enet_host_destroy(srv->e_host); }
    if (srv->peer_ip != NULL) { free(srv->peer_ip); }
    free(srv);
}

/** \brief Start a server so it will accept incoming connections
 *
 * \param srv[in] Server pointer
 *
 * \return 1 on success, 0 on failure or if the server is already started
 *
 */
int         SENET_API   SESrv_Start (server_t* srv)
{
    if (srv->e_host != NULL) { return 0; }

    //srv->peers = 0;

    srv->e_host = enet_host_create(&srv->e_addr, srv->max_peers, srv->channels, srv->max_in, srv->max_out);
    if (srv->e_host == NULL) {
        return 0;
    }
    return 1;
}

/** \brief Stop a server
 *
 * SESrv_DisconnectAll() is also called in this function
 *
 * \param srv[in] Server pointer
 * \param force[in] If 1, the peers will be disconnected without notifying the foreign host, and no callback will be called.
 * \param disconnect_data[in] Data describing the disconnection
 *
 */
void        SENET_API   SESrv_Stop (server_t* srv, const int force, const uint32 disconnect_data)
{
    if (srv->e_host == NULL) { return; }

    //srv->peers = 0;

    SESrv_DisconnectAll(srv, force, disconnect_data);

    enet_host_destroy(srv->e_host);
    srv->e_host = NULL;
}

/** \brief Check if a server is started
 *
 * \param srv[in] Server pointer
 *
 * \return 1 if started, 0 if stoped
 *
 */
int         SENET_API   SESrv_IsStarted (server_t* srv)
{
    if (srv->e_host == NULL) { return 0; }
    return 1;
}


/** \brief Set server's listening port
 *
 * You must (re)start the server to update this
 *
 * \param srv[in] Server pointer
 * \param port[in] Port number
 *
 */
void        SENET_API   SESrv_SetPort (server_t* srv, const uint16 port)
{
    srv->e_addr.port = port;
}

/** \brief Get server's listening port
 *
 * \param srv[in] Server pointer
 *
 * \return Port number
 *
 */
uint16      SENET_API   SESrv_GetPort (server_t* srv)
{
    return srv->e_addr.port;
}


/** \brief Set server's callbacks
 *
 * \param srv[in] Server pointer
 * \param on_conn_proc[in] Function called when a new peer connects
 * \param on_disconnect_proc[in] Function called when a peer disconnects or is timed-out
 * \param on_recv_proc[in] Function called when data is received from a peer
 *
 */
void        SENET_API   SESrv_SetCallbacks (server_t* srv, SESrvOnConnect on_conn_proc, SESrvOnRecv on_recv_proc, SESrvOnDisconnect on_disconnect_proc)
{
    srv->on_connect = on_conn_proc;
    srv->on_recv = on_recv_proc;
    srv->on_disconnect = on_disconnect_proc;
}


/** \brief Set server's bandwidth limits
 *
 * \param srv[in] Server pointer
 * \param max_in_bandwidth[in] Maximum downstream bandwidth (bytes/s) - 0 means unlimited
 * \param max_out_bandwidth[in] Maximum upstream bandwidth (bytes/s) - 0 means unlimited
 *
 */
void        SENET_API   SESrv_SetLimits (server_t* srv, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth)
{
    srv->max_in = max_in_bandwidth;
    srv->max_out = max_out_bandwidth;

    if (srv->e_host != NULL) {
        enet_host_bandwidth_limit(srv->e_host, max_in_bandwidth, max_out_bandwidth);
    }
}

/** \brief Set server's reliability in packets sending (activate by default)
 *
 * When activated, all packets have to flag ENET_PACKET_FLAG_RELIABLE
 *
 * \param srv[in] Server pointer
 * \param reliable[in] 1 to activate, 0 to deactivate reliable packets
 *
 */
void        SENET_API   SESrv_SetReliability (server_t* srv, const int reliable)
{
    srv->reliable = reliable;
}

/** \brief Set server's "send immediatly" option (deactivated by default)
 *
 * When activated, sends are executed immediatly (SESrv_Send(), SESrv_Broadcast()) and disconnect
 * requests are also sent immediatly (SESrv_Disconnect(), SESrv_DisconnectAll(), SESrv_Stop())
 *
 * Otherwise, the above actions are qued and executed on the next call of SESrv_Process()
 *
 * \param srv[in] Server pointer
 * \param send_immediatly[in] 1 to activate, 0 to deactivate
 *
 */
void        SENET_API   SESrv_SetSendImmediatly (server_t* srv, const int send_immediatly)
{
    srv->send_immediatly = send_immediatly;
}


/** \brief Set server's user data. Can be freely modified
 *
 * \param srv[in] Server pointer
 * \param user_data[in]
 *
 */
void        SENET_API   SESrv_SetUserData (server_t* srv, void* user_data)
{
    srv->user_data = user_data;
}

/** \brief Get server's user data
 *
 * \param srv[in] Server pointer
 *
 * \return User data
 *
 */
void*       SENET_API   SESrv_GetUserData (server_t* srv)
{
    return srv->user_data;
}


/** \brief Disconnect a peer
 *
 * When force = 1, the foreign host is not notified about the disconnection.
 *
 * When force = 0, a disconnection notification is sent (immediatly or when SESrv_Process() is called, see SESrv_SetSendImmediatly())
 * and when the foreign host acknowledges it, the disconnection callback is called on both ends of the connection
 *
 * \param srv[in] Server pointer
 * \param peer[in] Peer pointer
 * \param force[in] Force disconnection
 * \param disconnect_data[in] Data describing the disconnection
 *
 */
void        SENET_API   SESrv_Disconnect (server_t* srv, peer_t* peer, const int force, const uint32 disconnect_data)
{
    if (srv->e_host == NULL) { return; }

    if (force != 0) {
        enet_peer_reset((ENetPeer*)peer);
    } else {
        if (srv->send_immediatly != 0) {
            enet_peer_disconnect((ENetPeer*)peer, disconnect_data);
            enet_host_flush(srv->e_host);
        } else {
            enet_peer_disconnect_later((ENetPeer*)peer, disconnect_data);
        }
    }
}

/** \brief Disconnect all peers
 *
 * Same remarks as SESrv_Disconnect()
 *
 * \param srv[in] Server pointer
 * \param force[in] Force disconnection
 * \param disconnect_data[in] Data describing the disconnection
 *
 */
void        SENET_API   SESrv_DisconnectAll (server_t* srv, const int force, const uint32 disconnect_data)
{
    if (srv->e_host == NULL) { return; }

    int i = 0;
    if (force != 0) {
        for (i = 0; i < srv->e_host->peerCount; i++) {
            enet_peer_reset(&srv->e_host->peers[i]);
        }
    } else {
        if (srv->send_immediatly != 0) {
            for (i = 0; i < srv->e_host->peerCount; i++) {
                enet_peer_disconnect(&srv->e_host->peers[i], disconnect_data);
            }
            enet_host_flush(srv->e_host);
        } else {
            for (i = 0; i < srv->e_host->peerCount; i++) {
                enet_peer_disconnect_later(&srv->e_host->peers[i], disconnect_data);
            }
        }
    }
}


/** \brief Get current connected peers count
 *
 * \param srv[in] Server pointer
 *
 * \return Connected peers count
 *
 */
size_t      SENET_API   SESrv_PeersCount (server_t* srv)
{
    if (srv->e_host == NULL) { return 0; }

    //return srv->peers;
    return srv->e_host->connectedPeers;
}

/** \brief Fills an array with current connected peers
 *
 * \param srv[in] Server pointer
 * \param peers[out] peer_t pointers array
 * \param array_size[in] peers array size
 *
 * \return Number of elements added to the array
 *
 */
size_t      SENET_API   SESrv_PeersGetAll (server_t* srv, peer_t* peers[], size_t array_size)
{
    if (srv->e_host == NULL) { return -1; }
    //if (srv->peers == 0) { return 0; }
    if (srv->e_host->connectedPeers == 0) { return 0; }

    int i = 0, added = 0;

    do {
        if (srv->e_host->peers[i].state == ENET_PEER_STATE_CONNECTED) {
            peers[added] = (peer_t*)&srv->e_host->peers[i];
            added++;
        }
        i++;
    } while (i < srv->e_host->peerCount && added < array_size);

    return added;
}

/** \brief Call a user function for each connected peer
 *
 * \param srv[in] Server pointer
 * \param proc[in] SESrvIterateProc()
 * \param user_data[in] User data passed to the callback
 *
 */
void        SENET_API   SESrv_PeersIterate (server_t* srv, SESrvIterateProc proc, void* user_data)
{
    if (srv->e_host == NULL || proc == NULL) { return; }
    //if (srv->peers == 0) { return 0; }
    if (srv->e_host->connectedPeers == 0) { return; }

    int i = 0;
    for (i = 0; i < srv->e_host->peerCount; i++) {
        if ((peer_t*)&srv->e_host->peers[i].state != ENET_PEER_STATE_DISCONNECTED) {
            proc(srv, (peer_t*)&srv->e_host->peers[i], user_data);
        }
    }
}


/** \brief Set peer's user data. Can be freely modified
 *
 * \param srv[in] Server pointer
 * \param peer[in] Peer pointer
 * \param user_data[in] User data
 *
 */
void        SENET_API   SESrv_PeerSetUserData (server_t* srv, peer_t* peer, void* user_data)
{
    ((ENetPeer*)peer)->data = user_data;
}

/** \brief Get peer's user data
 *
 * \param srv[in] Server pointer
 * \param peer[in] Peer pointer
 *
 * \return User data
 *
 */
void*       SENET_API   SESrv_PeerGetUserData (server_t* srv, peer_t* peer)
{
    return ((ENetPeer*)peer)->data;
}


/** \brief Get peer's IP address
 *
 * \param srv[in] Server pointer
 * \param peer[in] Peer pointer
 *
 * \return Peer's IP address
 *
 */
char*       SENET_API   SESrv_PeerGetIP (server_t* srv, peer_t* peer)
{
    if (srv->peer_ip == NULL) { srv->peer_ip = malloc(40 * sizeof(char)); }
    if (srv->peer_ip == NULL) { return NULL; }

    enet_address_get_host_ip(&((ENetPeer*)peer)->address, srv->peer_ip, 40);
    return srv->peer_ip;
}

/** \brief Get peer's port
 *
 * \param srv[in] Server pointer
 * \param peer[in] Peer pointer
 *
 * \return Port number
 *
 */
uint16      SENET_API   SESrv_PeerGetPort (server_t* srv, peer_t* peer)
{
    return ((ENetPeer*)peer)->address.port;
}

/** \brief Get the time between sending a reliable packet and receiving its acknowledgement
 *
 * \param srv[in] Server pointer
 * \param peer[in] Peer pointer
 *
 * \return Peer's round trip time in ms
 *
 */
uint32      SENET_API   SESrv_PeerGetRoundTripTime (server_t* srv, peer_t* peer)
{
    return ((ENetPeer*)peer)->roundTripTime;
}


/** \brief Send data to a peer
 *
 * \param srv[in] Server pointer
 * \param peer[in] Peer pointer
 * \param channel[in] Channel N°
 * \param data[in] Data to be sent
 * \param data_len[in] Data size
 * \param alloc[in] If set to 1, the function will allocate memory for the data, otherwise, the user have to not deallocate the data buffer until the send occures (see SESrv_SetSendImmediatly())
 *
 * \return 1 on succes, 0 on failure, -1 if peer == NULL or the server isn't started
 *
 */
int         SENET_API   SESrv_Send (server_t* srv, peer_t* peer, const size_t channel, const uint8* data, const size_t data_len, const int alloc)
{
    if (srv->e_host == NULL || peer == NULL) { return -1; }

    unsigned int flags = 0;
    if (srv->reliable != 0) {
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

    if (srv->send_immediatly != 0) {
        enet_host_flush(srv->e_host);
    }

    if (ret == 0) { return 1; }
    return 0;
}

/** \brief Send data to all connected peers
 *
 * \param srv[in] Server pointer
 * \param channel[in] Channel N°
 * \param data[in] Data to be sent
 * \param data_len[in] Data size
 * \param alloc[in] If set to 1, the function will allocate memory for the data, otherwise, the user have to not deallocate the data buffer until the send occures (see SESrv_SetSendImmediatly())
 *
 */
void        SENET_API   SESrv_Broadcast (server_t* srv, const size_t channel, const uint8* data, const size_t data_len, const int alloc)
{
    if (srv->e_host == NULL) { return; }

    unsigned int flags = 0;
    if (srv->reliable != 0) {
        flags += ENET_PACKET_FLAG_RELIABLE;
    }
    if (alloc == 0) {
        flags += ENET_PACKET_FLAG_NO_ALLOCATE;
    }

    ENetPacket* packet = enet_packet_create(data, data_len, flags);
    if (packet == NULL) {
        return;
    }

    enet_host_broadcast(srv->e_host, channel, packet);

    if (srv->send_immediatly != 0) {
        enet_host_flush(srv->e_host);
    }
}


/** \brief
 *
 * \param srv[in] Server pointer
 * \param
 * \return
 *
 */
void        SENET_API   SESrv_Process (server_t* srv, const uint32 wait_time)
{
    if (srv->e_host == NULL) { DELAY(wait_time); return; }

    if (enet_host_service(srv->e_host, &srv->e_event, wait_time) > 0) {
        switch (srv->e_event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                #ifdef SENET_DEBUG
                debug("ENET_EVENT_TYPE_RECEIVE\n");
                #endif
                if (srv->on_recv != NULL) {
                    srv->on_recv(srv, (peer_t*)srv->e_event.peer, srv->e_event.channelID, srv->e_event.packet->data, srv->e_event.packet->dataLength);
                }
                enet_packet_destroy(srv->e_event.packet);
            break;
            case ENET_EVENT_TYPE_DISCONNECT:
                #ifdef SENET_DEBUG
                debug("ENET_EVENT_TYPE_DISCONNECT\n");
                #endif
                if (srv->on_disconnect != NULL) {
                    srv->on_disconnect(srv, (peer_t*)srv->e_event.peer, srv->e_event.data);
                }
                enet_peer_reset(srv->e_event.peer);
                //srv->peers--;
            break;
            case ENET_EVENT_TYPE_CONNECT:
                #ifdef SENET_DEBUG
                debug("ENET_EVENT_TYPE_CONNECT\n");
                #endif
                //srv->peers++;
                if (srv->on_connect != NULL) {
                    srv->on_connect(srv, (peer_t*)srv->e_event.peer, srv->e_event.data);
                }
            break;
            case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}
