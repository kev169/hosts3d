/* proto.h - 25 Jun 09
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

#include "common.h"

#define ETHERTYPE_IP    0x0800
#define ETHERTYPE_ARP   0x0806
#define ETHERTYPE_VLAN  0x8100  //802.1Q VLAN

#ifdef __MINGW32__
#define IPPROTO_GRE  47

#define UDP_NOCHECKSUM  1
#elif __APPLE__
#define UDP_NOCKSUM  0x01
#elif __FreeBSD__
#define UDPCTL_CHECKSUM  1
#endif

const int MAXPSZ = 357;  //max packet size ETH 14 VLAN 4 IP 20 GRE 4 IP 20 UDP 8 DNS HEAD 12 DNS NAME 255 DNS TAIL 20

//ethernet header
#pragma pack(1)
struct ether_header
{
  unsigned char ether_dhost[6], ether_shost[6];
  unsigned short ether_type;
};

//ARP header
struct ether_arp
{
  unsigned short ar_hrd, ar_pro;
  unsigned char ar_hln, ar_pln;
  unsigned short ar_op;
  unsigned char arp_sha[6], arp_spa[4], arp_tha[6], arp_tpa[4];
};

//fake link-layer header
struct sll_header
{
  unsigned short sll_pkttype, sll_hatype, sll_halen;
  unsigned char sll_addr[8];
  unsigned short sll_protocol;
};

//PPP header
struct ppp_header
{
  unsigned char ppp_addr, ppp_ctrl;
  unsigned short ppp_protocol;
};

//IPv4 header
struct iphdr
{
  unsigned char ihl:4, version:4, tos;
  unsigned short tot_len, id, frag_off;
  unsigned char ttl, protocol;
  unsigned short check;
  unsigned int saddr, daddr;
};

//GRE header
#pragma pack(1)
struct gre_hdr
{
  unsigned char crks:4, srec:4, fgvr;
  unsigned short type;
};

//TCP header
struct tcphdr
{
  unsigned short source, dest;
  unsigned int seq, ack_seq;
  unsigned short res1:4, doff:4, fin:1, syn:1, rst:1, psh:1, ack:1, urg:1, res2:2, window, check, urg_ptr;
};

//UDP header
struct udphdr
{
  unsigned short source, dest, len, check;
};

//UDP packet tracking
struct utrk_type
{
  unsigned short srcpt, dstpt;
  in_addr srcip, dstip;
  bool st;
  time_t ttime;
};

//DNS header
struct HEADER
{
  unsigned int id:16, rd:1, tc:1, aa:1, opcode:4, qr:1, rcode:4, cd:1, ad:1, unused:1, ra:1
    , qdcount:16, ancount:16, nscount:16, arcount:16;
};

//DNS tail
#pragma pack(1)
struct dns_tail
{
  unsigned short qtype, qclas, name, type, clas;
  unsigned int ttl;
  unsigned short len;
  in_addr addr;
};

enum udp_type { src, dst, stm };  //UDP packet type - service, destination, stream

//UDP packet tracking and identification
udp_type udpTrack(pkif_type *pi);

//destroy UDP packet tracking LL
void utrkDestroy();

//get IP address from type A class IN standard query response DNS packet payload
void addrGet(char *data, char *nm, in_addr *ip, unsigned int nlen);
