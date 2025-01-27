/**
  ******************************************************************************
  * @file    net_class_extension.c
  * @author  MCD Application Team
  * @brief   Specific class interface function implementation
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



int32_t net_wifi_scan(net_if_handle_t *pnetif,net_wifi_scan_mode_t mode)
{
  int32_t ret = NET_OK;

  pnetif = netif_check(pnetif);
  if (pnetif == NULL)
  {
    NET_DBG_ERROR ("No network interface defined");
    return NET_ERROR_PARAMETER;
  }

  if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_WIFI)
  {
    NET_DBG_ERROR ("Incorrect class interface when calling net_wifi_scan function\n");
    ret = NET_ERROR_PARAMETER;
  }
  if (pnetif->pdrv->extension.wifi->scan(mode) != NET_OK)
  {
   NET_DBG_ERROR ("Error when executing net_wifi_scan function\n");
   ret = NET_ERROR_GENERIC;
  }
  return ret;
}

int net_wifi_get_scan_results(net_if_handle_t *pnetif, net_wifi_scan_results_t *results, uint8_t number)
{
  if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_WIFI)
  {
    NET_DBG_ERROR ("Incorrect class interface when calling net_wifi_get_scan_results function\n");
    return NET_ERROR_PARAMETER;
  }

  return pnetif->pdrv->extension.wifi->get_scan_results(results, number);
}

int32_t net_wifi_set_credentials(net_if_handle_t *pnetif,const net_wifi_credentials_t *credentials)
{
  pnetif->pdrv->extension.wifi->credentials= credentials;
  return NET_OK;
}

int32_t net_wifi_set_powersave(net_if_handle_t *pnetif,const net_wifi_powersave_t *powersave)
{
  int32_t ret = NET_OK;

  pnetif = netif_check(pnetif);
  if (pnetif == NULL)
  {
    NET_DBG_ERROR ("No network interface defined");
    ret =  NET_ERROR_PARAMETER;
  }

  if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_WIFI)
  {
    NET_DBG_ERROR ("Incorrect class interface when calling net_wifi_set_powersave function\n");
    ret = NET_ERROR_PARAMETER;
  }

  pnetif->pdrv->extension.wifi->powersave = powersave;

  return ret;
}

int32_t net_wifi_set_param(net_if_handle_t *pnetif,const net_wifi_param_t param, void *data)
{
  if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_WIFI)
  {
    NET_DBG_ERROR ("Incorrect class interface when calling net_wifi_set_param function\n");
    return NET_ERROR_PARAMETER;
  }
  return pnetif->pdrv->extension.wifi->set_param(param, data);
}


int32_t net_cellular_set_credentials(net_if_handle_t *pnetif,const net_cellular_credentials_t *credentials)
{
  pnetif = netif_check(pnetif);
  if (pnetif == NULL)
  {
    NET_DBG_ERROR ("No network interface defined");
    return NET_ERROR_PARAMETER;
  }

  if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_CELLULAR)
  {
    NET_DBG_ERROR ("Incorrect class interface when calling net_cellular_get_radio_results function\n");
    return NET_ERROR_PARAMETER;
  }
  //FIXME , avoid such copy
  pnetif->pdrv->extension.cellular->credentials = credentials;
  return NET_OK;
}

int32_t net_cellular_get_radio_results(net_if_handle_t *pnetif, net_cellular_radio_results_t *results)
{
  pnetif = netif_check(pnetif);
  if (pnetif == NULL)
  {
    NET_DBG_ERROR ("No network interface defined");
    return NET_ERROR_PARAMETER;
  }

  if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_CELLULAR)
  {
    NET_DBG_ERROR ("Incorrect class interface when calling net_cellular_get_radio_results function\n");
    return NET_ERROR_PARAMETER;
  }
  return pnetif->pdrv->extension.cellular->get_radio_results(results);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
