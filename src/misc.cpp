/* misc.cpp - 13 Apr 11
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

#include <ctype.h>  //tolower()
#include <stdio.h>  //sprintf()
#include <stdlib.h>  //atoi()
#include <string.h>
#include <sys/stat.h>  //struct stat, stat()
#ifndef __MINGW32__
#include <arpa/inet.h>  //inet_addr()
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "common.h"
#include "misc.h"

//add extension to file
void extensionAdd(char *fl, const char *ex)
{
  if ((strlen(fl) < 252) && !strstr(fl, ex)) strcat(fl, ex);
}

//check if file exists
bool fileExists(const char *fl)
{
  struct stat fs;
  return (!stat(fl, &fs));
}

//create directory file list
void filelistCreate(const char* fl, const char *ex)
{
  FILE *flist;
  if ((flist = fopen(fl, "w")))
  {
#ifdef __MINGW32__
    WIN32_FIND_DATA nlist;
    HANDLE nl = FindFirstFile("./*", &nlist);
    if (nl != INVALID_HANDLE_VALUE)
    {
      do {
        if ((nlist.cFileName[0] != '.') && strstr(nlist.cFileName, ex)) fprintf(flist, "%s\n", nlist.cFileName);
      } while (FindNextFile(nl, &nlist));
    }
#else
    dirent **nlist;
    int nl = scandir(".", &nlist, regFile, alphasort);
    for (int cnt = 0; cnt < nl; cnt++)
    {
      if ((nlist[cnt]->d_name[0] != '.') && strstr(nlist[cnt]->d_name, ex)) fprintf(flist, "%s\n", nlist[cnt]->d_name);  //don't show hidden files
      free(nlist[cnt]);
    }
    free(nlist);
#endif
    fclose(flist);
  }
}

//format bytes into units
char *formatBytes(unsigned long long bs, char *fb)
{
  unsigned char cnt;
  float fp = 0.0;
  char uts[7][3] = {"B", "kB", "MB", "GB", "TB", "PB", "EB"};
  for (cnt = 0; (bs > 1023) && (cnt < 6);  fp = (bs % 1024) / 1024.0, bs /= 1024, cnt++);  //nothing
  sprintf(fb, "%.2f %s", bs + fp, uts[cnt]);
  return fb;
}

//check if an IP address is in a CIDR net
bool inNet(char *nt, char *ip)
{
  in_addr_t mask = 0;
  char ntmp[19], *cd;
  strcpy(ntmp, nt);
  if (!(cd = strchr(ntmp, '/'))) return false;
  *cd = '\0';
  if (atoi(++cd)) mask = htonl(0xffffffff << (32 - atoi(cd)));
  return ((inet_addr(ntmp) & mask) == (inet_addr(ip) & mask));
}

//convert timeval to time in microseconds
unsigned long long microTime(const timeval *tv)
{
  if (tv) return ((tv->tv_sec * 1000000) + tv->tv_usec);
  timeval ctm;
  gettimeofday(&ctm, 0);  //current time
  return ((ctm.tv_sec * 1000000) + ctm.tv_usec);
}

//convert timeval to time in milliseconds
unsigned long long milliTime(const timeval *tv)
{
  if (tv) return ((tv->tv_sec * 1000) + (tv->tv_usec / 1000));
  timeval ctm;
  gettimeofday(&ctm, 0);  //current time
  return ((ctm.tv_sec * 1000) + (ctm.tv_usec / 1000));
}

//protocol number to string
char *protoDecode(unsigned char pr, char *pd)
{
  switch (pr)
  {
    case IPPROTO_ICMP: strcpy(pd, "ICMP"); break;
    case IPPROTO_IGMP: strcpy(pd, "IGMP"); break;
    case IPPROTO_TCP: strcpy(pd, "TCP"); break;
    case IPPROTO_UDP: strcpy(pd, "UDP"); break;
    case IPPROTO_ARP: strcpy(pd, "ARP"); break;
    case IPPROTO_FRAG: strcpy(pd, "FRAG"); break;
    default: sprintf(pd, "%u", pr);
  }
  return pd;
}

//escape quotes in CSV strings
void quotecsv(const char *cs, char *ec)
{
  size_t sl;
  for (unsigned int cnt = 0; cnt < strlen(cs); cnt++)
  {
    if (cs[cnt] == '\"') strcat(ec, "\"");
    sl = strlen(ec);
    ec[sl + 1] = '\0';
    ec[sl] = cs[cnt];
  }
}

#ifndef __MINGW32__
//check for regular file
#ifdef __APPLE__
int regFile(dirent *ep)
#else
int regFile(const dirent *ep)
#endif
{
  return (ep->d_type == DT_REG);
}
#endif

//square function
double sqr(double x)
{
  return (x * x);
}

//convert string to lowercase
char *strLower(const char *ms, char *ls)
{
  ls[strlen(ms)] = '\0';
  for (unsigned int cnt = 0; cnt < strlen(ms); cnt++) ls[cnt] = tolower(ms[cnt]);
  return ls;
}
