/*******************************************************
                          cache.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include "cache.h"
using namespace std;

//Constructor
Cache::Cache(int s,int a,int b, int num_processors, Cache **cachesArray1)
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;
   memTransactions = 0;
   procs = num_processors;

   //initialize coherence protocol counters
   flushes = BusRdX = cache2cache = 0;
   interventions = invalidations = 0;  
   
   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));  
   
   cachesArray = cachesArray1;  
   
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
	tagMask <<= 1;
        tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
	     cache[i][j].invalidate();   //Make all lines invalid initially.
      }
   }      
   
}






//******************* Access function for MSI protocol *****************************//

void Cache::Access_MSI(int p, ulong addr, const char* op)
{
   self = p;            //Denotes self processor number.
   currentCycle++;     /*per cache global counter to maintain LRU order 
			               among cache ways, updated on every cache access*/
        	
	if( *op == 'w') writes++;
	else if ( *op == 'r' ) reads++;

/*** Processor requests ***/
if ( (*op == 'r') || (*op == 'w') )   
{   
	cacheLine * line = findLine(addr);

	if(line == NULL)	/*miss, means it is INVALID state */
	{
		if( *op == 'w' ) writeMisses++;
		else readMisses++;

      cacheLine *newline = fillLine(addr);
      //newLine is the MRU now.  

      if ( *op == 'w')
      {
         newline->setFlags(MODIFIED);

         //Issue BusRdX
         BusRdX++;
         for(int i=0;i<procs;i++)
         {
            if ( i == self )
               continue;
            string op = "x";
            cachesArray[i]->Access_MSI(i, addr, op.c_str());
         }
      }

      if ( *op == 'r' )
      {
         newline->setFlags(SHARED);

         //Issue BusRd
         for(int i=0;i<procs;i++)
         {
            if ( i == self )
               continue;
            string op = "d";
            cachesArray[i]->Access_MSI(i, addr, op.c_str());
         }
      }  
		
	}  //MISS end

	else     /*hit*/
	{
		/**since it's a hit, update LRU**/
		updateLRU(line);

      //MODIFIED state
      if( line->getFlags() == MODIFIED )
      {
         line->setFlags(MODIFIED);   //Stay in modified.
      }  //MODIFIED end

      //SHARED state
      else if( line->getFlags() == SHARED)
      {
         if( *op == 'r')
         {
            line->setFlags(SHARED);
         }

         if( *op == 'w')
         {
            line->setFlags(MODIFIED);

            //Issue BusRdX
            BusRdX++;
            memTransactions++;
            for(int i=0;i<procs;i++)
            {
               if( i == self )     /* OTHER caches only */
               continue;
               string op = "x";
               cachesArray[i]->Access_MSI(i, addr, op.c_str());
            }
         }
      }  //SHARED end

	}  //HIT end
} //Processor request ends.

/*** Bus transactions ***/
else if ( (*op == 'd') || (*op == 'x') )
{
   cacheLine * line = findLine(addr);

   if(line == NULL)  /*miss, means it is INVALID state */
   {

      if ( *op == 'd' || *op == 'x' )
      {         
	        //do nothing;
      }

   } //Miss ends

   else    /** Hit **/
   {
      //MODIFIED state
      if ( line->getFlags() == MODIFIED )
      {
         if ( *op == 'd')
         {
            line->setFlags(SHARED);
		      interventions++;	     
	
	        //Issue Flush *********************************************
	        writeBack(addr);	
           flushes++;
         }

         if( *op == 'x' )
         {
            line->setFlags(INVALID);
		      invalidations++;

	        //Issue Flush *********************************************
	        writeBack(addr);
           flushes++;
         }
      } //MODIFIED end.

      //SHARED state
      else if ( line->getFlags() == SHARED )
      {
         if ( *op == 'd')
         {
            //do nothing.
            line->setFlags(SHARED);
         }

         if ( *op == 'x')
         {
            line->setFlags(INVALID);
		      invalidations++;
         }
      } //SHARED end.

   }  //Hit ends
      

} //Bus transaction ends

}  ///Access_MSI End.











//******************* Access function for MESI protocol *****************************//

void Cache::Access_MESI(int p, ulong addr, const char* op)
{
   self = p;            //Denotes self processor number.
   currentCycle++;     /*per cache global counter to maintain LRU order 
                        among cache ways, updated on every cache access*/
         
   if( *op == 'w') writes++;
   else if ( *op == 'r' ) reads++;
   
   //Check the 'C' line, that is whether any other caches have the block.
   int check = 0;

   for(int i=0;i<procs;i++)
   {            
      if ( i == self )
         continue;
            
      check = cachesArray[i]->searchCache(addr);
      if ( check == 1 )
             break;
   }   

/*** Processor requests ***/
if ( (*op == 'r') || (*op == 'w') )   
{   
   cacheLine * line = findLine(addr);

   if(line == NULL)  /*miss, meaning it is in INVALID state */
   {
      if( *op == 'w' ) writeMisses++;
      else readMisses++;

      cacheLine *newline = fillLine(addr); 
      //newLine is the MRU now.

      if ( *op == 'w')
      {
         newline->setFlags(MODIFIED);

         if (check == 1)
            cache2cache++;

         //Issue BusRdX
         BusRdX++;
         for(int i=0;i<(int)procs;i++)
         {
            if ( i == self )
               continue;
            string op = "x";
            cachesArray[i]->Access_MESI(i, addr, op.c_str());
         }
      }  //PrWr end.

      if ( *op == 'r' )
      {
      
         //Check = 1 inplies some other cache also has the required block.
         //If check = 1, move to SHARED state.
         //If check = 0, move to EXCLUSIVE state.

         if ( check == 1 )
         {
            newline->setFlags(SHARED);
            cache2cache++;

            //Issue BusRd.
            for(int i=0;i<(int)procs;i++)
            {
               if ( i == self )
                  continue;
               string op = "d";
               cachesArray[i]->Access_MESI(i, addr, op.c_str());
            }
         }

         else if ( check == 0 )
         {
            newline->setFlags(EXCLUSIVE);

            //Issue BusRd in both cases.
            for(int i=0;i<(int)procs;i++)
            {
               if ( i == self )
                  continue;
               string op = "d";
               cachesArray[i]->Access_MESI(i, addr, op.c_str());
            }
         }

      }  //PrRd end.
      
   }  //MISS end

   else     /*hit*/
   {
      updateLRU(line);
      
      //MODIFIED state
      if( line->getFlags() == MODIFIED )
      {
         line->setFlags(MODIFIED);   //Stay in modified.
      }  //MODIFIED end

      //SHARED state
      else if( line->getFlags() == SHARED )
      {
         if( *op == 'r')
         {
            line->setFlags(SHARED);
         }

         if( *op == 'w')
         {
            line->setFlags(MODIFIED);

            //Issue BusUpgrade
            for(int i=0;i<(int)procs;i++)
            {
               if( i == self )     /* OTHER caches only */
               continue;
               string op = "u";
               cachesArray[i]->Access_MESI(i, addr, op.c_str());
            }
         }

      }  //SHARED end

      //EXCLUSIVE state
      else if ( line->getFlags() == EXCLUSIVE )
      {
         if ( *op == 'r')
         {
            line->setFlags(EXCLUSIVE);
         }

         if ( *op == 'w')
         {
            line->setFlags(MODIFIED);
         }
      }  //EXCLUSIVE end.

   }  //HIT end
} //Processor request ends.


/*** Bus transactions ***/
//BusRd, BusRdX and BusUpgr

else if ( (*op == 'd') || (*op == 'x') || (*op == 'u'))
{
   cacheLine * line = findLine(addr);

   if(line == NULL)  /*miss, means it is INVALID state */
   {

      if ( *op == 'd' || *op == 'x' || *op == 'u' )
      {         
         //do nothing;
      }

   } //Miss ends

   else    /** Hit **/
   {
      //MODIFIED state
      if ( line->getFlags() == MODIFIED )
      {
         if ( *op == 'd')
         {
            line->setFlags(SHARED);
            interventions++;       
   
            //Issue Flush *******
            writeBack(addr);  
            flushes++;  
         }

         if( *op == 'x' )
         {
            line->setFlags(INVALID);
            invalidations++;

            //Issue Flush ******
            writeBack(addr);
            flushes++;
         }

         if ( *op == 'u')
         {}
      } //MODIFIED end.

      //SHARED state
      else if ( line->getFlags() == SHARED )
      {
         if ( *op == 'd')
         {
            line->setFlags(SHARED);

            //Issue FlushOpt
         }

         if ( *op == 'x')
         {
            line->setFlags(INVALID);
            invalidations++;

            //Issue FlushOpt
         }

         if ( *op == 'u')
         {
            line->setFlags(INVALID);
            invalidations++;
         }
      } //SHARED end.

      //EXCLUSIVE state
      else if ( line->getFlags() == EXCLUSIVE )
      {
         if ( *op == 'd')
         {
            line->setFlags(SHARED);
            interventions++;
            //Issue FlushOpt
         }

         if ( *op == 'x' )
         {
            line->setFlags(INVALID);
            invalidations++;
            //Issue FlushOpt
         }

         if ( *op == 'u')
         {}
      } //EXCLUSIVE end.

   }  //Hit ends
      
} //Bus transaction ends

}  /// Access_MESI End.











//******************* Access function for Dragon protocol *****************************//

void Cache::Access_Dragon(int p, ulong addr, const char* op)
{
   self = p;            //Denotes self processor number.
   currentCycle++;     /*per cache global counter to maintain LRU order 
                        among cache ways, updated on every cache access*/
         
   if( *op == 'w') writes++;
   else if ( *op == 'r' ) reads++;

   //Check if other caches have the block.
   int check = 0;

   for(int i=0;i<procs;i++)
   {
      if ( i == self)
         continue;
      check = cachesArray[i]->searchCache(addr);
      if ( check == 1)
               break;
   }
   
/*** Processor requests ***/
//We have to check for PrRdMiss and PrWrMiss, and also check whether other caches have the required block.

//Processor read request.
if (*op == 'r')    
{   
   //Check for miss or hit first.
    cacheLine *  line = findLine(addr);

    //PrRdMiss
    if(line == NULL)
    {
         readMisses++;

         if( check == 1 )
         {
            cacheLine *newline = fillLine(addr);
            newline->setFlags(Sc);

            //Issue BusRd
            for(int i=0;i<procs;i++)
            {
               if( i == self )
                  continue;
               string op = "d";
               cachesArray[i]->Access_Dragon(i, addr, op.c_str());
            }
         }

         else if( check == 0 )
         {
            cacheLine *newline = fillLine(addr);
            newline->setFlags(EXCLUSIVE);

            //Issue BusRd
            for(int i=0;i<procs;i++)
            {
               if( i == self )
                  continue;
               string op = "d";
               cachesArray[i]->Access_Dragon(i, addr, op.c_str());
            }
         }

    }

    else   //Read hit
    {
      
      updateLRU(line);

      //MODIFIED state
      if( line->getFlags() == MODIFIED )
      {
         line->setFlags(MODIFIED);   //Stay in modified.
      }  //MODIFIED end

      else if(line->getFlags() == EXCLUSIVE)
      {
         line->setFlags(EXCLUSIVE);
      }

      //SHARED MODIFIED state
      else if( line->getFlags() == Sm)
      {
         line->setFlags(Sm);
      }

      //SHARED CLEAN state
      else if ( line->getFlags() == Sc)
      {
         line->setFlags(Sc);
      } 

    }

}  //Processor read request end.

//Processor write request.
if ( *op == 'w')
{
   //Check for miss or hit first.
    cacheLine *  line = findLine(addr);

    //PrWrMiss
    if(line == NULL)
    {
         writeMisses++;

         if(check == 1)
         {
            cacheLine *newline = fillLine(addr);
            newline->setFlags(Sm);

            //Issue BusRd, then issue BusUpdate
            //Issue BusRd
            for(int i=0;i<procs;i++)
            {
               if( i == self )
                  continue;
               string op = "d";
               cachesArray[i]->Access_Dragon(i, addr, op.c_str());
            }
            //Issue BusUpdate
            for(int i=0;i<procs;i++)
            {
               if ( i==self )
                  continue;
               string op = "p";
               cachesArray[i]->Access_Dragon(i, addr, op.c_str());
            }
         }

         else if(check == 0)
         {
            cacheLine *newline = fillLine(addr);
            newline->setFlags(MODIFIED);

            //Issue BusRd
            for(int i=0;i<procs;i++)
            {
               if( i == self )
                  continue;
               string op = "d";
               cachesArray[i]->Access_Dragon(i, addr, op.c_str());
            }
         }

    }

    else   //Read hit
    {
      updateLRU(line);
      
      //MODIFIED state
      if( line->getFlags() == MODIFIED )
      {
         line->setFlags(MODIFIED);   //Stay in modified.
      }  //MODIFIED end

      else if(line->getFlags() == EXCLUSIVE)
      {
         line->setFlags(MODIFIED);
      }

      //SHARED MODIFIED state
      else if( line->getFlags() == Sm)
      {

         if ( check == 1)
         {
            line->setFlags(Sm);

            //Issue BusUpdate
            for(int i=0;i<procs;i++)
            {
               if( i == self)
               continue;
               string op = "p";
               cachesArray[i]->Access_Dragon(i, addr, op.c_str());
            }

         }
         else if ( check == 0)
         {
            line->setFlags(MODIFIED);
            
            //Issue BusUpdate
            for(int i=0;i<procs;i++)
            {
               if( i == self)
               continue;
               string op = "p";
               cachesArray[i]->Access_Dragon(i, addr, op.c_str());
            }     
         }
      
      } //SHARED MODIFIED state end.

      //SHARED CLEAN state
      else if ( line->getFlags() == Sc)
      {

         if ( check == 1)
         {
            line->setFlags(Sm);

            //Issue BusUpdate
            for(int i=0;i<procs;i++)
            {
               if( i == self)
               continue;
               string op = "p";
               cachesArray[i]->Access_Dragon(i, addr, op.c_str());
            }   
         }
         
         else if ( check == 0)
         {
            line->setFlags(MODIFIED);

            //Issue BusUpdate
            for(int i=0;i<procs;i++)
            {
               if( i == self )
                  continue;
               string op = "p";
               cachesArray[i]->Access_Dragon(i, addr, op.c_str());
            } 
         }  

      } //SHARED CLEAN state end. 

    } //Hit end.
} //Processor write request end.


/*** Bus transactions ***/

if ( (*op == 'd') || (*op == 'p'))
{
   cacheLine *  line = findLine(addr);

   if (line == NULL)
   {
      //do nothing.
   }

   else
   {
      //MODIFIED State
      if ( line->getFlags() == MODIFIED)
      {
         if ( *op == 'd')
         {
            line->setFlags(Sm);
            interventions++;

            //Issue Flush
            flushes++;
         }

         if ( *op == 'p')
         {}
      }  //MODIFIED end.

      //SHARED MODIFIED state
      else if ( line->getFlags() == Sm)
      {
         if ( *op == 'd')
         {
            line->setFlags(Sm);

            //Issue Flush
            flushes++;
         }

         if ( *op == 'p')
         {
            line->setFlags(Sc);

            ulong tag = calcTag(addr);
            //Update the value in cache.
            line->setTag(tag);
            
         }
      }  //SHARED MODIFIED end.

      //SHARED CLEAN state.
      else if ( line->getFlags() == Sc)
      {
         if ( *op == 'd')
         {
            line->setFlags(Sc);
         }

         if ( *op == 'p' )
         {
            line->setFlags(Sc);

            ulong tag = calcTag(addr);
            //Update the value in cache.
            line->setTag(tag);
            
         }
      } //SHARED CLEAN  end.

      //EXCLUSIVE state.
      else if ( line->getFlags() == EXCLUSIVE)
      {
         if ( *op == 'd')
         {
            line->setFlags(Sc);
            interventions++;
         }

         if ( *op == 'p')
         {}
      } //EXCLUSIVE ends.

   }

} //Bus transactions ends.

}  ///Access_Dragon End.










/* Search cache to find whether it has a particular cache block.
This function is used to assert line 'C' if any cache copies of a block exist.
*/

int Cache::searchCache(ulong address)
{
   int found = 0;
   ulong tag, i;

   tag = calcTag(address);
   i   = calcIndex(address);

  for(ulong j=0; j<assoc; j++)
   if(cache[i][j].isValid())   //Either SHARED or MODIFIED or EXCLUSIVE
     if(cache[i][j].getTag() == tag)
      {
           found = 1; break; 
      }

   return found;
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())   //Either SHARED or MODIFIED
	  if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if it exists, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);     
   }   

   //Need to evict.
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) 
    { 
      victim = j; min = cache[i][j].getSeq();
    }
   } 
   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   if ( (victim->getFlags() == MODIFIED) || (victim->getFlags() == Sm) )
      writeBack(addr);
   assert(victim != 0);
    	
   tag = calcTag(addr);   
   victim->setTag(tag);   

   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::printStats(ulong m)
{ 
   float miss_rate = (float) (100 * ((float)(readMisses + writeMisses)/(float)(reads+writes)));
   
   cout 
   << "01. number of reads:      \t" << reads << '\n'
   << "02. number of read misses:\t" << readMisses << '\n'
   << "03. number of writes:     \t" << writes << '\n'   
   << "04. number of write misses:\t" << writeMisses << endl;
   if (miss_rate >= 10)
   cout << "05. total miss rate:      \t" << setprecision(4) << miss_rate << "%" <<endl;
   else if (miss_rate < 10)
   cout << "05. total miss rate:      \t" << setprecision(3) << miss_rate << "%" << endl;   
   cout << "06. number of writebacks: \t" << writeBacks << '\n'
   << "07. number of cache-to-cache transfers: " <<  cache2cache << '\n'
   << "08. number of memory transactions: " << m << '\n'
   << "09. number of interventions:\t" << interventions << '\n' 
   << "10. number of invalidations:\t" << invalidations << '\n'
   << "11. number of flushes:    \t" << flushes << '\n'
   << "12. number of BusRdX:     \t" <<  BusRdX << endl;
}


