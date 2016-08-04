/* glwin.cpp - 13 Apr 11
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
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "glwin.h"
#include "misc.h"

//object type
#define GLWIN_WINDOW  0x01
#define GLWIN_BITMAP  0x02
#define GLWIN_BUTTON  0x03
#define GLWIN_CHECK   0x04
#define GLWIN_INPUT   0x05
#define GLWIN_LABEL   0x06
#define GLWIN_LIST    0x07
#define GLWIN_VIEW    0x08
#define GLWIN_MENU    0x09

const int TITLEH = 20, MENUH = 18;  //title bar height, menu object height
const GLubyte GABE_5X10[96][10] =
{
  {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //032 space
  {  0,  32,   0,   0,  32,  32,  32,  32,  32,  32},  //033 !
  {  0,   0,   0,   0,   0,   0,   0,   0,  80,  80},  //034 "
  {  0,  80,  80,  80, 248,  80, 248,  80,  80,  80},  //035 #
  {  0,  32, 112, 168,  40, 112, 160, 168, 112,  32},  //036 $
  {  0, 128, 152,  88,  64,  32,  16, 208, 200,   8},  //037 %
  {  0, 104, 144, 144, 168,  64, 160, 144,  96,   0},  //038 &
  {  0,   0,   0,   0,   0,   0,   0,   0,  32,  32},  //039 '
  {  0,  16,  32,  64,  64,  64,  64,  64,  32,  16},  //040 (
  {  0,  64,  32,  16,  16,  16,  16,  16,  32,  64},  //041 )
  {  0,   0,   0,  80,  32, 248,  32,  80,   0,   0},  //042 *
  {  0,   0,   0,  32,  32, 248,  32,  32,   0,   0},  //043 +
  { 64,  32,   0,   0,   0,   0,   0,   0,   0,   0},  //044 ,
  {  0,   0,   0,   0,   0, 248,   0,   0,   0,   0},  //045 -
  {  0,  32,   0,   0,   0,   0,   0,   0,   0,   0},  //046 .
  {  0, 128, 128,  64,  64,  32,  16,  16,   8,   8},  //047 /
  {  0, 112, 136, 200, 200, 168, 152, 152, 136, 112},  //048 0
  {  0, 248,  32,  32,  32,  32,  32, 160,  96,  32},  //049 1
  {  0, 248, 128, 128,  64,  48,   8,   8, 136, 112},  //050 2
  {  0, 112, 136,   8,   8,  48,   8,   8, 136, 112},  //051 3
  {  0,  16,  16,  16, 248, 144, 144,  80,  48,  16},  //052 4
  {  0, 112, 136,   8,   8,   8, 240, 128, 128, 248},  //053 5
  {  0, 112, 136, 136, 136, 240, 128, 128, 136, 112},  //054 6
  {  0,  32,  32,  32,  32,  16,  16,   8,   8, 248},  //055 7
  {  0, 112, 136, 136, 136, 112, 136, 136, 136, 112},  //056 8
  {  0, 112, 136,   8,   8, 120, 136, 136, 136, 112},  //057 9
  {  0,  32,   0,   0,  32,   0,   0,   0,   0,   0},  //058 :
  { 64,  32,   0,   0,  32,   0,   0,   0,   0,   0},  //059 ;
  {  0,   8,  16,  32,  64, 128,  64,  32,  16,   8},  //060 <
  {  0,   0,   0,   0, 248,   0, 248,   0,   0,   0},  //061 =
  {  0, 128,  64,  32,  16,   8,  16,  32,  64, 128},  //062 >
  {  0,  32,   0,   0,  32,  32,  16,   8, 136, 112},  //063 ?
  {  0, 112, 136, 128, 152, 168, 168, 152, 136, 112},  //064 @
  {  0, 136, 136, 136, 136, 248, 136, 136, 136, 112},  //065 A
  {  0, 240, 136, 136, 136, 240, 136, 136, 136, 240},  //066 B
  {  0, 112, 136, 128, 128, 128, 128, 128, 136, 112},  //067 C
  {  0, 240, 136, 136, 136, 136, 136, 136, 136, 240},  //068 D
  {  0, 248, 128, 128, 128, 240, 128, 128, 128, 248},  //069 E
  {  0, 128, 128, 128, 128, 240, 128, 128, 128, 248},  //070 F
  {  0, 112, 136, 136, 136, 184, 128, 128, 136, 112},  //071 G
  {  0, 136, 136, 136, 136, 248, 136, 136, 136, 136},  //072 H
  {  0, 248,  32,  32,  32,  32,  32,  32,  32, 248},  //073 I
  {  0,  96, 144,  16,  16,  16,  16,  16,  16, 248},  //074 J
  {  0, 136, 136, 144, 160, 192, 160, 144, 136, 136},  //075 K
  {  0, 248, 128, 128, 128, 128, 128, 128, 128, 128},  //076 L
  {  0, 136, 136, 168, 168, 168, 216, 216, 216, 136},  //077 M
  {  0, 136, 152, 152, 168, 168, 168, 200, 200, 136},  //078 N
  {  0, 112, 136, 136, 136, 136, 136, 136, 136, 112},  //079 O
  {  0, 128, 128, 128, 128, 240, 136, 136, 136, 240},  //080 P
  {  0, 104, 144, 168, 136, 136, 136, 136, 136, 112},  //081 Q
  {  0, 136, 136, 144, 160, 240, 136, 136, 136, 240},  //082 R
  {  0, 112, 136,   8,   8, 112, 128, 128, 136, 112},  //083 S
  {  0,  32,  32,  32,  32,  32,  32,  32,  32, 248},  //084 T
  {  0, 112, 136, 136, 136, 136, 136, 136, 136, 136},  //085 U
  {  0,  32,  32,  80,  80,  80, 136, 136, 136, 136},  //086 V
  {  0,  80,  80,  80, 168, 168, 168, 136, 136, 136},  //087 W
  {  0, 136, 136,  80,  80,  32,  80,  80, 136, 136},  //088 X
  {  0,  32,  32,  32,  32,  32,  80,  80, 136, 136},  //089 Y
  {  0, 248, 128,  64,  64,  32,  16,  16,   8, 248},  //090 Z
  {  0, 112,  64,  64,  64,  64,  64,  64,  64, 112},  //091 [
  {  0,   8,   8,  16,  16,  32,  64,  64, 128, 128},  //092 backslash
  {  0, 112,  16,  16,  16,  16,  16,  16,  16, 112},  //093 ]
  {  0,   0,   0,   0,   0,   0,   0,   0,  80,  32},  //094 ^
  {248,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //095 _
  {  0,   0,   0,   0,   0,   0,   0,   0,  32,  64},  //096 `
  {  0, 120, 136, 120,   8, 136, 112,   0,   0,   0},  //097 a
  {  0, 240, 136, 136, 136, 200, 176, 128, 128, 128},  //098 b
  {  0, 112, 136, 128, 128, 136, 112,   0,   0,   0},  //099 c
  {  0, 120, 136, 136, 136, 152, 104,   8,   8,   8},  //100 d
  {  0, 112, 136, 128, 240, 136, 112,   0,   0,   0},  //101 e
  {  0,  64,  64,  64,  64, 240,  64,  64,  72,  48},  //102 f
  {112, 136,   8, 120, 136, 136, 120,   0,   0,   0},  //103 g
  {  0, 136, 136, 136, 136, 200, 176, 128, 128, 128},  //104 h
  {  0, 112,  32,  32,  32,  32,  96,   0,  32,   0},  //105 i
  { 96, 144,  16,  16,  16,  16,  16,   0,  16,   0},  //106 j
  {  0, 136, 144, 160, 192, 160, 144, 128, 128, 128},  //107 k
  {  0,  48,  32,  32,  32,  32,  32,  32,  32,  32},  //108 l
  {  0, 136, 136, 168, 168, 168,  80,   0,   0,   0},  //109 m
  {  0, 136, 136, 136, 136, 200, 176,   0,   0,   0},  //110 n
  {  0, 112, 136, 136, 136, 136, 112,   0,   0,   0},  //111 o
  {128, 128, 128, 240, 136, 136, 240,   0,   0,   0},  //112 p
  {  8,   8,   8, 120, 136, 136, 120,   0,   0,   0},  //113 q
  {  0, 128, 128, 128, 128, 200, 176,   0,   0,   0},  //114 r
  {  0, 112, 136,  48,  96, 136, 112,   0,   0,   0},  //115 s
  {  0,  32,  32,  32,  32,  32, 248,  32,  32,  32},  //116 t
  {  0, 104, 152, 136, 136, 136, 136,   0,   0,   0},  //117 u
  {  0,  32,  32,  80,  80, 136, 136,   0,   0,   0},  //118 v
  {  0,  80,  80, 168, 168, 136, 136,   0,   0,   0},  //119 w
  {  0, 136,  80,  32,  32,  80, 136,   0,   0,   0},  //120 x
  {112, 136,   8, 120, 136, 136, 136,   0,   0,   0},  //121 y
  {  0, 248, 128,  96,  48,   8, 248,   0,   0,   0},  //122 z
  {  0,  16,  32,  32,  32,  64,  32,  32,  32,  16},  //123 {
  { 32,  32,  32,  32,  32,  32,  32,  32,  32,  32},  //124 |
  {  0,  64,  32,  32,  32,  16,  32,  32,  32,  64},  //125 }
  {  0,   0,   0,   0,   0, 176, 104,   0,   0,   0},  //126 ~
  {  0, 248, 136, 136, 136, 136, 136, 136, 136, 248}   //127 unknown
}
, syms[8][7] =
{
  {130,  68,  40,  16,  40,  68, 130},  //cross
  {130, 132, 136, 144, 160, 192, 254},  //start
  { 16,  16,  16, 146,  84,  56,  16},  //up-arrow
  { 16,  56,  84, 146,  16,  16,  16},  //down-arrow
  { 16,  32,  64, 254,  64,  32,  16},  //left-arrow
  { 16,   8,   4, 254,   4,   8,  16},  //right-arrow
  {128, 192, 224, 240, 224, 192, 128},  //right-point
  { 16,  48, 112, 240, 112,  48,  16}   //left-point
}
, cursor[11] = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}   //cursor
, tick[11] =   { 16,  40,  72, 136,   4,   4,   4,   2,   2,   1,   1};  //tick

MyGLWin::MyGLWin()
{
  Reset();
  screenW = 0;
  screenH = 0;
  mouseX = 0;
  mouseY = 0;
}

void MyGLWin::Reset()
{
  lastWin = 0;
  lastInput = 0;
  names = 1;
  selected = 0;
  currInput = 0;
  currScroll = 0;
  menuY = 0;
}

//mouse over object
bool MyGLWin::MouseOver(int left, int top, int right, int bottom)
{
  return ((mouseX >= left) && (mouseY <= top) && (mouseX <= right) && (mouseY >= bottom));
}

//draw window object
void MyGLWin::DrawWin(GLenum mode, glwn_obj *wn)
{
  bool focus = (wn->name == (selected / 10000));
  int pr = wn->left + wn->width, pb = wn->top - wn->height;
  if (mode == GL_RENDER)
  {
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor3ub(dlblue[0], dlblue[1], dlblue[2]);
    glRecti(wn->left, wn->top, pr, pb);
    //glDisable(GL_BLEND);
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINE_STRIP);
      glVertex2i(pr, wn->top - TITLEH);
      glVertex2i(pr, pb);
      glVertex2i(wn->left, pb);
      glVertex2i(wn->left, wn->top);
      glVertex2i(pr, wn->top);
      glVertex2i(pr, wn->top - TITLEH);
      glVertex2i(wn->left, wn->top - TITLEH);
    glEnd();
    glColor3ub(white[0], white[1], white[2]);
    glRasterPos2i(wn->left + 6, wn->top - 15);
    DrawString((const unsigned char *)wn->title);  //title bar text
  }
  else if (focus)  //title bar selectable
  {
    glLoadName((wn->name * 10000) + GLWIN_TITLE);
    glRecti(wn->left, wn->top, pr - 20, wn->top - TITLEH);
  }
  if (wn->close)  //close object
  {
    if (mode == GL_RENDER)
    {
      if (focus && MouseOver(pr - 16, wn->top - 4, pr - 4, wn->top - 16))
      {
        glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
        glRecti(pr - 16, wn->top - 4, pr - 4, wn->top - 16);
      }
      glColor3ub(brblue[0], brblue[1], brblue[2]);
      glBegin(GL_LINE_LOOP);
        glVertex2i(pr - 16, wn->top - 4);
        glVertex2i(pr - 4, wn->top - 4);
        glVertex2i(pr - 4, wn->top - 16);
        glVertex2i(pr - 16, wn->top - 16);
      glEnd();
      glRasterPos2i(pr - 13, wn->top - 13);
      glBitmap(7, 7, 0.0, 0.0, 0.0, 0.0, syms[0]);  //cross close object
    }
    else if (focus)  //close object selectable
    {
      glLoadName((wn->name * 10000) + GLWIN_CLOSE);
      glRecti(pr - 16, wn->top - 4, pr - 4, wn->top - 16);
    }
  }
  if (wn->resize)  //resize object
  {
    if (mode == GL_RENDER)
    {
      glColor3ub(brblue[0], brblue[1], brblue[2]);
      glBegin(GL_LINES);
        glVertex2i(pr - 13, pb);
        glVertex2i(pr, pb + 13);
        glVertex2i(pr - 10, pb);
        glVertex2i(pr, pb + 10);
        glVertex2i(pr - 7, pb);
        glVertex2i(pr, pb + 7);
        glVertex2i(pr - 4, pb);
        glVertex2i(pr, pb + 4);
      glEnd();
    }
    else if (focus)  //resize object selectable
    {
      glLoadName((wn->name * 10000) + GLWIN_RESIZE);
      glBegin(GL_TRIANGLES);
        glVertex2i(pr - 13, pb);
        glVertex2i(pr, pb + 13);
        glVertex2i(pr, pb);
      glEnd();
    }
  }
}

//draw bitmap object
void MyGLWin::DrawBitmap(glbm_obj *bm)
{
  glColor3ub(bm->red, bm->green, bm->blue);
  glRasterPos2i(bm->pwn->left + bm->left, (bm->pwn->top - TITLEH) - (bm->top + 8));
  glBitmap(8, 8, 0.0, 0.0, 0.0, 0.0, bm->bitmap);
}

//draw button object
void MyGLWin::DrawButton(GLenum mode, glbt_obj *bt)
{
  bool focus = (bt->pwn->name == (selected / 10000));
  int pl = bt->pwn->left + (bt->align ? bt->left : bt->pwn->width - bt->left), pt = (bt->pwn->top - TITLEH) - (bt->align ?  bt->top : bt->pwn->height - bt->top)
    , pr = pl + ((strlen(bt->text) + (bt->text[0] == '!' ? 1 : 5)) * 6) + (bt->text[0] == '!' ? 1 : 0);
  if (mode == GL_RENDER)
  {
    if (focus && MouseOver(pl + 1, pt - 1, pr, pt - 30))
    {
      glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
      glRecti(pl, pt, pr, pt - 30);
    }
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINE_LOOP);
      glVertex2i(pl, pt);
      glVertex2i(pr, pt);
      glVertex2i(pr, pt - 30);
      glVertex2i(pl, pt - 30);
    glEnd();
    if (bt->defalt)
    {
      glBegin(GL_LINES);
        glVertex2i(pl, pt - 1);
        glVertex2i(pr, pt - 1);
        glVertex2i(pl, pt - 29);
        glVertex2i(pr, pt - 29);
      glEnd();
    }
    glColor3ub(white[0], white[1], white[2]);
    glRasterPos2i(pl + (bt->text[0] == '!' ? 7 : 10), pt - (bt->text[0] == '!' ? 18 : 20));
    if (bt->text[0] == '!')
    {
      if (bt->text[1] == 'u') glBitmap(7, 7, 0.0, 0.0, 0.0, 0.0, syms[2]);  //up-arrow object
      else if (bt->text[1] == 'd') glBitmap(7, 7, 0.0, 0.0, 0.0, 0.0, syms[3]);  //down-arrow object
    } 
    else DrawString((const unsigned char *)bt->text);
  }
  else if (focus)  //button object selectable
  {
    glLoadName((bt->pwn->name * 10000) + bt->value);
    glRecti(pl + 1, pt - 1, pr, pt - 30);
  }
}

//draw check object
void MyGLWin::DrawCheck(GLenum mode, glck_obj *ck)
{
  bool focus = (ck->pwn->name == (selected / 10000));
  int pl = ck->pwn->left + ck->left, pt = (ck->pwn->top - TITLEH) - ck->top;
  if (mode == GL_RENDER)
  {
    if (focus && MouseOver(pl + 1, pt - 1, pl + 20, pt - 20))
    {
      glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
      glRecti(pl, pt, pl + 20, pt - 20);
    }
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINE_LOOP);
      glVertex2i(pl, pt);
      glVertex2i(pl + 20, pt);
      glVertex2i(pl + 20, pt - 20);
      glVertex2i(pl, pt - 20);
    glEnd();
    if (ck->state)
    {
      glColor3ub(white[0], white[1], white[2]);
      glRasterPos2i(pl + 7, pt - 15);
      glBitmap(8, 11, 0.0, 0.0, 0.0, 0.0, tick);  //tick check object
    }
  }
  else if (focus)  //check object selectable
  {
    glLoadName((ck->pwn->name * 10000) + (ck->name * 100) + GLWIN_STATE);
    glRecti(pl + 1, pt - 1, pl + 20, pt - 20);
  }
}

//draw input object
void MyGLWin::DrawInput(GLenum mode, glin_obj *in)
{
  bool focus = (in->pwn->name == (selected / 10000));
  int pl = in->pwn->left + in->left, pt = (in->pwn->top - TITLEH) - in->top, pr = pl + ((in->cwidth + 2) * 6);
  if (mode == GL_RENDER)
  {
    char *ch = in->text + in->first;
    unsigned int cnt = 0;
    GLint rpos[4] = {0, 0, 0, 0};
    if (focus && MouseOver(pl + 1, pt - 1, pr, pt - 20))
    {
      glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
      glRecti(pl, pt, pr, pt - 20);
    }
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINE_LOOP);
      glVertex2i(pl, pt);
      glVertex2i(pr, pt);
      glVertex2i(pr, pt - 20);
      glVertex2i(pl, pt - 20);
    glEnd();
    glColor3ub(white[0], white[1], white[2]);
    glRasterPos2i(pl + 6, pt - 15);
    do {
      if ((in->name == currInput) && (cnt++ == (in->cursor - in->first)) && ((milliTime(0) % 1000) > 500)) glCallList(fontDraw + 96);  //cursor
      if (!*ch || (rpos[0] > (pr - 12))) break;
      DrawChar(*ch);
      ch++;
      glGetIntegerv(GL_CURRENT_RASTER_POSITION, rpos);
    } while (true);
  }
  else if (focus)  //input object selectable to gain focus (for keys)
  {
    glLoadName((in->pwn->name * 10000) + (in->name * 100) + GLWIN_TEXT);
    glRecti(pl + 1, pt - 1, pr, pt - 20);
  }
}

//draw label object
void MyGLWin::DrawLabel(gllb_obj *lb)
{
  glColor3ub(white[0], white[1], white[2]);
  glRasterPos2i(lb->pwn->left + lb->left, (lb->pwn->top - TITLEH) - (lb->top + 10));
  DrawString((const unsigned char *)lb->text);
}

//draw scroll objects
void MyGLWin::DrawScrollObj(glvw_obj *vw, int pl, int pt, int pr, int pb, bool focus, bool lines)
{
  if (vw->rbefore || vw->cbefore)
  {
    if (focus && MouseOver(pr - 11, pb + 11, pr, pb))
    {
      glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
      glRecti(pr - 12, pb + 12, pr, pb);
    }
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glRasterPos2i(pr - 9, pb + 3);
    glBitmap(7, 7, 0.0, 0.0, 0.0, 0.0, syms[1]);  //start scroll object
  }
  if (vw->rbefore || vw->rafter)
  {
    if (vw->rbefore)
    {
      if (focus && MouseOver(pr - 11, pt - 1, pr, pt - 12))
      {
        glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
        glRecti(pr - 12, pt, pr, pt - 12);
      }
      glColor3ub(brblue[0], brblue[1], brblue[2]);
      glRasterPos2i(pr - 9, pt - 9);
      glBitmap(7, 7, 0.0, 0.0, 0.0, 0.0, syms[2]);  //up-arrow scroll object
    }
    if (vw->rafter)
    {
      if (focus && MouseOver(pr - 11, pb + 23, pr, pb + 12))
      {
        glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
        glRecti(pr - 12, pb + 24, pr, pb + 12);
      }
      glColor3ub(brblue[0], brblue[1], brblue[2]);
      glRasterPos2i(pr - 9, pb + 15);
      glBitmap(7, 7, 0.0, 0.0, 0.0, 0.0, syms[3]);  //down-arrow scroll object
    }
    if ((vw->rbefore + vw->rafter) <= ((pt - pb) - 52)) vw->rstep = 1;
    else vw->rstep = ((vw->rbefore + vw->rafter) / ((pt - pb) - 52)) + 1;
    if (focus && MouseOver(pr - 9, (pt - 15) - (vw->rbefore / vw->rstep), pr - 2, pb + 26 + (vw->rafter / vw->rstep)))
    {
      glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
      glRecti(pr - 10, (pt - 14) - (vw->rbefore / vw->rstep), pr - 2, pb + 26 + (vw->rafter / vw->rstep));
    }
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINE_LOOP);
      glVertex2i(pr - 10, (pt - 14) - (vw->rbefore / vw->rstep));
      glVertex2i(pr - 2, (pt - 14) - (vw->rbefore / vw->rstep));
      glVertex2i(pr - 2, pb + 26 + (vw->rafter / vw->rstep));
      glVertex2i(pr - 10, pb + 26 + (vw->rafter / vw->rstep));
    glEnd();
    glBegin(GL_LINES);
      glVertex2i(pr - 12, pt);
      glVertex2i(pr - 12, pb);
      glVertex2i(pr - 12, pt - 12);
      glVertex2i(pr, pt - 12);
      glVertex2i(pr - 12, pb + 24);
      glVertex2i(pr, pb + 24);
      glVertex2i(pr - 12, pb + 12);
      glVertex2i(pr, pb + 12);
    glEnd();
  }
  else if (lines)
  {
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINES);
      glVertex2i(pr - 12, pt);
      glVertex2i(pr - 12, pb);
    glEnd();
  }
  if (vw->cbefore || vw->cafter)
  {
    if (vw->cbefore)
    {
      if (focus && MouseOver(pl + 1, pb + 11, pl + 12, pb))
      {
        glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
        glRecti(pl, pb + 12, pl + 12, pb);
      }
      glColor3ub(brblue[0], brblue[1], brblue[2]);
      glRasterPos2i(pl + 3, pb + 3);
      glBitmap(7, 7, 0.0, 0.0, 0.0, 0.0, syms[4]);  //left-arrow scroll object
    }
    if (vw->cafter)
    {
      if (focus && MouseOver(pr - 23, pb + 11, pr - 12, pb))
      {
        glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
        glRecti(pr - 24, pb + 12, pr - 12, pb);
      }
      glColor3ub(brblue[0], brblue[1], brblue[2]);
      glRasterPos2i(pr - 21, pb + 3);
      glBitmap(7, 7, 0.0, 0.0, 0.0, 0.0, syms[5]);  //right-arrow scroll object
    }
    if ((vw->cbefore + vw->cafter) <= ((pr - pl) - 52)) vw->cstep = 1;
    else vw->cstep = ((vw->cbefore + vw->cafter) / ((pr - pl) - 52)) + 1;
    if (focus && MouseOver(pl + 15 + (vw->cbefore / vw->cstep), pb + 9, (pr - 26) - (vw->cafter / vw->cstep), pb + 2))
    {
      glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
      glRecti(pl + 14 + (vw->cbefore / vw->cstep), pb + 10, (pr - 26) - (vw->cafter / vw->cstep), pb + 2);
    }
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINE_LOOP);
      glVertex2i(pl + 14 + (vw->cbefore / vw->cstep), pb + 10);
      glVertex2i((pr - 26) - (vw->cafter / vw->cstep), pb + 10);
      glVertex2i((pr - 26) - (vw->cafter / vw->cstep), pb + 2);
      glVertex2i(pl + 14 + (vw->cbefore / vw->cstep), pb + 2);
    glEnd();
    glBegin(GL_LINES);
      glVertex2i(pl, pb + 12);
      glVertex2i(pr, pb + 12);
      glVertex2i(pr - 24, pb + 12);
      glVertex2i(pr - 24, pb);
      glVertex2i(pr - 12, pb + 12);
      glVertex2i(pr - 12, pb);
      glVertex2i(pl + 12, pb + 12);
      glVertex2i(pl + 12, pb);
    glEnd();
  }
  else if (lines)
  {
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINES);
      glVertex2i(pl, pb + 12);
      glVertex2i(pr, pb + 12);
    glEnd();
  }
}

//draw scroll selection objects
void MyGLWin::DrawScrollSel(glvw_obj *vw, int pl, int pt, int pr, int pb)
{
  glLoadName((vw->pwn->name * 10000) + (vw->name * 100) + GLWIN_START);
  glRecti(pr - 11, pb + 11, pr, pb);
  glLoadName((vw->pwn->name * 10000) + (vw->name * 100) + GLWIN_UP);
  glRecti(pr - 11, pt - 1, pr, pt - 12);
  glLoadName((vw->pwn->name * 10000) + (vw->name * 100) + GLWIN_SPCUP);
  glRecti(pr - 11, pt - 13, pr, (pt - 14) - (vw->rbefore / vw->rstep));
  glLoadName((vw->pwn->name * 10000) + (vw->name * 100) + GLWIN_DOWN);
  glRecti(pr - 11, pb + 23, pr, pb + 12);
  glLoadName((vw->pwn->name * 10000) + (vw->name * 100) + GLWIN_SPCDN);
  glRecti(pr - 11, pb + 25 + (vw->rafter / vw->rstep), pr, pb + 24);
  glLoadName((vw->pwn->name * 10000) + (vw->name * 100) + GLWIN_LEFT);
  glRecti(pl + 1, pb + 11, pl + 14 + (vw->cbefore / vw->cstep), pb);
  glLoadName((vw->pwn->name * 10000) + (vw->name * 100) + GLWIN_RIGHT);
  glRecti((pr - 25) - (vw->cafter / vw->cstep), pb + 11, pr - 12, pb);
  glLoadName((vw->pwn->name * 10000) + (vw->name * 100) + GLWIN_DBAR);
  glRecti(pr - 9, (pt - 15) - (vw->rbefore / vw->rstep), pr - 2, pb + 26 + (vw->rafter / vw->rstep));
  glLoadName((vw->pwn->name * 10000) + (vw->name * 100) + GLWIN_RBAR);
  glRecti(pl + 15 + (vw->cbefore / vw->cstep), pb + 9, (pr - 26) - (vw->cafter / vw->cstep), pb + 2);

}

//draw list object
void MyGLWin::DrawList(GLenum mode, glls_obj *ls)
{
  bool focus = (ls->pwn->name == (selected / 10000));
  int pl = ls->pwn->left + ls->left, pt = (ls->pwn->top - TITLEH) - ls->top, pr = (ls->pwn->left + ls->pwn->width) - ls->right
    , pb = (ls->pwn->top - ls->pwn->height) + ls->bottom, itms = ((pt - pb) - 12) / 20, cnt;
  if (itms > LSTITMS) itms = LSTITMS;
  if (mode == GL_RENDER)
  {
    FILE *text;
    if ((text = fopen(ls->fname, "r")))
    {
      char chr, *end, *ch;
      int aft = 0, py = pt;
      GLint rpos[4];
      for (cnt = 0; cnt < ls->rbefore; cnt++)
      {
        while (((chr = getc(text)) != EOF) && (chr != '\n'));  //nothing
      }
      ls->rafter = 0;
      ls->cafter = 0;
      for (cnt = 0; cnt < itms; cnt++)
      {
        if (fgets(ls->items[cnt], 256, text))
        {
          if ((end = strchr(ls->items[cnt], '\n'))) *end = '\0';  //remove \n
          else while (((chr = getc(text)) != EOF) && (chr != '\n'));  //nothing
          if (focus && MouseOver(pl + 1, py - 1, pr - 12, py - 20))
          {
            glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
            glRecti(pl, py, pr - 12, py - 20);
          }
          else if (!strcmp(ls->lin->text, ls->items[cnt]))
          {
            glColor3ub(slblue[0], slblue[1], slblue[2]);
            glRecti(pl, py, pr - 12, py - 20);
          }
          glColor3ub(white[0], white[1], white[2]);
          glRasterPos2i(pl + 6, py - 15);
          ch = ls->items[cnt] + ls->cbefore;
          rpos[0] = 0;
          while (*ch && (rpos[0] <= (pr - 24)))  //12 + 6 + 6
          {
            DrawChar(*ch);
            ch++;
            glGetIntegerv(GL_CURRENT_RASTER_POSITION, rpos);
          }
          while (*ch)
          {
            aft++;
            ch++;
          }
          if (aft > ls->cafter) ls->cafter = aft;
          aft = 0;
          py -= 20;
        }
        else break;
      }
      while ((chr = getc(text)) != EOF)
      {
        if (chr == '\n') ls->rafter++;
      }
      fclose(text);
      DrawScrollObj((glvw_obj *)ls, pl, pt, pr, pb, focus, true);
    }
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINE_LOOP);
      glVertex2i(pl, pt);
      glVertex2i(pr, pt);
      glVertex2i(pr, pb);
      glVertex2i(pl, pb);
    glEnd();
  }
  else if (focus)  //list items/scroll objects selectable, also give object focus (for mouse wheel)
  {
    for (cnt = 0; cnt < itms; cnt++)
    {
      glLoadName((ls->pwn->name * 10000) + (ls->name * 100) + GLWIN_ITEM + cnt);
      glRecti(pl + 1, pt - ((cnt * 20) + 1), pr - 12, pt - ((cnt + 1) * 20));
    }
    DrawScrollSel((glvw_obj *)ls, pl, pt, pr, pb);
  }
}

//draw view object
void MyGLWin::DrawView(GLenum mode, glvw_obj *vw)
{
  bool focus = (vw->pwn->name == (selected / 10000));
  int pl = vw->pwn->left + vw->left, pt = (vw->pwn->top - TITLEH) - vw->top, pr = (vw->pwn->left + vw->pwn->width) - vw->right
    , pb = (vw->pwn->top - vw->pwn->height) + vw->bottom;
  if (mode == GL_RENDER)
  {
    FILE *text;
    if ((text = fopen(vw->fname, "r")))
    {
      char chr;
      int cnt, aft = 0, py = pt - 20;
      GLint rpos[4];
      for (cnt = 0; cnt < vw->rbefore; cnt++)
      {
        while (((chr = getc(text)) != EOF) && (chr != '\n'));  //nothing
      }
      vw->rafter = 0;
      vw->cafter = 0;
      cnt = 0;
      glColor3ub(white[0], white[1], white[2]);
      glRasterPos2i(pl + 10, py);
      chr = getc(text);
      while (chr != EOF)
      {
        if (chr == '\n')  //CR
        {
          if ((chr = getc(text)) != EOF)
          {
            py -= 13;
            if (py >= (pb + (vw->cbefore || vw->cafter ? 22 : 10))) glRasterPos2i(pl + 10, py);  //12 + 10
            else
            {
              while ((chr = getc(text)) != EOF)
              {
                if (chr == '\n') vw->rafter++;
              }
              break;
            }
            if (aft > vw->cafter) vw->cafter = aft;
            aft = 0;
            cnt = 0;
          }
        }
        else
        {
          if (chr == '\t')  //tab
          {
            if (vw->tab > vw->cbefore) glRasterPos2i(pl + 10 + ((vw->tab - vw->cbefore) * 6), py);
            cnt = vw->tab;
          }
          else if (++cnt > vw->cbefore)
          {
            glGetIntegerv(GL_CURRENT_RASTER_POSITION, rpos);
            if (rpos[0] <= (pr - (vw->rbefore || vw->rafter ? 28 : 16))) DrawChar(chr);  //12 + 10 + 6
            else aft++;
          }
          chr = getc(text);
        }
      }
      fclose(text);
      DrawScrollObj(vw, pl, pt, pr, pb, focus, false);
    }
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINE_LOOP);
      glVertex2i(pl, pt);
      glVertex2i(pr, pt);
      glVertex2i(pr, pb);
      glVertex2i(pl, pb);
    glEnd();
  }
  else if (focus) DrawScrollSel(vw, pl, pt, pr, pb);  //scroll objects selectable, also give object focus (for mouse wheel)
}

//draw menu object
void MyGLWin::DrawMenu(GLenum mode, glmu_obj *mu)
{
  int pr = mu->left + mu->width;
  if (mode == GL_RENDER)
  {
    if (MouseOver(mu->left + 1, mu->top - 1, pr, mu->top - MENUH) && mu->value) glColor3ub(hlblue[0], hlblue[1], hlblue[2]);
    else glColor3ub(dlblue[0], dlblue[1], dlblue[2]);
    glRecti(mu->left, mu->top, pr, mu->top - MENUH);
    glColor3ub(brblue[0], brblue[1], brblue[2]);
    glBegin(GL_LINE_LOOP);
      glVertex2i(mu->left, mu->top);
      glVertex2i(pr, mu->top);
      glVertex2i(pr, mu->top - MENUH);
      glVertex2i(mu->left, mu->top - MENUH);
    glEnd();
    if (mu->sub)
    {
      glRasterPos2i(pr - 10, mu->top - 12);
      glBitmap(4, 7, 0.0, 0.0, 0.0, 0.0, syms[(mu->sub == 1 ? 6 : 7)]);  //point object
    }
    glColor3ub(white[0], white[1], white[2]);
    glRasterPos2i(mu->left + 6, mu->top - 14);
    DrawString((const unsigned char *)mu->text);
  }
  else  //menu object selectable
  {
    glLoadName((mu->value * 100) + GLWIN_MNUITM);
    glRecti(mu->left + 1, mu->top - 1, pr, mu->top - MENUH);
  }
}

//make next/prev input object currInput
void MyGLWin::TabInput(bool nx)
{
  glin_obj *in;
  unsigned char *tp;
  (nx ? GLWinLL.Start(1) : GLWinLL.End(1));
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    if (*tp == GLWIN_INPUT)
    {
      in = (glin_obj *)tp;
      if (in->name == currInput) break;
    }
    (nx ? GLWinLL.Next(1) : GLWinLL.Prev(1));
  }
  (nx ? GLWinLL.Next(1) : GLWinLL.Prev(1));
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    if (*tp == GLWIN_INPUT)
    {
      in = (glin_obj *)tp;
      if (in->pwn->name == (selected / 10000))
      {
        currInput = in->name;
        return;
      }
    }
    (nx ? GLWinLL.Next(1) : GLWinLL.Prev(1));
  }
  (nx ? GLWinLL.Start(1) : GLWinLL.End(1));
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    if (*tp == GLWIN_INPUT)
    {
      in = (glin_obj *)tp;
      if (in->pwn->name == (selected / 10000))
      {
        currInput = in->name;
        break;
      }
    }
    (nx ? GLWinLL.Next(1) : GLWinLL.Prev(1));
  }
}

//destroy child objects for parent window (pw), or all objects if passed null
void MyGLWin::GLWinDestroy(glwn_obj *pw)
{
  glwn_obj *wn;
  glbm_obj *bm;
  glbt_obj *bt;
  glck_obj *ck;
  glin_obj *in;
  gllb_obj *lb;
  glls_obj *ls;
  glvw_obj *vw;
  glmu_obj *mu;
  unsigned char *tp;
  GLWinLL.Start(1);
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    switch (*tp)
    {
      case GLWIN_WINDOW:
        if (!pw)
        {
          wn = (glwn_obj *)tp;
          delete wn;
          GLWinLL.Delete(1);
        }
        else GLWinLL.Next(1);
        break;
      case GLWIN_BITMAP:
        bm = (glbm_obj *)tp;
        if (!pw || (bm->pwn == pw))
        {
          delete bm;
          GLWinLL.Delete(1);
        }
        else GLWinLL.Next(1);
        break;
      case GLWIN_BUTTON:
        bt = (glbt_obj *)tp;
        if (!pw || (bt->pwn == pw))
        {
          delete bt;
          GLWinLL.Delete(1);
        }
        else GLWinLL.Next(1);
        break;
      case GLWIN_CHECK:
        ck = (glck_obj *)tp;
        if (!pw || (ck->pwn == pw))
        {
          delete ck;
          GLWinLL.Delete(1);
        }
        else GLWinLL.Next(1);
        break;
      case GLWIN_INPUT:
        in = (glin_obj *)tp;
        if (!pw || (in->pwn == pw))
        {
          delete in;
          GLWinLL.Delete(1);
        }
        else GLWinLL.Next(1);
        break;
      case GLWIN_LABEL:
        lb = (gllb_obj *)tp;
        if (!pw || (lb->pwn == pw))
        {
          delete lb;
          GLWinLL.Delete(1);
        }
        else GLWinLL.Next(1);
        break;
      case GLWIN_LIST:
        ls = (glls_obj *)tp;
        if (!pw || (ls->pwn == pw))
        {
          delete ls;
          GLWinLL.Delete(1);
        }
        else GLWinLL.Next(1);
        break;
      case GLWIN_VIEW:
        vw = (glvw_obj *)tp;
        if (!pw || (vw->pwn == pw))
        {
          delete vw;
          GLWinLL.Delete(1);
        }
        else GLWinLL.Next(1);
        break;
      case GLWIN_MENU:
        if (!pw)
        {
          mu = (glmu_obj *)tp;
          delete mu;
          GLWinLL.Delete(1);
        }
        else GLWinLL.Next(1);
        break;
    }
  }
}

//2D GUI active
bool MyGLWin::On()
{
  return GLWinLL.Num();
}

void MyGLWin::InitFont()
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  fontDraw = glGenLists(97);
  for (unsigned char cnt = 0; cnt < 96; cnt++)
  {
    glNewList(fontDraw + cnt, GL_COMPILE);
    glBitmap(5, 10, 0.0, 0.0, 6.0, 0.0, GABE_5X10[cnt]);
    glEndList();
  }
  glNewList(fontDraw + 96, GL_COMPILE);
  glBitmap(1, 11, 0.0, 0.0, 0.0, 0.0, cursor);
  glEndList();
}

void MyGLWin::DrawChar(const unsigned char dchr)
{
  unsigned char chr = dchr - 32;
  glCallList(fontDraw + (chr > 95 ? 95 : chr));
}

void MyGLWin::DrawString(const unsigned char *dstr)
{
  GLint rpos[4] = {0, 0, 0, 0};
  glGetIntegerv(GL_CURRENT_RASTER_POSITION, rpos);
  for (unsigned int cnt = 0; dstr[cnt]; cnt++)
  {
    if (dstr[cnt] == '\n')
    {
      rpos[1] -= 13;
      glRasterPos2i(rpos[0], rpos[1]);
    }
    else DrawChar(dstr[cnt]);
  }
}

//create window object
void MyGLWin::CreateWin(int left, int top, int width, int height, const char *title, bool close, bool resize)
{
  glwn_obj glwn;
  glwn.type = GLWIN_WINDOW;
  glwn.close = close;  //display close object
  glwn.resize = resize;  //display resize object
  glwn.name = names++;
  glwn.psel = selected;  //previous selected
  glwn.minw = width;  //minimum width
  glwn.width = ((left < -1) && (width < (screenW - 20)) ? screenW - 20 : width);
  glwn.minh = height;  //minimum height
  glwn.height = ((top < -1) && (height < (screenH - 20)) ? screenH - 20 : height);
  if (left > -1) glwn.left = left;
  else if (glwn.width < screenW) glwn.left = (screenW - glwn.width) / 2;
  else glwn.left = 0;
  if (top > -1) glwn.top = top;
  else if (glwn.height < screenH) glwn.top = (screenH - glwn.height) / 2;
  else glwn.top = 1;
  glwn.top = screenH - glwn.top;  //invert as 0 is at bottom of screen
  strcpy(glwn.title, title);  //title bar text
  selected = glwn.name * 10000;
  lastWin = (glwn_obj *)GLWinLL.Write(new glwn_obj(glwn));
}

//create bitmap object
void MyGLWin::AddBitmap(int left, int top, int red, int green, int blue, const GLubyte *bitmap)
{
  if (!lastWin) return;  //check parent window object exists
  glbm_obj glbm = {GLWIN_BITMAP, lastWin, bitmap, left, top, red, green, blue};
  GLWinLL.Write(new glbm_obj(glbm));
}

//create button object
void MyGLWin::AddButton(int left, int top, int value, const char *text, bool align, bool defalt)
{
  if (!lastWin) return;  //check parent window object exists
  glbt_obj glbt = {GLWIN_BUTTON, lastWin, align, defalt, left, top, value};
  strcpy(glbt.text, text);
  GLWinLL.Write(new glbt_obj(glbt));
}

//create check object
int MyGLWin::AddCheck(int left, int top, bool state)
{
  if (!lastWin) return -1;  //check parent window object exists
  glck_obj glck = {GLWIN_CHECK, lastWin, state, names++, left, top};
  GLWinLL.Write(new glck_obj(glck));
  return glck.name;
}

//create input object, return name of input object
int MyGLWin::AddInput(int left, int top, unsigned int cwidth, int max, const char *text, bool lower)
{
  if (!lastWin) return -1;  //check parent window object exists
  glin_obj glin = {GLWIN_INPUT, lastWin, lower, names++, left, top, cwidth, strlen(text), (strlen(text) > cwidth ? strlen(text) - cwidth : 0), max};
  strcpy(glin.text, text);  //default text
  currInput = glin.name;  //set input object focus (for keys)
  lastInput = (glin_obj *)GLWinLL.Write(new glin_obj(glin));
  return glin.name;
}

//create label object, return name of label object
int MyGLWin::AddLabel(int left, int top, const char *text)
{
  if (!lastWin) return -1;  //check parent window object exists
  gllb_obj gllb = {GLWIN_LABEL, lastWin, names++, left, top};
  strcpy(gllb.text, text);
  GLWinLL.Write(new gllb_obj(gllb));
  return gllb.name;
}

//create list object
void MyGLWin::AddList(int left, int top, int right, int bottom, const char *fl)
{
  if (!lastWin || !lastInput) return;  //check parent window/linked input object exists
  glls_obj glls = {GLWIN_LIST, lastWin, names++, left, top, right, bottom, 0, 0, 1, 0, 0, 1, lastInput};
  for (int cnt = 0; cnt < LSTITMS; cnt++) strcpy(glls.items[cnt], "");
  strcpy(glls.fname, fl);  //file to list
  currScroll = (glls.pwn->name * 10000) + (glls.name * 100);  //set scroll object focus (for mouse wheel)
  GLWinLL.Write(new glls_obj(glls));
}

//create view object
void MyGLWin::AddView(int left, int top, int right, int bottom, int tab, const char *fl)
{
  if (!lastWin) return;  //check parent window object exists
  glvw_obj glvw = {GLWIN_VIEW, lastWin, names++, left, top, right, bottom, 0, 0, 1, 0, 0, 1, tab};
  strcpy(glvw.fname, fl);  //file to view
  currScroll = (glvw.pwn->name * 10000) + (glvw.name * 100);  //set scroll object focus (for mouse wheel)
  GLWinLL.Write(new glvw_obj(glvw));
}

//create menu object
void MyGLWin::AddMenu(int width, const char *text, int itms, int sub, int value)
{
  glmu_obj glmu;
  glmu.type = GLWIN_MENU;
  if (mouseX < (screenW - (width + 1))) glmu.left = mouseX;
  else glmu.left = screenW - (width + 1);
  if (glmu.left < 0) glmu.left = 0;
  if (mouseY > (itms * MENUH)) glmu.top = mouseY;
  else glmu.top = itms * MENUH;
  if (glmu.top > (screenH - 1)) glmu.top = screenH - 1;
  glmu.top -= menuY;
  menuY += MENUH;
  glmu.width = width;
  glmu.sub = sub;  //display sub-menu object
  glmu.value = value;
  strcpy(glmu.text, text);
  GLWinLL.Write(new glmu_obj(glmu));
}

//draw all 2D GUI objects
void MyGLWin::Draw(GLenum mode)
{
  glwn_obj *wn;
  glbm_obj *bm;
  glbt_obj *bt;
  glck_obj *ck;
  glin_obj *in;
  gllb_obj *lb;
  glls_obj *ls;
  glvw_obj *vw;
  glmu_obj *mu;
  unsigned char *tp;
  GLWinLL.Start(1);
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    switch (*tp)
    {
      case GLWIN_WINDOW:
        wn = (glwn_obj *)tp;
        DrawWin(mode, wn);
        break;
      case GLWIN_BITMAP:
        bm = (glbm_obj *)tp;
        DrawBitmap(bm);
        break;
      case GLWIN_BUTTON:
        bt = (glbt_obj *)tp;
        DrawButton(mode, bt);
        break;
      case GLWIN_CHECK:
        ck = (glck_obj *)tp;
        DrawCheck(mode, ck);
        break;
      case GLWIN_INPUT:
        in = (glin_obj *)tp;
        DrawInput(mode, in);
        break;
      case GLWIN_LABEL:
        lb = (gllb_obj *)tp;
        DrawLabel(lb);
        break;
      case GLWIN_LIST:
        ls = (glls_obj *)tp;
        DrawList(mode, ls);
        break;
      case GLWIN_VIEW:
        vw = (glvw_obj *)tp;
        DrawView(mode, vw);
        break;
      case GLWIN_MENU:
        mu = (glmu_obj *)tp;
        DrawMenu(mode, mu);
        break;
    }
    GLWinLL.Next(1);
  }
}

//return default button value
int MyGLWin::DefaultButton()
{
  glbt_obj *bt;
  unsigned char *tp;
  GLWinLL.Start(1);
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    if (*tp == GLWIN_BUTTON)
    {
      bt = (glbt_obj *)tp;
      if ((bt->pwn->name == (selected / 10000)) && bt->defalt) return bt->value;
    }
    GLWinLL.Next(1);
  }
  return 0;
}

//return check object state
bool MyGLWin::GetCheck(int name)
{
  glck_obj *ck;
  unsigned char *tp;
  GLWinLL.Start(1);
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    if (*tp == GLWIN_CHECK)
    {
      ck = (glck_obj *)tp;
      if (ck->name == name) return ck->state;
    }
    GLWinLL.Next(1);
  }
  return false;
}

//return input object text
char *MyGLWin::GetInput(int name)
{
  int gn = (name ? name : currInput);
  glin_obj *in;
  unsigned char *tp;
  GLWinLL.Start(1);
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    if (*tp == GLWIN_INPUT)
    {
      in = (glin_obj *)tp;
      if (in->name == gn) return in->text;
    }
    GLWinLL.Next(1);
  }
  return 0;
}

//change input object text
void MyGLWin::PutInput(const char *text, int name)
{
  int gn = (name ? name : currInput);
  glin_obj *in;
  unsigned char *tp;
  GLWinLL.Start(1);
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    if (*tp == GLWIN_INPUT)
    {
      in = (glin_obj *)tp;
      if (in->name == gn)
      {
        strncpy(in->text, text, in->max);
        if (strlen(text) >= in->max) in->text[in->max] = '\0';
        in->cursor = strlen(in->text);
        in->first = (in->cursor > in->cwidth ? in->cursor - in->cwidth : 0);
        break;
      }
    }
    GLWinLL.Next(1);
  }
}

//change label object text
void MyGLWin::PutLabel(const char *text, int name)
{
  gllb_obj *lb;
  unsigned char *tp;
  GLWinLL.Start(1);
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    if (*tp == GLWIN_LABEL)
    {
      lb = (gllb_obj *)tp;
      if (lb->name == name)
      {
        strcpy(lb->text, text);
        break;
      }
    }
    GLWinLL.Next(1);
  }
}

//return selected object
int MyGLWin::GetSelected()
{
  return (selected / 100);
}

//update screen size
void MyGLWin::ScreenSize(int w, int h)
{
  screenW = w;
  screenH = h;
}

//update mouse position
void MyGLWin::MousePos(int x, int y)
{
  mouseX = x;
  mouseY = y;
}

//mouse left-click and drag motion
void MyGLWin::Motion(int x, int y)
{
  if (((selected % 100) == GLWIN_UBAR) || ((selected % 100) == GLWIN_DBAR)) Scroll((y < 0 ? GLWIN_UBAR : GLWIN_DBAR), false);
  else if (((selected % 100) == GLWIN_LBAR) || ((selected % 100) == GLWIN_RBAR)) Scroll((x < 0 ? GLWIN_LBAR : GLWIN_RBAR), false);
  else
  {
    glwn_obj *wn;
    unsigned char *tp;
    GLWinLL.Start(1);
    while ((tp = (unsigned char *)GLWinLL.Read(1)))
    {
      if (*tp == GLWIN_WINDOW)
      {
        wn = (glwn_obj *)tp;
        if (wn->name == (selected / 10000))
        {
          switch (selected % 100)
          {
            case GLWIN_TITLE:  //move window object
              wn->left += x;
              if (wn->left < 0) wn->left = 0;
              else if (wn->left > (screenW - 20)) wn->left = screenW - 20;
              wn->top -= y;
              if (wn->top > (screenH - 1)) wn->top = screenH - 1;
              else if (wn->top < 20) wn->top = 20;
              break;
            case GLWIN_RESIZE:  //resize window object
              wn->width += x;
              if (wn->width < wn->minw) wn->width = wn->minw;
              else if (wn->width > (screenW - (wn->left + 1))) wn->width = screenW - (wn->left + 1);
              wn->height += y;
              if (wn->height < wn->minh) wn->height = wn->minh;
              else if (wn->height > wn->top) wn->height = wn->top;
              break;
          }
          break;
        }
      }
      GLWinLL.Next(1);
    }
  }
}

//scroll calculation with direction and opposite
void ScrollCalc(int *direct, int *oposit, int step)
{
  if (*direct)
  {
    if (*direct < step)
    {
      *oposit += *direct;
      *direct = 0;
    }
    else
    {
      *oposit += step;
      *direct -= step;
    }
  }
}

//scroll object/mouse wheel action
void MyGLWin::Scroll(int gs, bool one)
{
  if (gs)
  {
    if (!currScroll) return;
    selected = currScroll + gs;
  }
  glvw_obj *vw;
  unsigned char *tp;
  GLWinLL.Start(1);
  while ((tp = (unsigned char *)GLWinLL.Read(1)))
  {
    if ((*tp == GLWIN_LIST) || (*tp == GLWIN_VIEW))
    {
      vw = (glvw_obj *)tp;
      if (vw->name == ((selected % 10000) / 100))
      {
        switch (selected % 100)
        {
          case GLWIN_START:
            vw->rbefore = 0;
            vw->cbefore = 0;
            break;
          case GLWIN_UBAR: case GLWIN_UP: case GLWIN_SPCUP: ScrollCalc(&vw->rbefore, &vw->rafter, (one ? 1 : vw->rstep)); break;
          case GLWIN_DBAR: case GLWIN_DOWN: case GLWIN_SPCDN: ScrollCalc(&vw->rafter, &vw->rbefore, (one ? 1 : vw->rstep)); break;
          case GLWIN_LBAR: case GLWIN_LEFT: ScrollCalc(&vw->cbefore, &vw->cafter, (one ? 1 : vw->cstep)); break;
          case GLWIN_RBAR: case GLWIN_RIGHT: ScrollCalc(&vw->cafter, &vw->cbefore, (one ? 1 : vw->cstep)); break;
        }
        currScroll = (vw->pwn->name * 10000) + (vw->name * 100);  //set scroll object focus (for mouse wheel)
        break;
      }
    }
    GLWinLL.Next(1);
  }
}

//close window object and child objects, or all objects
void MyGLWin::Close(bool all)
{
  if (all) GLWinDestroy(0);
  else
  {
    glwn_obj *wn;
    unsigned char *tp;
    GLWinLL.Start(2);
    while ((tp = (unsigned char *)GLWinLL.Read(2)))
    {
      if (*tp == GLWIN_WINDOW)
      {
        wn = (glwn_obj *)tp;
        if (wn->name == (selected / 10000))
        {
          selected = wn->psel;  //previous selected
          GLWinDestroy(wn);
          delete wn;
          GLWinLL.Delete(2);
          break;
        }
      }
      GLWinLL.Next(2);
    }
  }
  if (!On()) Reset();
}

//mouse left-click object selected
int MyGLWin::Select(bool btnup)
{
  GLint hits, vpt[4];
  GLuint selbuf[4];
  glGetIntegerv(GL_VIEWPORT, vpt);
  glSelectBuffer(4, selbuf);
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(0);
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(mouseX, mouseY, 1.0, 1.0, vpt);
  gluOrtho2D(0.0, (GLdouble)screenW, 0.0, (GLdouble)screenH);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  Draw(GL_SELECT);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glEnable(GL_DEPTH_TEST);
  if (!(hits = glRenderMode(GL_RENDER)))
  {
    if (btnup && menuY) Close();
    return 0;
  }
  selected = selbuf[3];
  int sel = selected % 100;
  if (btnup)
  {
    unsigned char *tp;
    switch (sel)
    {
      case GLWIN_STATE:
        glck_obj *ck;
        GLWinLL.Start(1);
        while ((tp = (unsigned char *)GLWinLL.Read(1)))
        {
          if (*tp == GLWIN_CHECK)
          {
            ck = (glck_obj *)tp;
            if (ck->name == ((selected % 10000) / 100))
            {
              ck->state = !ck->state;
              break;
            }
          }
          GLWinLL.Next(1);
        }
        break;
      case GLWIN_TEXT:
        currInput = (selected % 10000) / 100;  //set input object focus (for keys)
        glin_obj *in;
        GLWinLL.Start(1);
        while ((tp = (unsigned char *)GLWinLL.Read(1)))
        {
          if (*tp == GLWIN_INPUT)
          {
            in = (glin_obj *)tp;
            if (in->name == currInput)
            {
              if (mouseX < (in->pwn->left + in->left + 3))
              {
                if (in->first) in->first--;
                in->cursor = in->first;
              }
              else
              {
                in->cursor = in->first + ((mouseX - (in->pwn->left + in->left + 3)) / 6);
                if (in->cursor > strlen(in->text)) in->cursor = strlen(in->text);
                else if ((in->cursor - in->first) > in->cwidth) in->first++;
              }
              break;
            }
          }
          GLWinLL.Next(1);
        }
        break;
      case GLWIN_UP: case GLWIN_DOWN: case GLWIN_LEFT: case GLWIN_RIGHT: Scroll(0, true); break;
      case GLWIN_START: case GLWIN_SPCUP: case GLWIN_SPCDN: Scroll(0, false); break;
      default:
        if ((sel >= GLWIN_ITEM) && (sel < (GLWIN_ITEM + LSTITMS)))
        {
          glls_obj *ls;
          GLWinLL.Start(1);
          while ((tp = (unsigned char *)GLWinLL.Read(1)))
          {
            if (*tp == GLWIN_LIST)
            {
              ls = (glls_obj *)tp;
              if (ls->name == ((selected % 10000) / 100))
              {
                strcpy(ls->lin->text, ls->items[sel - GLWIN_ITEM]);  //set linked input object text
                ls->lin->cursor = strlen(ls->lin->text);
                ls->lin->first = (ls->lin->cursor > ls->lin->cwidth ? ls->lin->cursor - ls->lin->cwidth : 0);
                currScroll = (ls->pwn->name * 10000) + (ls->name * 100);  //set scroll object focus (for mouse wheel)
                break;
              }
            }
            GLWinLL.Next(1);
          }
        }
    }
  }
  return sel;
}

//keyboard input
void MyGLWin::Key(int key, bool shift)
{
  if (!currInput) return;
  if ((key == GLFW_KEY_TAB) || (key == 805)) TabInput((key == GLFW_KEY_TAB));
  else
  {
    unsigned int cnt;
    glin_obj *in;
    unsigned char *tp;
    GLWinLL.Start(1);
    while ((tp = (unsigned char *)GLWinLL.Read(1)))
    {
      if (*tp == GLWIN_INPUT)
      {
        in = (glin_obj *)tp;
        if (in->name == currInput)  //input object with focus
        {
          switch (key)
          {
            case GLFW_KEY_BACKSPACE:
              if (in->cursor)
              {
                for (cnt = in->cursor - 1; cnt < strlen(in->text); cnt++) in->text[cnt] = in->text[cnt + 1];
                in->cursor--;
                if (in->first) in->first--;
              }
              break;
            case GLFW_KEY_LEFT:
              if (in->cursor)
              {
                in->cursor--;
                if (in->cursor < in->first) in->first--;
              }
              break;
            case GLFW_KEY_RIGHT:
              if (in->cursor < strlen(in->text))
              {
                in->cursor++;
                if ((in->cursor - in->first) > in->cwidth) in->first++;
              }
              break;
            case GLFW_KEY_DEL:
              if (in->cursor < strlen(in->text))
              {
                for (cnt = in->cursor; cnt < strlen(in->text); cnt++) in->text[cnt] = in->text[cnt + 1];
                if (in->first && ((in->cursor - in->first) < in->cwidth)) in->first--;
              }
              break;
            default:
              if ((key >= GLFW_KEY_SPACE) && (key <= 126) && (strlen(in->text) < in->max))  //~
              {
                if (shift)
                {
                  int nonan[42] = {96, 126, 49, 33, 50, 64, 51, 35, 52, 36, 53, 37, 54, 94, 55, 38, 56, 42, 57, 40
                    , 48, 41, 45, 95, 61, 43, 91, 123, 93, 125, 92, 124, 59, 58, 39, 34, 44, 60, 46, 62, 47, 63};  //non-alphanumeric
                  for (cnt = 0; cnt < 42; cnt += 2)
                  {
                    if (nonan[cnt] == key)
                    {
                      key = nonan[cnt + 1];
                      break;
                    }
                  }
                }
                else key = tolower(key);
                for (cnt = strlen(in->text) + 1; cnt > in->cursor; cnt--) in->text[cnt] = in->text[cnt - 1];
                in->text[in->cursor] = (in->lower ? tolower(key) : key);
                in->cursor++;
                if ((in->cursor - in->first) > in->cwidth) in->first++;
              }
          }
          break;
        }
      }
      GLWinLL.Next(1);
    }
  }
}

MyGLWin::~MyGLWin()
{
  GLWinDestroy(0);
  glDeleteLists(fontDraw, 97);
}
