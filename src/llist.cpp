/* llist.cpp - 03 Apr 09
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "llist.h"

#define UINT32_MAX  (4294967295U)  //max unsigned int

MyLL::Item::Item(void *dat, Item *nxt, Item *prv)
{
  Data = dat;
  Next = nxt;
  Prev = prv;
}

MyLL::Item::~Item() { }

MyLL::MyLL()
{
  pFront = 0;
  pBack = 0;
  wCnt = 0;
  dCnt = 0;
}

unsigned int MyLL::Num()
{
  if (wCnt < dCnt) return ((UINT32_MAX - dCnt) + wCnt + 1);
  return (wCnt - dCnt);
}

void MyLL::Start(unsigned char pc)
{
  pCurr[pc] = pFront;
}

void MyLL::End(unsigned char pc)
{
  pCurr[pc] = pBack;
}

void *MyLL::Write(void *dat)
{
  if (pFront)
  {
    pBack->Next = new Item(dat, pBack->Next, pBack);
    pBack = pBack->Next;
  }
  else
  {
    pFront = new Item(dat, pBack, pFront);
    pBack = pFront;
  }
  wCnt++;
  return pBack->Data;
}

void *MyLL::Read(unsigned char pc)
{
  if (pCurr[pc]) return pCurr[pc]->Data;
  return 0;
}

void MyLL::Next(unsigned char pc)
{
  if (pCurr[pc]) pCurr[pc] = pCurr[pc]->Next;
}

void MyLL::Prev(unsigned char pc)
{
  if (pCurr[pc]) pCurr[pc] = pCurr[pc]->Prev;
}

void *MyLL::Find(unsigned short itm)
{
  Item *tp = pFront;
  for (unsigned short cnt = 1; cnt < itm; cnt++)
  {
    if (tp) tp = tp->Next;
  }
  if (tp) return tp->Data;
  return 0;
}

void MyLL::Delete(unsigned char pc)
{
  dCnt++;
  Item *tp = pCurr[pc];
  Next(pc);
  (tp->Prev ? tp->Prev->Next : pFront) = tp->Next;
  (tp->Next ? tp->Next->Prev : pBack) = tp->Prev;
  delete tp;
}

void MyLL::Destroy()
{
  Start(0);
  while (pCurr[0]) Delete(0);
}

MyLL::~MyLL()
{
  Destroy();
}
