#include <stdlib.h>
#include <stdio.h>

#include "enet/enet.h"
#include "s_enet_internal.h"
#include "s_enet.h"

struct client_t {
    ENetAddress e_addr;
    ENetHost* e_host;
    ENetPeer* e_peer;
    ENetEvent e_event;

    char host_ip[16];
    char* host;
    uint16 port;

    int reliable;
    int send_immediatly;

    SEClnOnRecv on_recv;
    SEClnOnDisconnect on_disconnect;

    void* user_data;
};


/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
client_t*   SENET_API   SECln_Create (const char* host, const uint16 port, const size_t channels, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth)
{
    client_t* cln = malloc(sizeof(client_t));
    if (cln == NULL) {
        return NULL;
    }

    size_t host_len = strlen(host);
    if (host_len > 0) {
        cln->host = malloc((host_len + 1) * sizeof(char));
        if (cln->host == NULL) {
            free(cln);
            return NULL;
        }
        strcpy(cln->host, host);
        enet_address_set_host(&cln->e_addr, host);
        enet_address_get_host_ip(&cln->e_addr, cln->host_ip, 16);
    } else {
        cln->host = NULL;
        cln->host_ip[0] = '\0';
    }

    cln->e_addr.port = port;

    cln->e_host = enet_host_create(NULL, 1, channels, max_in_bandwidth, max_out_bandwidth);
    if (cln->e_host == NULL) {
        free(cln->host);
        free(cln);
        return NULL;
    }

    cln->e_peer = NULL;

    cln->reliable = 1;
    cln->send_immediatly = 0;

    cln->on_recv = NULL;
    cln->on_disconnect = NULL;

    cln->user_data = NULL;

    return cln;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void        SENET_API   SECln_Destroy (client_t* cln)
{
    if (cln->e_peer != NULL) {
        enet_peer_reset(cln->e_peer);
    }

    enet_host_destroy(cln->e_host);
    free(cln->host);
    free(cln);
}


/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
int         SENET_API   SECln_Connect (client_t* cln, const uint32 timeout, const uint32 connect_data)
{
    if (cln->e_peer != NULL) { return -1; }

    cln->e_peer = enet_host_connect(cln->e_host, &cln->e_addr, cln->e_host->channelLimit, connect_data);
    if (cln->e_peer == NULL) { return -1; }

    if (enet_host_service(cln->e_host, &cln->e_event, timeout) > 0 && cln->e_event.type == ENET_EVENT_TYPE_CONNECT && cln->e_event.peer == cln->e_peer) {
        return 1;
    } else {
        enet_peer_reset(cln->e_peer);
        cln->e_peer = NULL;
        return 0;
    }
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void        SENET_API   SECln_Disconnect (client_t* cln, const int force, const uint32 disconnect_data)
{
    if (cln->e_peer == NULL) { return; }

    if (force != 0) {
        enet_peer_reset(cln->e_peer);
        cln->e_peer = NULL;
    } else {
        if (cln->send_immediatly != 0) {
            enet_peer_disconnect(cln->e_peer, disconnect_data);
            enet_host_flush(cln->e_host);
        } else {
            enet_peer_disconnect_later(cln->e_peer, disconnect_data);
        }
    }
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
int         SENET_API   SECln_IsConnected (client_t* cln)
{
    if (cln->e_peer != NULL) { return 1; }
    return 0;
}


/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void        SENET_API   SECln_SetHost (client_t* cln, const char* host)
{
    size_t host_len = strlen(host);
    if (host_len > 0) {
        cln->host = realloc(cln->host, (host_len + 1) * sizeof(char));
        strcpy(cln->host, host);
        enet_address_set_host(&cln->e_addr, host);
        enet_address_get_host_ip(&cln->e_addr, cln->host_ip, 16);
    } else {
        if (cln->host != NULL) { free(cln->host); }
        cln->host = NULL;
        cln->host_ip[0] = '\0';
    }
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void        SENET_API   SECln_SetPort (client_t* cln, const uint16 port)
{
    cln->e_addr.port = port;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
char*       SENET_API   SECln_GetHost (client_t* cln)
{
    return cln->host;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
char*       SENET_API   SECln_GetHostIP (client_t* cln)
{
    return cln->host_ip;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
uint16      SENET_API   SECln_GetPort (client_t* cln)
{
    return cln->e_addr.port;
}


/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void        SENET_API   SECln_SetCallbacks (client_t* cln, SEClnOnRecv on_recv_proc, SEClnOnDisconnect on_disconnect_proc)
{
    cln->on_recv = on_recv_proc;
    cln->on_disconnect = on_disconnect_proc;
}


/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void        SENET_API   SECln_SetLimits (client_t* cln, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth)
{
    enet_host_bandwidth_limit(cln->e_host, max_in_bandwidth, max_out_bandwidth);
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void        SENET_API   SECln_SetReliability (client_t* cln, const int reliable)
{
    cln->reliable = reliable;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void        SENET_API   SECln_SetSendImmediatly (client_t* cln, const int send_immediatly)
{
    cln->send_immediatly = send_immediatly;
}


/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void        SENET_API   SECln_SetUserData (client_t* cln, void* user_data)
{
    cln->user_data = user_data;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void*       SENET_API   SECln_GetUserData (client_t* cln)
{
    return cln->user_data;
}


/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
int         SENET_API   SECln_Send (client_t* cln, const size_t channel, const uint8* data, const size_t data_len, const int alloc)
{
    if (cln->e_peer == NULL) {
        return -1;
    }

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

    int ret = enet_peer_send(cln->e_peer, channel, packet);

    if (cln->send_immediatly) {
        enet_host_flush(cln->e_host);
    }

    if (ret == 0) { return 1; }
    return 0;
}


/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void        SENET_API   SECln_Process (client_t* cln, const uint32 wait_time)
{
    if (cln->e_peer == NULL) { DELAY(wait_time); return; }

    if (enet_host_service(cln->e_host, &cln->e_event, wait_time) > 0) {
        switch (cln->e_event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                #ifdef SENET_DEBUG
                debug("ENET_EVENT_TYPE_RECEIVE\n");
                #endif
                if (cln->on_recv != NULL) {
                    cln->on_recv(cln, cln->e_event.channelID, cln->e_event.packet->data, cln->e_event.packet->dataLength);
                }
                enet_packet_destroy(cln->e_event.packet);
            break;
            case ENET_EVENT_TYPE_DISCONNECT:
                #ifdef SENET_DEBUG
                debug("ENET_EVENT_TYPE_DISCONNECT\n");
                #endif
                if (cln->on_disconnect != NULL) {
                    cln->on_disconnect(cln, cln->e_event.data);
                }
                cln->e_peer = NULL;
            break;
            case ENET_EVENT_TYPE_CONNECT:
            case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}
