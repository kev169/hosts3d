/* llist.h - 25 Jun 09
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

//doubly-linked list
class MyLL
{
  private:
    struct Item
    {
      void *Data;
      Item *Next, *Prev;
      Item(void *dat, Item *nxt, Item *prv);
      ~Item();
    } *pFront, *pBack, *pCurr[3];
    unsigned int wCnt, dCnt;
  public:
    MyLL();
    ~MyLL();
    unsigned int Num();
    void Start(unsigned char pc);
    void End(unsigned char pc);
    void *Write(void *dat);
    void *Read(unsigned char pc);
    void Next(unsigned char pc);
    void Prev(unsigned char pc);
    void *Find(unsigned short itm);
    void Delete(unsigned char pc);
    void Destroy();
};
