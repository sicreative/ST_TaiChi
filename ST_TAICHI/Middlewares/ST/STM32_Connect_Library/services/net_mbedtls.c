/**
  ******************************************************************************
  * @file    net_mbedtls.c
  * @author  MCD Application Team
  * @brief   Network abstraction at transport layer level. mbedTLS implementation.
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
#include "net_connect.h"
#include "net_internals.h"
#ifdef NET_MBEDTLS_HOST_SUPPORT

int mbedtls_rng_poll_cb( void *data, unsigned char *output, size_t len, size_t *olen );

extern struct __RNG_HandleTypeDef hrng;

/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void mbedtls_free_resource(net_socket_t * sock);
static int  mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len,uint32_t timeout);
static int  mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len);

#ifdef NET_USE_RTOS
extern void *pxCurrentTCB;


#ifdef  MBEDTLS_THREADING_ALT


void mutex_init( mbedtls_threading_mutex_t  * mutex );
void mutex_free( mbedtls_threading_mutex_t  * mutex );
int mutex_lock(mbedtls_threading_mutex_t * mutex );
int mutex_unlock( mbedtls_threading_mutex_t * mutex );

void mutex_init( mbedtls_threading_mutex_t  * mutex )
{
    mutex->id= osSemaphoreCreate(&mutex->def,1);
}

void mutex_free( mbedtls_threading_mutex_t  * mutex )
{
  osSemaphoreDelete(mutex->id);
}

int mutex_lock(mbedtls_threading_mutex_t * mutex )
{
  BaseType_t ret;
  ret = osSemaphoreWait(mutex->id,osWaitForever);
  return ret >= 0 ? 0 : -1;
}

int mutex_unlock( mbedtls_threading_mutex_t * mutex )
{
  BaseType_t ret;
  ret = osSemaphoreRelease(mutex->id);
  return ret >= 0 ? 0 : -1;
}

#endif
#endif

void net_tls_init(void)
{
#ifdef MBEDTLS_THREADING_ALT
  mbedtls_threading_set_alt(mutex_init, mutex_free, mutex_lock, mutex_unlock);
#endif
}

void net_tls_destroy(void)
{
#ifdef MBEDTLS_THREADING_ALT
  mbedtls_threading_free_alt();
#endif
}

/* Functions Definition ------------------------------------------------------*/
bool net_mbedtls_check_tlsdata(net_socket_t *sock)
{
  bool ret = true;
  if (NULL == sock->tlsData)
  {
    sock->tlsData = net_malloc(sizeof(net_tls_data_t));
    if (sock->tlsData == NULL)
    {
      NET_DBG_ERROR ("Error during setting option.\n");
      ret = false;
    }
    else
    {
      memset(sock->tlsData,0,sizeof(net_tls_data_t));
      sock->tlsData->tls_srv_verification=true;
      ret = true;
    }
  }
  return ret;
}



static void DebugPrint( void *ctx,
                                 int level,
                                 const char *file,
                                 int line,
                                 const char *str )
{
  /* Unused parameters. */
  ( void )ctx;

  /* Send the debug string to the portable logger. */
#ifdef NET_USE_RTOS
  printf("%p => %s:%04d: %s\n",pxCurrentTCB, file, line, str );
#else
  printf("%s:%04d: %s\n", file, line, str );
#endif
}


void net_mbedtls_set_read_timeout(net_socket_t *sock)
{
	net_tls_data_t *tlsData = sock->tlsData;
        if (tlsData)
        {
	  mbedtls_ssl_conf_read_timeout(&tlsData->conf, sock->read_timeout);
        }
}

int net_mbedtls_start(net_socket_t *sock)
{
  int32_t       ret;
  net_tls_data_t *tlsData = sock->tlsData;
  const unsigned char *pers = (unsigned char *)"net_tls";

  mbedtls_platform_set_calloc_free(net_calloc,net_free);
  mbedtls_ssl_init( &tlsData->ssl );
  mbedtls_ssl_config_init(&tlsData->conf);
  mbedtls_ssl_conf_dbg(&tlsData->conf, DebugPrint, NULL);

  mbedtls_debug_set_threshold( NET_MBEDTLS_DEBUG_LEVEL );
  mbedtls_ctr_drbg_init(&tlsData->ctr_drbg);
  mbedtls_x509_crt_init(&tlsData->cacert);
  if (tlsData->tls_dev_cert != NULL)
  {
    mbedtls_x509_crt_init(&tlsData->clicert);
  }
  if (tlsData->tls_dev_key != NULL)
  {
    mbedtls_pk_init(&tlsData->pkey);
  }

  /* Entropy generator init */
  mbedtls_entropy_init(&tlsData->entropy);
  if( (ret = mbedtls_entropy_add_source(&tlsData->entropy, mbedtls_rng_poll_cb, &hrng, 1, MBEDTLS_ENTROPY_SOURCE_STRONG)) != 0 )
  {
    NET_DBG_ERROR( " failed\n  ! mbedtls_entropy_add_source returned -0x%lx\n", -ret );
    mbedtls_free_resource(sock);
    return NET_ERROR_MBEDTLS_ENTROPY;
  }
  if( (ret = mbedtls_ctr_drbg_seed(&tlsData->ctr_drbg, mbedtls_entropy_func, &tlsData->entropy, pers, strlen((char const *)pers))) != 0 )
  {
    NET_DBG_ERROR(" failed\n  ! mbedtls_ctr_drbg_seed returned -0x%lx\n", -ret);
    mbedtls_free_resource(sock);
    return NET_ERROR_MBEDTLS_SEED;
  }

  /* Root CA */
  if (tlsData->tls_ca_certs != NULL)
  {
    if( (ret = mbedtls_x509_crt_parse(&tlsData->cacert, (unsigned char const *)tlsData->tls_ca_certs, strlen((char const *) tlsData->tls_ca_certs) + 1)) != 0 )
    {
      NET_DBG_ERROR(" failed\n  !  mbedtls_x509_crt_parse returned -0x%lx while parsing root cert\n", -ret);
      mbedtls_free_resource(sock);
      return NET_ERROR_MBEDTLS_CRT_PARSE;
    }
  }
  else
  {
	  // no root so cannot check server identification ?
	 // tlsData->tls_srv_verification =
  }


  /* Client cert. and key */
  if( (tlsData->tls_dev_cert != NULL) && (tlsData->tls_dev_key != NULL) )
  {
    if( (ret = mbedtls_x509_crt_parse(&tlsData->clicert, (unsigned char const *)tlsData->tls_dev_cert, strlen((char const *)tlsData->tls_dev_cert) + 1)) != 0 )
    {
      NET_DBG_ERROR(" failed\n  !  mbedtls_x509_crt_parse returned -0x%lx while parsing device cert\n", -ret);
      mbedtls_free_resource(sock);
      return NET_ERROR_MBEDTLS_CRT_PARSE;
    }

    if( (ret = mbedtls_pk_parse_key(&tlsData->pkey, (unsigned char const *)tlsData->tls_dev_key, strlen((char const *)tlsData->tls_dev_key) + 1,
           (unsigned char const *)tlsData->tls_dev_pwd, tlsData->tls_dev_pwd_len)) != 0 )
    {
      NET_DBG_ERROR(" failed\n  !  mbedtls_pk_parse_key returned -0x%lx while parsing private key\n\n", -ret);
      mbedtls_free_resource(sock);
      return NET_ERROR_MBEDTLS_KEY_PARSE;
    }
  }

  /* TLS Connection */
  if( (ret = mbedtls_ssl_config_defaults(&tlsData->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
  {
    NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_config_defaults returned -0x%lx\n\n", -ret);
    mbedtls_free_resource(sock);
    return NET_ERROR_MBEDTLS_CONFIG;
  }

  /* Allow the user to select a TLS profile? */
  if(tlsData->tls_cert_prof != NULL)
  {
    mbedtls_ssl_conf_cert_profile(&tlsData->conf, tlsData->tls_cert_prof);
  }

  /* Only for debug
   * mbedtls_ssl_conf_verify(&(tlsDataParams->conf), _iot_tls_verify_cert, NULL); */
  if (tlsData->tls_srv_verification == true)
  {
    mbedtls_ssl_conf_authmode(&tlsData->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
  }
  else
  {
    mbedtls_ssl_conf_authmode(&tlsData->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
  }

  // no verification because no certificat
  if (tlsData->tls_ca_certs == NULL)
  {
   //  mbedtls_ssl_conf_authmode(&tlsData->conf, MBEDTLS_SSL_VERIFY_NONE);
  }

  mbedtls_ssl_conf_rng(&tlsData->conf, mbedtls_ctr_drbg_random, &tlsData->ctr_drbg);
  mbedtls_ssl_conf_ca_chain(&tlsData->conf, &tlsData->cacert, NULL);

  if( (tlsData->tls_dev_cert != NULL) && (tlsData->tls_dev_key != NULL) )
  {
    if( (ret = mbedtls_ssl_conf_own_cert(&tlsData->conf, &tlsData->clicert, &tlsData->pkey)) != 0)
    {
      NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_conf_own_cert returned -0x%lx\n\n", -ret);
      mbedtls_free_resource(sock);
      return NET_ERROR_MBEDTLS_CONFIG;
    }
  }

  if( (ret = mbedtls_ssl_setup(&tlsData->ssl, &tlsData->conf)) != 0 )
  {
    NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_setup returned -0x%lx\n\n", -ret);
    mbedtls_free_resource(sock);
    return NET_ERROR_MBEDTLS_SSL_SETUP;
  }
  if(tlsData->tls_srv_name != NULL)
  {
    if( (ret = mbedtls_ssl_set_hostname(&tlsData->ssl, tlsData->tls_srv_name)) != 0 )
    {
      NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_set_hostname returned %ld\n\n", ret);
      mbedtls_free_resource(sock);
      return NET_ERROR_MBEDTLS_SET_HOSTNAME;
    }
  }

  mbedtls_ssl_set_bio(&tlsData->ssl, (void *) sock, mbedtls_net_send, NULL, mbedtls_net_recv);
  mbedtls_ssl_conf_read_timeout(&tlsData->conf, sock->read_timeout);


  NET_DBG_INFO("\n\nSSL state connect : %d ", sock->tlsData->ssl.state);


  NET_DBG_INFO("\n\nSSL state connect : %d ", sock->tlsData->ssl.state);
  NET_DBG_INFO("  . Performing the SSL/TLS handshake...");

  while( (ret = mbedtls_ssl_handshake(&tlsData->ssl)) != 0 )
  {
    if( (ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE) )
    {
      if( (tlsData->flags = mbedtls_ssl_get_verify_result(&tlsData->ssl)) != 0 )
      {
        char vrfy_buf[512];
        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", tlsData->flags);
        if (tlsData->tls_srv_verification == true)
        {
          NET_DBG_ERROR("Server verification:\n%s\n", vrfy_buf);
        }
        else
        {
          NET_DBG_INFO("Server verification:\n%s\n", vrfy_buf);
        }
      }
      NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_handshake returned -0x%lx\n", -ret);

      mbedtls_free_resource(sock);
      return (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) ? NET_ERROR_MBEDTLS_REMOTE_AUTH : NET_ERROR_MBEDTLS_CONNECT;
    }
  }

  NET_DBG_INFO(" ok\n    [ Protocol is %s ]\n    [ Ciphersuite is %s ]\n",
  mbedtls_ssl_get_version(&sock->tlsData->ssl),
  mbedtls_ssl_get_ciphersuite(&sock->tlsData->ssl));

  if( (ret = mbedtls_ssl_get_record_expansion(&tlsData->ssl)) >= 0)
  {
    NET_DBG_INFO("    [ Record expansion is %d ]\n", ret);
  }
  else
  {
    NET_DBG_INFO("    [ Record expansion is unknown (compression) ]\n");
  }

  NET_DBG_INFO("  . Verifying peer X.509 certificate...");


#ifdef NET_DBG_INFO
#define NET_CERTIFICATE_DISPLAY_LEN     2048
  if (mbedtls_ssl_get_peer_cert(&sock->tlsData->ssl) != NULL)
  {
    char        *buf = net_malloc(NET_CERTIFICATE_DISPLAY_LEN);
    if (buf)
    {
      NET_DBG_INFO("  . Peer certificate information    ...\n");
      mbedtls_x509_crt_info((char *) buf, NET_CERTIFICATE_DISPLAY_LEN-1, "      ", mbedtls_ssl_get_peer_cert(&sock->tlsData->ssl));
      NET_DBG_INFO("%s\n", buf);
    }
    else
    {
            NET_DBG_INFO("  . Cannot alocate memory to display certificate information    ...\n");
    }
    net_free(buf);
  }
#endif
  return NET_OK;

}


int net_mbedtls_sock_recv(net_socket_t *sock, uint8_t * buf, size_t len)
{
  net_tls_data_t * tlsData = sock->tlsData;
  int ret = 0;

  ret = mbedtls_ssl_read(&tlsData->ssl, buf , len);
  if (ret <= 0)
  {
    switch (ret)
    {
      case 0:
        ret = NET_ERROR_DISCONNECTED;
        break;

      case MBEDTLS_ERR_SSL_TIMEOUT: /* In case a blocking read function was passed through mbedtls_ssl_set_bio() */
      case MBEDTLS_ERR_SSL_WANT_READ:
        ret = NET_TIMEOUT;
        break;

      default:
        NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_read returned -0x%x\n\n", -ret);
        ret = NET_ERROR_MBEDTLS;
    }
  }
  return ret;
}


int net_mbedtls_sock_send( net_socket_t *sock, const uint8_t * buf, size_t len)
{
  int ret = 0;
  net_tls_data_t * tlsData = sock->tlsData;

  ret = mbedtls_ssl_write(&tlsData->ssl, buf, len );
  if (ret == 0)
  {
    ret = NET_ERROR_DISCONNECTED;
  }
  else if (ret < 0)
  {
    NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_write returned -0x%x\n\n", -ret);
    ret = NET_ERROR_MBEDTLS;
  }

  return (ret);
}


int net_mbedtls_stop(net_socket_t *sock)
{
  int ret = 0;
  /* Closure notification is required by TLS if the session was not already closed by the remote host. */
  net_tls_data_t * tlsData = sock->tlsData;
  do
  {
    ret = mbedtls_ssl_close_notify(&tlsData->ssl);
  }
  while ( (ret == MBEDTLS_ERR_SSL_WANT_WRITE) || (ret == MBEDTLS_ERR_SSL_WANT_READ) );

  /* All other negative return values indicate connection needs to be reset.
   * No further action required since this is disconnect call */

  mbedtls_free_resource(sock);
  return NET_OK;
}


static void mbedtls_free_resource(net_socket_t *sock)
{
  net_tls_data_t * tlsData = sock->tlsData;

  mbedtls_x509_crt_free(&tlsData->clicert);
  mbedtls_pk_free(&tlsData->pkey);
  mbedtls_x509_crt_free(&tlsData->cacert);
  mbedtls_ssl_free(&tlsData->ssl);
  mbedtls_ssl_config_free(&tlsData->conf);
  mbedtls_ctr_drbg_free(&tlsData->ctr_drbg);
  mbedtls_entropy_free(&tlsData->entropy);
  net_free(tlsData);
  sock->tlsData = 0;
  return;
}



/* received interface implementation.*/
static int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
  int ret = 0;
  net_socket_t  *pSocket=(net_socket_t*) ctx;

  if (pSocket->read_timeout != timeout)
  {
    ret = pSocket->pnetif->pdrv->setsockopt(pSocket->ulsocket, NET_SOL_SOCKET, NET_SO_RCVTIMEO,&timeout, sizeof(uint32_t));
    if (ret != NET_OK) return ret;
    pSocket->read_timeout = timeout;
  }

  UNLOCK_SOCK(pSocket->idx);
  ret = pSocket->pnetif->pdrv->recv(pSocket->ulsocket, buf, len, 0);
  LOCK_SOCK(pSocket->idx);
  if (ret > 0)
  {
      return ret;
  }
  else
  {
      switch(ret)
      {
        case 0:
          return MBEDTLS_ERR_SSL_WANT_READ;
        case NET_TIMEOUT:
          /* According to mbedtls headers, MBEDTLS_ERR_SSL_TIMEOUT should be returned. */
          /* But it saturates the error log with false errors. By contrast, MBEDTLS_ERR_SSL_WANT_READ does not raise any error. */
          return MBEDTLS_ERR_SSL_WANT_READ;
        default:
          ;
      }
  }

  NET_DBG_ERROR("mbedtls_net_recv() : error %d in recv() - requestedLen=%d\n", ret, len);
  return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
}


static int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
  net_socket_t  *pSocket=(net_socket_t*) ctx;
  int ret;

  UNLOCK_SOCK(pSocket->idx);
  ret = pSocket->pnetif->pdrv->send(pSocket->ulsocket, (unsigned char*) buf, len, 0);
  LOCK_SOCK(pSocket->idx);
  if(ret > 0)
  {
	 return ret;
  }

  if(ret == 0)
  {
    return  MBEDTLS_ERR_SSL_WANT_WRITE;
  }
  NET_DBG_ERROR("mbedtls_net_send(): error %d in send() - requestedLen=%d\n", ret, len);

  /* TODO: The underlying layers do not allow to distinguish between
   *          MBEDTLS_ERR_SSL_INTERNAL_ERROR,
   *          MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY,
   *          MBEDTLS_ERR_SSL_CONN_EOF.
   *  Most often, the error is due to the closure of the connection by the remote host. */

  return MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY;

}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
