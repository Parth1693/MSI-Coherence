/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include "cache.h"
#include <string>
using namespace std;

int main(int argc, char *argv[])
{	
	ifstream traceFile;
	string fileName;
	char mystr[40];	
	char *p;
	string addressFull;
	ulong address;
	int i;

	p = &mystr[0];

	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 return 0;
        }

	/*****uncomment the next five lines*****/
	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/
 	fileName = argv[6];
	
	//Print personal information at start of each run.
	cout <<"===== 506 Personal information =====" << '\n'
	<< "Parth Prashant Bhogate" << '\n'
	<< "UnityID: 200108628" << '\n'
	<< "ECE492 Students? NO" << endl;

	//****************************************************//
	//*******print out simulator configuration here*******//
	//****************************************************//
	cout << "===== 506 SMP Simulator configuration =====" << '\n'
	<< "L1_SIZE: " <<  cache_size << '\n'
	<< "L1_ASSOC: "<< cache_assoc << '\n'
	<< "L1_BLOCKSIZE: " << blk_size << '\n'
	<< "NUMBER OF PROCESSORS: " << num_processors << endl;
	if (protocol == 0 )
	cout << "COHERENCE PROTOCOL: MSI" << endl;
	else if (protocol == 1)
	cout << "COHERENCE PROTOCOL: MESI" << endl;
	else if (protocol == 2)
	cout << "COHERENCE PROTOCOL: Dragon" << endl; 
	cout << "TRACE FILE: " << fileName << endl;
 
	//*********************************************//
        //*****create an array of caches here**********//
	//*********************************************//	

	Cache **cachesArray = new Cache *[num_processors];

	for(i=0;i<num_processors;i++)
	{
		cachesArray[i] = new Cache(cache_size, cache_assoc, blk_size, num_processors, cachesArray);
	}

	/*if( cachesArray == NULL)
	{	assert(0);  }*/

	//Open traceFile for reading
	strncpy(mystr, fileName.c_str(), fileName.length());
	mystr[fileName.length()]=0;

	traceFile.open(p);
	
	int proc = 0;
	int cnt = 0;
	string sub2;
	
	
	//Read processor number, r/w address from trace file
	while(traceFile>>proc>>sub2>>hex>>address)
	{
	
			//**sub1:proc. number, sub2:r/w, sub3:address**//
			
			cnt++;
			//proc = atoi(sub1.c_str());
			//cout << "Access no " << cnt << endl;
			//cout << "Proc no. is " << proc << " Op is " << sub2 << " Address is " << address << endl;

			//Send r/w address to a particular processor.
			//Call different Access_xx functions according to the protocol to be implemented.
			if( protocol == 0 )
			cachesArray[proc]->Access_MSI(proc, (ulong) address, sub2.c_str());

			else if( protocol == 1 )
			cachesArray[proc]->Access_MESI(proc, (ulong) address, sub2.c_str());

			else if( protocol == 2 )
			cachesArray[proc]->Access_Dragon(proc, (ulong) address, sub2.c_str()); 		
	} 
	
	traceFile.close();
		
	//********************************//
	//print out all caches' statistics //
	//********************************//
	
	for(i=0;i<num_processors;i++)
	{	
		cout << "============ Simulation results (Cache " << i << ") ============" <<  endl;
		
		//Calculate memTransactions
		ulong m;

		if ( protocol == 0 )
		m = cachesArray[i]->memTransactions + cachesArray[i]->readMisses + cachesArray[i]->writeMisses + cachesArray[i]->writeBacks;

		if ( protocol == 1 )
		m = cachesArray[i]->readMisses + cachesArray[i]->writeMisses + cachesArray[i]->writeBacks - cachesArray[i]->cache2cache;

		if ( protocol == 2 )
		m = cachesArray[i]->readMisses + cachesArray[i]->writeMisses + cachesArray[i]->writeBacks;	

		cachesArray[i]->printStats(m);
	}

}
