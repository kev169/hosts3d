/* objects.h - 26 Jul 10
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

#include <sys/time.h>  //timeval

#include "common.h"
#include "misc.h"

#define HSD_EDTNAME  0x01  //edit hostname
#define HSD_EDTRMKS  0x02  //edit host remarks
#define HSD_EXPTCSV  0x03  //export CSV
#define HSD_FNDHSTS  0x04  //find hosts
#define HSD_HNLOPEN  0x05  //open network layout file
#define HSD_HNLSAVE  0x06  //save network layout file
#define HSD_HPTOPEN  0x07  //open packet traffic file
#define HSD_HPTSAVE  0x08  //save packet traffic file
#define HSD_HSTINFO  0x09  //host info
#define HSD_MAKEHST  0x0A  //make host
#define HSD_NETPOS   0x0B  //edit netpos
#define HSD_PKTSPRO  0x0C  //packets protocol
#define HSD_PKTSPRT  0x0D  //packets port
#define HSD_SETCMDS  0x0E  //set commands
#define HSD_SLINACT  0x0F  //select inactive
#ifndef __MINGW32__
#define HSD_HSENRUN  0x10  //run hsen
#define HSD_HSENSTP  0x11  //stop hsen
#endif

const int SPC = 32, MOV = 3, HPR = 20, SELBUF = 50000, SVCS = 32;  //host spacing, movement spacing, hosts per row, host selection buffer, services per host
const double DEPTH = 8192.0;  //perseptive depth
const char tipn[3][6] = {"IP", "IP/Nm", "Nm"}, tips[3][4] = {"Off", "Sel", "All"}, tona[5][6] = {"Off", "Alert", "IP/Nm", "Host", "Sel"}
  , tclr[10][8] = {"default", "orange", "yellow", "fluro", "green", "mint", "aqua", "blue", "purple", "violet"};
const GLubyte bitmaps[17][8] =
{
  { 31,  31,  31,  15,  15,   7,   3,   0},  //posse body 1
  {248, 248, 248, 240, 240, 224, 192,   0},  //posse body 2
  { 15,   7,   3,   3,   7,  15,  15,  31},  //posse body 3
  {240, 224, 192, 192, 224, 240, 240, 248},  //posse body 4
  {111, 239, 239, 239, 255, 255, 127, 127},  //posse body 5
  {246, 247, 247, 247, 255, 255, 254, 254},  //posse body 6
  { 47,  47,  47,  47,  47, 111, 111, 111},  //posse body 7
  {244, 244, 244, 244, 244, 246, 246, 246},  //posse body 8
  { 32,  32,  32,  16,  16,   8,   4,   3},  //posse edge 1
  {  4,   4,   4,   8,   8,  16,  32, 192},  //posse edge 2
  {112,   8,   4,   4,   8,  16,  16,  32},  //posse edge 3
  { 14,  16,  32,  32,  16,   8,   8,   4},  //posse edge 4
  { 72, 136, 136, 136, 128, 128,  64,  64},  //posse edge 5
  { 18,  17,  17,  17,   1,   1,   2,   2},  //posse edge 6
  { 80,  80,  80,  80,  80, 144, 144, 144},  //posse edge 7
  { 10,  10,  10,  10,  10,   9,   9,   9},  //posse edge 8
  {254, 254, 254, 254, 254, 254, 254,   0}   //box
};

//GL position
struct pos_type
{
  double x, y, z;
};

//GL vertex
struct vtx_type
{
  float x, y, z;
};

//GL triangle
struct tri_type
{
  unsigned char a, b, c;
};

//GL square
struct sqr_type
{
  unsigned char a, b, c, d;
};

//GL view direction
struct view_type
{
  double ax, ay;  //angle
  pos_type ee, dr;  //eye, direction
};

//GL cross object
struct cobj_type
{
  vtx_type vtx[28];
  sqr_type sqr[8];
};

//GL host object
struct hobj_type
{
  vtx_type vtx[10];
  tri_type tri[16];
};

//GL multiple-hosts object
struct mobj_type
{
  vtx_type vtx[11];
  tri_type tri[16];
};

//GL packet object
struct pobj_type
{
  vtx_type vtx[8];
  tri_type tri[12];
};

//host object info
struct host_type
{
  unsigned char lsn, clr, vis, sld:1, pip:1, anm:1, shp:1, lck:1, alk:1, spr:2;
    //last sensor, colour, visible, selected, persistant IP, anomaly, show packets, lock, auto link, spare (future use)
  int px, py, pz;
  unsigned long long dld, uld;  //downloads, uploads
  time_t lpk;  //last packet
  host_type *col;
  in_addr hip;
  char htip[16], htmc[18], htnm[256], htrm[256];  //IP, MAC, name, remarks
  int svc[SVCS];  //service
};

//link object info
struct link_type
{
  host_type *sht, *dht;  //source, destination host
  unsigned char spr;  //spare (future use)
};

enum aobj_type { anom, actv, text };  //alert object - anomaly, active, text

//alert object info
struct alrt_type
{
  aobj_type ao;  //alert object
  unsigned char pr;
  unsigned short sz, frm;  //size/duration, frame
  host_type *ht;
};

//packet object info
struct pckt_type
{
  unsigned char pr;
  unsigned short dstpt, frm;  //frame
  pos_type cu;  //current
  host_type *ht;  //destination host
};

enum ptrc_type { stp, hlt, rec, rpy };  //packet traffic - stop, halt, record, replay

//packet traffic record
struct pkrc_type
{
  timeval ptime;  //time of packet
  pkif_type pk;  //packet
};

enum sipn_type { ips, ins, nms };  //display - IP, IP/name, name only
enum sips_type { off, sel, all };  //IP/name display - off, selection, all
enum sona_type { don, alt, ipn, hst, slt };  //on-active - do nothing, alert, IP/name, host, select

//hosts3d settings
struct sett_type
{
  bool hspm;  //start hsen promiscuous mode
  unsigned char sen, pr, mvd;  //display packets from hsen (0 for all), display packets by protocol (0 for all), master visibility duration
  unsigned short prt, osd:1, bct:1, fsp:1, adh:1, anm:1, spe:1, nhl:1, nhp:1, pdp:1, spr:7;  //display packets by port (0 for all)
    //OSD, broadcasts, fast packets, add dest hosts, anomaly detect, spare, new host link, new host packets, packet dest port, spare (future use)
  unsigned int pks;  //number of packets allowed on screen before drop
  char clip[256], hsst[128], hsid[4], hsif[256], hshd[16], hssp[256], cmd[4][256];  //input text clipboard, start/stop hsen, commands
  sipn_type sipn;  //display IP/name
  sips_type sips;  //IP/name display
  sona_type sona;  //on-active
  view_type vws[5];
};

//net position/colour
struct netp_type
{
  int px, py, pz;
  char clr, nt[19];  //colour, CIDR net
};

//draw cross object
void cobjDraw();

//draw host object
void hobjDraw(int r, int g, int b);

//draw multiple-hosts object
void mobjDraw(bool sel);

//draw packet object
void pobjDraw(int c);

//draw alert object
void aobjDraw(bool an);

//return path with hosts3d data directory
char *hsddata(const char *fl);

//add service to host
void svcAdd(host_type *sht, int svc, bool anm);

//load netpos file into memory, create empty
void netpsLoad();

//destroy net positions LL
void netpsDestroy();

//check if an IP address is a broadcast address
in_addr_t isBroadcast(in_addr ip);

//check if a host is to be positioned into a net
int hostNet(host_type *ht);

//copy packet traffic file from/to
void pkrcCopy(const char *ifn, const char *ofn);

//check for controls file, create it
void checkControls();
