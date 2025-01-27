/**
  ******************************************************************************
  * @file    net_address.c
  * @author  MCD Application Team
  * @brief   Implements network address conversion routines
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

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
char * net_ntoa_r(const net_in_addr_t *addr, char *buf, int buflen)
{
    uint32_t s_addr;
    char inv[3];
    char *rp;
    uint8_t *ap;
    uint8_t rem;
    uint8_t n;
    uint8_t i;
    int len = 0;

    s_addr = S_ADDR(*addr);

    rp = buf;
    ap = (uint8_t *)&s_addr;
    for(n = 0; n < 4; n++) {
        i = 0;
        do {
            rem = *ap % (uint8_t)10;
            *ap /= (uint8_t)10;
            inv[i++] = '0' + rem;
        } while(*ap);
        while(i--) {
            if (len++ >= buflen) {
                return NULL;
            }
            *rp++ = inv[i];
        }
        if (len++ >= buflen) {
            return NULL;
        }
        *rp++ = '.';
        ap++;
    }
    *--rp = 0;
    return buf;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
char * net_ntoa(const net_in_addr_t *addr)
{
  static char str[16];
  return net_ntoa_r(addr, str, 16);
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
int net_aton(int af, const char *cp, net_in_addr_t *addr)
{
    uint32_t val;
    uint8_t base;
    char c;
    uint32_t parts[4];
    uint32_t *pp = parts;

    if (af != NET_AF_INET) return -1;

    c = *cp;
    for (;;) {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, 1-9=decimal.
         */
        if (!net_isdigit(c))
            return (0);
        val = 0;
        base = 10;
        if (c == '0') {
            c = *++cp;
            if (c == 'x' || c == 'X') {
                base = 16;
                c = *++cp;
            } else
                base = 8;
        }
        for (;;) {
            if (net_isdigit(c)) {
                val = (val * base) + (int)(c - '0');
                c = *++cp;
            } else if (base == 16 && net_isxdigit(c)) {
                val = (val << 4) | (int)(c + 10 - (net_islower(c) ? 'a' : 'A'));
                c = *++cp;
            } else
                break;
        }
        if (c == '.') {
            /*
             * Internet format:
             *  a.b.c.d
             *  a.b.c   (with c treated as 16 bits)
             *  a.b (with b treated as 24 bits)
             */
            if (pp >= parts + 3) {
                return (0);
            }
            *pp++ = val;
            c = *++cp;
        } else
            break;
    }
    /*
     * Check for trailing characters.
     */
    if (c != '\0' && !net_isspace(c)) {
        return (0);
    }
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    switch (pp - parts + 1) {

        case 0:
            return (0);       /* initial nondigit */

        case 1:             /* a -- 32 bits */
            break;

        case 2:             /* a.b -- 8.24 bits */
            if (val > 0xffffffUL) {
                return (0);
            }
            val |= parts[0] << 24;
            break;

        case 3:             /* a.b.c -- 8.8.16 bits */
            if (val > 0xffff) {
                return (0);
            }
            val |= (parts[0] << 24) | (parts[1] << 16);
            break;

        case 4:             /* a.b.c.d -- 8.8.8.8 bits */
            if (val > 0xff) {
                return (0);
            }
            val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
            break;
        default:
            break;
    }

    if (addr) {
        S_ADDR(*addr) = net_htonl(val);
    }
    return (1);
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
int net_aton_r(const char *cp)
{
    net_in_addr_t val;

    if (net_aton(NET_AF_INET, cp, &val)) {
        return S_ADDR(val);
    }
    return (0);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
