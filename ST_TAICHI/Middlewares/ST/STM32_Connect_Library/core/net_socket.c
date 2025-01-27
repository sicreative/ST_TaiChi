  /**
  ******************************************************************************
  * @file    net_socket.c
  * @author  MCD Application Team
  * @brief   BSD Like Socket APIs
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

/* Includes ------------------------------------------------------------------*/
#include "net_connect.h"
#include "net_internals.h"


#define OPTCHECKTYPE(type,optlen)       if (sizeof(type)!= optlen) {ret = NET_ERROR_PARAMETER; goto END_SETSOCK;}
#define OPTCHECKSTRING(opt,optlen)       if (strlen(opt)!= (optlen-1)) { ret = NET_ERROR_PARAMETER;goto END_SETSOCK;}


static int32_t create_low_level_socket(int32_t sock);
static int32_t check_low_level_socket(int32_t sock);
static int32_t clone_socket(int32_t sock);

static net_socket_t sockets[NET_MAX_SOCKETS_NBR] = {0};


static net_socket_t* net_socket_get_and_lock(int32_t sock)
{
  net_socket_t *pSocket;
  LOCK_SOCK(sock);
  pSocket = &sockets[sock];
  return    pSocket;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
static int32_t is_valid_socket(int32_t s)
{
  if (( s >= 0) && (s < NET_MAX_SOCKETS_NBR))
  {
    if(sockets[s].status != 0)
    {
      return 1;
    }
  }
  return 0;
}


/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
static int32_t create_low_level_socket(int32_t sock)
{
  net_socket_t *pSocket;
  pSocket=&sockets[sock];
  if (pSocket->ulsocket == -1)
  {
    if (net_access_control(pSocket->pnetif,NET_ACCESS_SOCKET,&pSocket->ulsocket))
    {
      pSocket->ulsocket = pSocket->pnetif->pdrv->socket(sockets[sock].domain,
                                                        sockets[sock].type,
                                                        sockets[sock].protocol);
    }
  }
  return pSocket->ulsocket;
}

static int32_t check_low_level_socket(int32_t sock)
{
  net_socket_t *pSocket;
  pSocket=&sockets[sock];
  return pSocket->ulsocket;
}


/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
static int32_t find_free_socket(void)
{
  int32_t s;
  LOCK_SOCK_ARRAY();
  for (s = 0; s < NET_MAX_SOCKETS_NBR; s++)
  {
    if (sockets[s].status == 0)
    {
      sockets[s].idx = s;
      sockets[s].status = SOCKET_ALLOCATED;
      sockets[s].domain   = 0;
      sockets[s].type     = 0;
      sockets[s].protocol = 0;
#ifdef NET_MBEDTLS_HOST_SUPPORT
      sockets[s].is_secure = false;
      sockets[s].tlsData = 0;
#endif
      sockets[s].read_timeout = NET_SOCK_DEFAULT_RECEIVE_TO;
      sockets[s].write_timeout =NET_SOCK_DEFAULT_SEND_TO;
      sockets[s].blocking = 1;
      sockets[s].ulsocket = -1;
      LOCK_SOCK(s);
      UNLOCK_SOCK_ARRAY();
      return s;
    }
  }
  UNLOCK_SOCK_ARRAY();
  return NET_ERROR_INVALID_SOCKET;
}

static int32_t   clone_socket(int32_t sock)
{
  int32_t   newsock;
  newsock=find_free_socket();
  if (newsock >= 0)
  {
    memcpy(&sockets[newsock],&sockets[sock],sizeof(net_socket_t));
  }
  return newsock;
}


int32_t net_socket(int32_t domain, int32_t type, int32_t protocol)
{
  int32_t   newsock;
  newsock=find_free_socket();
  if (newsock >= 0)
  {
    sockets[newsock].domain   = domain;
    sockets[newsock].type     = type;
    sockets[newsock].protocol = protocol;
    UNLOCK_SOCK(newsock);
  }
  else
  {
    NET_DBG_ERROR ("Socket allocation failed.\n");
  }
  return newsock;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_bind(int32_t sock, sockaddr_t *addr, int32_t addrlen )
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;
  net_if_handle_t *pnetif;
  sockaddr_in *ifaddr = (sockaddr_in *)addr;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    /* find the interface */
    pnetif = net_if_find(S_ADDR(ifaddr->sin_addr));
    if(pnetif != NULL)
    {
      pSocket = net_socket_get_and_lock(sock);
      pSocket->pnetif = pnetif;
      if (create_low_level_socket(sock)< 0)
      {
        ret = NET_ERROR_SOCKET_FAILURE;
        NET_DBG_ERROR ("low level socket creation failed.\n");
      }
      else
      {
        if (net_access_control(pnetif,NET_ACCESS_BIND,&ret))
        {
           UNLOCK_SOCK(sock);
           ret = pSocket->pnetif->pdrv->bind(pSocket->ulsocket, addr, addrlen);
           LOCK_SOCK(sock);
          if(ret != NET_OK)
          {
            NET_DBG_ERROR ("Socket cannot be bound");
          }
        }
      }
      UNLOCK_SOCK(sock);
    }
    else
    {
      NET_DBG_ERROR ("No physical interface can be bound");
    }
  }
  return ret;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_accept(int32_t sock, sockaddr_t *addr, int32_t *addrlen )
{
  int32_t newsock = NET_ERROR_FRAMEWORK;
  int32_t ulnewsock;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    newsock =  NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if(check_low_level_socket(sock) < 0)
    {
     NET_DBG_ERROR ("low level Socket has not been created.\n");
     newsock =  NET_ERROR_SOCKET_FAILURE;
    }
    else
    {
      pSocket = net_socket_get_and_lock(sock);
      if (net_access_control(pSocket->pnetif,NET_ACCESS_BIND,&ulnewsock))
      {
        UNLOCK_SOCK(sock);
        ulnewsock = pSocket->pnetif->pdrv->accept(pSocket->ulsocket, addr, addrlen);
        LOCK_SOCK(sock);

      }
      if(ulnewsock < 0)
      {
        NET_DBG_ERROR ("No connection has been established.\n");
        newsock =ulnewsock;
      }
      else
      {
        sockets[sock].status= SOCKET_CONNECTED;
        newsock = clone_socket(sock);
        sockets[newsock].ulsocket = ulnewsock;
        sockets[newsock].cloneserver = true;
        UNLOCK_SOCK(newsock);
      }
      UNLOCK_SOCK(sock);
    }
  }
  return newsock;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_listen(int32_t sock, int32_t backlog)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if(check_low_level_socket(sock) < 0)
    {
      NET_DBG_ERROR ("low level socket has not been created.\n");
      ret = NET_ERROR_SOCKET_FAILURE;
    }
    else
    {
      pSocket = net_socket_get_and_lock(sock);
      if (net_access_control(pSocket->pnetif,NET_ACCESS_LISTEN,&ret))
      {
        UNLOCK_SOCK(sock);
        ret = pSocket->pnetif->pdrv->listen(pSocket->ulsocket, backlog);
        LOCK_SOCK(sock);

        if(ret != NET_OK)
        {
          NET_DBG_ERROR ("Listen state cannot be set.\n");
        }
      }
      UNLOCK_SOCK(sock);
    }
  }
  return ret;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_connect(int32_t sock, sockaddr_t *addr, int32_t addrlen )
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    pSocket = net_socket_get_and_lock(sock);
#if (NET_USE_DEFAULT_INTERFACE == 1)
    if( pSocket->pnetif == NULL)
    {
      pSocket->pnetif = net_if_find(0);
    }
#endif

    if( pSocket->pnetif == NULL)
    {
      ret = NET_ERROR_INTERFACE_FAILURE;
      NET_DBG_ERROR ("No physical interface can be bound");
    }
    else
    {
      if(create_low_level_socket(sock) < 0)
      {
        NET_DBG_ERROR ("low level socket creation failed.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        if (net_access_control(pSocket->pnetif,NET_ACCESS_CONNECT,&ret))
        {
          UNLOCK_SOCK(sock);
          ret = pSocket->pnetif->pdrv->connect(pSocket->ulsocket, addr, addrlen);
          LOCK_SOCK(sock);

          if(ret != NET_OK)
          {
            // clear flag to avoid issue on clean up , mbedtls not started
#ifdef NET_MBEDTLS_HOST_SUPPORT
            pSocket->is_secure = 0;
#endif
            NET_DBG_ERROR ("Connection cannot be established.\n");
          }
        }
      }
      if (ret == NET_OK)
      {
#ifdef NET_MBEDTLS_HOST_SUPPORT
        if(pSocket->is_secure)
        {
          if (net_mbedtls_start(pSocket)!=NET_OK)
          {
            // to avoid useless cleanup
            pSocket->is_secure=false;
            UNLOCK_SOCK(sock);
            net_closesocket(sock);
            LOCK_SOCK(sock);
            ret = NET_ERROR_SOCKET_FAILURE;
          }
          else
          {
            pSocket->tls_started=true;
          }
        }
        if (NET_OK == ret)
        {
#endif
         pSocket->status = SOCKET_CONNECTED;
#ifdef NET_MBEDTLS_HOST_SUPPORT
        }
#endif
      }
    }
    UNLOCK_SOCK(sock);
  }
  return ret;
}


/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_send (int32_t sock, uint8_t *buf, int32_t len, int32_t flags)
{
  int32_t       ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (buf==0)
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if(check_low_level_socket(sock) < 0)
      {
        NET_DBG_ERROR ("low level socket has not been created.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        pSocket = net_socket_get_and_lock(sock);

#ifdef NET_MBEDTLS_HOST_SUPPORT
        if (pSocket->is_secure)
        {
          ret = net_mbedtls_sock_send(pSocket,  buf,  len);
        }
        else
#endif
        {
          if (net_access_control(pSocket->pnetif,NET_ACCESS_SEND,&ret))
          {
            UNLOCK_SOCK(sock);
            ret = pSocket->pnetif->pdrv->send(pSocket->ulsocket, buf, len, flags);
            LOCK_SOCK(sock);

            if ((ret < 0) && (ret != NET_ERROR_CLOSE_SOCKET))
            {
              NET_DBG_ERROR ("Error during sending data.\n");
            }
          }
        }
        UNLOCK_SOCK(sock);
      }
    }
  }
  return ret;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_recv (int32_t sock, uint8_t *buf, int32_t len, int32_t flags)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (buf==0)
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if(check_low_level_socket(sock) < 0)
      {
        NET_DBG_ERROR ("low level socket has not been created.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
       pSocket = net_socket_get_and_lock(sock);

#ifdef NET_MBEDTLS_HOST_SUPPORT
        if (pSocket->is_secure)
        {
          ret = net_mbedtls_sock_recv(pSocket,  buf,  len);
        }
        else
#endif
        {
          if (net_access_control(pSocket->pnetif,NET_ACCESS_RECV,&ret))
          {
            UNLOCK_SOCK(sock);
            ret = pSocket->pnetif->pdrv->recv(pSocket->ulsocket, buf, len, flags);
            LOCK_SOCK(sock);
          if (( ret < 0) && (ret != NET_TIMEOUT) && (ret != NET_ERROR_CLOSE_SOCKET) )
            {
              NET_DBG_ERROR ("Error during receiving data. %ld\n",ret);
            }
          }
        }
        UNLOCK_SOCK(sock);
      }
    }
  }

  return ret;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_sendto (int32_t sock, uint8_t *buf, int32_t len, int32_t flags, sockaddr_t *to, int32_t tolen)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    return  NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (buf==0)
    {
        ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if (create_low_level_socket(sock)<0)
      {
        NET_DBG_ERROR ("low level socket creation failed.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        pSocket = net_socket_get_and_lock(sock);
        if (net_access_control(pSocket->pnetif,NET_ACCESS_SENDTO,&ret))
        {
          UNLOCK_SOCK(sock);
          ret = pSocket->pnetif->pdrv->sendto(pSocket->ulsocket, buf, len, flags, to, tolen);
          LOCK_SOCK(sock);
          if ((ret < 0) && (ret != NET_ERROR_CLOSE_SOCKET))
          {
            NET_DBG_ERROR ("Error during sending data.\n");
          }
        }
        UNLOCK_SOCK(sock);
      }
    }
  }
  return ret;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_recvfrom (int32_t sock, uint8_t *buf, int32_t len, int32_t flags, sockaddr_t *from, int32_t *fromlen)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (buf==0)
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if (create_low_level_socket(sock)<0)
      {
        NET_DBG_ERROR ("low level socket creation failed.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        pSocket = net_socket_get_and_lock(sock);
        if (net_access_control(pSocket->pnetif,NET_ACCESS_RECVFROM,&ret))
        {
          UNLOCK_SOCK(sock);
          ret = pSocket->pnetif->pdrv->recvfrom(pSocket->ulsocket, buf, len, flags, from, fromlen);
          LOCK_SOCK(sock);
          if ((ret < 0) && (ret != NET_TIMEOUT) && (ret != NET_ERROR_CLOSE_SOCKET))
          {
            NET_DBG_ERROR ("Error during receiving data %ld\n",ret);
          }
        }
        UNLOCK_SOCK(sock);
      }
    }
  }
  return ret;
}


/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_shutdown(int32_t sock,uint32_t      mode)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
     pSocket = net_socket_get_and_lock(sock);
#ifdef NET_MBEDTLS_HOST_SUPPORT
    if (pSocket->is_secure)
    {
      if (pSocket->tls_started)
      {
        pSocket->tls_started = false;
        ret =net_mbedtls_stop(pSocket);
        if (ret != NET_OK)
        {
           NET_DBG_ERROR ("Failed to closed mnedtls socket.\n");
        }
      }
      pSocket->is_secure= 0;
    }
#endif

    if(check_low_level_socket(sock) < 0)
    {
      NET_DBG_ERROR ("failed to shutdown :low level socket not existing.\n");
      pSocket = &sockets[sock];
      pSocket->status = SOCKET_NOT_ALIVE;
      ret = NET_OK;
    }
    else
    {
      if (net_access_control(pSocket->pnetif,NET_ACCESS_CLOSE,&ret))
      {
        UNLOCK_SOCK(sock);
        ret = pSocket->pnetif->pdrv->shutdown (pSocket->ulsocket,mode);
        LOCK_SOCK(sock);

        if(ret != NET_OK )
        {
          NET_DBG_ERROR ("Socket cannot be shutdown.\n");
        }
//      FIXME
        //pSocket->status = SOCKET_NOT_ALIVE;
      }
    }
    UNLOCK_SOCK(sock);
  }

  return ret;
}
/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_closesocket(int32_t sock)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    pSocket = net_socket_get_and_lock(sock);
#ifdef NET_MBEDTLS_HOST_SUPPORT
    if (pSocket->is_secure)
    {
      if (pSocket->tls_started)
      {
        pSocket->tls_started = false;
        ret =net_mbedtls_stop(pSocket);
        if (ret != NET_OK)
        {
           NET_DBG_ERROR ("Failed to closed mnedtls socket.\n");
        }
      }
      pSocket->is_secure= 0;
    }
#endif

    if(check_low_level_socket(sock) < 0)
    {
      NET_DBG_ERROR ("failed to close :low level socket not existing.\n");
      pSocket = &sockets[sock];
      pSocket->status = SOCKET_NOT_ALIVE;
      ret = NET_OK;
    }
    else
    {
      if (net_access_control(pSocket->pnetif,NET_ACCESS_CLOSE,&ret))
      {
        UNLOCK_SOCK(sock);
        ret = pSocket->pnetif->pdrv->close (pSocket->ulsocket,pSocket->cloneserver);
        LOCK_SOCK(sock);

        if(ret != NET_OK)
        {
          NET_DBG_ERROR ("Socket cannot be closed.\n");
        }
        pSocket->ulsocket = -1;
        pSocket->status = SOCKET_NOT_ALIVE;
      }
    }
    UNLOCK_SOCK(sock);
  }

  return ret;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
int32_t net_setsockopt( int32_t sock, int32_t level, int32_t optname, const void *optvalue, int32_t optlen)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR ("Invalid socket.\n");
    return NET_ERROR_INVALID_SOCKET;
  }


  pSocket = net_socket_get_and_lock(sock);

  if(optname == NET_SO_BINDTODEVICE)
  {
    OPTCHECKTYPE(void*,optlen);
    pSocket->pnetif = (net_if_handle_t *)optvalue;
    ret = NET_OK;
    goto END_SETSOCK;
  }

  if(optname == NET_SO_RCVTIMEO)
  {
    OPTCHECKTYPE(int32_t,optlen);
    // not possible to have timeout set to zero
    if (*(uint32_t*)optvalue == 0) *(uint32_t*)optvalue=1;
    pSocket->read_timeout = *(uint32_t*)optvalue;
  }
  if(optname == NET_SO_SNDTIMEO)
  {
    OPTCHECKTYPE(int32_t,optlen);
    pSocket->write_timeout = *(uint32_t*)optvalue;
  }
#ifdef NET_MBEDTLS_HOST_SUPPORT
    if(optname == NET_SO_RCVTIMEO)
    {
       net_mbedtls_set_read_timeout(pSocket);
    }

    if(optname == NET_SO_SECURE)
    {
     if (pSocket->status == SOCKET_CONNECTED)
     {
       ret = NET_ERROR_IS_CONNECTED;
     }
     else
     {
       if (!net_mbedtls_check_tlsdata(pSocket))
       {
          NET_DBG_ERROR ("Failed to set tls device certificats, Allocation failure\n");
         ret = NET_ERROR_NO_MEMORY;
       }
       else
       {
         pSocket->is_secure=true;
         ret = NET_OK;
       }
     }
     goto END_SETSOCK;
    }

    if(optname == NET_SO_TLS_DEV_CERT)
    {
      if (pSocket->status == SOCKET_CONNECTED)
      {
        ret = NET_ERROR_IS_CONNECTED;
      }
      else
      {
        OPTCHECKSTRING(optvalue,optlen);
        if (!net_mbedtls_check_tlsdata(pSocket))
        {
          NET_DBG_ERROR ("Failed to set tls device certificats, Allocation failure\n");
          ret = NET_ERROR_NO_MEMORY;
        }
        else
        {
          pSocket->tlsData->tls_dev_cert = (unsigned char*) optvalue;
          ret = NET_OK;
        }
      }
      goto END_SETSOCK;
    }

    if(optname == NET_SO_TLS_DEV_KEY)
    {
      if (pSocket->status == SOCKET_CONNECTED)
      {
        ret = NET_ERROR_IS_CONNECTED;
      }
      else
      {
        OPTCHECKSTRING(optvalue,optlen);
        if (!net_mbedtls_check_tlsdata(pSocket))
        {
          NET_DBG_ERROR ("Failed to set tls device key, Allocation failure\n");
          ret = NET_ERROR_NO_MEMORY;
        }
        else
        {
          pSocket->tlsData->tls_dev_key = (unsigned char*) optvalue;
          ret = NET_OK;
        }
      }
      goto END_SETSOCK;
    }

    if(optname == NET_SO_TLS_PASSWORD)
    {
      if (pSocket->status == SOCKET_CONNECTED)
      {
        ret = NET_ERROR_IS_CONNECTED;
      }
      else
      {
        OPTCHECKSTRING(optvalue,optlen);
        if (!net_mbedtls_check_tlsdata(pSocket))
        {
          NET_DBG_ERROR ("Failed to set tls password, Allocation failure\n");
          ret = NET_ERROR_NO_MEMORY;
        }
        else
        {
          pSocket->tlsData->tls_dev_pwd = (unsigned char*) optvalue;
          ret = NET_OK;
        }
      }
      goto END_SETSOCK;
    }

    if(optname == NET_SO_TLS_CA_CERT)
    {
      if (pSocket->status == SOCKET_CONNECTED)
      {
        ret = NET_ERROR_IS_CONNECTED;
      }
      else
      {
        OPTCHECKSTRING(optvalue,optlen);
        if (!net_mbedtls_check_tlsdata(pSocket))
        {
          NET_DBG_ERROR ("Failed to set tls root ca, Allocation failure\n");
          return NET_ERROR_NO_MEMORY;
        }
        else
        {
          pSocket->tlsData->tls_ca_certs = (unsigned char*) optvalue;
          ret = NET_OK;
        }
      }
      goto END_SETSOCK;
    }

    if(optname == NET_SO_TLS_CA_CRL)
    {
      if (pSocket->status == SOCKET_CONNECTED)
      {
        ret = NET_ERROR_IS_CONNECTED;
      }
      else
      {
        OPTCHECKSTRING(optvalue,optlen);
        if (!net_mbedtls_check_tlsdata(pSocket))
        {
          NET_DBG_ERROR ("Failed to set tls revocation certification list, Allocation failure\n");
          ret = NET_ERROR_NO_MEMORY;
        }
        else
        {
          pSocket->tlsData->tls_ca_certs = (unsigned char*) optvalue;
          ret = NET_OK;
        }
      }goto END_SETSOCK;
    }

    if(optname == NET_SO_TLS_SERVER_VERIFICATION)
    {
      if (pSocket->status == SOCKET_CONNECTED)
      {
        ret = NET_ERROR_IS_CONNECTED;
      }
      else
      {
        OPTCHECKTYPE(bool,optlen);
        if (!net_mbedtls_check_tlsdata(pSocket))
        {
          NET_DBG_ERROR ("Failed to set tls server verification mode, Allocation failure\n");
          ret = NET_ERROR_NO_MEMORY;
        }
        else
        {
          pSocket->tlsData->tls_srv_verification= ( *(bool*)optvalue>0)?true:false;
          ret = NET_OK;
        }
      }
      goto END_SETSOCK;
    }

    if(optname == NET_SO_TLS_SERVER_NAME)
    {
      if (pSocket->status == SOCKET_CONNECTED)
      {
        ret = NET_ERROR_IS_CONNECTED;
      }
      else
      {
        OPTCHECKSTRING(optvalue,optlen);
        if (!net_mbedtls_check_tlsdata(pSocket))
        {
          NET_DBG_ERROR ("Failed to set tls server name, Allocation failure\n");
          ret = NET_ERROR_NO_MEMORY;
        }
        else
        {
          pSocket->tlsData->tls_srv_name= (char*)optvalue;
          ret = NET_OK;
        }
      }
      goto END_SETSOCK;
    }

  /* Set the X.509 security profile */
  if(optname == NET_SO_TLS_CERT_PROF)
    {
      if (pSocket->status == SOCKET_CONNECTED)
      {
        ret = NET_ERROR_IS_CONNECTED;
      }
      else
      {
        OPTCHECKTYPE(mbedtls_x509_crt_profile,optlen);
        if (!net_mbedtls_check_tlsdata(pSocket))
        {
          NET_DBG_ERROR ("Failed to set tls X.509 security profile, Allocation failure\n");
          ret = NET_ERROR_NO_MEMORY;
        }
        else
        {
          pSocket->tlsData->tls_cert_prof = (mbedtls_x509_crt_profile*) optvalue;
          ret = NET_OK;
        }
      }
      goto END_SETSOCK;
    }
#endif


#if (NET_USE_DEFAULT_INTERFACE == 1)
    if( pSocket->pnetif == NULL)
    {
      pSocket->pnetif = net_if_find(0);
    }
#endif
    if( pSocket->pnetif == NULL)
    {
      NET_DBG_ERROR ("No physical interface can be bound");
      ret = NET_ERROR_INTERFACE_FAILURE;
    }
    else
    {
      if (create_low_level_socket(sock)<0)
      {
        NET_DBG_ERROR ("low level socket creation failed.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        if (net_access_control(pSocket->pnetif,NET_ACCESS_SETSOCKOPT,&ret))
        {
          UNLOCK_SOCK(sock);
          ret = pSocket->pnetif->pdrv->setsockopt(pSocket->ulsocket, level, optname, optvalue, optlen);
          LOCK_SOCK(sock);
          if(ret < 0)
          {
            NET_DBG_ERROR ("Error during setting option.\n");
          }
        }
      }
    }
END_SETSOCK:
  UNLOCK_SOCK(sock);
  return ret;
}

bool    net_access_control(net_if_handle_t *pnetif,net_access_t func,int32_t *code)
{
  bool ret = true;
  if (pnetif->state == NET_STATE_LINK_LOST)
  {
    /* send ,recv functino return zero , so user application should normaly retry transfer */
    ret =false;
    *code = 0;
  }
  switch(func)
  {
    case NET_ACCESS_SOCKET:
     break;

    case NET_ACCESS_BIND:
     break;

    case NET_ACCESS_LISTEN:
     break;

    case NET_ACCESS_CONNECT:
     break;

    case NET_ACCESS_CLOSE:
     ret = true;
     break;

    case NET_ACCESS_SEND:
     *code = 0;
     break;

    case NET_ACCESS_SENDTO:
     *code = 0;
     break;

    case NET_ACCESS_RECV:
     *code = 0;
     break;

    case NET_ACCESS_RECVFROM:
     *code = 0;
     break;

   default:
     *code = NET_ERROR_FRAMEWORK;
     ret = NET_ERROR_FRAMEWORK;
     break;
  }
  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

