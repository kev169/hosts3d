/* hsen.cpp - 10 May 11
   Hosts3D - 3D Real-Time Network Monitor
   Copyright (c) 2006-2011  Del Castle

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details. */

#include <pcap.h>
#include <signal.h>  //signal()
#include <stdlib.h>  //atoi()
#include <string.h>  //memcpy()
#ifdef __FreeBSD__
#include <sys/socket.h>
#include <sys/sysctl.h>
#endif
#ifdef __MINGW32__
#include <winsock2.h>
#else
#include <arpa/inet.h>  //inet_addr()
#include <sys/stat.h>  //umask()
#include <syslog.h>  //syslog()
#include <unistd.h>  //close(), fork(), setsid(), usleep()
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "proto.h"
#include "misc.h"

const int pesz = sizeof(pkex_type), dnsz = sizeof(pdns_type);
const unsigned int ehsz = sizeof(ether_header), ahsz = ehsz + sizeof(ether_arp), shsz = sizeof(sll_header), phsz = sizeof(ppp_header)
  , ihsz = sizeof(iphdr), xhsz = ihsz + sizeof(gre_hdr), ghsz = ihsz + xhsz, thsz = ihsz + sizeof(tcphdr), uhsz = ihsz + sizeof(udphdr)
  , dhsz = uhsz + sizeof(HEADER), dtsz = dhsz + sizeof(dns_tail);

bool rpy = false;
#ifdef __MINGW32__
int pdl;
SOCKET hsock;  //hosts3d UDP socket
#else
int pdl, hsock;  //hosts3d UDP socket
#endif
sockaddr_in hadr;  //destination IP/broadcast address
unsigned int hasz = sizeof(hadr);
unsigned long long nowtm, reltm, rpoff = 0;  //pcap replay time offset
pcap_t *pcap;

#ifdef __MINGW32__
#define usleep(usec) (Sleep((usec) / 1000))
#endif

void hsenStop(int sig)
{
#ifndef __MINGW32__
  syslog(LOG_INFO, "stopping...\n");
#endif
  pcap_breakloop(pcap);
}

//pcap_loop
void pktProcess(u_char *u, const struct pcap_pkthdr *hdr, const u_char *pkt)
{
  if (rpy)  //pcap replay
  {
    nowtm = microTime(0);
    if (!rpoff) rpoff = nowtm - microTime(&hdr->ts);
    reltm = microTime(&hdr->ts) + rpoff;
    if (reltm > nowtm) usleep(reltm - nowtm);
  }
#ifdef __MINGW32__
  pkif_type pkif = {0, 0, {{{0, 0, 0, 0}}}, {{{0, 0, 0, 0}}}, *u, 0};
#else
  pkif_type pkif = {0, 0, {0}, {0}, *u, 0};
#endif
  pkex_type pkex = {85, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0}};  //85 used to identify packet info
  pkex.pk = pkif;
  bool arp = false;
  unsigned int clen = hdr->caplen;
  if (clen < ihsz) return;  //malformed packet check
  if (pdl == DLT_EN10MB)
  {
    ether_header *ehdr = (ether_header *)pkt;
    memcpy(&pkex.srcmc, &ehdr->ether_shost, 6);
    if (ntohs(ehdr->ether_type) == ETHERTYPE_VLAN)  //802.1Q VLAN
    {
      pkt += 4;  //sizeof 802.1Q VLAN header
      clen -= 4;
      ehdr = (ether_header *)pkt;
    }
    if (ntohs(ehdr->ether_type) == ETHERTYPE_IP)
    {
      pkt += ehsz;
      clen -= ehsz;
    }
    else if (ntohs(ehdr->ether_type) == ETHERTYPE_ARP)
    {
      if (clen < ahsz) return;  //malformed packet check
      ether_arp *ahdr = (ether_arp *)(pkt + ehsz);
      pkex.pk.pr = IPPROTO_ARP;
      memcpy(&pkex.pk.srcip, &ahdr->arp_spa, 4);
      memcpy(&pkex.pk.dstip, &ahdr->arp_tpa, 4);
      arp = true;
    }
    else return;
  }
  else if (pdl == DLT_LINUX_SLL)
  {
    sll_header *shdr = (sll_header *)pkt;
    if (ntohs(shdr->sll_halen) == 6) memcpy(&pkex.srcmc, &shdr->sll_addr, 6);
    if (ntohs(shdr->sll_protocol) == ETHERTYPE_IP)
    {
      pkt += shsz;
      clen -= shsz;
    }
    else return;
  }
  else if (pdl == DLT_PPP)
  {
    ppp_header *phdr = (ppp_header *)pkt;
    if (ntohs(phdr->ppp_protocol) == 0x21)  //IPv4
    {
      pkt += phsz;
      clen -= phsz;
    }
    else return;
  }
  if (!arp)
  {
    if (clen < ihsz) return;  //malformed packet check
    iphdr *ihdr = (iphdr *)pkt;
    if (ihdr->version != 4) return;  //not IPv4
    if (ihdr->ihl != 5) return;  //IPv4 options check
    unsigned short frag = ntohs(ihdr->frag_off) << 3;
    if (ihdr->protocol == IPPROTO_GRE)  //generic routing encapsulation
    {
      if (frag) return;  //fragmented packet check
      if (clen < ghsz) return;  //malformed packet check
      gre_hdr *ghdr = (gre_hdr *)(pkt + ihsz);
      if (ghdr->crks) return;  //GRE options check
      if (ntohs(ghdr->type) != ETHERTYPE_IP) return;  //not IP
      pkt += xhsz;
      clen -= xhsz;
      ihdr = (iphdr *)pkt;
      if (ihdr->version != 4) return;  //not IPv4
      if (ihdr->ihl != 5) return;  //IPv4 options check
      frag = ntohs(ihdr->frag_off) << 3;
    }
    pkex.sz = ntohs(ihdr->tot_len);
    pkex.pk.pr = ihdr->protocol;
    pkex.pk.srcip.s_addr = ihdr->saddr;
    pkex.pk.dstip.s_addr = ihdr->daddr;
    if (frag) pkex.pk.pr = IPPROTO_FRAG;
    else if (pkex.pk.pr == IPPROTO_TCP)
    {
      if (clen >= thsz)
      {
        tcphdr *thdr = (tcphdr *)(pkt + ihsz);
        pkex.pk.srcpt = ntohs(thdr->source);
        pkex.pk.dstpt = ntohs(thdr->dest);
        pkex.syn = thdr->syn;
        pkex.ack = thdr->ack;
      }
      else pkex.pk.pr = IPPROTO_FRAG;
    }
    else if (pkex.pk.pr == IPPROTO_UDP)
    {
      if (clen >= uhsz)
      {
        udphdr *uhdr = (udphdr *)(pkt + ihsz);
        if (hadr.sin_port == uhdr->dest) return;  //ignore hsen to hosts3d packets
        pkex.pk.srcpt = ntohs(uhdr->source);
        pkex.pk.dstpt = ntohs(uhdr->dest);
        udp_type udpt = udpTrack(&pkex.pk);
        if (udpt == src) pkex.ack = 1;  //ACK used to identify service
        if ((pkex.pk.srcpt == 53) && (clen > dtsz))  //port 53 DNS, malformed packet check
        {
          HEADER *dhdr = (HEADER *)(pkt + uhsz);
          if ((dhdr->qr == 1) && !dhdr->opcode && !dhdr->rcode && (ntohs(dhdr->qdcount) == 1) && (ntohs(dhdr->ancount) == 1))
          {
#ifdef __MINGW32__
            pdns_type pdns = {42, "", {{{0, 0, 0, 0}}}};  //42 used to identify DNS info
#else
            pdns_type pdns = {42, "", {0}};  //42 used to identify DNS info
#endif
            char *dnsdata = (char *)(pkt + dhsz);
            addrGet(dnsdata, pdns.htnm, &pdns.dnsip, clen - dtsz);
            if (pdns.dnsip.s_addr)
            {
#ifdef __MINGW32__
              if (sendto(hsock, (const char *)&pdns, dnsz, 0, (sockaddr *)&hadr, hasz) != dnsz)  //send DNS info to hosts3d
              {
#else
              if (sendto(hsock, &pdns, dnsz, 0, (sockaddr *)&hadr, hasz) != dnsz)  //send DNS info to hosts3d
              {
                syslog(LOG_ERR, "socket send failed (dns info)\n");
#endif
                hsenStop(0);
              }
            }
          }
        }
      }
      else pkex.pk.pr = IPPROTO_FRAG;
    }
  }
#ifdef __MINGW32__
  if (sendto(hsock, (const char *)&pkex, pesz, 0, (sockaddr *)&hadr, hasz) != pesz)  //send packet info to hosts3d
  {
#else
  if (sendto(hsock, &pkex, pesz, 0, (sockaddr *)&hadr, hasz) != pesz)  //send packet info to hosts3d
  {
    syslog(LOG_ERR, "socket send failed (pkt info)\n");
#endif
    hsenStop(0);
  }
}

int main(int argc, char *argv[])
{
  char errbuf[PCAP_ERRBUF_SIZE];
  if (argc > 1)
  {
    if (!strcmp(argv[argc - 1], "-d"))
    {
#ifdef __MINGW32__
      pcap_if_t *dev;
      if (pcap_findalldevs(&dev, errbuf) == -1) fprintf(stderr, "hsen error: pcap_findalldevs() failed- %s\n", errbuf);
      else while (dev)
      {
        fprintf(stdout, "%s", dev->name);
        if (dev->description) fprintf(stdout, "  :%s", dev->description);
        fprintf(stdout, "\n");
        dev = dev->next;
      }
      return 0;
#else
      pid_t pid = fork();
      if (pid == -1)
      {
        fprintf(stderr, "hsen error: fork() failed\n");
        return 1;
      }
      if (pid > 0) return 0;
      pid_t sid = setsid();
      if (sid == -1)
      {
        fprintf(stderr, "hsen error: setsid() failed\n");
        return 1;
      }
      umask(0);
      argc--;
#endif
    }
  }
  int promisc = 0;
  if (argc > 1)
  {
    if (!strcmp(argv[argc - 1], "-p"))
    {
      promisc = 1;
      argc--;
    }
  }
  if ((argc < 3) || (argc > 4) || !strcmp(argv[argc - 1], "-d"))
  {
#ifdef __MINGW32__
    fprintf(stderr, "hsen 1.15 usage: %s [-d] <id> <interface/file> [<destination>] [-p]\n"
      "  -d : display interfaces\n"
#else
    fprintf(stderr, "hsen 1.15 usage: %s <id> <interface/file> [<destination>] [-p] [-d]\n"
#endif
      "  id : identify packets from a specific hsen when multiple exist (1-255)\n"
#ifdef __MINGW32__
      "  interface : listen on interface, or\n"
      "  file : read packets from pcap file\n"
#else
      "  interface : listen on interface (eth0, eth1, ppp0, wlan0, etc.), or\n"
      "  file : read packets from pcap file, '-' for stdin\n"
#endif
      "  destination : hosts3d ip or broadcast address (default localhost)\n"
      "  -p : enable promiscuous mode (default disable)\n"
#ifndef __MINGW32__
      "  -d : run as daemon (default disable)\n"
#endif
      , argv[0]);
    return 1;
  }
  if ((atoi(argv[1]) < 1) || (atoi(argv[1]) > 255))
  {
    fprintf(stderr, "hsen error: id out of range (1-255)\n");
    return 1;
  }
#ifdef __MINGW32__
  WORD wVersionRequested = MAKEWORD(2, 0);  //WSAStartup parameter
  WSADATA wsaData;  //WSAStartup parameter
  WSAStartup(wVersionRequested, &wsaData);  //initiate use of the Winsock DLL
#else
  syslog(LOG_INFO, "started\n");
#endif
  unsigned char sen = atoi(argv[1]);
  hadr.sin_family = AF_INET;
  hadr.sin_addr.s_addr = inet_addr((argc == 4 ? argv[3] : "127.0.0.1"));
  hadr.sin_port = htons(HOST3D_PORT);
#ifdef __MINGW32__
  if ((hsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
#else
  if ((hsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
#endif
  {
    fprintf(stderr, "hsen error: socket create failed\n");
#ifndef __MINGW32__
    syslog(LOG_ERR, "socket create failed\n");
#endif
    return 1;
  }
  int sopt = 1;
#ifdef __MINGW32__
  if (setsockopt(hsock, IPPROTO_UDP, UDP_NOCHECKSUM, (const char *)&sopt, sizeof(sopt)) == SOCKET_ERROR)  //disable UDP checksum
#elif __APPLE__
  if (setsockopt(hsock, IPPROTO_UDP, UDP_NOCKSUM, &sopt, sizeof(sopt)) == -1)  //disable UDP checksum
#elif __FreeBSD__
  int mib[4], sval = 0;
  mib[0] = CTL_NET;
  mib[1] = AF_INET;
  mib[2] = IPPROTO_UDP;
  mib[3] = UDPCTL_CHECKSUM;
  if (sysctl(mib, 4, 0, 0, &sval, sizeof(sval)) == -1)  //disable UDP checksum
#else
  if (setsockopt(hsock, SOL_SOCKET, SO_NO_CHECK, &sopt, sizeof(sopt)) == -1)  //disable UDP checksum
#endif
  {
    fprintf(stderr, "hsen error: socket opt failed- checksum\n");
#ifdef __MINGW32__
    closesocket(hsock);
    WSACleanup();
#else
    syslog(LOG_ERR, "socket opt failed- checksum\n");
    close(hsock);
#endif
    return 1;
  }
#ifdef __MINGW32__
  if (setsockopt(hsock, SOL_SOCKET, SO_BROADCAST, (const char *)&sopt, sizeof(sopt)) == SOCKET_ERROR)  //enable UDP broadcasts
#else
  if (setsockopt(hsock, SOL_SOCKET, SO_BROADCAST, &sopt, sizeof(sopt)) == -1)  //enable UDP broadcasts
#endif
  {
    fprintf(stderr, "hsen error: socket opt failed- broadcast\n");
#ifdef __MINGW32__
    closesocket(hsock);
    WSACleanup();
#else
    syslog(LOG_ERR, "socket opt failed- broadcast\n");
    close(hsock);
#endif
    return 1;
  }
  if (!(pcap = pcap_open_offline(argv[2], errbuf)))  //test for file or interface input
  {
#ifndef __MINGW32__
    syslog(LOG_INFO, "pcap_open_offline() failed- %s, trying pcap_open_live()\n", errbuf);
#endif
    if (!(pcap = pcap_open_live(argv[2], MAXPSZ, promisc, 20, errbuf)))
    {
      fprintf(stderr, "hsen error: pcap_open_live() failed- %s, interface up- %s, r u root?\n", errbuf, argv[2]);
#ifdef __MINGW32__
      closesocket(hsock);
      WSACleanup();
#else
      syslog(LOG_ERR, "pcap_open_live() failed- %s, interface up- %s, r u root?\n", errbuf, argv[2]);
      close(hsock);
#endif
      return 1;
    }
  }
  else if (strcmp(argv[2], "-")) rpy = true;
  pdl = pcap_datalink(pcap);
  if ((pdl != DLT_EN10MB) && (pdl != DLT_PPP) && (pdl != DLT_LINUX_SLL))
  {
    fprintf(stderr, "hsen error: interface not supported- %d\n", pdl);
#ifdef __MINGW32__
    closesocket(hsock);
    WSACleanup();
#else
    syslog(LOG_ERR, "interface not supported- %d\n", pdl);
    close(hsock);
#endif
    return 1;
  }
#ifdef __MINGW32__
  signal(SIGINT, hsenStop);  //capture ctrl+c
#endif
  signal(SIGTERM, hsenStop);  //capture kill
  pcap_loop(pcap, -1, pktProcess, &sen);
  pcap_close(pcap);
#ifdef __MINGW32__
  closesocket(hsock);
  WSACleanup();
#else
  close(hsock);
#endif
  utrkDestroy();  //destroy UDP packet tracking data
#ifndef __MINGW32__
  syslog(LOG_INFO, "stopped\n");
#endif
  return 0;
}
