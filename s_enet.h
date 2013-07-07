#ifndef __S_ENET_H__
#define __S_ENET_H__

#ifdef SENET_BUILD_DLL
    #define SENET_API __declspec(dllexport)
#else
    #define SENET_API __declspec(dllimport)
#endif

/* Typedefs */
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef struct peer_t peer_t;
typedef struct server_t server_t;
typedef struct client_t client_t;
typedef struct client2_t client2_t;

/* Callback */
// Server
typedef void (*SESrvOnConnect)       (server_t* srv, peer_t* peer, uint32 data);
typedef void (*SESrvOnDisconnect)    (server_t* srv, peer_t* peer, uint32 data);
typedef void (*SESrvOnRecv)          (server_t* srv, peer_t* peer, size_t channel, uint8* data, size_t data_len);
typedef void (*SESrvIterateProc)     (server_t* srv, peer_t* peer, void* user_data);

// Client
typedef void (*SEClnOnDisconnect)    (client_t* cln, uint32 data);
typedef void (*SEClnOnRecv)          (client_t* cln, size_t channel, uint8* data, size_t data_len);

// Advanced client
typedef int  (*SECln2OnConnect)      (client2_t* cln, peer_t* peer, uint32 data);
typedef void (*SECln2OnDisconnect)   (client2_t* cln, peer_t* peer, uint32 data);
typedef void (*SECln2OnRecv)         (client2_t* cln, peer_t* peer, size_t channel, uint8* data, size_t data_len);
typedef void (*SECln2IterateProc)    (client2_t* srv, peer_t* peer, void* user_data);


/* Init/Shutdown */
int         SENET_API   SE_Startup ();
void        SENET_API   SE_Shutdown ();

/* Server functions */
server_t*   SENET_API   SESrv_Create (const uint16 port, const size_t max_peers, const size_t channels, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth);
void        SENET_API   SESrv_Destroy (server_t* srv);
int         SENET_API   SESrv_Start (server_t* srv);
void        SENET_API   SESrv_Stop (server_t* srv, const int force, const uint32 disconnect_data);
int         SENET_API   SESrv_IsStarted (server_t* srv);

void        SENET_API   SESrv_SetPort (server_t* srv, const uint16 port);
uint16      SENET_API   SESrv_GetPort (server_t* srv);

void        SENET_API   SESrv_SetCallbacks (server_t* srv, SESrvOnConnect on_conn_proc, SESrvOnRecv on_recv_proc, SESrvOnDisconnect on_disconnect_proc);

void        SENET_API   SESrv_SetLimits (server_t* srv, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth);
void        SENET_API   SESrv_SetReliability (server_t* srv, const int reliable);
void        SENET_API   SESrv_SetSendImmediatly (server_t* srv, const int send_immediatly);

void        SENET_API   SESrv_SetUserData (server_t* srv, void* user_data);
void*       SENET_API   SESrv_GetUserData (server_t* srv);

void        SENET_API   SESrv_Disconnect (server_t* srv, peer_t* peer, const int force, const uint32 disconnect_data);
void        SENET_API   SESrv_DisconnectAll (server_t* srv, const int force, const uint32 disconnect_data);

size_t      SENET_API   SESrv_PeersCount (server_t* srv);
size_t      SENET_API   SESrv_PeersGetAll (server_t* srv, peer_t* peers[], size_t array_size);
void        SENET_API   SESrv_PeersIterate (server_t* srv, SESrvIterateProc proc, void* user_data);

void        SENET_API   SESrv_PeerSetUserData (server_t* srv, peer_t* peer, void* user_data);
void*       SENET_API   SESrv_PeerGetUserData (server_t* srv, peer_t* peer);

char*       SENET_API   SESrv_PeerGetIP (server_t* srv, peer_t* peer);
uint16      SENET_API   SESrv_PeerGetPort (server_t* srv, peer_t* peer);

uint32      SENET_API   SESrv_PeerGetRoundTripTime (server_t* srv, peer_t* peer);

int         SENET_API   SESrv_Send (server_t* srv, peer_t* peer, const size_t channel, const uint8* data, const size_t data_len, const int alloc);
void        SENET_API   SESrv_Broadcast (server_t* srv, const size_t channel, const uint8* data, const size_t data_len, const int alloc);

void        SENET_API   SESrv_Process (server_t* srv, const uint32 wait_time);

/* Client functions */
client_t*   SENET_API   SECln_Create (const char* host, const uint16 port, const size_t channels, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth);
void        SENET_API   SECln_Destroy (client_t* cln);

int         SENET_API   SECln_Connect (client_t* cln, const uint32 timeout, const uint32 connect_data);
void        SENET_API   SECln_Disconnect (client_t* cln, const int force, const uint32 disconnect_data);
int         SENET_API   SECln_IsConnected (client_t* cln);

void        SENET_API   SECln_SetHost (client_t* cln, const char* host);
void        SENET_API   SECln_SetPort (client_t* cln, const uint16 port);
char*       SENET_API   SECln_GetHost (client_t* cln);
char*       SENET_API   SECln_GetHostIP (client_t* cln);
uint16      SENET_API   SECln_GetPort (client_t* cln);

void        SENET_API   SECln_SetCallbacks (client_t* cln, SEClnOnRecv on_recv_proc, SEClnOnDisconnect on_disconnect_proc);

void        SENET_API   SECln_SetLimits (client_t* cln, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth);
void        SENET_API   SECln_SetReliability (client_t* cln, const int reliable);
void        SENET_API   SECln_SetSendImmediatly (client_t* cln, const int send_immediatly);

void        SENET_API   SECln_SetUserData (client_t* cln, void* user_data);
void*       SENET_API   SECln_GetUserData (client_t* cln);

int         SENET_API   SECln_Send (client_t* cln, const size_t channel, const uint8* data, const size_t data_len, const int alloc);

void        SENET_API   SECln_Process (client_t* cln, const uint32 wait_time);

/* Advanced multi-connection client functions */
client2_t*  SENET_API   SECln2_Create (const size_t max_peers, const size_t channels, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth);
void        SENET_API   SECln2_Destroy (client2_t* cln);

peer_t*     SENET_API   SECln2_Connect (client2_t* cln, const char* host, const uint16 port, const uint32 connect_data);
void        SENET_API   SECln2_PeerDisconnect (client2_t* cln, peer_t* peer, const int force, const uint32 disconnect_data);
void        SENET_API   SECln2_PeersDisconnectAll (client2_t* cln, const int force, const uint32 disconnect_data);
int         SENET_API   SECln2_PeerIsConnected (client2_t* cln, peer_t* peer);

void        SENET_API   SECln2_SetLimits (client2_t* cln, const uint32 max_in_bandwidth, const uint32 max_out_bandwidth);
void        SENET_API   SECln2_SetReliability (client2_t* cln, const int reliable);
void        SENET_API   SECln2_SetSendImmediatly (client2_t* cln, const int send_immediatly);

void        SENET_API   SECln2_SetCallbacks (client2_t* cln, SECln2OnConnect on_connect_proc, SECln2OnDisconnect on_disconnect_proc, SECln2OnRecv on_recv_proc);

void        SENET_API   SECln2_SetUserData (client2_t* cln, void* user_data);
void*       SENET_API   SECln2_GetUserData (client2_t* cln);

size_t      SENET_API   SECln2_PeersCount (client2_t* srv);
size_t      SENET_API   SECln2_PeersGetAll (client2_t* srv, peer_t* peers[], size_t array_size);
void        SENET_API   SECln2_PeersIterate (client2_t* srv, SECln2IterateProc proc, void* user_data);

char*       SENET_API   SECln2_PeerGetHostIP (client2_t* cln, peer_t* peer);
uint16      SENET_API   SECln2_PeerGetPort (client2_t* cln, peer_t* peer);

void        SENET_API   SECln2_PeerSetUserData (client2_t* cln, peer_t* peer, void* user_data);
void*       SENET_API   SECln2_PeerGetUserData (client2_t* cln, peer_t* peer);

int         SENET_API   SECln2_Send (client2_t* cln, peer_t* peer, const size_t channel, const uint8* data, const size_t data_len, const int alloc);
void        SENET_API   SECln2_Broadcast (client2_t* cln, const size_t channel, const uint8* data, const size_t data_len, const int alloc);

void        SENET_API   SECln2_Process (client2_t* cln, const uint32 wait_time);

#endif // __S_ENET_H__
