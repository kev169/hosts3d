/* proto.cpp - 06 Jun 10
   Hosts3D - 3D Real-Time Network Monitor
   Copyright (c) 2006-2010  Del Castle

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details. */

#include <string.h>  //memcmp(), memcpy(), strcat(), strlen(), strncat()
#include <time.h>  //time()

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "proto.h"
#include "llist.h"

const time_t UDPSTO = 30;  //UDP stream time-out seconds

MyLL utrkLL;  //dynamic data struct for UDP packet tracking

//UDP packet tracking and identification
udp_type udpTrack(pkif_type *pi)
{
  utrk_type *tk;
  utrkLL.Start(1);
  while ((tk = (utrk_type *)utrkLL.Read(1)))
  {
    if ((time(0) - tk->ttime) > UDPSTO)
    {
      delete tk;
      utrkLL.Delete(1);
    }
    else if (!memcmp(pi, tk, 12))
    {
      time(&tk->ttime);
      return stm;  //stream
    }
    else if ((pi->srcip.s_addr == tk->dstip.s_addr) && (pi->dstip.s_addr == tk->srcip.s_addr) && (pi->srcpt == tk->dstpt) && (pi->dstpt == tk->srcpt))
    {
      time(&tk->ttime);
      if (tk->st) return stm;  //stream
      tk->st = true;
      return src;  //service
    }
    else utrkLL.Next(1);
  }
  utrk_type utrk;
  utrk.st = false;
  memcpy(&utrk, pi, 12);
  time(&utrk.ttime);
  utrkLL.Write(new utrk_type(utrk));
  return dst;  //destination
}

//destroy UDP packet tracking LL
void utrkDestroy()
{
  utrk_type *tk;
  utrkLL.Start(1);
  while ((tk = (utrk_type *)utrkLL.Read(1)))
  {
    delete tk;
    utrkLL.Next(1);
  }
  utrkLL.Destroy();
}

//get IP address from type A class IN standard query response DNS packet payload
void addrGet(char *data, char *nm, in_addr *ip, unsigned int nlen)
{
  unsigned char lbl;
  while (*data)
  {
    for (lbl = *data; lbl; lbl--)  //3www6google3com0
    {
      if (!--nlen) return;  //out of data
      data++;
      if (strlen(nm) == 255) return;  //buffer overflow check
      strncat(nm, data, 1);
    }
    if (!--nlen) return;  //out of data
    data++;
    if (*data && (strlen(nm) < 254)) strcat(nm, ".");
  }
  data++;
  dns_tail *dntl = (dns_tail *)data;
  if (*nm && (ntohs(dntl->qtype) == 1) && (ntohs(dntl->qclas) == 1) && (ntohs(dntl->type) == 1) && (ntohs(dntl->clas) == 1)) ip->s_addr = dntl->addr.s_addr;
}
