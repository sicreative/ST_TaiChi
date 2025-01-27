/**
  ******************************************************************************
  * @file    net_core.h
  * @author  MCD Application Team
  * @brief   Provides the network interface driver APIs.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef NET_CORE_H
#define NET_CORE_H
#include "stdbool.h"
#include "net_wifi.h"
#include "net_class_extension.h"

#ifdef __cplusplus
 extern "C" {
#endif

uint32_t HAL_GetTick(void);

int32_t icmp_ping(net_if_handle_t *netif,sockaddr_t *addr,int count , int timeout, int response[]);

#ifdef NET_USE_RTOS
void    *net_calloc(size_t n,size_t m);


void net_init_locks(void);
void net_destroy_locks(void);

void net_lock(int32_t idx, uint32_t to);
void net_unlock(int32_t idx);

void net_lock_nochk(int32_t idx, uint32_t to);
void net_unlock_nochk(int32_t idx);

/* OS */
#define NET_OS_WAIT_FOREVER     0xffffffff


#define NET_LOCK_SOCKET_ARRAY   NET_MAX_SOCKETS_NBR
#define NET_LOCK_NETIF_LIST     NET_MAX_SOCKETS_NBR+1
#define NET_LOCK_STATE_EVENT    NET_MAX_SOCKETS_NBR+2

#define NET_LOCK_NUMBER          NET_LOCK_STATE_EVENT+1

#define  LOCK_SOCK(s)           net_lock(s,NET_OS_WAIT_FOREVER)
#define  UNLOCK_SOCK(s)         net_unlock(s)
    
#define  LOCK_SOCK_ARRAY()      net_lock(NET_LOCK_SOCKET_ARRAY,NET_OS_WAIT_FOREVER)
#define  UNLOCK_SOCK_ARRAY()    net_unlock(NET_LOCK_SOCKET_ARRAY )

#define  LOCK_NETIF_LIST()      net_lock(NET_LOCK_NETIF_LIST,NET_OS_WAIT_FOREVER )
#define  UNLOCK_NETIF_LIST()    net_unlock(NET_LOCK_NETIF_LIST )

#define  WAIT_STATE_CHANGE(to)  net_lock_nochk(NET_LOCK_STATE_EVENT,to )
#define  SIGNAL_STATE_CHANGE()  net_unlock_nochk(NET_LOCK_STATE_EVENT )

#else

#define  LOCK_SOCK(s)
#define  UNLOCK_SOCK(s)
#define  LOCK_SOCK_ARRAY()
#define  UNLOCK_SOCK_ARRAY()
#define  LOCK_NETIF_LIST()
#define  UNLOCK_NETIF_LIST()
#define  WAIT_STATE_CHANGE(to)
#define  SIGNAL_STATE_CHANGE()



#endif



typedef enum
{
   NET_INTERFACE_CLASS_WIFI,
   NET_INTERFACE_CLASS_CELLULAR,
   NET_INTERFACE_CLASS_ETHERNET,
   NET_INTERFACE_CLASS_CUSTOM
 }
 net_interface_class_t;


typedef enum
{
   NET_ACCESS_SOCKET,
   NET_ACCESS_BIND,
   NET_ACCESS_LISTEN,
   NET_ACCESS_CONNECT,
   NET_ACCESS_SEND,
   NET_ACCESS_SENDTO,
   NET_ACCESS_RECV,
   NET_ACCESS_RECVFROM,
   NET_ACCESS_CLOSE,
   NET_ACCESS_SETSOCKOPT,
 }
 net_access_t;

struct net_if_drv_s {
  net_interface_class_t     if_class;
  /* Interface APIs */
  int32_t (* if_init) (net_if_handle_t *pnetif);
  int32_t (* if_deinit) (net_if_handle_t *pnetif);
  int32_t (* if_start) (net_if_handle_t *pnetif);
  int32_t (* if_stop) (net_if_handle_t *pnetif);
  int32_t (* if_connect) (net_if_handle_t *pnetif);
  int32_t (* if_disconnect) (net_if_handle_t *pnetif);
  int32_t (* if_powersave_enable) (net_if_handle_t *pnetif);
  int32_t (* if_powersave_disable) (net_if_handle_t *pnetif);

  void    *context;
  /* Socket BSD Like APIs */
  int32_t (* socket) (int32_t domain, int32_t type, int32_t protocol);
  int32_t (* bind) (int32_t sock, const sockaddr_t *addr, int32_t addrlen);
  int32_t (* listen) (int32_t sock, int32_t backlog);
  int32_t (* accept) (int32_t sock, sockaddr_t *addr, int32_t *addrlen);
  int32_t (* connect) (int32_t sock, const sockaddr_t *addr, int32_t addrlen);
  int32_t (* send) (int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
  int32_t (* recv) (int32_t sock, uint8_t *buf, int32_t len , int32_t flags);
  int32_t (* sendto) (int32_t sock, uint8_t *buf, int32_t len, int32_t flags, sockaddr_t *to, int32_t tolen);
  int32_t (* recvfrom) (int32_t sock, uint8_t *buf, int32_t len, int32_t flags, sockaddr_t *from, int32_t *fromlen);
  int32_t (* setsockopt)( int32_t sock, int32_t level, int32_t optname, const void *optvalue, int32_t optlen);
  int32_t (* getsockopt)( int32_t sock, int32_t level, int32_t optname, void *optvalue, int32_t *optlen);
  int32_t (* getsockname)( int32_t sock, sockaddr_t *name, int32_t *namelen);
  int32_t (* getpeername)( int32_t sock,sockaddr_t *name, int32_t *namelen);
  int32_t (* close) (int32_t sock,bool Clone);
  int32_t (* shutdown) (int32_t sock,uint32_t mode);

  /* Service */
  int32_t (* gethostbyname)(net_if_handle_t*,sockaddr_t *addr,char *name);
  int32_t (* ping)(net_if_handle_t*,sockaddr_t *addr,int count , int delay, int reponse[]);
  /* class extension */
  union
  {
    net_if_wifi_class_extension_t         *wifi;
    net_if_ethernet_class_extension_t     *ethernet;
    net_if_cellular_class_extension_t     *cellular;
    net_if_custom_class_extension_t       *custom;
  } extension;
};





net_if_handle_t * net_if_find (ipaddr_t  addr);
net_if_handle_t * netif_check(net_if_handle_t *pnetif);


bool    net_access_control(net_if_handle_t *,net_access_t,int32_t *);

typedef   void  (*sock_notify_func) (int32_t, int32_t, const uint8_t *, uint32_t);

typedef struct net_tls_data net_tls_data_t;
typedef int32_t net_ulsock_t;

typedef enum { SOCKET_NOT_ALIVE = 0,SOCKET_ALLOCATED  ,SOCKET_CONNECTED  } socket_state_t;


typedef struct net_socket_s {
  net_if_handle_t  *pnetif;
  net_ulsock_t     ulsocket;
  socket_state_t   status;
  int32_t          domain;
  int32_t          type;
  int32_t          protocol;
  bool             cloneserver;
  bool             connected;
#ifdef NET_MBEDTLS_HOST_SUPPORT
  bool             is_secure;
  net_tls_data_t   *tlsData;
  bool             tls_started;
#endif
  int32_t          read_timeout;
  int32_t          write_timeout;
  bool             blocking;
  uint32_t         idx;
} net_socket_t;


typedef struct
{
  int32_t (*add_if)(net_if_handle_t * pnetif,int32_t (*if_output)(net_buf_t *net_buf), uint8_t default_flag);
  int32_t (*remove_if)(net_if_handle_t * pnetif);
  int32_t (*connect)(net_if_handle_t * pnetif);
  int32_t (*disconnect)(net_if_handle_t * pnetif);
} iplib_t;




#ifdef  NET_MBEDTLS_HOST_SUPPORT
#include "net_mbedtls.h"
#endif
#endif /* NET_CORE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
