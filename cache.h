/*******************************************************
                          cache.h
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <assert.h>
#include <iomanip>

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

/****add new states, based on the protocol****/
enum{
	INVALID = 0,
	SHARED,
	MODIFIED,
  EXCLUSIVE,
  Sm,
  Sc
};

class cacheLine 
{
public:
   ulong tag;
   ulong Flags;   // 0:INVALID, 1:SHARED, 2:MODIFIED, 3:EXCLUSIVE 
   ulong seq; 
 
//public:
   cacheLine()            { tag = 0; Flags = 0; }
   ulong getTag()         { return tag; }
   ulong getFlags()			{ return Flags;}
   ulong getSeq()         { return seq; }
   void setSeq(ulong Seq)			{ seq = Seq;}
   void setFlags(ulong flags)			{  Flags = flags;}
   void setTag(ulong a)   { tag = a; }
   void invalidate()      { tag = 0; Flags = INVALID; }         //useful function
   bool isValid()         { return ((Flags) != INVALID); }
};

class Cache
{
public:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks, flushes, BusRdX, interventions, invalidations, cache2cache;	
   ulong memTransactions;
   int procs;	

   cacheLine **cache;           //For cache blocks
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)  { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag)   { return (tag << (log2Blk));}
   
//public:
    ulong currentCycle;  
    int self;
    Cache **cachesArray;   
     
    Cache(int,int,int,int, Cache *a[]);
   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM(){return readMisses;} ulong getWM(){return writeMisses;} 
   ulong getReads(){return reads;}ulong getWrites(){return writes;}
   ulong getWB(){return writeBacks;}

   ulong getInvalidations() { return invalidations; }
   ulong getInterventions() { return interventions; } 
   ulong getFlushes() { return flushes;	}
   ulong getBusRdX()   { return BusRdX;  }
   ulong getCache2Cache()  { return cache2cache; }	
   
   //Different Access functions for each protocol.
   void Access_MSI(int,ulong, const char*);
   void Access_MESI(int,ulong, const char*);
   void Access_Dragon(int, ulong, const char*);

   void writeBack(ulong)   {writeBacks++;}
   void printStats(ulong);
   void updateLRU(cacheLine *);

   /*Cache search function to assert 'C' line. 
    Function returns 1 if cache block exists in the cache.
    Used by MESI and Dragon protocols.
    */

    int searchCache(ulong);
};

#endif
