/* glwin.h - 10 May 11
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

#include <GL/glfw.h>  //GLenum

#include "llist.h"

//selection value
#define GLWIN_OK      0x01
#define GLWIN_DEL     0x02
#define GLWIN_CLOSE   0x03
#define GLWIN_TITLE   0x04
#define GLWIN_RESIZE  0x05
#define GLWIN_UBAR    0x06
#define GLWIN_DBAR    0x07
#define GLWIN_LBAR    0x08
#define GLWIN_RBAR    0x09
#define GLWIN_STATE   0x0A
#define GLWIN_TEXT    0x0B
#define GLWIN_START   0x0C
#define GLWIN_UP      0x0D
#define GLWIN_DOWN    0x0E
#define GLWIN_LEFT    0x0F
#define GLWIN_RIGHT   0x10
#define GLWIN_SPCUP   0x11
#define GLWIN_SPCDN   0x12
#define GLWIN_MNUITM  0x13
#define GLWIN_MISC1   0x14
#define GLWIN_MISC2   0x15
#define GLWIN_ITMUP   0x16
#define GLWIN_ITMDN   0x17
#define GLWIN_ITEM    0x18

const int LSTITMS = 16;

//2D GUI
class MyGLWin
{
  private:
    struct glwn_obj  //window object
    {
      unsigned char type;  //GLWIN_WINDOW
      bool close, resize;  //display close, resize object
      int name, psel, left, top, width, height, minw, minh;  //minimum width, height
      char title[64];  //title bar text
    } *lastWin;  //last window created
    struct glbm_obj  //bitmap object
    {
      unsigned char type;  //GLWIN_BITMAP
      glwn_obj *pwn;  //parent window object
      const GLubyte *bitmap;
      int left, top, red, green, blue;
    };
    struct glbt_obj  //button object
    {
      unsigned char type;  //GLWIN_BUTTON
      glwn_obj *pwn;  //parent window object
      bool align, defalt;  //align top-left, default
      int left, top, value;  //selection value
      char text[16];
    };
    struct glck_obj  //check object
    {
      unsigned char type;  //GLWIN_CHECK
      glwn_obj *pwn;  //parent window object
      bool state;  //checked
      int name, left, top;
    };
    struct glin_obj  //input object
    {
      unsigned char type;  //GLWIN_INPUT
      glwn_obj *pwn;  //parent window object
      bool lower;  //lowercase
      int name, left, top;
      unsigned int cwidth, cursor, first, max;  //cursor position, first char to show, maximum length of text
      char text[256];  //default text
    } *lastInput;  //last input created
    struct gllb_obj  //label object
    {
      unsigned char type;  //GLWIN_LABEL
      glwn_obj *pwn;  //parent window object
      int name, left, top;
      char text[64];
    };
    struct glls_obj  //list object
    {
      unsigned char type;  //GLWIN_LIST
      glwn_obj *pwn;  //parent window object
      int name, left, top, right, bottom, rbefore, rafter, rstep, cbefore, cafter, cstep;
      glin_obj *lin;  //linked input object
      char items[LSTITMS][256], fname[256];  //file to list
    };
    struct glvw_obj  //view object
    {
      unsigned char type;  //GLWIN_VIEW
      glwn_obj *pwn;  //parent window object
      int name, left, top, right, bottom, rbefore, rafter, rstep, cbefore, cafter, cstep, tab;  //tab spacing
      char fname[256];  //file to view
    };
    struct glmu_obj  //menu object
    {
      unsigned char type;  //GLWIN_MENU
      int left, top, width, sub, value;  //display sub-menu object
      char text[64];
    };
    int names, selected, currInput, currScroll, screenW, screenH, mouseX, mouseY, menuY;  //input object with focus (for keys), scroll object with focus (for mouse wheel)
    GLuint fontDraw;  //GL compiled font
    MyLL GLWinLL;
    void Reset();
    bool MouseOver(int left, int top, int right, int bottom);
    void DrawWin(GLenum mode, glwn_obj *wn);
    void DrawBitmap(glbm_obj *bm);
    void DrawButton(GLenum mode, glbt_obj *bt);
    void DrawCheck(GLenum mode, glck_obj *ck);
    void DrawInput(GLenum mode, glin_obj *in);
    void DrawLabel(gllb_obj *lb);
    void DrawScrollObj(glvw_obj *vw, int pl, int pt, int pr, int pb, bool focus, bool lines);
    void DrawScrollSel(glvw_obj *vw, int pl, int pt, int pr, int pb);
    void DrawList(GLenum mode, glls_obj *ls);
    void DrawView(GLenum mode, glvw_obj *vw);
    void DrawMenu(GLenum mode, glmu_obj *mu);
    void TabInput(bool nx);
    void GLWinDestroy(glwn_obj *pw);
  public:
    MyGLWin();
    ~MyGLWin();
    bool On();
    void InitFont();
    void DrawChar(const unsigned char dchr);
    void DrawString(const unsigned char *dstr);
    void CreateWin(int left, int top, int width, int height, const char *title, bool close = true, bool resize = false);
    void AddBitmap(int left, int top, int red, int green, int blue, const GLubyte *bitmap);
    void AddButton(int left, int top, int value, const char *text, bool align = true, bool defalt = false);
    int AddCheck(int left, int top, bool state);
    int AddInput(int left, int top, unsigned int cwidth, int max, const char *text, bool lower = false);
    int AddLabel(int left, int top, const char *text);
    void AddList(int left, int top, int right, int bottom, const char *fl);
    void AddView(int left, int top, int right, int bottom, int tab, const char *fl);
    void AddMenu(int width, const char *text, int itms, int sub, int value);
    void Draw(GLenum mode);
    int DefaultButton();
    bool GetCheck(int name);
    char *GetInput(int name = 0);
    void PutInput(const char *text, int name = 0);
    void PutLabel(const char *text, int name);
    int GetSelected();
    void ScreenSize(int w, int h);
    void MousePos(int x, int y);
    void Motion(int x, int y);
    void Scroll(int gs = GLWIN_START, bool one = false);
    void Close(bool all = true);
    int Select(bool btnup);
    void Key(int key, bool shift);
};
