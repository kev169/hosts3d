/* objects.cpp - 10 May 11
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

#include <GL/glfw.h>
#include <stdio.h>
#include <stdlib.h>  //atoi()
#include <string.h>  //strcat(), strcpy()
#include <sys/stat.h>  //mkdir() (linux), struct stat, stat()
#ifndef __MINGW32__
#include <arpa/inet.h>  //inet_addr()
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "objects.h"
#include "llist.h"

#ifdef __MINGW32__
const char HSDDATA[] = "hsd-data/";  //hosts3d data directory (win)
#else
const char HSDDATA[] = ".hosts3d/";  //hosts3d data directory (linux)
#endif

char fullpath[256];
MyLL ntpsLL;  //dynamic data struct for net positions file entries

//cross object vertexs, squares
cobj_type cobj =
{
  {
    { 640.0, 0.0,    4.0},
    { 640.0, 0.0,    8.0},
    {   8.0, 0.0,    8.0},
    {   8.0, 0.0,  640.0},
    {   4.0, 0.0,  640.0},
    {   4.0, 0.0,    4.0},
    {   8.0, 0.0,    4.0},

    {  -4.0, 0.0,    4.0},
    {  -4.0, 0.0,  640.0},
    {  -8.0, 0.0,  640.0},
    {  -8.0, 0.0,    8.0},
    {-640.0, 0.0,    8.0},
    {-640.0, 0.0,    4.0},
    {  -8.0, 0.0,    4.0},

    {  -4.0, 0.0, -640.0},
    {  -4.0, 0.0,   -4.0},
    {  -8.0, 0.0,   -4.0},
    {-640.0, 0.0,   -4.0},
    {-640.0, 0.0,   -8.0},
    {  -8.0, 0.0,   -8.0},
    {  -8.0, 0.0, -640.0},

    {   8.0, 0.0, -640.0},
    {   8.0, 0.0,   -8.0},
    { 640.0, 0.0,   -8.0},
    { 640.0, 0.0,   -4.0},
    {   8.0, 0.0,   -4.0},
    {   4.0, 0.0,   -4.0},
    {   4.0, 0.0, -640.0}
  },
  {
    { 0,  1,  2,  6},
    { 6,  3,  4,  5},
    { 7,  8,  9, 13},
    {13, 10, 11, 12},
    {14, 15, 16, 20},
    {19, 16, 17, 18},
    {21, 25, 26, 27},
    {23, 24, 25, 22}
  }
};

//host object vertexs, triangles
hobj_type hobj =
{
  {
    { 0.0, 10.0,  0.0},
    {-3.0,  8.0,  3.0},
    {-3.0,  8.0, -3.0},
    { 3.0,  8.0, -3.0},
    { 3.0,  8.0,  3.0},
    {-3.0,  2.0,  3.0},
    {-3.0,  2.0, -3.0},
    { 3.0,  2.0, -3.0},
    { 3.0,  2.0,  3.0},
    { 0.0,  0.0,  0.0}
  },
  {
    {1, 0, 4},
    {4, 0, 3},
    {3, 0, 2},
    {2, 0, 1},
    {1, 4, 8},
    {8, 5, 1},
    {4, 3, 7},
    {7, 8, 4},
    {3, 2, 6},
    {6, 7, 3},
    {2, 1, 5},
    {5, 6, 2},
    {5, 8, 9},
    {8, 7, 9},
    {7, 6, 9},
    {6, 5, 9}
  }
};

//multiple-hosts object vertexs, triangles
mobj_type mobj =
{
  {
    { 0.0, 10.0,  0.0},
    {-3.0,  8.0,  3.0},
    {-3.0,  8.0, -3.0},
    { 3.0,  8.0, -3.0},
    { 3.0,  8.0,  3.0},
    { 0.0,  5.0,  0.0},
    {-3.0,  2.0,  3.0},
    {-3.0,  2.0, -3.0},
    { 3.0,  2.0, -3.0},
    { 3.0,  2.0,  3.0},
    { 0.0,  0.0,  0.0}
  },
  {
    {1, 0,  4},
    {4, 0,  3},
    {3, 0,  2},
    {2, 0,  1},
    {1, 4,  5},
    {4, 3,  5},
    {3, 2,  5},
    {2, 1,  5},
    {6, 5,  9},
    {9, 5,  8},
    {8, 5,  7},
    {7, 5,  6},
    {6, 9, 10},
    {9, 8, 10},
    {8, 7, 10},
    {7, 6, 10}
  }
};

//packet object vertexs, triangles
pobj_type pobj =
{
  {
    {-1.0, 6.0,  1.0},
    {-1.0, 6.0, -1.0},
    { 1.0, 6.0, -1.0},
    { 1.0, 6.0,  1.0},
    {-1.0, 4.0,  1.0},
    {-1.0, 4.0, -1.0},
    { 1.0, 4.0, -1.0},
    { 1.0, 4.0,  1.0}
  },
  {
    {0, 1, 2},
    {2, 3, 0},
    {0, 3, 7},
    {7, 4, 0},
    {3, 2, 6},
    {6, 7, 3},
    {2, 1, 5},
    {5, 6, 2},
    {1, 0, 4},
    {4, 5, 1},
    {4, 5, 6},
    {6, 7, 4}
  }
};

//draw cross object
void cobjDraw()
{
  glBegin(GL_QUADS);
  for (unsigned char obj = 0; obj < 8; obj++)
  {
    switch (obj)
    {
      case 0: case 1: glColor3ub(grey[0], grey[1], grey[2]); break;
      case 2: case 3: glColor3ub(blue[0], blue[1], blue[2] - 50); break;
      case 4: case 5: glColor3ub(green[0], green[1] - 50, green[2]); break;
      case 6: case 7: glColor3ub(red[0] - 50, red[1], red[2]); break;
    }
    glVertex3f(cobj.vtx[cobj.sqr[obj].a].x, cobj.vtx[cobj.sqr[obj].a].y, cobj.vtx[cobj.sqr[obj].a].z);
    glVertex3f(cobj.vtx[cobj.sqr[obj].b].x, cobj.vtx[cobj.sqr[obj].b].y, cobj.vtx[cobj.sqr[obj].b].z);
    glVertex3f(cobj.vtx[cobj.sqr[obj].c].x, cobj.vtx[cobj.sqr[obj].c].y, cobj.vtx[cobj.sqr[obj].c].z);
    glVertex3f(cobj.vtx[cobj.sqr[obj].d].x, cobj.vtx[cobj.sqr[obj].d].y, cobj.vtx[cobj.sqr[obj].d].z);
  }
  glEnd();
}

//draw host object
void hobjDraw(int r, int g, int b)
{
  glBegin(GL_TRIANGLES);
  for (unsigned char obj = 0; obj < 16; obj++)
  {
    glColor3ub(r, g, b);
    glVertex3f(hobj.vtx[hobj.tri[obj].a].x, hobj.vtx[hobj.tri[obj].a].y, hobj.vtx[hobj.tri[obj].a].z);
    glColor3ub((r == 50 ? r : r - 50), (g == 50 ? g : g - 50), (b == 50 ? b : b - 50));
    glVertex3f(hobj.vtx[hobj.tri[obj].b].x, hobj.vtx[hobj.tri[obj].b].y, hobj.vtx[hobj.tri[obj].b].z);
    glColor3ub((r == 50 ? r : r - 100), (g == 50 ? g : g - 100), (b == 50 ? b : b - 100));
    glVertex3f(hobj.vtx[hobj.tri[obj].c].x, hobj.vtx[hobj.tri[obj].c].y, hobj.vtx[hobj.tri[obj].c].z);
  }
  glEnd();
}

//draw multiple-hosts object
void mobjDraw(bool sel)
{
  glBegin(GL_TRIANGLES);
  unsigned char obj = 0;
  for (; obj < 8; obj++)
  {
    (sel ? glColor3ub(brred[0], brred[1], brred[2]) : glColor3ub(red[0], red[1], red[2]));
    glVertex3f(mobj.vtx[mobj.tri[obj].a].x, mobj.vtx[mobj.tri[obj].a].y, mobj.vtx[mobj.tri[obj].a].z);
    (sel ? glColor3ub(brred[0] - 50, brred[1], brred[2]) : glColor3ub(yellow[0], yellow[1], yellow[2]));
    glVertex3f(mobj.vtx[mobj.tri[obj].b].x, mobj.vtx[mobj.tri[obj].b].y, mobj.vtx[mobj.tri[obj].b].z);
    (sel ? glColor3ub(brred[0] - 100, brred[1], brred[2]) : glColor3ub(green[0], green[1], green[2]));
    glVertex3f(mobj.vtx[mobj.tri[obj].c].x, mobj.vtx[mobj.tri[obj].c].y, mobj.vtx[mobj.tri[obj].c].z);
  }
  for (; obj < 16; obj++)
  {
    glColor3ub(red[0], red[1], red[2]);
    glVertex3f(mobj.vtx[mobj.tri[obj].a].x, mobj.vtx[mobj.tri[obj].a].y, mobj.vtx[mobj.tri[obj].a].z);
    glColor3ub(yellow[0], yellow[1], yellow[2]);
    glVertex3f(mobj.vtx[mobj.tri[obj].b].x, mobj.vtx[mobj.tri[obj].b].y, mobj.vtx[mobj.tri[obj].b].z);
    glColor3ub(green[0], green[1], green[2]);
    glVertex3f(mobj.vtx[mobj.tri[obj].c].x, mobj.vtx[mobj.tri[obj].c].y, mobj.vtx[mobj.tri[obj].c].z);
  }
  glEnd();
}

//draw packet object
void pobjDraw(int c)
{
  glBegin(GL_TRIANGLES);
  for (unsigned char obj = 0; obj < 12; obj++)
  {
    switch (c)
    {
      case 0: glColor3ub(brgrey[0], brgrey[1], brgrey[2]); break;
      case 1: glColor3ub(red[0], red[1] + 100, red[2] + 100); break;
      case 2: glColor3ub(green[0] + 100, green[1], green[2] + 100); break;
      case 3: glColor3ub(blue[0] + 100, blue[1] + 100, blue[2]); break;
      case 4: glColor3ub(yellow[0], yellow[1], yellow[2] + 100); break;
    }
    glVertex3f(pobj.vtx[pobj.tri[obj].a].x, pobj.vtx[pobj.tri[obj].a].y, pobj.vtx[pobj.tri[obj].a].z);
    switch (c)
    {
      case 0: glColor3ub(grey[0], grey[1], grey[2]); break;
      case 1: glColor3ub(red[0], red[1] + 50, red[2] + 50); break;
      case 2: glColor3ub(green[0] + 50, green[1], green[2] + 50); break;
      case 3: glColor3ub(blue[0] + 50, blue[1] + 50, blue[2]); break;
      case 4: glColor3ub(yellow[0], yellow[1], yellow[2] + 50); break;
    }
    glVertex3f(pobj.vtx[pobj.tri[obj].b].x, pobj.vtx[pobj.tri[obj].b].y, pobj.vtx[pobj.tri[obj].b].z);
    switch (c)
    {
      case 0: glColor3ub(dlgrey[0], dlgrey[1], dlgrey[2]); break;
      case 1: glColor3ub(red[0], red[1], red[2]); break;
      case 2: glColor3ub(green[0], green[1], green[2]); break;
      case 3: glColor3ub(blue[0], blue[1], blue[2]); break;
      case 4: glColor3ub(yellow[0], yellow[1], yellow[2]); break;
    }
    glVertex3f(pobj.vtx[pobj.tri[obj].c].x, pobj.vtx[pobj.tri[obj].c].y, pobj.vtx[pobj.tri[obj].c].z);
  }
  glEnd();
}

//draw alert object
void aobjDraw(bool an)
{
  if (an)
  {
    glBegin(GL_LINE_LOOP);
      glVertex3f(-0.4,  0.7,  0.4);
      glVertex3f(-0.4,  0.7, -0.4);
      glVertex3f(-0.7,  0.0, -0.7);
      glVertex3f(-0.4, -0.7, -0.4);
      glVertex3f(-0.4, -0.7,  0.4);
      glVertex3f(-0.7,  0.0,  0.7);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3f( 0.4,  0.7,  0.4);
      glVertex3f( 0.4,  0.7, -0.4);
      glVertex3f( 0.7,  0.0, -0.7);
      glVertex3f( 0.4, -0.7, -0.4);
      glVertex3f( 0.4, -0.7,  0.4);
      glVertex3f( 0.7,  0.0,  0.7);
    glEnd();
  }
  else
  {
    glBegin(GL_LINE_LOOP);
      glVertex3f(-0.5,  0.5,  0.5);
      glVertex3f(-0.5,  0.5, -0.5);
      glVertex3f( 0.5,  0.5, -0.5);
      glVertex3f( 0.5,  0.5,  0.5);
    glEnd();
    glBegin(GL_LINES);
      glVertex3f(-0.5,  0.5,  0.5);
      glVertex3f(-0.5, -0.5,  0.5);
    glEnd();
    glBegin(GL_LINES);
      glVertex3f(-0.5,  0.5, -0.5);
      glVertex3f(-0.5, -0.5, -0.5);
    glEnd();
    glBegin(GL_LINES);
      glVertex3f( 0.5,  0.5, -0.5);
      glVertex3f( 0.5, -0.5, -0.5);
    glEnd();
    glBegin(GL_LINES);
      glVertex3f( 0.5,  0.5,  0.5);
      glVertex3f( 0.5, -0.5,  0.5);
    glEnd();
    glBegin(GL_LINE_LOOP);
      glVertex3f(-0.5, -0.5,  0.5);
      glVertex3f(-0.5, -0.5, -0.5);
      glVertex3f( 0.5, -0.5, -0.5);
      glVertex3f( 0.5, -0.5,  0.5);
    glEnd();
  }
}

//return path with hosts3d data directory
char *hsddata(const char *fl)
{
  strcpy(fullpath, HSDDATA);
  return strcat(fullpath, fl);
}

//add service to host
void svcAdd(host_type *sht, int svc, bool anm)
{
  for (unsigned char cnt = 0; cnt < SVCS; cnt++)
  {
    if (sht->svc[cnt] == svc) break;
    if (sht->svc[cnt] == -1)
    {
      sht->svc[cnt] = svc;
      if (anm) sht->anm = 1;
      break;
    }
  }
}

//load netpos file into memory, create empty
void netpsLoad()
{
  FILE *npos;
  if ((npos = fopen(hsddata("netpos.txt"), "r")))
  {
    netpsDestroy();
    netp_type ne;
    char ch;
    do {
      if ((ch = getc(npos)) == 'p')
      {
        if (fscanf(npos, "%*s%18s%d%d%d%*c%c", (char *)&ne.nt, &ne.px, &ne.py, &ne.pz, &ne.clr) == 5) ntpsLL.Write(new netp_type(ne));  //pos 123.123.123.123/32 10 0 -10 green
      }
      else while ((ch != '\n') && (ch != EOF)) ch = getc(npos);
    } while (ch != EOF);
    fclose(npos);
  }
  else if ((npos = fopen(hsddata("netpos.txt"), "w")))
  {
    fputs("#pos net x-position y-position z-position colour\n", npos);
    fclose(npos);
  }
}

//destroy net positions LL
void netpsDestroy()
{
  netp_type *ne;
  ntpsLL.Start(1);
  while ((ne = (netp_type *)ntpsLL.Read(1)))
  {
    delete ne;
    ntpsLL.Next(1);
  }
  ntpsLL.Destroy();
}

//check if an IP address is a broadcast address
in_addr_t isBroadcast(in_addr ip)
{
  in_addr_t mask;
  char ntmp[19], *cd;
  netp_type *ne;
  ntpsLL.Start(2);
  while ((ne = (netp_type *)ntpsLL.Read(2)))
  {
    strcpy(ntmp, ne->nt);
    if ((cd = strchr(ntmp, '/')))
    {
      *cd = '\0';
      if (atoi(++cd)) mask = htonl(0xffffffff << (32 - atoi(cd)));
      else mask = 0;
      if ((inet_addr(ntmp) | ~mask) == ip.s_addr) return mask;
    }
    ntpsLL.Next(2);
  }
  return 0;
}

//check if a host is to be positioned into a net
int hostNet(host_type *ht)
{
  netp_type *ne;
  ntpsLL.Start(1);
  while ((ne = (netp_type *)ntpsLL.Read(1)))
  {
    if (inNet(ne->nt, ht->htip))
    {
      switch (ne->clr)
      {
        case 'd': ht->clr = 0; break;  //grey (default) host object
        case 'o': ht->clr = 1; break;  //orange host object
        case 'y': ht->clr = 2; break;  //yellow host object
        case 'f': ht->clr = 3; break;  //fluro host object
        case 'g': ht->clr = 4; break;  //green host object
        case 'm': ht->clr = 5; break;  //mint host object
        case 'a': ht->clr = 6; break;  //aqua host object
        case 'b': ht->clr = 7; break;  //blue host object
        case 'p': ht->clr = 8; break;  //purple host object
        case 'v': ht->clr = 9; break;  //violet host object
      }
      ht->px = ne->px * SPC;
      ht->py = ne->py * SPC;
      ht->pz = ne->pz * SPC;
      if (ne->clr == 'h') return 2;  //hold host object
      return 1;
    }
    ntpsLL.Next(1);
  }
  return 0;
}

//copy packet traffic file from/to
void pkrcCopy(const char *ifn, const char *ofn)
{
  FILE *ifile, *ofile;
  if ((ifile = fopen(ifn, "rb")))
  {
    char hpt[4];
    if (fgets(hpt, 4, ifile))
    {
      if (!strcmp(hpt, "HPT"))
      {
        if ((ofile = fopen(ofn, "wb")))
        {
          fputs("HPT", ofile);
          pkrc_type pkcp;
          size_t pcsz = sizeof(pkcp);
          while (fread(&pkcp, pcsz, 1, ifile) == 1)
          {
            if (fwrite(&pkcp, pcsz, 1, ofile) != 1) break;
          }
          fclose(ofile);
        }
      }
    }
    fclose(ifile);
  }
}

//check for controls file, create it
void checkControls()
{
  FILE *ctls;
  struct stat fs;
#ifdef __MINGW32__
  if (!fileExists(hsddata("controls.txt"))) mkdir(hsddata(""));
#else
  if (!fileExists(hsddata("controls.txt"))) mkdir(hsddata(""), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);  //drwxr-xr-x
#endif
  else if (!stat(hsddata("controls.txt"), &fs))
  {
#ifdef __MINGW32__
    if (fs.st_size == 2253) return;
#else
    if (fs.st_size == 2190) return;
#endif
  }
  if ((ctls = fopen(hsddata("controls.txt"), "w")))
  {
    fputs("Left Mouse Button\tSelect Host\n", ctls);
    fputs("\tClick-and-Drag to Select Multiple Hosts\n", ctls);
    fputs("\tClick Selected Host to Toggle Persistant IP/Name\n", ctls);
    fputs("Middle Mouse Button\tClick-and-Drag to Change View\n", ctls);
    fputs("Right Mouse Button\tShow Menu\n", ctls);
    fputs("Mouse Wheel\tMove Up/Down\n", ctls);
    fputs("Up/Down\tMove Forward/Back\n", ctls);
    fputs("Left/Right\tMove Left/Right\n", ctls);
    fputs("Shift + Up/Down/Left/Right\tMove at Triple Speed\n",ctls);
    fputs("Home\tRecall Home View\n", ctls);  //10
    fputs("Ctrl + Home\tRecall Alternate Home View\n", ctls);
    fputs("Ctrl + F1-F4\tRecall View Position 1-4\n", ctls);
    fputs("Ctrl\tMulti-Select\n", ctls);
    fputs("Ctrl + A\tSelect All Hosts\n", ctls);
    fputs("Ctrl + S\tInvert Selection\n", ctls);
    fputs("Q/E\tMove Selection Up/Down\n", ctls);
    fputs("W/S\tMove Selection Forward/Back\n", ctls);
    fputs("A/D\tMove Selection Left/Right\n", ctls);
    fputs("F\tFind Hosts\n", ctls);
    fputs("Tab\tSelect Next Host in Selection\n", ctls);  //20
    fputs("Ctrl + Tab\tSelect Previous Host in Selection\n", ctls);
    fputs("T\tToggle Persistant IP/Name for Selection\n", ctls);
    fputs("C\tCycle Show IP - IP/Name - Name Only\n", ctls);
    fputs("Ctrl + D\tToggle Add Destination Hosts [D]\n", ctls);
    fputs("M\tMake Host\n", ctls);
    fputs("N\tEdit Hostname for Selected Host\n", ctls);
    fputs("Ctrl + N\tSelect All Named Hosts\n", ctls);
    fputs("R\tEdit Remarks for Selected Host\n", ctls);
    fputs("L\tPress Twice with Different Selected Hosts for Link Line\n", ctls);
    fputs("Ctrl + L\tDelete Link Line (2nd Selected Host, Press L on 1st)\n", ctls);  //30
    fputs("Y\tAutomatic Link Lines for All Hosts\n", ctls);
    fputs("Ctrl + Y\tToggle Automatic Link Lines for New Hosts [L]\n", ctls);
    fputs("J\tAutomatic Link Lines for Selection\n", ctls);
    fputs("Ctrl + J\tStop Automatic Link Lines for Selection\n", ctls);
    fputs("Ctrl + R\tDelete Link Lines for All Hosts\n", ctls);
    fputs("P\tShow Packets for Selection\n", ctls);
    fputs("Ctrl + P\tStop Showing Packets for Selection\n", ctls);
    fputs("U\tShow Packets for All Hosts\n", ctls);
    fputs("Ctrl + U\tToggle Show Packets for New Hosts [P]\n", ctls);
    fputs("F1-F4\tShow Packets from Sensor 1-4\n", ctls);  //40
    fputs("F5\tShow Packets from All Sensors\n", ctls);
    fputs("[  ]\tChange Sensor to Show Packets from\n", ctls);
    fputs("B\tToggle Show Simulated Broadcasts [B]\n", ctls);
    fputs("-  +\tDecrease/Increase Allowed Packets\n", ctls);
    fputs("Ctrl + T\tToggle Show Packet Destination Port\n", ctls);
    fputs("Z\tToggle Double Speed Packets [F]\n", ctls);
    fputs("K\tPackets Off\n", ctls);
    fputs("Page Down\tRecord Packet Traffic\n", ctls);
    fputs("Insert\tReplay Recorded Packet Traffic\n", ctls);
    fputs("Page Up\tSkip to Next Packet during Replay Traffic\n", ctls);  //50
    fputs("End\tStop Record/Replay of Packet Traffic\n", ctls);
    fputs("F7\tOpen Packet Traffic File...\n", ctls);
    fputs("F8\tSave Packet Traffic File As...\n", ctls);
    fputs("Space\tToggle Pause Animation\n", ctls);
    fputs("Ctrl + X\tCut Input Box Text\n", ctls);
    fputs("Ctrl + C\tCopy Input Box Text\n", ctls);
    fputs("Ctrl + V\tPaste Input Box Text\n", ctls);
    fputs("Ctrl + K\tAcknowledge All Anomalies\n", ctls);
    fputs("O\tToggle Show OSD\n", ctls);
    fputs("X\tExport Selection Details in CSV File As...\n", ctls);  //60
    fputs("I\tShow Selected Host Information\n", ctls);
    fputs("G\tShow Selection Information\n", ctls);
    fputs("H\tShow Help\n", ctls);
    fclose(ctls);
  }
}
