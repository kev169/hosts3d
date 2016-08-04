/* misc.h - 13 Apr 11
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

#include <dirent.h>  //dirent (linux), mkdir() (win)
#include <sys/time.h>  //timeval

#ifdef __MINGW32__
typedef unsigned int in_addr_t;
#endif

const double RAD = 0.0174532925;  //radians in a degree
const unsigned char white[3] =  {255, 255, 255},  //white
                    brred[3] =  {255,  50,  50},  //bright red
                    red[3] =    {200,  50,  50},  //red
                    orange[3] = {220, 170,  50},  //orange
                    yellow[3] = {200, 200,  50},  //yellow
                    fluro[3] =  {170, 220,  50},  //fluro
                    green[3] =  { 50, 200,  50},  //green
                    mint[3] =   { 50, 220, 170},  //mint
                    aqua[3] =   { 50, 200, 200},  //aqua
                    sky[3] =    { 50, 170, 220},  //sky
                    brblue[3] = {  0,   0, 255},  //bright blue
                    blue[3] =   { 50,  50, 200},  //blue
                    dlblue[3] = {  0,   0,  50},  //dull blue
                    slblue[3] = { 50,  50, 100},  //selection blue
                    hlblue[3] = {120, 120, 170},  //highlight blue
                    purple[3] = {170,  50, 220},  //purple
                    violet[3] = {200,  50, 200},  //violet
                    brgrey[3] = {200, 200, 200},  //bright grey
                    grey[3] =   {150, 150, 150},  //grey
                    dlgrey[3] = {100, 100, 100},  //dull grey
                    black[3] =  {  0,   0,   0};  //black

//add extension to file
void extensionAdd(char *fl, const char *ex);

//check if file exists
bool fileExists(const char *fl);

//create directory file list
void filelistCreate(const char* fl, const char *ex);

//format bytes into units
char *formatBytes(unsigned long long bs, char *fb);

//check if an IP address is in a CIDR net
bool inNet(char *nt, char *ip);

//convert timeval to time in microseconds
unsigned long long microTime(const timeval *tv);

//convert timeval to time in milliseconds
unsigned long long milliTime(const timeval *tv);

//protocol number to string
char *protoDecode(unsigned char pr, char *pd);

//escape quotes in CSV strings
void quotecsv(const char *cs, char *ec);

#ifndef __MINGW32__
//check for regular file
#ifdef __APPLE__
int regFile(dirent *ep);
#else
int regFile(const dirent *ep);
#endif
#endif

//square function
double sqr(double x);

//convert string to lowercase
char *strLower(const char *ms, char *ls);
