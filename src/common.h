/* common.h - 21 Jul 09
   Hosts3D - 3D Real-Time Network Monitor
   Copyright (c) 2006-2009  Del Castle

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details. */

#ifdef __MINGW32__
#include <winsock2.h>  //in_addr
#else
#include <netinet/in.h>  //in_addr
#endif

#define	IPPROTO_ARP   249  //protocol 249 unassigned used to identify ARP packet
#define	IPPROTO_FRAG  250  //protocol 250 unassigned used to identify fragmented IP packet

#define HOST3D_PORT  10111  //port 10111 unassigned, used for hsen-to-hosts3d comms

//packet info
#pragma pack(1)
struct pkif_type
{
  unsigned short srcpt, dstpt;
  in_addr srcip, dstip;
  unsigned char sen, pr;
};

//packet extra info
#pragma pack(1)
struct pkex_type
{
  char id, syn:1, ack:1, spr:6;  //spare (future use)
  unsigned int sz;
  unsigned char srcmc[6];
  pkif_type pk;
};

//DNS info
#pragma pack(1)
struct pdns_type
{
  char id, htnm[256];
  in_addr dnsip;
};
