#include <stdio.h>
#include <stdlib.h>
#include <libvirt/libvirt.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <unistd.h>
#include <vector>

using namespace std;

class memory_info
	{
	public:
		memory_info(virDomainPtr new_DomainPtr, unsigned long long new_maxMemoryAssigned, unsigned long long new_currentMemoryAssigned, unsigned long long new_currentFreeMemory);
		virDomainPtr DomainPtr;
		unsigned long long maxMemoryAssigned;
		unsigned long long currentMemoryAssigned;
		unsigned long long currentFreeMemory;
	};

memory_info::memory_info(virDomainPtr new_DomainPtr, unsigned long long new_maxMemoryAssigned, unsigned long long new_currentMemoryAssigned, unsigned long long new_currentFreeMemory)
	{
	DomainPtr = new_DomainPtr;
	maxMemoryAssigned = new_maxMemoryAssigned;
	currentMemoryAssigned = new_currentMemoryAssigned;
	currentFreeMemory = new_currentFreeMemory;
	}

int main(int argc, char *argv[])
	{
	map<int, virDomainPtr> ID_to_DomainPtr; 
	int i, j;
	int sleep_time = atoi( argv[1] );

    virConnectPtr conn;

    conn = virConnectOpen("qemu:///system");
    if (conn == NULL) 
    	{
        fprintf(stderr, "Failed to open connection to qemu:///system\n");
        return 1;
    	}

    cout << "======== Info of host ========" << endl << endl;

    virNodeInfo nodeinfo;
    virNodeGetInfo(conn, &nodeinfo);

    printf("Model: %s\n", nodeinfo.model);
    printf("Memory size: %lukb\n", nodeinfo.memory);
    printf("Number of CPUs: %u\n", nodeinfo.cpus);
    printf("MHz of CPUs: %u\n", nodeinfo.mhz);
    printf("Number of NUMA nodes: %u\n", nodeinfo.nodes);
    printf("Number of CPU sockets: %u\n", nodeinfo.sockets);
    printf("Number of CPU cores per socket: %u\n", nodeinfo.cores);
    printf("Number of CPU threads per core: %u\n", nodeinfo.threads);

    cout << endl;
    
    cout << "======== List of active domains' ID ========" << endl << endl;

    
	int numDomains;
	int *activeDomains;

	numDomains = virConnectNumOfDomains(conn);

	activeDomains = new int [numDomains];
	numDomains = virConnectListDomains(conn, activeDomains, numDomains);

	virDomainPtr temp_DomainPtr;
	virDomainInfo info;

	vector<memory_info*> vec_memory_info;
	memory_info* temp_memory_info;
	unsigned long long temp_currentFreeMemory;
	unsigned long long avg_FreeMemory;
	unsigned long long new_MemoryAssigned;
	unsigned memory_assignment_buffer = 0;

	while(true)
		{
		for (i = 0 ; i < numDomains ; i++) 
			{
			temp_DomainPtr = virDomainLookupByID(conn, activeDomains[i]);
			ID_to_DomainPtr[ activeDomains[i] ] = temp_DomainPtr;
			cout << "Active domain IDs: " << activeDomains[i] << endl;
			virDomainGetInfo(temp_DomainPtr, &info); 
			cout << "state: " << (int)info.state
				 << ", maxmem: " << info.maxMem
				 << ", memused: " << info.memory
				 << ", cpunum: " << info.nrVirtCpu
				 << ", cputime: " << info.cpuTime << endl;
			
			virDomainSetMemoryStatsPeriod(temp_DomainPtr, 5, VIR_DOMAIN_AFFECT_CURRENT);
			virDomainMemoryStatStruct stats[9];
			cout << "Memory Info: " << endl;
    		int res = virDomainMemoryStats(temp_DomainPtr, stats, 9, 0);
    		if(res >= 0) 
    			{
        		for(j = 0 ; j < res; j++) 
        			{
            		printf("Tag: %d, Value: %llu\n", stats[j].tag, stats[j].val);
            		if( stats[j].tag == 4)
            			{
            			temp_currentFreeMemory = stats[j].val;
            			}
        			}
    			}

    		// Initialize object for storing memory information
			temp_memory_info = new memory_info( temp_DomainPtr, info.maxMem, info.memory, temp_currentFreeMemory);
			vec_memory_info.push_back( temp_memory_info );

	    	cout << "----------" << endl;
			}
		// Calculate the avg free memory
		i = 0;
		avg_FreeMemory = memory_assignment_buffer;
		memory_assignment_buffer = 0;
		while( i < vec_memory_info.size() )
			{
			avg_FreeMemory = avg_FreeMemory + vec_memory_info[i]->currentFreeMemory;
			++i;
			}
		avg_FreeMemory = avg_FreeMemory / vec_memory_info.size();
		// Assigned new memory for each VMs domain
		i = 0;
		while( i < vec_memory_info.size() )
			{
			new_MemoryAssigned = vec_memory_info[i]->currentMemoryAssigned - vec_memory_info[i]->currentFreeMemory + avg_FreeMemory;
			if( new_MemoryAssigned > vec_memory_info[i]->maxMemoryAssigned )
				{
				memory_assignment_buffer = memory_assignment_buffer + new_MemoryAssigned - vec_memory_info[i]->maxMemoryAssigned;
				new_MemoryAssigned = vec_memory_info[i]->maxMemoryAssigned;
				}
			// virDomainSetMemory(vec_memory_info[i]->DomainPtr, 1000000);
			virDomainSetMemory(vec_memory_info[i]->DomainPtr, new_MemoryAssigned);
			++i;
			}
		// Delete the obj and ptr
		i = 0;
		while( i < vec_memory_info.size() )
			{
			virDomainFree( vec_memory_info[i]->DomainPtr );
			delete vec_memory_info[i];
			++i;
			}
		vec_memory_info.clear();
	
		//break;
		sleep(sleep_time);
		}
	cout << endl;

	///////////////////////////////////////////////////////
	//virDomainGetVcpuPinInfo	();
    //cout<<"cpuinfo:"<<endl; 


    virConnectClose(conn);
    delete[] activeDomains;
    
    return 0;
	}