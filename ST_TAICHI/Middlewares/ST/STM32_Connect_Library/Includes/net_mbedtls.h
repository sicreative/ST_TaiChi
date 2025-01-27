/**
  ******************************************************************************
  * @file    net_mbedtls.h
  * @author  MCD Application Team
  * @brief   Header for the network TLS functions.
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
#ifndef MBEDTLS_NET_H
#define MBEDTLS_NET_H

/* Includes ------------------------------------------------------------------*/
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"



/* Private defines -----------------------------------------------------------*/

struct net_tls_data {
  unsigned char * tls_ca_certs; /**< Socket option. */
  unsigned char * tls_ca_crl;   /**< Socket option. */
  unsigned char * tls_dev_cert; /**< Socket option. */
  unsigned char * tls_dev_key;  /**< Socket option. */
  uint8_t * tls_dev_pwd;        /**< Socket option. */
  size_t tls_dev_pwd_len;       /**< Socket option / meta. */
  bool tls_srv_verification;    /**< Socket option. */
  char * tls_srv_name;          /**< Socket option. */
  /* mbedTLS objects */
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  uint32_t flags;
  mbedtls_x509_crt cacert;
  mbedtls_x509_crt clicert;
  mbedtls_pk_context pkey;
  mbedtls_x509_crt_profile * tls_cert_prof; /**< Socket option. */
} ;

void net_tls_init(void);
void net_tls_destroy(void);

int net_mbedtls_start(net_socket_t *sockhnd );
int net_mbedtls_stop(net_socket_t *sockhnd);
int net_mbedtls_sock_recv(net_socket_t *sockhnd, uint8_t * buf, size_t len);
int net_mbedtls_sock_send( net_socket_t *sockhnd, const uint8_t * buf, size_t len);
bool net_mbedtls_check_tlsdata(net_socket_t *sockhnd);
void net_mbedtls_set_read_timeout(net_socket_t *sock);

extern int mbedtls_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen );


#endif /* MBEDTLS_NET_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
