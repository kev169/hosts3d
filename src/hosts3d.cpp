/* hosts3d.cpp - 10 May 11
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

#include <math.h>  //cos(), sin(), sqrt()
#include <signal.h>  //signal()
#include <stdio.h>
#include <stdlib.h>  //abs(), atoi(), system()
#include <string.h>
#include <time.h>
#ifdef __MINGW32__
#include <getopt.h>  //getopt()
#else
#include <arpa/inet.h>  //inet_addr(), inet_ntoa()
#include <fcntl.h>  //fcntl()
#include <syslog.h>  //syslog()
#include <unistd.h>  //close(), usleep()
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "glwin.h"
#include "objects.h"

FILE *pfile;  //packet traffic record file
bool goRun = true, goSize = true, goAnim = true, mMove = false, mView = false, dAnom = false, refresh = false, animate = false, fullscn = false;
int wWin = 580, hWin = 420, mBxx, mBxy, mPsx = 0, mPsy = 0, mWhl = 0, GLResult[6] = {0, 0, 0, 0, 0, 0};  //2D GUI result identification
time_t atime = 0, distm;  //packet traffic display time offset
ptrc_type ptrc = stp;  //packet traffic state
pkrc_type pkrp;  //packet traffic replay packet
unsigned short frame = 0;
unsigned long long fps = 0, rpoff;  //packet traffic replay time offset
char goHosts = 2, osdtxt[83], htdtls[570];
view_type vwdef[2] = {{0.0, 0.0, {0.0, 75.0, -360.0}, {0.0, 75.0, MOV - 360.0}}, {90.0, 0.0, {-360.0, 75.0, 0.0}, {MOV - 360.0, 75.0, 0.0}}};  //default views
sett_type setts = {false, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 5000, "", "<sudo command> hsen", "1", "eth0", "", "<sudo command> killall hsen"
  , {"", "", "", ""}, ins, off, alt, {vwdef[0], vwdef[0], vwdef[0], vwdef[0], vwdef[0]}};  //settings
host_type *seltd = 0, *lnkht = 0;  //selected host, link line host
MyLL hstsLL, lnksLL, altsLL, pktsLL;  //dynamic data struct for hosts, links, alerts, packets
GLuint objsDraw;  //GL compiled objects
MyGLWin GLWin;  //2D GUI

#ifdef __MINGW32__
#define usleep(usec) (Sleep((usec) / 1000))
#endif

void hsdStop(int sig)
{
  goRun = false;
}

//check if a host is at a position
host_type *hostAtpos(int px, int py, int pz, host_type *pht)
{
  host_type *ht;
  hstsLL.Start(2);
  while ((ht = (host_type *)hstsLL.Read(2)))
  {
    if ((ht != pht) && (ht->px == px) && (ht->py == py) && (ht->pz == pz)) return ht;
    hstsLL.Next(2);
  }
  return 0;
}

//find blank host position
void hostPos(host_type *ht, unsigned char hr, unsigned char sp)
{
  unsigned char hsp = sp * SPC;
  unsigned short hdm = hr * hsp;  //dimensions
  int tpx = ht->px, tpy = ht->py, tpz = ht->pz, spx = tpx, spz = tpz
    , addx = (tpx >= 0 ? hsp : -hsp), addy = (tpy >= 0 ? hsp : -hsp), addz = (tpz >= 0 ? hsp : -hsp);
  while (hostAtpos(tpx, tpy, tpz, ht))
  {
    tpx += addx;
    if (abs(tpx - spx) == hdm)
    {
      tpx = spx;
      tpz += addz;
      if (abs(tpz - spz) == hdm)
      {
        tpz = spz;
        tpy += addy;
      }
    }
  }
  ht->px = tpx;
  ht->py = tpy;
  ht->pz = tpz;
}

//detect host collision on move
void moveCollision(host_type *cht)
{
  host_type *ht;
  if ((ht = cht->col))
  {
    if (ht->col == cht) ht->col = 0;
    else
    {
      while (ht->col != cht) ht = ht->col;
      ht->col = cht->col;
    }
    cht->col = 0;
  }
  if ((ht = hostAtpos(cht->px, cht->py, cht->pz, cht)))
  {
    cht->col = (ht->col ? ht->col : ht);
    ht->col = cht;
  }
}

//create host
host_type *hostCreate(in_addr ip)
{
  host_type host = {0, 0, 0, 0, 0, setts.anm, setts.nhp, 0, setts.nhl, 0, SPC, 0, SPC, 0, 0, 0, 0, ip, "", "", "", ""}, *ht;
  strcpy(host.htip, inet_ntoa(ip));
  for (unsigned char cnt = 0; cnt < SVCS; cnt++) host.svc[cnt] = -1;
  int hnet = hostNet(&host);
  if (hnet != 2) hostPos(&host, HPR, 1);
  ht = (host_type *)hstsLL.Write(new host_type(host));
  if (hnet == 2) moveCollision(ht);
  refresh = true;
  return ht;
}

//create/delete link
void linkCreDel(host_type *sht, host_type *dht, unsigned char sp, bool dl = false)
{
  link_type *lk;
  lnksLL.Start(2);
  while ((lk = (link_type *)lnksLL.Read(2)))
  {
    if (((lk->sht == sht) && (lk->dht == dht)) || ((lk->sht == dht) && (lk->dht == sht)) || (!dht && ((lk->sht == sht) || (lk->dht == sht))))
    {
      if (dl)
      {
        delete lk;
        lk = 0;
        lnksLL.Delete(2);
      }
      if (dht) return;
    }
    if (lk) lnksLL.Next(2);
  }
  if (!dht) return;
  link_type link = {sht, dht, sp};
  lnksLL.Write(new link_type(link));
}

//create alert
void alrtCreate(aobj_type ao, unsigned char pr, host_type *ht)
{
  if (ao)
  {
    alrt_type *al;
    altsLL.End(1);
    while ((al = (alrt_type *)altsLL.Read(1)))
    {
      if (al->ao == actv)
      {
        if (al->frm != frame) break;
        if (al->ht == ht) return;
      }
      else if ((ao == text) && (al->ht == ht))
      {
        al->pr = pr;
        al->sz = 7;
        return;
      }
      altsLL.Prev(1);
    }
  }
  alrt_type alrt = {ao, pr, 7, frame, ht};
  altsLL.Write(new alrt_type(alrt));
}

//create packet
void pcktCreate(pkif_type *pkt, host_type *sht, host_type *dht, bool lk)
{
  if ((pktsLL.Num() < setts.pks) && (sht->shp || dht->shp) && (!setts.sen || (pkt->sen == setts.sen))
    && (!setts.pr || (pkt->pr == setts.pr)) && (!setts.prt || (pkt->srcpt == setts.prt) || (pkt->dstpt == setts.prt)) && goAnim)
  {
    pckt_type pckt = {pkt->pr, pkt->dstpt, frame, {sht->px, sht->py, sht->pz}, dht}, *pk; 
    pktsLL.End(2);
    while ((pk = (pckt_type *)pktsLL.Read(2)))
    {
      if (pk->frm != frame) break;
      if ((pk->cu.x == pckt.cu.x) && (pk->cu.y == pckt.cu.y) && (pk->cu.z == pckt.cu.z) && (pk->ht == pckt.ht)) return;
      pktsLL.Prev(2);
    }
    pktsLL.Write(new pckt_type(pckt));
    if (lk && (sht->alk || dht->alk)) linkCreDel(sht, dht, 0);
    if (setts.sona == hst)
    {
      sht->vis = 254;
      setts.mvd = 255;
    }
    else if (setts.sona == slt) sht->sld = 1;
  }
}

//destroy hosts LL
void hostsDestroy()
{
  host_type *ht;
  hstsLL.Start(1);
  while ((ht = (host_type *)hstsLL.Read(1)))
  {
    delete ht;
    hstsLL.Next(1);
  }
  hstsLL.Destroy();
}

//destroy links LL
void linksDestroy()
{
  lnkht = 0;
  link_type *lk;
  lnksLL.Start(1);
  while ((lk = (link_type *)lnksLL.Read(1)))
  {
    delete lk;
    lnksLL.Next(1);
  }
  lnksLL.Destroy();
}

//destroy alerts LL
void alrtsDestroy()
{
  alrt_type *al;
  altsLL.Start(1);
  while ((al = (alrt_type *)altsLL.Read(1)))
  {
    delete al;
    altsLL.Next(1);
  }
  altsLL.Destroy();
}

//update osd text
void osdUpdate()
{
  char sbuf[34];
  strcpy(osdtxt, "Sen: ");  //5
  if (setts.sen) sprintf(sbuf, "%u", setts.sen);  //3
  else strcpy(sbuf, "All");
  strcat(sbuf, "\nPro: ");  //6
  strcat(osdtxt, sbuf);
  if (setts.pr) sprintf(sbuf, "%u", setts.pr);  //3
  else strcpy(sbuf, "All");
  strcat(sbuf, "\nPrt: ");  //6
  strcat(osdtxt, sbuf);
  if (setts.prt) sprintf(sbuf, "%u", setts.prt);  //5
  else strcpy(sbuf, "All");
  strcat(osdtxt, sbuf);
  sprintf(sbuf, "\n%s: %s\nAct: %s\nPkts: ", tipn[setts.sipn], (setts.sona == ipn ? "Act" : tips[setts.sips]), tona[setts.sona]);  //29
  if (setts.fsp) strcat(sbuf, "F");  //1
  if (setts.adh) strcat(sbuf, "D");  //1
  else if (setts.bct) strcat(sbuf, "B");
  if (setts.nhp) strcat(sbuf, "P");  //1
  if (setts.nhl) strcat(sbuf, "L");  //1
  strcat(osdtxt, sbuf);
  sprintf(sbuf, "\n%u", setts.pks);  //11
  strcat(osdtxt, sbuf);
}

//destroy packets LL
void pcktsDestroy(bool gh = false)
{
  if (!goHosts)
  {
    goHosts = 1;
    while (goHosts == 1) usleep(1000);
  }
  pckt_type *pk;
  pktsLL.Start(1);
  while ((pk = (pckt_type *)pktsLL.Read(1)))
  {
    delete pk;
    pktsLL.Next(1);
  }
  pktsLL.Destroy();
  if (gh)
  {
    goHosts = 0;
    osdUpdate();
  }
}

//destroy all LLs
void allDestroy()
{
  seltd = 0;
  pcktsDestroy();
  alrtsDestroy();
  linksDestroy();
  hostsDestroy();
}

//draw host objects
void hostsDraw(GLenum mode)
{
  bool dips = ((mode == GL_RENDER) && (setts.sona != ipn)), anms = (setts.anm && (time(0) - atime) && goAnim);  //every second
  if (anms) time(&atime);
  if (seltd)  //draw selected host
  {
    glPushMatrix();
    glLoadName(0);
    glTranslated(seltd->px, seltd->py, seltd->pz);
    glCallList(objsDraw + (seltd->col ? 13 : 11));
    glPopMatrix();
    if (dips && (seltd->pip || setts.sips) && ((setts.sipn != nms) || *seltd->htnm))  //draw host IP/name
    {
      glColor3ub(white[0], white[1], white[2]);
      glRasterPos3i(seltd->px, seltd->py + 11, seltd->pz);
      GLWin.DrawString((const unsigned char *)(*seltd->htnm && setts.sipn ? seltd->htnm : seltd->htip));
    }
    if (anms && seltd->anm)
    {
      alrtCreate(anom, 248, seltd);  //create anomaly alert
      if (setts.mvd < 25) setts.mvd = 25;
    }
    if (animate && seltd->vis) seltd->vis--;
  }
  unsigned short cnt = 1;
  host_type *ht;
  hstsLL.Start(1);
  while ((ht = (host_type *)hstsLL.Read(1)))
  {
    if (ht != seltd)  //draw host
    {
      if ((setts.sona != hst) || ht->vis || ht->sld || ht->anm)
      {
        glPushMatrix();
        glLoadName(cnt);
        glTranslated(ht->px, ht->py, ht->pz);
        if (ht->col) glCallList(objsDraw + 12);
        else if (ht->sld) glCallList(objsDraw + 10);
        else glCallList(objsDraw + ht->clr);
        glPopMatrix();
        if (dips && !ht->col && (ht->pip || (setts.sips == all) || ((setts.sips == sel) && ht->sld)) && ((setts.sipn != nms) || *ht->htnm))  //draw host IP/name
        {
          glColor3ub(white[0], white[1], white[2]);
          glRasterPos3i(ht->px, ht->py + 11, ht->pz);
          GLWin.DrawString((const unsigned char *)(*ht->htnm && setts.sipn ? ht->htnm : ht->htip));
        }
        if (anms && ht->anm)
        {
          alrtCreate(anom, 248, ht);  //create anomaly alert
          if (setts.mvd < 25) setts.mvd = 25;
        }
        if (animate && ht->vis) ht->vis--;
      }
    }
    cnt++;
    hstsLL.Next(1);
  }
}

//draw link objects
void linksDraw()
{
  glColor3ub(dlgrey[0], dlgrey[1], dlgrey[2]);
  link_type *lk;
  lnksLL.Start(1);
  while ((lk = (link_type *)lnksLL.Read(1)))
  {
    if ((setts.sona != hst) || ((lk->sht->vis || lk->sht->sld || lk->sht->anm) && (lk->dht->vis || lk->dht->sld || lk->dht->anm)))
    {
      glBegin(GL_LINES);
        glVertex3i(lk->sht->px, lk->sht->py + 5, lk->sht->pz);
        glVertex3i(lk->dht->px, lk->dht->py + 5, lk->dht->pz);
      glEnd();
    }
    lnksLL.Next(1);
  }
}

//draw alert objects
void alrtsDraw()
{
  alrt_type *al;
  altsLL.Start(1);
  while ((al = (alrt_type *)altsLL.Read(1)))
  {
    if (al->frm == frame) break;
    switch (al->pr)
    {
      case 248: glColor3ub(brred[0], brred[1], brred[2]); break;  //bright red
      case IPPROTO_ICMP: glColor3ub(red[0], red[1], red[2]); break;  //ICMP red
      case IPPROTO_TCP: glColor3ub(green[0], green[1], green[2]); break;  //TCP green
      case IPPROTO_UDP: glColor3ub(blue[0], blue[1], blue[2]); break;  //UDP blue
      case IPPROTO_ARP: glColor3ub(yellow[0], yellow[1], yellow[2]); break;  //ARP yellow
      default: glColor3ub(brgrey[0], brgrey[1], brgrey[2]);  //other grey
    }
    if (al->ao == text)
    {
      glRasterPos3i(al->ht->px, al->ht->py + 11, al->ht->pz);
      GLWin.DrawString((const unsigned char *)(*al->ht->htnm && setts.sipn ? al->ht->htnm : al->ht->htip));
    }
    else
    {
      glPushMatrix();
      glTranslated(al->ht->px, al->ht->py + 5, al->ht->pz);
      glScalef(al->sz, al->sz, al->sz);
      glCallList(objsDraw + (al->ao == actv ? 20 : 21));
      glPopMatrix();
    }
    if (animate)
    {
      al->sz++;
      if ((al->sz < 16) || ((al->ao == text) && (al->sz < 255)))  //size/duration of alert
      {
        if (al->ao == anom) dAnom = false;
        altsLL.Next(1);
      }
      else
      {
        delete al;
        altsLL.Delete(1);
        if (!altsLL.Num()) refresh = true;
      }
    }
    else altsLL.Next(1);
  }
}

//draw packet objects
void pcktsDraw()
{
  double itrns;
  char sbuf[6];
  pckt_type *pk;
  pktsLL.Start(1);
  while ((pk = (pckt_type *)pktsLL.Read(1)))
  {
    if (pk->frm == frame) break;
    glPushMatrix();
    glTranslated(pk->cu.x, pk->cu.y, pk->cu.z);
    switch (pk->pr)
    {
      case IPPROTO_ICMP: glCallList(objsDraw + 16); break;  //ICMP red
      case IPPROTO_TCP: glCallList(objsDraw + 17); break;  //TCP green
      case IPPROTO_UDP: glCallList(objsDraw + 18); break;  //UDP blue
      case IPPROTO_ARP: glCallList(objsDraw + 19); break;  //ARP yellow
      default: glCallList(objsDraw + 15);  //other grey
    }
    glPopMatrix();
    if (setts.pdp)
    {
      glColor3ub(white[0], white[1], white[2]);
      sprintf(sbuf, "%u", pk->dstpt);
      glRasterPos3d(pk->cu.x, pk->cu.y + 7.0, pk->cu.z);
      GLWin.DrawString((const unsigned char *)sbuf);
    }
    if (animate)
    {
      itrns = sqrt(sqr(pk->cu.x - pk->ht->px) + sqr(pk->cu.y - pk->ht->py) + sqr(pk->cu.z - pk->ht->pz)) / (setts.fsp ? 6.0 : 3.0);  //constant speed for all packets
      if (itrns > 1.0)
      {
        pk->cu.x -= (pk->cu.x - pk->ht->px) / itrns;
        pk->cu.y -= (pk->cu.y - pk->ht->py) / itrns;
        pk->cu.z -= (pk->cu.z - pk->ht->pz) / itrns;
        pktsLL.Next(1);
      }
      else
      {
        if (setts.sona)
        {
          alrtCreate((setts.sona == ipn ? text : actv), pk->pr, pk->ht);  //on-active create alert
          if (setts.sona == hst)
          {
            pk->ht->vis = 254;
            setts.mvd = 255;
          }
          else if (setts.sona == slt) pk->ht->sld = 1;
        }
        delete pk;
        pktsLL.Delete(1);
        if (!pktsLL.Num()) refresh = true;
      }
    }
    else pktsLL.Next(1);
  }
}

//draw 2D objects
void draw2D()
{
  int px = wWin - 66, py = hWin - 16;
  char sbuf[25];
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0.0, (GLdouble)wWin, 0.0, (GLdouble)hWin);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0.375, 0.375, 0.0);  //rasterisation fix
  if (seltd)  //draw selected host details
  {
    glColor3ub(white[0], white[1], white[2]);
    glRasterPos2i(6, py);
    GLWin.DrawString((const unsigned char *)htdtls);
  }
  if (setts.osd && goSize)
  {
    glColor3ub(white[0], white[1], white[2]);
    glRasterPos2i(px, py);
    GLWin.DrawString((const unsigned char *)osdtxt);
    if (lnkht)
    {
      strcpy(sbuf, "Link Line");
      glRasterPos2i(px, 19);
      GLWin.DrawString((const unsigned char *)sbuf);
    }
    if (goAnim) sprintf(sbuf, "%u", pktsLL.Num());
    else strcpy(sbuf, "PAUSED");
    glRasterPos2i(px, 6);
    GLWin.DrawString((const unsigned char *)sbuf);
    if (dAnom)
    {
      glColor3ub(brred[0], brred[1], brred[2]);
      glRasterPos2i(px + 54, py);
      GLWin.DrawChar('A');
    }
  }
  if (ptrc > hlt)  //draw packet traffic record/replay status
  {
    time_t dt = time(0) - distm;
    if (ptrc == rec)
    {
      glColor3ub(red[0], red[1], red[2]);
      if (distm) strftime(sbuf, 17, "REC %j %H:%M:%S", gmtime(&dt));
      else strcpy(sbuf, "REC");
    }
    else
    {
      glColor3ub(green[0], green[1], green[2]);
      strftime(sbuf, 25, "REPLAY %d-%m-%y %H:%M:%S", localtime(&dt));
    }
    glRasterPos2i(6, 6);
    GLWin.DrawString((const unsigned char *)sbuf);
  }
  if (GLWin.On()) GLWin.Draw(GL_RENDER);  //draw 2D GUI
  else if (mMove)  //draw selection box
  {
    glColor3ub(brgrey[0], brgrey[1], brgrey[2]);
    glBegin(GL_LINE_LOOP);
      glVertex2i(mBxx, mBxy);
      glVertex2i(mPsx, mBxy);
      glVertex2i(mPsx, mPsy);
      glVertex2i(mBxx, mPsy);
    glEnd();
  }
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glEnable(GL_DEPTH_TEST);
}

//draw graphics
void displayGL()
{
  refresh = false;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(setts.vws[0].ee.x, setts.vws[0].ee.y, setts.vws[0].ee.z, setts.vws[0].dr.x, setts.vws[0].dr.y, setts.vws[0].dr.z, 0.0, 1.0, 0.0);
  glCallList(objsDraw + 14);  //draw cross object
  if (hstsLL.Num()) hostsDraw(GL_RENDER);
  if (lnksLL.Num()) linksDraw();
  if (altsLL.Num()) alrtsDraw();
  if (pktsLL.Num()) pcktsDraw();
  if (setts.osd || seltd || mMove || (ptrc > hlt) || GLWin.On()) draw2D();
  animate = false;
  glfwSwapBuffers();
}

//glfwSetWindowRefreshCallback
void GLFWCALL refreshGL()
{
  refresh = true;
}

//glfwSetWindowSizeCallback
void GLFWCALL resizeGL(int w, int h)
{
  wWin = w;
  hWin = h;
  goSize = ((wWin >= 300) && (hWin >= 160));  //don't draw OSD if window smaller than 300x160
  glViewport(0, 0, wWin, hWin);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (GLdouble)wWin / (GLdouble)hWin, 1.0, DEPTH);
  GLWin.ScreenSize(wWin, hWin);
}

//load selected host details into htdtls
void hostDetails()
{
  strcpy(htdtls, "IP: ");  //4
  strcat(htdtls, seltd->htip);  //15
  if (*seltd->htmc && strcmp(seltd->htmc, "00:00:00:00:00:00"))
  {
    strcat(htdtls, "\nMAC: ");  //6
    strcat(htdtls, seltd->htmc);  //17
  }
  if (*seltd->htnm)
  {
    strcat(htdtls, "\nName: ");  //7
    strcat(htdtls, seltd->htnm);  //255
  }
  if (*seltd->htrm)
  {
    strcat(htdtls, "\nRemarks: ");  //10
    strcat(htdtls, seltd->htrm);  //255
  }
}

//2D GUI window "please wait"
void waitShow()
{
  GLWin.CreateWin(-1, -1, 104, 50, "PROCESSING", false);
  GLWin.AddLabel(10, 10, "Please Wait...");
  displayGL();
}

//display message box
void messageBox(const char *ttl, const char *msg)
{
  GLWin.CreateWin(-1, -1, 230, 90, ttl);
  GLWin.AddLabel(10, 10, msg);
  GLWin.AddButton(178, 30, GLWIN_CLOSE, "OK", true, true);
  displayGL();
}

//set value for all hosts
void hostsSet(unsigned char mbr, unsigned char val)
{
  host_type *ht;
  hstsLL.Start(1);
  while ((ht = (host_type *)hstsLL.Read(1)))
  {
    switch (mbr)
    {
      case 0:
        if (!val && (setts.sona == hst) && ht->sld)
        {
          ht->vis = 254;
          setts.mvd = 255;
        }
        ht->sld = val;
        break;
      case 1: ht->pip = val; break;
      case 2: ht->anm = val; break;
      case 3: ht->shp = val; break;
      case 4: ht->alk = val; break;
    }
    hstsLL.Next(1);
  }
}

//return pointer to host from IP address
host_type *hostIP(in_addr ip, bool crt)
{
  host_type *ht;
  hstsLL.Start(2);
  while ((ht = (host_type *)hstsLL.Read(2)))
  {
    if (ht->hip.s_addr == ip.s_addr) return ht;
    hstsLL.Next(2);
  }
  if (crt) return hostCreate(ip);
  return 0;
}

//load network layout
void netLoad(const char *fl)
{
  FILE *net;
  if ((net = fopen(fl, "rb")))
  {
    char hnl[4];
    if (fgets(hnl, 4, net))
    {
      host_type host, *ht, *dht;
      size_t htsz = sizeof(host);
      if (!strcmp(hnl, "HN1"))
      {
        allDestroy();
        unsigned char spr;
        unsigned int hsts;
        in_addr hip;
        size_t ipsz = sizeof(hip);
        if (fread(&hsts, sizeof(hsts), 1, net) == 1)
        {
          for (; hsts; hsts--)
          {
            if (fread(&host, htsz, 1, net) != 1)
            {
              goHosts = 0;
              fclose(net);
              return;
            }
            host.col = 0;
            ht = (host_type *)hstsLL.Write(new host_type(host));
            moveCollision(ht);
          }
          while (fread(&hip, ipsz, 1, net) == 1)
          {
            ht = hostIP(hip, false);
            if (fread(&hip, ipsz, 1, net) != 1) break;
            dht = hostIP(hip, false);
            if (fread(&spr, 1, 1, net) != 1) break;
            if (ht && dht) linkCreDel(ht, dht, spr);
          }
        }
        goHosts = 0;
      }
      else if (!strcmp(hnl, "HNL"))
      {
        allDestroy();
        while (fread(&host, htsz, 1, net) == 1)
        {
          host.col = 0;
          ht = (host_type *)hstsLL.Write(new host_type(host));
          moveCollision(ht);
        }
        goHosts = 0;
      }
    }
    fclose(net);
  }
}

//save network layout
void netSave(const char* fl)
{
  FILE *net;
  if ((net = fopen(fl, "wb")))
  {
    unsigned int hsts = hstsLL.Num(), cnt = 0;
    fputs("HN1", net);
    if (fwrite(&hsts, sizeof(hsts), 1, net) == 1)
    {
      size_t htsz = sizeof(host_type), ipsz = sizeof(in_addr);
      host_type *ht;
      hstsLL.Start(1);
      while ((ht = (host_type *)hstsLL.Read(1)))
      {
        if (cnt++ == hsts) break;
        if (fwrite(ht, htsz, 1, net) != 1)
        {
          fclose(net);
          remove(fl);
          return;
        }
        hstsLL.Next(1);
      }
      link_type *lk;
      lnksLL.Start(1);
      while ((lk = (link_type *)lnksLL.Read(1)))
      {
        if (fwrite(&lk->sht->hip, ipsz, 1, net) != 1) break;
        if (fwrite(&lk->dht->hip, ipsz, 1, net) != 1) break;
        if (fwrite(&lk->spr, 1, 1, net) != 1) break;  //spare (future use)
        lnksLL.Next(1);
      }
    }
    fclose(net);
  }
}

//process 2D GUI button selected
void btnProcess(int gs)
{
  char *gi1 = GLWin.GetInput(GLResult[0] / 100), *gi2, *gi3, *gi4, *gi5;  //get input text
  if ((gs == GLWIN_CLOSE) || !gi1)
  {
    GLWin.Close(false);  //close focused 2D GUI window
    return;
  }
  int gr = GLResult[0] % 100;  //result identification
  switch (gr)
  {
    case HSD_EDTNAME: case HSD_EDTRMKS:
      strcpy((gr == HSD_EDTNAME ? seltd->htnm : seltd->htrm), gi1);
      hostDetails();
      GLWin.Close();  //close all 2D GUI windows
      break;
    case HSD_EXPTCSV:
      if (*gi1)
      {
        waitShow();
        extensionAdd(gi1, ".csv");
        FILE *cf;
        if ((cf = fopen(gi1, "w")))
        {
          unsigned char cnt;
          char buf[5], svc[12], row[1097];
          fputs("\"IP\",\"MAC\",\"Name\",\"Remarks\",\"Services\"\n", cf);
          host_type *ht;
          hstsLL.Start(1);
          while ((ht = (host_type *)hstsLL.Read(1)))
          {
            if (ht->sld)
            {
              strcpy(row, "\"");
              quotecsv(ht->htip, row);
              strcat(row, "\",\"");
              quotecsv(ht->htmc, row);
              strcat(row, "\",\"");
              quotecsv(ht->htnm, row);
              strcat(row, "\",\"");
              quotecsv(ht->htrm, row);
              strcat(row, "\",\"");
              for (cnt = 0; cnt < SVCS; cnt++)
              {
                if (ht->svc[cnt] == -1) break;
                sprintf(svc, "%s:%d ", protoDecode((ht->svc[cnt] % 10000) / 10, buf), ht->svc[cnt] / 10000);
                strcat(row, svc);
              }
              strcat(row, "\"\n");
              fputs(row, cf);
            }
            hstsLL.Next(1);
          }
          fclose(cf);
        }
        GLWin.Close();  //close all 2D GUI windows
      }
      else messageBox("ERROR", "Enter File Name!");
      break;
    case HSD_FNDHSTS:
      gi2 = GLWin.GetInput(GLResult[1]);  //get input text
      gi3 = GLWin.GetInput(GLResult[2]);
      gi4 = GLWin.GetInput(GLResult[3]);
      gi5 = GLWin.GetInput(GLResult[4]);
      if (*gi1 || *gi2 || *gi3 || *gi4 || *gi5)
      {
        if (gs == GLWIN_DEL)
        {
          GLWin.PutInput("", GLResult[0] / 100);  //put input text
          GLWin.PutInput("", GLResult[1]);
          GLWin.PutInput("", GLResult[2]);
          GLWin.PutInput("", GLResult[3]);
          GLWin.PutInput("", GLResult[4]);
        }
        else
        {
          bool anet = false, ipok;
          unsigned char cnt;
          unsigned short sprt = 0;
          unsigned int found = 0;
          in_addr_t mask = 0, net = 0;
          char ttmp[19], lmc[18], lnm[256], lrm[256], *cidr;  //lowercase MAC, name, remarks
          if (*gi1)
          {
            strcpy(ttmp, gi1);
            if ((cidr = strchr(ttmp, '/')))
            {
              *cidr = '\0';
              if (atoi(++cidr)) mask = htonl(0xffffffff << (32 - atoi(cidr)));
              net = inet_addr(ttmp) & mask;
              anet = true;
            }
          }
          if (*gi5) sprt = atoi(gi5);
          seltd = 0;
          if (!glfwGetKey(GLFW_KEY_LCTRL) && !glfwGetKey(GLFW_KEY_RCTRL)) hostsSet(0, 0);  //ht->sld
          host_type *ht;
          hstsLL.Start(1);
          while ((ht = (host_type *)hstsLL.Read(1)))
          {
            ipok = true;
            if (*gi1)
            {
              if (anet)
              {
                if ((ht->hip.s_addr & mask) != net) ipok = false;
              }
              else if (strcmp(ht->htip, gi1)) ipok = false;
            }
            if (ipok && strstr(strLower(ht->htmc, lmc), gi2) && strstr(strLower(ht->htnm, lnm), gi3) && strstr(strLower(ht->htrm, lrm), gi4))
            {
              if (!*gi5)
              {
                ht->sld = 1;
                found++;
              }
              else
              {
                for (cnt = 0; cnt < SVCS; cnt++)
                {
                  if (ht->svc[cnt] == -1) break;
                  if ((ht->svc[cnt] / 10000) == sprt)
                  {
                    ht->sld = 1;
                    found++;
                    break;
                  }
                }
              }
            }
            hstsLL.Next(1);
          }
          sprintf(ttmp, "Found: %u", found);
          GLWin.PutLabel(ttmp, GLResult[5]);
        }
      }
      break;
    case HSD_HNLOPEN: case HSD_HNLSAVE:
      if (*gi1)
      {
        waitShow();
        if (gr == HSD_HNLOPEN) netLoad(gi1);
        else
        {
          extensionAdd(gi1, ".hnl");
          netSave(gi1);
        }
        GLWin.Close();  //close all 2D GUI windows
      }
      else messageBox("ERROR", "Enter File Name!");
      break;
    case HSD_HPTOPEN: case HSD_HPTSAVE:
      if (*gi1)
      {
        waitShow();
        if (ptrc == rec) ptrc = hlt;
        else if (ptrc == rpy)
        {
          pcktsDestroy();
          ptrc = hlt;
          goHosts = 0;
        }
        while (ptrc == hlt) usleep(1000);
        if (gr == HSD_HPTOPEN) pkrcCopy(gi1, hsddata("traffic.hpt"));
        else
        {
          extensionAdd(gi1, ".hpt");
          pkrcCopy(hsddata("traffic.hpt"), gi1);
        }
        GLWin.Close();  //close all 2D GUI windows
      }
      else messageBox("ERROR", "Enter File Name!");
      break;
    case HSD_MAKEHST:
      if (*gi1)
      {
        bool anm = setts.anm;
        in_addr mip;
        mip.s_addr = inet_addr(gi1);
        goHosts = 1;
        while (goHosts == 1) usleep(1000);
        setts.anm = false;  //don't create anomaly
        seltd = hostIP(mip, true);
        setts.anm = anm;
        goHosts = 0;
        seltd->sld = 1;
        hostDetails();
        GLWin.Close();  //close all 2D GUI windows
      }
      else messageBox("ERROR", "Enter IP Address!");
      break;
    case HSD_NETPOS:
      if (*gi1)
      {
        FILE *nf, *tf;
        if ((nf = fopen(hsddata("netpos.txt"), "r")))
        {
          if ((tf = fopen(hsddata("tmp-netpos-hsd"), "w")))
          {
            switch (gs)
            {
              case GLWIN_ITMUP: GLWin.Scroll(GLWIN_UP, true); break;
              case GLWIN_ITMDN: GLWin.Scroll(GLWIN_DOWN, true); break;
              case GLWIN_DEL: break;  //nothing
              default:
                GLWin.Scroll();  //start
                fputs(gi1, tf);
                fputc('\n', tf);
            }
            bool btmp = false;
            char fbuf[256], tbuf[256], *end;
            while (fgets(fbuf, 256, nf))
            {
              if ((end = strchr(fbuf, '\n'))) *end = '\0';  //remove \n for comparison
              if (strcmp(fbuf, gi1))
              {
                if (gs == GLWIN_ITMUP)
                {
                  if (btmp)
                  {
                    fputs(tbuf, tf);
                    fputc('\n', tf);
                    btmp = false;
                  }
                  strcpy(tbuf, fbuf);
                  btmp = true;
                }
                else
                {
                  fputs(fbuf, tf);
                  fputc('\n', tf);
                  if (btmp)
                  {
                    fputs(tbuf, tf);
                    fputc('\n', tf);
                    btmp = false;
                  }
                }
              }
              else if (gs == GLWIN_ITMUP)
              {
                if (btmp)
                {
                  fputs(fbuf, tf);
                  fputc('\n', tf);
                  fputs(tbuf, tf);
                  fputc('\n', tf);
                  btmp = false;
                }
                else
                {
                  fputs(fbuf, tf);
                  fputc('\n', tf);
                }
              }
              else if (gs == GLWIN_ITMDN)
              {
                strcpy(tbuf, fbuf);
                btmp = true;
              }
            }
            if (btmp)
            {
              fputs(tbuf, tf);
              fputc('\n', tf);
            }
            fclose(tf);
            fclose(nf);
            remove(hsddata("netpos.txt"));
            strcpy(fbuf, hsddata("tmp-netpos-hsd"));
            rename(fbuf, hsddata("netpos.txt"));
          }
          else fclose(nf);
        }
        goHosts = 1;
        while (goHosts == 1) usleep(1000);
        netpsLoad();  //reload netpos
        goHosts = 0;
      }
      break;
    case HSD_PKTSPRO: case HSD_PKTSPRT:
      if (*gi1)
      {
        if (gr == HSD_PKTSPRO)
        {
          setts.pr = atoi(gi1);
          if (setts.pr) pcktsDestroy(true);
          else osdUpdate();
        }
        else
        {
          setts.prt = atoi(gi1);
          if (setts.prt) pcktsDestroy(true);
          else osdUpdate();
        }
        GLWin.Close();  //close all 2D GUI windows
      }
      else messageBox("ERROR", "Enter Number!");
      break;
    case HSD_SETCMDS:
      strcpy(setts.cmd[0], gi1);
      strcpy(setts.cmd[1], GLWin.GetInput(GLResult[1]));
      strcpy(setts.cmd[2], GLWin.GetInput(GLResult[2]));
      strcpy(setts.cmd[3], GLWin.GetInput(GLResult[3]));
      GLWin.Close();  //close all 2D GUI windows
      break;
    case HSD_SLINACT:
      gi2 = GLWin.GetInput(GLResult[1]);  //get input text
      gi3 = GLWin.GetInput(GLResult[2]);
      if (*gi1 || *gi2 || *gi3)
      {
        seltd = 0;
        time_t itime = time(0);
        host_type *ht;
        hstsLL.Start(1);
        while ((ht = (host_type *)hstsLL.Read(1)))
        {
          ht->sld = ((itime - ht->lpk) > ((atoi(gi1) * 86400) + (atoi(gi2) * 360) + (atoi(gi3) * 60)));
          hstsLL.Next(1);
        }
        GLWin.Close();  //close all 2D GUI windows
      }
      else messageBox("ERROR", "Enter Days, Hours or Minutes!");
      break;
#ifndef __MINGW32__
    case HSD_HSENRUN:
      gi2 = GLWin.GetInput(GLResult[1]);  //get input text
      gi3 = GLWin.GetInput(GLResult[2]);
      gi4 = GLWin.GetInput(GLResult[3]);
      if (*gi2 && *gi3)
      {
        strcpy(setts.hsst, gi1);
        strcpy(setts.hsid, gi2);
        strcpy(setts.hsif, gi3);
        strcpy(setts.hshd, gi4);
        setts.hspm = GLWin.GetCheck(GLResult[4]);
        unsigned char sen = atoi(setts.hsid);
        if (sen)
        {
          char scmd[407];
          sprintf(scmd, "%s %s %s %s %s", setts.hsst, setts.hsid, setts.hsif, setts.hshd, (setts.hspm ? "-p" : ""));
          if (system(scmd)) messageBox("ERROR", "Start hsen Failed!");
          else GLWin.Close();  //close all 2D GUI windows
        }
        else messageBox("ERROR", "Invalid Sensor ID!");
      }
      else messageBox("ERROR", "Enter Sensor ID and Interface/File!");
      break;
    case HSD_HSENSTP:
      if (*gi1)
      {
        strcpy(setts.hssp, gi1);
        if (system(setts.hssp)) messageBox("ERROR", "Stop hsen Failed!");
        else GLWin.Close();  //close all 2D GUI windows
      }
      else messageBox("ERROR", "Enter Stop hsen Command!");
      break;
#endif
  }
}

//select next/prev host in selection
void hostTab(bool nx)
{
  host_type *ht;
  (nx ? hstsLL.Start(1) : hstsLL.End(1));
  if (seltd)
  {
    while ((ht = (host_type *)hstsLL.Read(1)))
    {
      if (ht == seltd) break;
      (nx ? hstsLL.Next(1) : hstsLL.Prev(1));
    }
    (nx ? hstsLL.Next(1) : hstsLL.Prev(1));
  }
  while ((ht = (host_type *)hstsLL.Read(1)))
  {
    if (ht->sld)
    {
      seltd = ht;
      hostDetails();
      return;
    }
    (nx ? hstsLL.Next(1) : hstsLL.Prev(1));
  }
  if (seltd)
  {
    (nx ? hstsLL.Start(1) : hstsLL.End(1));
    while ((ht = (host_type *)hstsLL.Read(1)))
    {
      if (ht->sld)
      {
        seltd = ht;
        hostDetails();
        break;
      }
      (nx ? hstsLL.Next(1) : hstsLL.Prev(1));
    }
  }
}

//generate host info
void infoHost()
{
  FILE *info;
  if ((info = fopen(hsddata("tmp-hinfo-hsd"), "w")))
  {
    char buf[11];
    fputs(htdtls, info);
    fprintf(info, "\n\nAnomaly: %s\nLast Sensor: %u\nLast Packet: %sShow Packets: %s\nDownloads: %s"
      , (seltd->anm ? "Yes" : "No"), seltd->lsn, ctime(&seltd->lpk), (seltd->shp ? "Yes" : "No"), formatBytes(seltd->dld, buf));
    fprintf(info, "\nUploads: %s\nAuto Link Lines: %s\nLock: %s"
      , formatBytes(seltd->uld, buf), (seltd->alk ? "Yes" : "No"), (seltd->lck ? "On" : "Off"));  //reuse buf
    if (seltd->svc[0] == -1) fprintf(info, "\n\nNO SERVICES");
    else
    {
      fprintf(info, "\n\nSERVICES\nProtocol\tPort");
      for (unsigned char cnt = 0; cnt < SVCS; cnt++)
      {
        if (seltd->svc[cnt] != -1) fprintf(info, "\n%s\t%d", protoDecode((seltd->svc[cnt] % 10000) / 10, buf), seltd->svc[cnt] / 10000);
        else break;
      }
    }
    fclose(info);
  }
}

//reset packet traffic replay time offset
void offsetReset()
{
  distm = time(0) - pkrp.ptime.tv_sec;
  rpoff = milliTime(0) - milliTime(&pkrp.ptime);
}

//generate selection info
void infoSelection()
{
  FILE *info;
  if ((info = fopen(hsddata("tmp-hinfo-hsd"), "w")))
  {
    fprintf(info, "CURRENT SELECTION\n");
    host_type *ht;
    hstsLL.Start(1);
    while ((ht = (host_type *)hstsLL.Read(1)))
    {
      if (ht->sld) fprintf(info, "\n%s\t%s", ht->htip, ht->htnm);
      hstsLL.Next(1);
    }
    fclose(info);
  }
}

//glfwSetKeyCallback
void GLFWCALL keyboardGL(int key, int action)
{
  if (!action) return;
  if ((glfwGetKey(GLFW_KEY_LCTRL) || glfwGetKey(GLFW_KEY_RCTRL)) && (key != GLFW_KEY_LCTRL) && (key != GLFW_KEY_RCTRL)) key += 512;
  if (GLWin.On())
  {
    char *clip;
    switch (key)
    {
      case 600:  //ctrl+x
        if ((clip = GLWin.GetInput()))
        {
          strcpy(setts.clip, clip);  //cut input text to clipboard
          GLWin.PutInput("");
        }
        break;
      case 579: if ((clip = GLWin.GetInput())) strcpy(setts.clip, clip); break;  //copy input text to clipboard, ctrl+c
      case 598: GLWin.PutInput(setts.clip); break;  //paste input text from clipboard, ctrl+v
      case GLFW_KEY_ENTER: btnProcess(GLWin.DefaultButton()); break;  //process 2D GUI default button
      case GLFW_KEY_ESC: GLWin.Close(false); break;  //close focused 2D GUI window
      case GLFW_KEY_TAB: case 805:  //select next/prev host in selection, update host info
        if ((GLResult[0] % 100) == HSD_HSTINFO)
        {
          hostTab((key == GLFW_KEY_TAB));
          if (seltd)
          {
            GLWin.Scroll();  //start
            infoHost();
          }
          break;
        }  //pass tab to 2D GUI
      default: GLWin.Key(key, (glfwGetKey(GLFW_KEY_LSHIFT) || glfwGetKey(GLFW_KEY_RSHIFT)));  //pass key to 2D GUI
    }
  }
  else
  {
    int kymv = (glfwGetKey(GLFW_KEY_LSHIFT) || glfwGetKey(GLFW_KEY_RSHIFT) ? 3 : 1) * MOV;
    double angx, angy;
    switch (key)
    {
      case GLFW_KEY_UP: case GLFW_KEY_DOWN:
        if (key == GLFW_KEY_DOWN) kymv *= -1;
        angx = setts.vws[0].ax * RAD;
        angy = setts.vws[0].ay * RAD;
        setts.vws[0].ee.x += sin(angx) * kymv;
        setts.vws[0].dr.x += sin(angx) * kymv;
        setts.vws[0].ee.y += sin(angy) * kymv;
        setts.vws[0].dr.y += sin(angy) * kymv;
        setts.vws[0].ee.z += cos(angx) * cos(angy) * kymv;
        setts.vws[0].dr.z += cos(angx) * cos(angy) * kymv;
        break;
      case GLFW_KEY_LEFT: case GLFW_KEY_RIGHT:  //move left/right
        angx = (setts.vws[0].ax + (key == GLFW_KEY_LEFT ? 90.0 : -90.0)) * RAD;
        setts.vws[0].ee.x += sin(angx) * kymv;
        setts.vws[0].dr.x += sin(angx) * kymv;
        setts.vws[0].ee.z += cos(angx) * kymv;
        setts.vws[0].dr.z += cos(angx) * kymv;
        break;
      case GLFW_KEY_HOME: case 812: setts.vws[0] = (key == GLFW_KEY_HOME ? vwdef[0] : vwdef[1]); break;  //recall default views
      case 770: case 771: case 772: case 773: memcpy(&setts.vws[0], &setts.vws[key - 769], sizeof(view_type)); break;  //recall view position 1-4, ctrl+f1-f4
      case 577: hostsSet(0, 1); break;  //select all hosts, ht->sld, ctrl+a
      case 'Q': case 'E': case 'W': case 'A': case 'D': case 'S':
        goHosts = 1;
        while (goHosts == 1) usleep(1000);
      case 595: case 590: case 'T': case 'J': case 586: case 'P': case 592:
        if ((key == 595) || (key == 590)) seltd = 0;
        int dx, dz;
        double angt;        
        host_type *ht;
        hstsLL.Start(1);
        while ((ht = (host_type *)hstsLL.Read(1)))
        {
          if (key == 595) ht->sld = !ht->sld;  //invert selection, ctrl+s
          else if (key == 590) ht->sld = (*ht->htnm != '\0');  //select all named hosts, ctrl+n
          else if (ht->sld)
          {
            switch (key)
            {
              case 'T': ht->pip = !ht->pip; break;  //toggle selection persistant IP
              case 'J': ht->alk = 1; break;  //automatic link lines for selection
              case 586: ht->alk = 0; break;  //stop automatic link lines for selection, ctrl+j
              case 'P': ht->shp = 1; break;  //show packets for selection
              case 592: ht->shp = 0; break;  //stop showing packets for selection, ctrl+p
              default:
                if (ht->lck) break;
                if (key == 'Q') ht->py += SPC;  //move selection up
                else if (key == 'E') ht->py -= SPC;  //move selection down
                else
                {
                  if (key == 'W') angt = setts.vws[0].ax * RAD;  //move selection forward
                  else if (key == 'A') angt = (setts.vws[0].ax + 90.0) * RAD;  //move selection left
                  else if (key == 'D') angt = (setts.vws[0].ax - 90.0) * RAD;  //move selection right
                  else angt = (setts.vws[0].ax + 180.0) * RAD;  //move selection back
                  if (cos(angt) >= 0.707)
                  {
                    dx = 0;
                    dz = 1;
                  }
                  else if (sin(angt) > 0.707)
                  {
                    dx = 1;
                    dz = 0;
                  }
                  else if (sin(angt) < -0.707)
                  {
                    dx = -1;
                    dz = 0;
                  }
                  else
                  {
                    dx = 0;
                    dz = -1;
                  }
                  ht->px += dx * SPC;
                  ht->pz += dz * SPC;
                }
                moveCollision(ht);
            }
          }
          hstsLL.Next(1);
        }
        goHosts = 0;
        break;
      case 'F':  //2D GUI find hosts
        GLWin.CreateWin(-1, -1, 350, 220, "FIND HOSTS");
        GLWin.AddLabel(10, 15, "IP/Net:");
        GLWin.AddLabel(202, 15, "(CIDR Notation for Net)");
        GLWin.AddLabel(10, 45, "MAC:");
        GLResult[1] = GLWin.AddInput(70, 40, 17, 17, "", true);
        GLWin.AddLabel(10, 75, "Hostname:");
        GLResult[2] = GLWin.AddInput(70, 70, 43, 255, "", true);
        GLWin.AddLabel(10, 105, "Remarks:");
        GLResult[3] = GLWin.AddInput(70, 100, 43, 255, "", true);
        GLWin.AddLabel(10, 135, "Port No.:");
        GLResult[4] = GLWin.AddInput(70, 130, 5, 5, "", true);
        GLResult[5] = GLWin.AddLabel(156, 135, "");
        GLResult[0] = (GLWin.AddInput(70, 10, 18, 18, "", true) * 100) + HSD_FNDHSTS;  //last for focus
        GLWin.AddButton(146, 160, GLWIN_OK, "Find", true, true);
        GLWin.AddButton(210, 160, GLWIN_DEL, "Clear");
        GLWin.AddButton(280, 160, GLWIN_CLOSE, "Close");
        break;
      case GLFW_KEY_TAB: case 805: hostTab((key == GLFW_KEY_TAB)); break;  //select next/prev host in selection
      case 'C':  //cycle show IP - IP/name - name only
        if (setts.sipn == ips) setts.sipn = ins;
        else if (setts.sipn == ins) setts.sipn = nms;
        else setts.sipn = ips;
        osdUpdate();
        break;
      case 580:  //toggle add destination hosts, ctrl+d
        setts.adh = !setts.adh;
        if (setts.adh && setts.bct) setts.bct = 0;
        osdUpdate();
        break;
      case 'M':  //make host
        GLWin.CreateWin(-1, -1, 230, 100, "MAKE HOST");
        GLWin.AddLabel(10, 15, "Enter IP Address:");
        GLResult[0] = (GLWin.AddInput(118, 10, 15, 15, "") * 100) + HSD_MAKEHST;
        GLWin.AddButton(102, 40, GLWIN_OK, "OK", true, true);
        GLWin.AddButton(154, 40, GLWIN_CLOSE, "Cancel");
        break;
      case 'N':  //edit selected host hostname
        if (!seltd) break;
        GLWin.CreateWin(-1, -1, 320, 116, seltd->htip);
        GLWin.AddLabel(10, 10, "Edit Hostname:");
        GLResult[0] = (GLWin.AddInput(10, 26, 48, 255, seltd->htnm) * 100) + HSD_EDTNAME;
        GLWin.AddButton(192, 56, GLWIN_OK, "OK", true, true);
        GLWin.AddButton(244, 56, GLWIN_CLOSE, "Cancel");
        break;
      case 'R':  //edit selected host remarks
        if (!seltd) break;
        GLWin.CreateWin(-1, -1, 320, 116, seltd->htip);
        GLWin.AddLabel(10, 10, "Edit Remarks:");
        GLResult[0] = (GLWin.AddInput(10, 26, 48, 255, seltd->htrm) * 100) + HSD_EDTRMKS;
        GLWin.AddButton(192, 56, GLWIN_OK, "OK", true, true);
        GLWin.AddButton(244, 56, GLWIN_CLOSE, "Cancel");
        break;
      case 'L': case 588:  //create/delete start/end link line with selected host, ctrl+l
        if (seltd)
        {
          if (lnkht)
          {
            if (lnkht != seltd)
            {
              goHosts = 1;
              while (goHosts == 1) usleep(1000);
              linkCreDel(lnkht, seltd, 0, (key == 588));
              goHosts = 0;
            }
            lnkht = 0;
          }
          else lnkht = seltd;
        }
        else lnkht = 0;
        break;
      case 'Y':  //automatic link lines for all hosts
        setts.nhl = 1;
        hostsSet(4, 1);  //ht->alk
        osdUpdate();
        break;
      case 601:  //toggle automatic link lines for new hosts, ctrl+y
        setts.nhl = !setts.nhl;
        osdUpdate();
        break;
      case 594:  //delete link lines for all hosts, ctrl+r
        setts.nhl = 0;
        hostsSet(4, 0);  //ht->alk
        goHosts = 1;
        while (goHosts == 1) usleep(1000);
        linksDestroy();
        goHosts = 0;
        osdUpdate();
        break;
      case 'U':  //show packets for all hosts
        setts.nhp = 1;
        hostsSet(3, 1);  //ht->shp
        osdUpdate();
        break;
      case 597:  //toggle show packets for new hosts, ctrl+u
        setts.nhp = !setts.nhp;
        osdUpdate();
        break;
      case GLFW_KEY_F1: case GLFW_KEY_F2: case GLFW_KEY_F3: case GLFW_KEY_F4:
        setts.sen = key - 257;
        pcktsDestroy(true);
        break;
      case GLFW_KEY_F5:  //show packets from all hsen
        setts.sen = 0;
        osdUpdate();
        break;
      case '[':  //decrement hsen to show packets from
        setts.sen--;
        pcktsDestroy(true);
        break;
      case ']':  //increment hsen to show packets from
        setts.sen++;
        pcktsDestroy(true);
        break;
      case 'B':  //toggle broadcasts
        setts.bct = !setts.bct;
        if (setts.bct && setts.adh) setts.adh = 0;
        osdUpdate();
        break;
      case '-':  //decrement number of packets allowed on screen before drop
        if (!setts.pks) break;
        setts.pks -= 100;
        osdUpdate();
        break;
      case '=':  //increment number of packets allowed on screen before drop
        if (setts.pks < 1000000)
        {
          setts.pks += 100;
          osdUpdate();
        }
        break;
      case 596: setts.pdp = !setts.pdp; break;  //toggle packet destination port, ctrl+t
      case 'Z':  //toggle packet speed
        setts.fsp = !setts.fsp;
        osdUpdate();
        break;
      case 'K':  //packets off
        setts.nhp = 0;
        hostsSet(3, 0);  //ht->shp
        pcktsDestroy(true);
        break;
      case GLFW_KEY_PAGEDOWN:  //packet traffic record
        if (ptrc > hlt) ptrc = hlt;
        while (ptrc == hlt) usleep(1000);
        if ((pfile = fopen(hsddata("traffic.hpt"), "wb")))
        {
          fputs("HPT", pfile);
          distm = 0;
          ptrc = rec;
        }
        break;
      case GLFW_KEY_INSERT:  //packet traffic replay
        if (ptrc > hlt) ptrc = hlt;
        while (ptrc == hlt) usleep(1000);
        if ((pfile = fopen(hsddata("traffic.hpt"), "rb")))
        {
          fseek(pfile, 3, SEEK_SET);
          if (fread(&pkrp, sizeof(pkrp), 1, pfile) == 1)
          {
            pcktsDestroy();
            ptrc = rpy;
            offsetReset();
            goHosts = 0;
          }
          else fclose(pfile);
        }
        break;
      case GLFW_KEY_PAGEUP: if (ptrc == rpy) offsetReset(); break;  //packet traffic replay - jump to next packet
      case GLFW_KEY_END:  //packet traffic record/replay stop
        if (ptrc == rec) ptrc = hlt;
        else if (ptrc == rpy)
        {
          pcktsDestroy();
          ptrc = hlt;
          goHosts = 0;
        }
        break;
      case GLFW_KEY_F7: case GLFW_KEY_F8:  //2D GUI open/save packet traffic file
        filelistCreate(hsddata("tmp-flist-hsd"), ".hpt");
        if (key == GLFW_KEY_F7)
        {
          GLWin.CreateWin(-1, -1, 332, 234, "OPEN PACKET TRAFFIC FILE...", true, true);
          GLResult[0] = (GLWin.AddInput(10, 26, 50, 251, "") * 100) + HSD_HPTOPEN;
        }
        else
        {
          GLWin.CreateWin(-1, -1, 332, 234, "SAVE PACKET TRAFFIC FILE AS...", true, true);
          GLResult[0] = (GLWin.AddInput(10, 26, 50, 251, "") * 100) + HSD_HPTSAVE;
        }
        GLWin.AddLabel(10, 10, "Enter File Name:");
        GLWin.AddList(10, 52, 10, 50, hsddata("tmp-flist-hsd"));
        GLWin.AddButton(128, 60, GLWIN_OK, "OK", false, true);
        GLWin.AddButton(76, 60, GLWIN_CLOSE, "Cancel", false, false);
        break;
      case GLFW_KEY_SPACE: goAnim = !goAnim; break;  //toggle animation
      case 587: hostsSet(2, 0); break;  //acknowledge all anomalies, ht->anm, ctrl+k
      case 'O': setts.osd = !setts.osd; break;  //toggle OSD
      case 'X':  //export selection details in CSV file
        filelistCreate(hsddata("tmp-flist-hsd"), ".csv");
        GLWin.CreateWin(-1, -1, 332, 234, "EXPORT SELECTION DETAILS IN CSV FILE AS...", true, true);
        GLWin.AddLabel(10, 10, "Enter File Name:");
        GLResult[0] = (GLWin.AddInput(10, 26, 50, 251, "") * 100) + HSD_EXPTCSV;
        GLWin.AddList(10, 52, 10, 50, hsddata("tmp-flist-hsd"));
        GLWin.AddButton(128, 60, GLWIN_OK, "OK", false, true);
        GLWin.AddButton(76, 60, GLWIN_CLOSE, "Cancel", false, false);
        break;
      case 'I':  //2D GUI selected host info
        if (!seltd) break;
        infoHost();
        GLWin.CreateWin(-2, -2, 352, 277, "HOST INFORMATION", true, true);
        GLWin.AddView(10, 10, 10, 10, 10, hsddata("tmp-hinfo-hsd"));
        GLResult[0] = HSD_HSTINFO;
        break;
      case 'G':  //2D GUI selection info
        infoSelection();
        GLWin.CreateWin(-2, -2, 352, 270, "SELECTION INFORMATION", true, true);
        GLWin.AddButton(10, 6, GLWIN_MISC1, "Selection");
        GLWin.AddButton(104, 6, GLWIN_MISC2, "General");
        GLWin.AddButton(186, 6, GLWIN_CLOSE, "Close", true, true);
        GLWin.AddView(10, 42, 10, 10, 17, hsddata("tmp-hinfo-hsd"));
        GLResult[0] = 0;
        break;
      case 'H':  //2D GUI help
        GLWin.CreateWin(-2, -2, 352, 271, "HELP", true, true);
        GLWin.AddBitmap(10, 10, red[0], red[1], red[2], bitmaps[16]);
        GLWin.AddLabel(23, 10, "ICMP");
        GLWin.AddBitmap(59, 10, green[0], green[1], green[2], bitmaps[16]);
        GLWin.AddLabel(72, 10, "TCP");
        GLWin.AddBitmap(102, 10, blue[0], blue[1], blue[2], bitmaps[16]);
        GLWin.AddLabel(115, 10, "UDP");
        GLWin.AddBitmap(145, 10, yellow[0], yellow[1], yellow[2], bitmaps[16]);
        GLWin.AddLabel(158, 10, "ARP");
        GLWin.AddBitmap(188, 10, brgrey[0], brgrey[1], brgrey[2], bitmaps[16]);
        GLWin.AddLabel(201, 10, "OTHER");
        GLWin.AddView(10, 30, 10, 10, 28, hsddata("controls.txt"));
        GLResult[0] = 0;
        break;
    }
  }
  refresh = true;
}

//process 2D GUI menu item selected
void mnuProcess(int m)
{
  GLWin.Close();  //close all 2D GUI windows
  GLResult[0] = 0;
  if (!m) return;
  if (m <= 9)
  {
    waitShow();
    bool init = true;
    int spx = 0, spy = 0, spz = 0;
    goHosts = 1;
    while (goHosts == 1) usleep(1000);
    if (m == 9)
    {
      seltd = 0;
      pcktsDestroy();
      alrtsDestroy();
    }
    host_type *ht;
    hstsLL.Start(1);
    while ((ht = (host_type *)hstsLL.Read(1)))
    {
      if (ht->sld && !ht->lck)
      {
        switch (m)
        {
          case 1:  //move selection to grey zone
            if (ht->px < 0) ht->px *= -1;
            if (ht->pz < 0) ht->pz *= -1;
            hostPos(ht, HPR, 1);
            moveCollision(ht);
            break;
          case 2:  //move selection to blue zone
            if (ht->px > 0) ht->px *= -1;
            if (ht->pz < 0) ht->pz *= -1;
            hostPos(ht, HPR, 1);
            moveCollision(ht);
            break;
          case 3:  //move selection to green zone
            if (ht->px > 0) ht->px *= -1;
            if (ht->pz > 0) ht->pz *= -1;
            hostPos(ht, HPR, 1);
            moveCollision(ht);
            break;
          case 4:  //move selection to red zone
            if (ht->px < 0) ht->px *= -1;
            if (ht->pz > 0) ht->pz *= -1;
            hostPos(ht, HPR, 1);
            moveCollision(ht);
            break;
          case 5: case 6: case 7:  //auto arrange
            if (init)
            {
              hostPos(ht, (m == 5 ? HPR : 10), (m == 7 ? 2 : 1));
              moveCollision(ht);
              spx = ht->px;
              spy = ht->py;
              spz = ht->pz;
              init = false;
            }
            else
            {
              ht->px = spx;
              ht->py = spy;
              ht->pz = spz;
              hostPos(ht, (m == 5 ? HPR : 10), (m == 7 ? 2 : 1));
              moveCollision(ht);
            }
            break;
          case 8:  //arrange into nets
            if (!(spx = hostNet(ht))) break;
            if (spx == 1) hostPos(ht, HPR, 1);
            moveCollision(ht);
            break;
          case 9:  //delete selection
            if (ht == lnkht) lnkht = 0;
            linkCreDel(ht, 0, 0, true);
            hostPos(ht, HPR, 1);
            moveCollision(ht);
            delete ht;
            ht = 0;
            hstsLL.Delete(1);
            break;
        }
      }
      if (ht) hstsLL.Next(1);
    }
    goHosts = 0;
    GLWin.Close();  //close all 2D GUI windows
  }
  else if (m <= 36)
  {
    if (m >= 31) seltd = 0;
    unsigned char cnt;
    time_t itime = time(0);
    char scmd[274];
    host_type *ht;
    hstsLL.Start(1);
    while ((ht = (host_type *)hstsLL.Read(1)))
    {
      switch (m)
      {
        case 31: ht->sld = ht->anm; break;  //select all anomalies
        case 32: ht->sld = ht->shp; break;  //select all showing packets
        case 33: ht->sld = ((itime - ht->lpk) > 300); break;  //5 minutes
        case 34: ht->sld = ((itime - ht->lpk) > 3600); break;  //1 hour
        case 35: ht->sld = ((itime - ht->lpk) > 86400); break;  //1 day
        case 36: ht->sld = ((itime - ht->lpk) > 604800); break;  //1 week
        default:
          if (!ht->sld) break;
          switch (m)
          {
            case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19: ht->clr = m % 10; break;  //colour
            case 21: ht->lck = 1; break;  //lock on
            case 22: ht->lck = 0; break;  //lock off
            case 23: ht->dld = 0; break;  //reset downloads
            case 24: ht->uld = 0; break;  //reset uploads
            case 25: for (cnt = 0; cnt < SVCS; cnt++) ht->svc[cnt] = -1; break;  //reset services list
            case 26: ht->anm = 0; break;  //acknowledge anomaly
            case 27: case 28: case 29: case 30:
              if (!*setts.cmd[m - 27]) break;
              sprintf(scmd, "%s %s &", setts.cmd[m - 27], ht->htip);
              if (system(scmd)) break;  //nothing
              break;
          }
      }
      hstsLL.Next(1);
    }
  }
  else
  {
    char sbuf[256] = "";
    switch (m)
    {
      case 37:
        GLWin.CreateWin(-1, -1, 254, 100, "SELECT INACTIVE >");
        GLWin.AddLabel(10, 15, "Days:");
        GLWin.AddLabel(88, 15, "Hours:");
        GLResult[1] = GLWin.AddInput(130, 10, 2, 2, "");
        GLWin.AddLabel(166, 15, "Minutes:");
        GLResult[2] = GLWin.AddInput(220, 10, 2, 2, "");
        GLResult[0] = (GLWin.AddInput(46, 10, 3, 3, "") * 100) + HSD_SLINACT;
        GLWin.AddButton(126, 40, GLWIN_OK, "OK", true, true);
        GLWin.AddButton(178, 40, GLWIN_CLOSE, "Cancel");
        break;
      case 38:
        GLWin.CreateWin(-1, -1, 320, 210, "SET COMMANDS");
        GLWin.AddLabel(10, 10, "Host IP Address will be Appended to End of Command");
        GLWin.AddLabel(10, 35, "1:");
        GLWin.AddLabel(10, 65, "2:");
        GLResult[1] = GLWin.AddInput(28, 60, 45, 255, setts.cmd[1]);
        GLWin.AddLabel(10, 95, "3:");
        GLResult[2] = GLWin.AddInput(28, 90, 45, 255, setts.cmd[2]);
        GLWin.AddLabel(10, 125, "4:");
        GLResult[3] = GLWin.AddInput(28, 120, 45, 255, setts.cmd[3]);
        GLResult[0] = (GLWin.AddInput(28, 30, 45, 255, setts.cmd[0]) * 100) + HSD_SETCMDS;  //last for focus
        GLWin.AddButton(186, 150, GLWIN_OK, "Set", true, true);
        GLWin.AddButton(244, 150, GLWIN_CLOSE, "Cancel");
        break;
      case 39:  //reset selection link lines
        goHosts = 1;
        while (goHosts == 1) usleep(1000);
        lnkht = 0;
        link_type *lk;
        lnksLL.Start(1);
        while ((lk = (link_type *)lnksLL.Read(1)))
        {
          if (lk->sht->sld || lk->dht->sld)
          {
            delete lk;
            lnksLL.Delete(1);
          }
          else lnksLL.Next(1);
        }
        goHosts = 0;
        break;
      case 40: keyboardGL('I', 2); break;  //2D GUI selected host info
      case 41:  //show selected host packets only
        if (!seltd) break;
        keyboardGL('K', 2);  //packets off
        seltd->shp = 1;  //show packets
        break;
      case 42:  //move selected host here
        if (!seltd) break;
        if (seltd->lck) break;
        goHosts = 1;
        while (goHosts == 1) usleep(1000);
        seltd->px += (int)((setts.vws[0].ee.x - seltd->px) / SPC) * SPC;
        seltd->py += (int)((setts.vws[0].ee.y - seltd->py) / SPC) * SPC;
        seltd->pz += (int)((setts.vws[0].ee.z - seltd->pz) / SPC) * SPC;
        moveCollision(seltd);
        goHosts = 0;
        break;
      case 43:  //go to selected host
        if (seltd)
        {
          view_type tview = {(seltd->pz < 0 ? 0.0 : 180.0), 0.0, {seltd->px, seltd->py + 5.0, seltd->pz + ((seltd->pz < 0 ? -16 : 16) * MOV)}
            , {seltd->px, seltd->py + 5.0, seltd->pz + ((seltd->pz < 0 ? -15 : 15) * MOV)}};
          setts.vws[0] = tview;
        }
        break;
      case 44: keyboardGL('N', 2); break;  //edit selected host hostname
      case 45: keyboardGL('R', 2); break;  //edit selected host remarks
      case 46: keyboardGL('G', 2); break;  //2D GUI selection info
      case 47: keyboardGL(587, 2); break;  //acknowledge all anomalies, ctrl+k
      case 48:  //toggle anomaly detection
        setts.anm = !setts.anm;
        dAnom = setts.anm;
        break;
      case 50:  //show selection IPs/names
        setts.sips = sel;
        if (setts.sona == ipn) setts.sona = don;
        osdUpdate();
        break;
      case 51:  //show all IPs/names
        setts.sips = all;
        if (setts.sona == ipn) setts.sona = don;
        osdUpdate();
        break;
      case 52:  //on-active show IP/name
        setts.sona = ipn;
        osdUpdate();
        break;
      case 54: hostsSet(1, 0);  //show no IPs/names, including persistant, ht->pip
      case 53:  //show IPs/names off
        setts.sips = off;
        if (setts.sona == ipn) setts.sona = don;
        osdUpdate();
        break;
      case 55:  //on-active alert
        setts.sona = alt;
        osdUpdate();
        break;
      case 56:  //on-active show host
        setts.sona = hst;
        osdUpdate();
        break;
      case 57:  //on-active select host
        setts.sona = slt;
        osdUpdate();
        break;
      case 58:  //on-active do nothing
        setts.sona = don;
        osdUpdate();
        break;
      case 60: keyboardGL('U', 2); break;  //show packets for all hosts
      case 61:  //show all protocols packets
        setts.pr = 0;
        osdUpdate();
        break;
      case 62:  //show ICMP packets
        setts.pr = IPPROTO_ICMP;
        pcktsDestroy(true);
        break;
      case 63:  //show TCP packets
        setts.pr = IPPROTO_TCP;
        pcktsDestroy(true);
        break;
      case 64:  //show UDP packets
        setts.pr = IPPROTO_UDP;
        pcktsDestroy(true);
        break;
      case 65:  //show ARP packets
        setts.pr = IPPROTO_ARP;
        pcktsDestroy(true);
        break;
      case 66:  //enter protocol to show packets for
        GLWin.CreateWin(-1, -1, 170, 100, "PACKETS PROTOCOL");
        GLWin.AddLabel(10, 15, "Enter Protocol No.:");
        GLResult[0] = (GLWin.AddInput(130, 10, 3, 3, "") * 100) + HSD_PKTSPRO;
        GLWin.AddButton(42, 40, GLWIN_OK, "OK", true, true);
        GLWin.AddButton(94, 40, GLWIN_CLOSE, "Cancel");
        break;
      case 67:  //show all ports packets
        setts.prt = 0;
        osdUpdate();
        break;
      case 68:  //enter port to show packets for
        GLWin.CreateWin(-1, -1, 158, 100, "PACKETS PORT");
        GLWin.AddLabel(10, 15, "Enter Port No.:");
        GLResult[0] = (GLWin.AddInput(106, 10, 5, 5, "") * 100) + HSD_PKTSPRT;
        GLWin.AddButton(30, 40, GLWIN_OK, "OK", true, true);
        GLWin.AddButton(82, 40, GLWIN_CLOSE, "Cancel");
        break;
      case 69: keyboardGL('K', 2); break;  //packets off
      case 70: keyboardGL(GLFW_KEY_HOME, 2); break;  //recall default views
      case 71: case 72: case 73: case 74: keyboardGL(m + 699, 2); break;  //recall view position 1-4, ctrl+f1-f4
      case 75: case 76: case 77: case 78: memcpy(&setts.vws[m - 74], &setts.vws[0], sizeof(view_type)); break;  //save current view as view position 1-4
      case 80: case 85:  //2D GUI open/save network layout file
        filelistCreate(hsddata("tmp-flist-hsd"), ".hnl");
        if (m == 80)
        {
          GLWin.CreateWin(-1, -1, 332, 234, "OPEN NETWORK LAYOUT FILE...", true, true);
          GLResult[0] = (GLWin.AddInput(10, 26, 50, 251, "") * 100) + HSD_HNLOPEN;
        }
        else
        {
          GLWin.CreateWin(-1, -1, 332, 234, "SAVE NETWORK LAYOUT FILE AS...", true, true);
          GLResult[0] = (GLWin.AddInput(10, 26, 50, 251, "") * 100) + HSD_HNLSAVE;
        }
        GLWin.AddLabel(10, 10, "Enter File Name:");
        GLWin.AddList(10, 52, 10, 50, hsddata("tmp-flist-hsd"));
        GLWin.AddButton(128, 60, GLWIN_OK, "OK", false, true);
        GLWin.AddButton(76, 60, GLWIN_CLOSE, "Cancel", false, false);
        break;
      case 81: netLoad(hsddata("1network.hnl")); break;  //restore network layout 1
      case 82: netLoad(hsddata("2network.hnl")); break;  //restore network layout 2
      case 83: netLoad(hsddata("3network.hnl")); break;  //restore network layout 3
      case 84: netLoad(hsddata("4network.hnl")); break;  //restore network layout 4
      case 86: netSave(hsddata("1network.hnl")); break;  //save current network layout as network layout 1
      case 87: netSave(hsddata("2network.hnl")); break;  //save current network layout as network layout 2
      case 88: netSave(hsddata("3network.hnl")); break;  //save current network layout as network layout 3
      case 89: netSave(hsddata("4network.hnl")); break;  //save current network layout as network layout 4
      case 90: case 91:  //2D GUI netpos editor
        if ((m == 91) && seltd) sprintf(sbuf, "pos %s/32 %d %d %d %s", seltd->htip, seltd->px / SPC, seltd->py / SPC, seltd->pz / SPC, tclr[seltd->clr]);  //add selected host net pos
        GLWin.CreateWin(-1, -1, 332, 234, "NET POSITIONS", true, true);
        GLWin.AddLabel(10, 10, "Line:");
        GLResult[0] = (GLWin.AddInput(10, 26, 50, 255, sbuf) * 100) + HSD_NETPOS;
        GLWin.AddList(10, 52, 10, 50, hsddata("netpos.txt"));
        GLWin.AddButton(292, 60, GLWIN_ITMUP, "!u", false, false);  //item up
        GLWin.AddButton(263, 60, GLWIN_ITMDN, "!d", false, false);  //item down
        GLWin.AddButton(204, 60, GLWIN_OK, "Add", false, true);
        GLWin.AddButton(146, 60, GLWIN_DEL, "Delete", false, false);
        GLWin.AddButton(70, 60, GLWIN_CLOSE, "Close", false, false);
        break;
      case 92:  //clear network layout
        allDestroy();
        goHosts = 0;
        break;
      case 93: keyboardGL('F', 2); break;  //2D GUI find hosts
#ifndef __MINGW32__
      case 94:  //start local hsen
        GLWin.CreateWin(-1, -1, 320, 254, "START LOCAL hsen");
        GLResult[3] = GLWin.AddInput(10, 164, 15, 15, setts.hshd);
        GLWin.AddLabel(10, 10, "Command:");
        GLResult[0] = (GLWin.AddInput(10, 26, 48, 127, setts.hsst) * 100) + HSD_HSENRUN;
        GLWin.AddLabel(10, 56, "Sensor ID:");
        GLResult[1] = GLWin.AddInput(10, 72, 3, 3, setts.hsid);
        GLWin.AddLabel(52, 77, "(1-255)");
        GLWin.AddLabel(182, 77, "Promiscuous Mode:");
        GLResult[4] = GLWin.AddCheck(290, 72, setts.hspm);
        GLWin.AddLabel(10, 102, "Interface/File:  (eth1, ppp0, wlan0, etc.)");
        GLWin.AddLabel(10, 148, "Hosts3D Destination:");
        GLWin.AddLabel(124, 169, "(Blank = localhost)");
        GLResult[2] = GLWin.AddInput(10, 118, 48, 255, setts.hsif);  //last for focus
        GLWin.AddButton(174, 194, GLWIN_OK, "Start", true, true);
        GLWin.AddButton(244, 194, GLWIN_CLOSE, "Cancel");
        break;
      case 95:  //stop local hsen
        GLWin.CreateWin(-1, -1, 320, 116, "STOP LOCAL hsen");
        GLWin.AddLabel(10, 10, "Command:");
        GLResult[0] = (GLWin.AddInput(10, 26, 48, 255, setts.hssp) * 100) + HSD_HSENSTP;
        GLWin.AddButton(180, 56, GLWIN_OK, "Stop", true, true);
        GLWin.AddButton(244, 56, GLWIN_CLOSE, "Cancel");
        break;
#endif
      case 97: keyboardGL('H', 2); break;  //2D GUI help
      case 98:  //about
        GLWin.CreateWin(-1, -1, 230, 138, "ABOUT");
        GLWin.AddLabel(10, 10, "Hosts3D 1.15");
        GLWin.AddLabel(10, 26, "Copyright (c) 2006-2011  Del Castle");
        GLWin.AddLabel(10, 42, "http://hosts3d.sourceforge.net");
        GLWin.AddLabel(10, 58, "hosts3d@gmail.com");
        GLWin.AddBitmap(11, 76, red[0], red[1], red[2], bitmaps[0]);
        GLWin.AddBitmap(19, 76, red[0], red[1], red[2], bitmaps[1]);
        GLWin.AddBitmap(11, 84, red[0], red[1], red[2], bitmaps[2]);
        GLWin.AddBitmap(19, 84, red[0], red[1], red[2], bitmaps[3]);
        GLWin.AddBitmap(11, 92, red[0], red[1], red[2], bitmaps[4]);
        GLWin.AddBitmap(11, 100, red[0], red[1], red[2], bitmaps[6]);
        GLWin.AddBitmap(11, 76, black[0], black[1], black[2], bitmaps[8]);
        GLWin.AddBitmap(19, 76, black[0], black[1], black[2], bitmaps[9]);
        GLWin.AddBitmap(11, 84, black[0], black[1], black[2], bitmaps[10]);
        GLWin.AddBitmap(19, 84, black[0], black[1], black[2], bitmaps[11]);
        GLWin.AddBitmap(10, 92, black[0], black[1], black[2], bitmaps[12]);
        GLWin.AddBitmap(11, 100, black[0], black[1], black[2], bitmaps[14]);
        GLWin.AddBitmap(25, 76, green[0], green[1], green[2], bitmaps[0]);
        GLWin.AddBitmap(33, 76, green[0], green[1], green[2], bitmaps[1]);
        GLWin.AddBitmap(25, 84, green[0], green[1], green[2], bitmaps[2]);
        GLWin.AddBitmap(33, 84, green[0], green[1], green[2], bitmaps[3]);
        GLWin.AddBitmap(33, 92, green[0], green[1], green[2], bitmaps[5]);
        GLWin.AddBitmap(33, 100, green[0], green[1], green[2], bitmaps[7]);
        GLWin.AddBitmap(25, 76, black[0], black[1], black[2], bitmaps[8]);
        GLWin.AddBitmap(33, 76, black[0], black[1], black[2], bitmaps[9]);
        GLWin.AddBitmap(25, 84, black[0], black[1], black[2], bitmaps[10]);
        GLWin.AddBitmap(33, 84, black[0], black[1], black[2], bitmaps[11]);
        GLWin.AddBitmap(34, 92, black[0], black[1], black[2], bitmaps[13]);
        GLWin.AddBitmap(33, 100, black[0], black[1], black[2], bitmaps[15]);
        GLWin.AddBitmap(18, 76, yellow[0], yellow[1], yellow[2], bitmaps[0]);
        GLWin.AddBitmap(26, 76, yellow[0], yellow[1], yellow[2], bitmaps[1]);
        GLWin.AddBitmap(18, 84, yellow[0], yellow[1], yellow[2], bitmaps[2]);
        GLWin.AddBitmap(26, 84, yellow[0], yellow[1], yellow[2], bitmaps[3]);
        GLWin.AddBitmap(18, 92, yellow[0], yellow[1], yellow[2], bitmaps[4]);
        GLWin.AddBitmap(26, 92, yellow[0], yellow[1], yellow[2], bitmaps[5]);
        GLWin.AddBitmap(18, 100, yellow[0], yellow[1], yellow[2], bitmaps[6]);
        GLWin.AddBitmap(26, 100, yellow[0], yellow[1], yellow[2], bitmaps[7]);
        GLWin.AddBitmap(18, 76, black[0], black[1], black[2], bitmaps[8]);
        GLWin.AddBitmap(26, 76, black[0], black[1], black[2], bitmaps[9]);
        GLWin.AddBitmap(18, 84, black[0], black[1], black[2], bitmaps[10]);
        GLWin.AddBitmap(26, 84, black[0], black[1], black[2], bitmaps[11]);
        GLWin.AddBitmap(17, 92, black[0], black[1], black[2], bitmaps[12]);
        GLWin.AddBitmap(27, 92, black[0], black[1], black[2], bitmaps[13]);
        GLWin.AddBitmap(18, 100, black[0], black[1], black[2], bitmaps[14]);
        GLWin.AddBitmap(26, 100, black[0], black[1], black[2], bitmaps[15]);
        GLWin.AddButton(178, 78, GLWIN_CLOSE, "OK", true, true);
        GLResult[0] = 0;
        break;
      case 99: goRun = false; break;  //exit
      case 100:
        GLWin.AddMenu(82, "MAIN", 11, 0, 0);  //nothing
        if (seltd) GLWin.AddMenu(82, "Selected", 11, 1, 101);
        GLWin.AddMenu(82, "Selection", 11, 1, 102);  //6+(9x6)+12+10=82
        GLWin.AddMenu(82, "Anomalies", 11, 1, 103);
        GLWin.AddMenu(82, "IP/Name", 11, 1, 104);
        GLWin.AddMenu(82, "Packets", 11, 1, 105);
        GLWin.AddMenu(82, "On-Active", 11, 1, 106);
        GLWin.AddMenu(82, "View", 11, 1, 107);
        GLWin.AddMenu(82, "Layout", 11, 1, 108);
        GLWin.AddMenu(82, "Other", 11, 1, 109);
        if (fullscn) GLWin.AddMenu(82, "Exit", 11, 0, 99);
        break;
      case 101:
        GLWin.AddMenu(114, "SELECTED", 8, 2, 100);
        GLWin.AddMenu(114, "Information (I)", 8, 0, 40);
        GLWin.AddMenu(114, "Show Packets Only", 8, 0, 41);  //6+(17x6)+6=114
        GLWin.AddMenu(114, "Move Here", 8, 0, 42);
        GLWin.AddMenu(114, "Go To", 8, 0, 43);
        GLWin.AddMenu(114, "Hostname (N)", 8, 0, 44);
        GLWin.AddMenu(114, "Remarks (R)", 8, 0, 45);
        GLWin.AddMenu(114, "Add Net Position", 8, 0, 91);
        break;
      case 102:
        GLWin.AddMenu(102, "SELECTION", 9, 2, 100);
        GLWin.AddMenu(102, "Information (G)", 9, 0, 46);  //6+(15x6)+6=102
        GLWin.AddMenu(102, "Colour", 9, 1, 110);
        GLWin.AddMenu(102, "Lock", 9, 1, 111);
        GLWin.AddMenu(102, "Move To Zone", 9, 1, 112);
        GLWin.AddMenu(102, "Arrange", 9, 1, 113);
        GLWin.AddMenu(102, "Commands", 9, 1, 114);
        GLWin.AddMenu(102, "Reset", 9, 1, 115);
        GLWin.AddMenu(102, "Delete", 9, 1, 116);
        break;
      case 103:
        GLWin.AddMenu(108, "ANOMALIES", 4, 2, 100);
        GLWin.AddMenu(108, "Select All", 4, 0, 31);
        GLWin.AddMenu(108, "Acknowledge", 4, 1, 117);
        GLWin.AddMenu(108, "Toggle Detection", 4, 0, 48);  //6+(16x6)+6=108
        break;
      case 104:
        GLWin.AddMenu(96, "IP/NAME", 6, 2, 100);
        if (setts.sips != sel) GLWin.AddMenu(96, "Show Selection", 6, 0, 50);  //6+(14x6)+6=96
        if (setts.sips != all) GLWin.AddMenu(96, "Show All", 6, 0, 51);
        if (setts.sona != ipn) GLWin.AddMenu(96, "Show On-Active", 6, 0, 52);
        if (setts.sips != off) GLWin.AddMenu(96, "Show Off", 6, 0, 53);
        GLWin.AddMenu(96, "All Off", 6, 0, 54);
        break;
      case 105:
        GLWin.AddMenu(96, "PACKETS", 6, 2, 100);
        GLWin.AddMenu(96, "Show All (U)", 6, 0, 60);
        GLWin.AddMenu(96, "Protocol", 6, 1, 118);
        GLWin.AddMenu(96, "Port", 6, (setts.prt ? 1 : 0), (setts.prt ? 119 : 68));
        GLWin.AddMenu(96, "Select Showing", 6, 0, 32);  //6+(14x6)+6=96
        GLWin.AddMenu(96, "Off (K)", 6, 0, 69);
        break;
      case 106:
        GLWin.AddMenu(84, "ON-ACTIVE", 6, 2, 100);
        if (setts.sona != alt) GLWin.AddMenu(84, "Alert", 6, 0, 55);
        if (setts.sona != ipn) GLWin.AddMenu(84, "Show IP/Name", 6, 0, 52);  //6+(12x6)+6=84
        if (setts.sona != hst) GLWin.AddMenu(84, "Show Host", 6, 0, 56);
        if (setts.sona != slt) GLWin.AddMenu(84, "Select", 6, 0, 57);
        if (setts.sona != don) GLWin.AddMenu(84, "Do Nothing", 6, 0, 58);
        break;
      case 107:
        GLWin.AddMenu(64, "VIEW", 3, 2, 100);
        GLWin.AddMenu(64, "Recall", 3, 1, 120);  //6+(6x6)+12+10=64
        GLWin.AddMenu(64, "Save", 3, 1, 121);
        break;
      case 108:
        GLWin.AddMenu(90, "LAYOUT", 5, 2, 100);
        GLWin.AddMenu(90, "Restore", 5, 1, 122);
        GLWin.AddMenu(90, "Save", 5, 1, 123);
        GLWin.AddMenu(90, "Net Positions", 5, 0, 90);  //6+(13x6)+6=90
        GLWin.AddMenu(90, "Clear", 5, 1, 124);
        break;
      case 109:
        GLWin.AddMenu(118, "OTHER", 6, 2, 100);
        GLWin.AddMenu(118, "Find Hosts (F)", 6, 0, 93);
        GLWin.AddMenu(118, "Select Inactive", 6, 1, 125);  //6+(15x6)+12+10=118
#ifndef __MINGW32__
        GLWin.AddMenu(118, "Local hsen", 6, 1, 126);
#endif
        GLWin.AddMenu(118, "Help (H)", 6, 0, 97);
        GLWin.AddMenu(118, "About", 6, 0, 98);
        break;
      case 110:
        GLWin.AddMenu(64, "COLOUR", 11, 2, 102);  //6+(6x6)+12+10=64
        GLWin.AddMenu(64, "Grey", 11, 0, 10);
        GLWin.AddMenu(64, "Orange", 11, 0, 11);
        GLWin.AddMenu(64, "Yellow", 11, 0, 12);
        GLWin.AddMenu(64, "Fluro", 11, 0, 13);
        GLWin.AddMenu(64, "Green", 11, 0, 14);
        GLWin.AddMenu(64, "Mint", 11, 0, 15);
        GLWin.AddMenu(64, "Aqua", 11, 0, 16);
        GLWin.AddMenu(64, "Blue", 11, 0, 17);
        GLWin.AddMenu(64, "Purple", 11, 0, 18);
        GLWin.AddMenu(64, "Violet", 11, 0, 19);
        break;
      case 111:
        GLWin.AddMenu(52, "LOCK", 3, 2, 102);  //6+(4x6)+12+10=52
        GLWin.AddMenu(52, "On", 3, 0, 21);
        GLWin.AddMenu(52, "Off", 3, 0, 22);
        break;
      case 112:
        GLWin.AddMenu(100, "MOVE TO ZONE", 5, 2, 102);  //6+(12x6)+12+10=100
        GLWin.AddMenu(100, "Grey", 5, 0, 1);
        GLWin.AddMenu(100, "Blue", 5, 0, 2);
        GLWin.AddMenu(100, "Green", 5, 0, 3);
        GLWin.AddMenu(100, "Red", 5, 0, 4);
        break;
      case 113:
        GLWin.AddMenu(78, "ARRANGE", 5, 2, 102);
        GLWin.AddMenu(78, "Default", 5, 0, 5);
        GLWin.AddMenu(78, "10x10", 5, 0, 6);
        GLWin.AddMenu(78, "10x10 2xSpc", 5, 0, 7);  //6+(11x6)+6=78
        GLWin.AddMenu(78, "Into Nets", 5, 0, 8);
        break;
      case 114:
        GLWin.AddMenu(76, "COMMANDS", 6, 2, 102);  //6+(8x6)+12+10=76
        GLWin.AddMenu(76, "Command 1", 6, 0, 27);
        GLWin.AddMenu(76, "Command 2", 6, 0, 28);
        GLWin.AddMenu(76, "Command 3", 6, 0, 29);
        GLWin.AddMenu(76, "Command 4", 6, 0, 30);
        GLWin.AddMenu(76, "Set", 6, 0, 38);
        break;
      case 115:
        GLWin.AddMenu(72, "RESET", 5, 2, 102);
        GLWin.AddMenu(72, "Link Lines", 5, 0, 39);  //6+(10x6)+6=72
        GLWin.AddMenu(72, "Downloads", 5, 0, 23);
        GLWin.AddMenu(72, "Uploads", 5, 0, 24);
        GLWin.AddMenu(72, "Services", 5, 0, 25);
        break;
      case 116:
        GLWin.AddMenu(64, "DELETE", 2, 2, 102);  //6+(6x6)+12+10=64
        GLWin.AddMenu(64, "Confirm", 2, 0, 9);
        break;
      case 117:
        GLWin.AddMenu(94, "ACKNOWLEDGE", 3, 2, 103);  //6+(11x6)+12+10=94
        GLWin.AddMenu(94, "Selection", 3, 0, 26);
        GLWin.AddMenu(94, "All (Ctrl+K)", 3, 0, 47);
        break;
      case 118:
        GLWin.AddMenu(76, "PROTOCOL", 7, 2, 105);  //6+(8x6)+12+10=76
        if (setts.pr) GLWin.AddMenu(76, "All", 7, 0, 61);
        if (setts.pr != IPPROTO_ICMP) GLWin.AddMenu(76, "ICMP", 7, 0, 62);
        if (setts.pr != IPPROTO_TCP) GLWin.AddMenu(76, "TCP", 7, 0, 63);
        if (setts.pr != IPPROTO_UDP) GLWin.AddMenu(76, "UDP", 7, 0, 64);
        if (setts.pr != IPPROTO_ARP) GLWin.AddMenu(76, "ARP", 7, 0, 65);
        GLWin.AddMenu(76, "Other", 7, 0, 66);
        break;
      case 119:
        GLWin.AddMenu(52, "PORT", 3, 2, 105);  //6+(4x6)+12+10=52
        if (setts.prt) GLWin.AddMenu(52, "All", 3, 0, 67);
        GLWin.AddMenu(52, "Enter", 3, 0, 68);
        break;
      case 120:
        GLWin.AddMenu(108, "RECALL", 6, 2, 107);
        GLWin.AddMenu(108, "Home (Ctrl+Home)", 6, 0, 70);  //6+(16x6)+6=108
        GLWin.AddMenu(108, "Pos 1 (Ctrl+F1)", 6, 0, 71);
        GLWin.AddMenu(108, "Pos 2 (Ctrl+F2)", 6, 0, 72);
        GLWin.AddMenu(108, "Pos 3 (Ctrl+F3)", 6, 0, 73);
        GLWin.AddMenu(108, "Pos 4 (Ctrl+F4)", 6, 0, 74);
        break;
      case 121:
        GLWin.AddMenu(52, "SAVE", 5, 2, 107);  //6+(4x6)+12+10=52
        GLWin.AddMenu(52, "Pos 1", 5, 0, 75);
        GLWin.AddMenu(52, "Pos 2", 5, 0, 76);
        GLWin.AddMenu(52, "Pos 3", 5, 0, 77);
        GLWin.AddMenu(52, "Pos 4", 5, 0, 78);
        break;
      case 122:
        GLWin.AddMenu(70, "RESTORE", 6, 2, 108);  //6+(7x6)+12+10=70
        GLWin.AddMenu(70, "File", 6, 0, 80);
        GLWin.AddMenu(70, "Net 1", 6, 0, 81);
        GLWin.AddMenu(70, "Net 2", 6, 0, 82);
        GLWin.AddMenu(70, "Net 3", 6, 0, 83);
        GLWin.AddMenu(70, "Net 4", 6, 0, 84);
        break;
      case 123:
        GLWin.AddMenu(52, "SAVE", 6, 2, 108);  //6+(4x6)+12+10=52
        GLWin.AddMenu(52, "File", 6, 0, 85);
        GLWin.AddMenu(52, "Net 1", 6, 0, 86);
        GLWin.AddMenu(52, "Net 2", 6, 0, 87);
        GLWin.AddMenu(52, "Net 3", 6, 0, 88);
        GLWin.AddMenu(52, "Net 4", 6, 0, 89);
        break;
      case 124:
        GLWin.AddMenu(58, "CLEAR", 2, 2, 108);  //6+(5x6)+12+10=58
        GLWin.AddMenu(58, "Confirm", 2, 0, 92);
        break;
      case 125:
        GLWin.AddMenu(118, "SELECT INACTIVE", 6, 2, 109);  //6+(15x6)+12+10=118
        GLWin.AddMenu(118, "> 5 Minutes", 6, 0, 33);
        GLWin.AddMenu(118, "> 1 Hour", 6, 0, 34);
        GLWin.AddMenu(118, "> 1 Day", 6, 0, 35);
        GLWin.AddMenu(118, "> 1 Week", 6, 0, 36);
        GLWin.AddMenu(118, "> Other", 6, 0, 37);
        break;
#ifndef __MINGW32__
      case 126:
        GLWin.AddMenu(88, "LOCAL hsen", 3, 2, 109);  //6+(10x6)+12+10=88
        GLWin.AddMenu(88, "Start", 3, 0, 94);
        GLWin.AddMenu(88, "Stop", 3, 0, 95);
        break;
#endif
    }
  }
}

//generate general info
void infoGeneral()
{
  FILE *info;
  if ((info = fopen(hsddata("tmp-hinfo-hsd"), "w")))
  {
    unsigned int cnt = 0;
    unsigned long long sdld = 0, suld = 0;
    char buf[11];
    host_type *ht;
    hstsLL.Start(1);
    while ((ht = (host_type *)hstsLL.Read(1)))
    {
      if (ht->sld)
      {
        sdld += ht->dld;
        suld += ht->uld;
        cnt++;
      }
      hstsLL.Next(1);
    }
    fprintf(info, "GENERAL\n\nHosts: %u\nDownloads: %s", cnt, formatBytes(sdld, buf));
    fprintf(info, "\nUploads: %s", formatBytes(suld, buf));  //reuse buf
    fclose(info);
  }
}

//glfwSetMouseButtonCallback
void GLFWCALL clickGL(int button, int action)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT)
  {
    if (GLWin.On())  //if 2D GUI displayed
    {
      int gs = GLWin.Select(!action);  //2D GUI object clicked
      if (action) mMove = ((gs >= GLWIN_TITLE) && (gs <= GLWIN_RBAR));  //2D GUI move
      else
      {
        mMove = false;
        switch (gs)
        {
          case GLWIN_OK: case GLWIN_DEL: case GLWIN_CLOSE: case GLWIN_ITMUP: case GLWIN_ITMDN: btnProcess(gs); break;  //process 2D GUI button selected
          case GLWIN_MNUITM: mnuProcess(GLWin.GetSelected()); break;
          case GLWIN_MISC1:
            GLWin.Scroll();  //start
            infoSelection();  //generate selection info
            break;
          case GLWIN_MISC2:
            GLWin.Scroll();  //start
            infoGeneral();  //generate general info
            break;
        }
      }
    }
    else if (action)  //start selection box
    {
      mPsy = hWin - mPsy;
      mBxx = mPsx;
      mBxy = mPsy;
      mMove = true;
    }
    else
    {
      mMove = false;
      if (hstsLL.Num())
      {
        if (!glfwGetKey(GLFW_KEY_LCTRL) && !glfwGetKey(GLFW_KEY_RCTRL)) hostsSet(0, 0);  //ht->sld
        int sx = mBxx - mPsx, sy = mBxy - mPsy, asx = abs(sx), asy = abs(sy);
        GLint hits, vpt[4];
        GLuint selbuf[SELBUF];
        glGetIntegerv(GL_VIEWPORT, vpt);
        glSelectBuffer(SELBUF, selbuf);
        glRenderMode(GL_SELECT);
        glInitNames();
        glPushName(0);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluPickMatrix((GLdouble)mBxx - ((GLdouble)sx / 2.0), (GLdouble)mBxy - ((GLdouble)sy / 2.0), (asx < 3 ? 3 : asx), (asy < 3 ? 3 : asy), vpt);
        gluPerspective(45.0, (GLdouble)wWin / (GLdouble)hWin, 1.0, DEPTH);
        glMatrixMode(GL_MODELVIEW);
        hostsDraw(GL_SELECT);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        if ((hits = glRenderMode(GL_RENDER)))
        {
          host_type *ht;
          GLuint names, minZ = 0xffffffff, *ptr = (GLuint *)selbuf, *closest = 0, *ptrnames;
          for (; hits; hits--)
          {
            names = *ptr;
            ptr++;
            if (*ptr < minZ)
            {
              minZ = *ptr;
              closest = ptr + 2;
            }
            if ((asx >= 3) || (asy >= 3))  //multi-select
            {
              ptrnames = ptr + 2;
              if (*ptrnames)
              {
                ht = (host_type *)hstsLL.Find(*ptrnames);
                ht->sld = 1;
              }
              else seltd->sld = 1;
            }
            ptr += names + 2;
          }
          if ((asx < 3) && (asy < 3))  //single-select
          {
            if (*closest)
            {
              ht = (host_type *)hstsLL.Find(*closest);
              if (ht->sld)
              {
                ht->sld = 0;
                if (ht->col)  //deselect all in cluster
                {
                  host_type *tht = ht->col;
                  while (tht != ht)
                  {
                    tht->sld = 0;
                    tht = tht->col;
                  }
                }
                seltd = 0;
              }
              else
              {
                seltd = ht;
                seltd->sld = 1;
                hostDetails();
              }
            }
            else if ((ht = seltd->col))
            {
              seltd->sld = 0;
              seltd = ht;  //cycle thru hosts in cluster
              seltd->sld = 1;
              hostDetails();
            }
            else if (seltd->pip)
            {
              seltd->sld = 0;
              seltd->pip = 0;
              seltd = 0;
            }
            else
            {
              seltd->sld = 1;
              seltd->pip = 1;  //activate persistant IP
            }
          }
          else seltd = 0;
        }
        else seltd = 0;
      }
    }
  }
  else if (button == GLFW_MOUSE_BUTTON_MIDDLE) mView = action;  //start move view
  else if (!GLWin.On()) mnuProcess(100);
  refresh = true;
}

//glfwSetMouseWheelCallback
void GLFWCALL wheelGL(int pos)
{
  if (GLWin.On()) GLWin.Scroll((pos > mWhl ? GLWIN_UP : GLWIN_DOWN), false);  //scroll text in 2D GUI
  else  //move view up/down
  {
    setts.vws[0].ee.y += (pos > mWhl ? 1 : -1) * MOV;
    setts.vws[0].dr.y += (pos > mWhl ? 1 : -1) * MOV;
  }
  mWhl = pos;
  refresh = true;
}

//glfwSetMousePosCallback
void GLFWCALL motionGL(int x, int y)
{
  int gy = hWin - y;
  GLWin.MousePos(x, gy);
  if (mMove)
  {
    if (GLWin.On()) GLWin.Motion(x - mPsx, y - mPsy);  //2D GUI motion
    else y = gy;  //resize selection box
  }
  else if (mView)  //move view
  {
    setts.vws[0].ax -= (x - mPsx) * 0.2;
    if (setts.vws[0].ax < 0.0) setts.vws[0].ax = 359.9;
    else if (setts.vws[0].ax > 359.9) setts.vws[0].ax = 0.0;
    setts.vws[0].ay -= (y - mPsy) * 0.2;
    if (setts.vws[0].ay < -90.0) setts.vws[0].ay = -90.0;
    else if (setts.vws[0].ay > 90.0) setts.vws[0].ay = 90.0;
    setts.vws[0].dr.x = (sin(setts.vws[0].ax * RAD) * MOV) + setts.vws[0].ee.x;
    setts.vws[0].dr.y = (sin(setts.vws[0].ay * RAD) * MOV) + setts.vws[0].ee.y;
    setts.vws[0].dr.z = (cos(setts.vws[0].ax * RAD) * cos(setts.vws[0].ay * RAD) * MOV) + setts.vws[0].ee.z;
  }
  mPsx = x;
  mPsy = y;
  refresh = true;
}

//load settings
void settsLoad()
{
  FILE *sts;
  if ((sts = fopen(hsddata("settings-hsd"), "rb")))
  {
    fseek(sts , 0 , SEEK_END);
    if (ftell(sts) == sizeof(setts))
    {
      rewind(sts);
      if (fread(&setts, sizeof(setts), 1, sts) == 1)
      {
        fclose(sts);
        return;
      }
    }
    fclose(sts);
  }
  remove(hsddata("settings-hsd"));
}

//save settings
void settsSave()
{
  FILE *sts;
  if ((sts = fopen(hsddata("settings-hsd"), "wb")))
  {
    if (fwrite(&setts, sizeof(setts), 1, sts) != 1)
    {
      fclose(sts);
      remove(hsddata("settings-hsd"));
      return;
    }
    fclose(sts);
  }
}

//compile GL objects
void initObjsGL()
{
  objsDraw = glGenLists(22);
  glNewList(objsDraw, GL_COMPILE);  //grey host object
    hobjDraw(brgrey[0], brgrey[1], brgrey[2]);
  glEndList();
  glNewList(objsDraw + 1, GL_COMPILE);  //orange host object
    hobjDraw(orange[0], orange[1], orange[2]);
  glEndList();
  glNewList(objsDraw + 2, GL_COMPILE);  //yellow host object
    hobjDraw(yellow[0], yellow[1], yellow[2]);
  glEndList();
  glNewList(objsDraw + 3, GL_COMPILE);  //fluro host object
    hobjDraw(fluro[0], fluro[1], fluro[2]);
  glEndList();
  glNewList(objsDraw + 4, GL_COMPILE);  //green host object
    hobjDraw(green[0], green[1], green[2]);
  glEndList();
  glNewList(objsDraw + 5, GL_COMPILE);  //mint host object
    hobjDraw(mint[0], mint[1], mint[2]);
  glEndList();
  glNewList(objsDraw + 6, GL_COMPILE);  //aqua host object
    hobjDraw(aqua[0], aqua[1], aqua[2]);
  glEndList();
  glNewList(objsDraw + 7, GL_COMPILE);  //blue host object
    hobjDraw(sky[0], sky[1], sky[2]);
  glEndList();
  glNewList(objsDraw + 8, GL_COMPILE);  //purple host object
    hobjDraw(purple[0], purple[1], purple[2]);
  glEndList();
  glNewList(objsDraw + 9, GL_COMPILE);  //violet host object
    hobjDraw(violet[0], violet[1], violet[2]);
  glEndList();
  glNewList(objsDraw + 10, GL_COMPILE);  //red host object
    hobjDraw(red[0], red[1], red[2]);
  glEndList();
  glNewList(objsDraw + 11, GL_COMPILE);  //br red selected host object
    hobjDraw(brred[0], brred[1], brred[2]);
  glEndList();
  glNewList(objsDraw + 12, GL_COMPILE);  //multiple-hosts object
    mobjDraw(false);
  glEndList();
  glNewList(objsDraw + 13, GL_COMPILE);  //br red selected multiple-hosts object
    mobjDraw(true);
  glEndList();
  glNewList(objsDraw + 14, GL_COMPILE);  //cross object
    cobjDraw();
  glEndList();
  glNewList(objsDraw + 15, GL_COMPILE);  //grey packet object
    pobjDraw(0);
  glEndList();
  glNewList(objsDraw + 16, GL_COMPILE);  //red packet object
    pobjDraw(1);
  glEndList();
  glNewList(objsDraw + 17, GL_COMPILE);  //green packet object
    pobjDraw(2);
  glEndList();
  glNewList(objsDraw + 18, GL_COMPILE);  //blue packet object
    pobjDraw(3);
  glEndList();
  glNewList(objsDraw + 19, GL_COMPILE);  //yellow packet object
    pobjDraw(4);
  glEndList();
  glNewList(objsDraw + 20, GL_COMPILE);  //active alert object
    aobjDraw(false);
  glEndList();
  glNewList(objsDraw + 21, GL_COMPILE);  //anomaly alert object
    aobjDraw(true);
  glEndList();
}

//glfwCreateThread
void GLFWCALL pktProcess(void *arg)
{
#ifdef __MINGW32__
  WORD wVersionRequested = MAKEWORD(2, 0);  //WSAStartup parameter
  WSADATA wsaData;  //WSAStartup parameter
  WSAStartup(wVersionRequested, &wsaData);  //initiate use of the Winsock DLL
  SOCKET psock;
#else
  int psock;
#endif
  sockaddr_in padr;  //hsen address
  padr.sin_family = AF_INET;
  padr.sin_addr.s_addr = INADDR_ANY;
  padr.sin_port = htons(HOST3D_PORT);
#ifdef __MINGW32__
  if ((psock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
  {
    MessageBoxA(0, "Socket create failed, continuing", "Hosts3D", MB_ICONWARNING | MB_OK);
#else
  if ((psock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
  {
    syslog(LOG_WARNING, "socket create failed, continuing\n");
#endif
  }
  else if (bind(psock, (sockaddr *)&padr, sizeof(padr)) == -1)
  {
#ifdef __MINGW32__
    MessageBoxA(0, "Socket bind failed, continuing", "Hosts3D", MB_ICONWARNING | MB_OK);
    closesocket(psock);
    WSACleanup();
#else
    syslog(LOG_WARNING, "socket bind failed, continuing\n");
    close(psock);
#endif
  }
#ifdef __MINGW32__
  unsigned long sopt = 1;
  ioctlsocket(psock, FIONBIO, &sopt);
#else
  fcntl(psock, F_SETFL, O_NONBLOCK);
#endif
  enum pksc_type { none, pkfl, sckt };  //packet source
  pksc_type pksc;
  pkif_type *pkif;
  pkex_type *pkex = 0;  //packet info from hsen
  pdns_type *pdns;  //DNS info from hsen
  timeval ptm;  //time of packet
  int rcsz, prsz = sizeof(pkrp), pisz = sizeof(pkif_type), pesz = sizeof(pkex_type), dnsz = sizeof(pdns_type), ptsz = sizeof(ptm);
  in_addr_t mask, dstip;
  unsigned long long rpytm;
  char pbuf[dnsz];
  host_type *sh, *dh;
  while (goRun)
  {
    if (!goHosts)
    {
      pksc = none;
      if (ptrc == hlt)
      {
        fclose(pfile);
        ptrc = stp;
      }
      if (ptrc == rpy)
      {
        rpytm = milliTime(0) - rpoff;
        if (!feof(pfile) && (rpytm >= milliTime(&pkrp.ptime)))
        {
          pkif = &pkrp.pk;
          pksc = pkfl;
        }
        else if (feof(pfile) && !pktsLL.Num())  //stop replay when last packet ends
        {
          ptrc = hlt;
          refresh = true;
        }
      }
      else if ((rcsz = recv(psock, pbuf, dnsz, 0)) > 0)  //received UDP packet
      {
        if ((pbuf[0] == 85) && (rcsz == pesz))  //85 used to identify packet info
        {
          pkex = (pkex_type *)pbuf;
          pkif = (pkif_type *)&pkex->pk;
          pksc = sckt;
          if (ptrc == rec)  //record packet
          {
            if (!distm) time(&distm);
            gettimeofday(&ptm, 0);
            if (fwrite(&ptm, ptsz, 1, pfile) != 1) ptrc = hlt;
            else if (fwrite(pkif, pisz, 1, pfile) != 1) ptrc = hlt;
          }
        }
        else if ((pbuf[0] == 42) && (rcsz == dnsz))  //42 used to identify DNS info
        {
          pdns = (pdns_type *)pbuf;
          if ((sh = hostIP(pdns->dnsip, false)))
          {
            if (!*sh->htnm) strcpy(sh->htnm, pdns->htnm);
          }
        }
      }
      if (pksc)
      {
        sh = hostIP(pkif->srcip, true);
        if (pksc == sckt)
        {
          sh->lsn = pkif->sen;
          sh->uld += pkex->sz;
          time(&sh->lpk);
          if (!*sh->htmc) sprintf(sh->htmc, "%02X:%02X:%02X:%02X:%02X:%02X", pkex->srcmc[0], pkex->srcmc[1], pkex->srcmc[2], pkex->srcmc[3], pkex->srcmc[4], pkex->srcmc[5]);
          if (!(((pkif->pr == IPPROTO_TCP) && !(pkex->syn && pkex->ack)) || ((pkif->pr == IPPROTO_UDP) && !pkex->ack)))  //true if other protocol
          {
            svcAdd(sh, (pkif->srcpt * 10000) + (pkif->pr * 10), setts.anm);
            refresh = true;
          }
        }
        if ((dh = hostIP(pkif->dstip, setts.adh)))
        {
          if (pksc == sckt)
          {
            dh->lsn = pkif->sen;
            dh->dld += pkex->sz;
            time(&dh->lpk);
          }
          pcktCreate(pkif, sh, dh, true);
        }
        else if (setts.bct && (mask = isBroadcast(pkif->dstip)))
        {
          dstip = pkif->dstip.s_addr & mask;
          hstsLL.Start(2);
          while ((dh = (host_type *)hstsLL.Read(2)))
          {
            if ((dh != sh) && ((dh->hip.s_addr & mask) == dstip)) pcktCreate(pkif, sh, dh, false);
            hstsLL.Next(2);
          }
        }
        if (pksc == pkfl)
        {
          if (fread(&pkrp, prsz, 1, pfile) != 1) ptrc = hlt;
        }
      }
      else usleep(10000);
    }
    else
    {
      if (goHosts == 1) goHosts = 2;
      usleep(10000);
    }
  }
#ifdef __MINGW32__
  closesocket(psock);
  WSACleanup();
#else
  close(psock);
#endif
  if (ptrc) fclose(pfile);
}

int main(int argc, char *argv[])
{
  if (getopt(argc, argv, "f") == 'f') fullscn = true;
  else if (argc != 1)
  {
    fprintf(stderr, "hosts3d 1.15 usage: %s [-f]\n  -f : display full screen\n", argv[0]);
    return 1;
  }
#ifndef __MINGW32__
  syslog(LOG_INFO, "started\n");
#endif
  if (!glfwInit())
  {
    fprintf(stderr, "hosts3d error: glfwInit() failed\n");
#ifndef __MINGW32__
    syslog(LOG_ERR, "glfwInit() failed\n");
#endif
    return 1;
  }
  if (fullscn)
  {
    GLFWvidmode vid;
    glfwGetDesktopMode(&vid);
    wWin = vid.Width;
    hWin = vid.Height;
  }
  if (!glfwOpenWindow(wWin, hWin, 0, 0, 0, 0, 24, 0, (fullscn ? GLFW_FULLSCREEN : GLFW_WINDOW)))
  {
    fprintf(stderr, "hosts3d error: glfwOpenWindow() failed\n");
#ifndef __MINGW32__
    syslog(LOG_ERR, "glfwOpenWindow() failed\n");
#endif
    glfwTerminate();
    return 1;
  }
  checkControls();  //check for controls file
  settsLoad();  //load settings
  osdUpdate();
  netLoad(hsddata("0network.hnl"));  //load network layout 0
  if (goHosts) goHosts = 0;
  netpsLoad();  //load netpos
  glfwSetWindowTitle("Hosts3D");  //window title
  glfwSetWindowRefreshCallback(refreshGL);
  glfwSetWindowSizeCallback(resizeGL);
  glfwSetKeyCallback(keyboardGL);
  glfwSetMouseButtonCallback(clickGL);
  glfwSetMouseWheelCallback(wheelGL);
  glfwSetMousePosCallback(motionGL);
  glfwDisable(GLFW_AUTO_POLL_EVENTS);
  glfwEnable(GLFW_KEY_REPEAT);
  glfwEnable(GLFW_MOUSE_CURSOR);
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  initObjsGL();
  GLWin.InitFont();
  GLFWthread gthrd = glfwCreateThread(pktProcess, 0);  //packet gatherer
  signal(SIGINT, hsdStop);  //capture ctrl+c
  signal(SIGTERM, hsdStop);  //capture kill
  while (goRun)
  {
    if ((milliTime(0) - fps) > 40)  //25 FPS
    {
      if (setts.anm) dAnom = true;
      if ((ptrc > hlt) || GLWin.On()) refresh = true;
      if (goAnim)
      {
        if (pktsLL.Num() || altsLL.Num() || setts.mvd) animate = true;
        if (setts.mvd) setts.mvd--;
      }
      frame++;
      fps = milliTime(0);
    }
    if (refresh || animate) displayGL();
    else usleep(10000);
    glfwPollEvents();
    goRun = (goRun && glfwGetWindowParam(GLFW_OPENED));
  }
#ifndef __MINGW32__
  syslog(LOG_INFO, "stopping...\n");
#endif
  goHosts = 2;
  glfwWaitThread(gthrd, GLFW_WAIT);
  settsSave();  //save settings
  netSave(hsddata("0network.hnl"));  //save network layout 0
  allDestroy();
  netpsDestroy();
  glDeleteLists(objsDraw, 22);
  glfwTerminate();
#ifndef __MINGW32__
  syslog(LOG_INFO, "stopped\n");
#endif
  return 0;
}
