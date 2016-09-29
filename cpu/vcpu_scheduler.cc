#include <stdio.h>
#include <stdlib.h>
#include <libvirt/libvirt.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <unistd.h>

using namespace std;

class cpu_time
	{
	public:
		cpu_time();
		unsigned long long int old_cpu_time;
		unsigned long long int duration;
		int cpu_assigned;
	};

cpu_time::cpu_time()
	{
	old_cpu_time = 0;
	duration = -1;
	}

class cpu_time_and_domain
	{
	public:
		cpu_time_and_domain();
		unsigned long long int duration;
		virDomainPtr DomainPtr;
		int domainID;
		cpu_time_and_domain* prev;
		cpu_time_and_domain* next;
	};

cpu_time_and_domain::cpu_time_and_domain()
	{
	domainID = -1;
	duration = -1;
	DomainPtr = NULL;
	prev = NULL;
	next = NULL;
	}

class double_linked_list
	{
	public:
		double_linked_list();
		~double_linked_list();
		void insert(cpu_time_and_domain* new_node);
		cpu_time_and_domain* head;
		cpu_time_and_domain* tail;
	};

double_linked_list::double_linked_list()
	{
	head = new cpu_time_and_domain;
	tail = NULL;
	}

double_linked_list::~double_linked_list()
	{
	delete head;
	}

void double_linked_list::insert(cpu_time_and_domain* new_node)
	{
	cpu_time_and_domain* temp_node;
	temp_node = head->next;

	if(temp_node == NULL)
		{
		// First node added~~~
		head->next = new_node;
		new_node->prev = head;
		tail = new_node;
		return;
		}
	else
		{
		while( temp_node->next != NULL )
			{
			if( new_node->duration >= temp_node->duration)
				{
				// insert before temp_node
				new_node->next = temp_node;
				temp_node->prev->next = new_node;
				new_node->prev = temp_node->prev;
				temp_node->prev = new_node;
				return;
				}
			// Nothing happen, move on to the next node
			temp_node = temp_node->next;
			}
		temp_node->next = new_node;
		new_node->prev = temp_node;
		tail = new_node;
		return;
		}
	}

int main(int argc, char *argv[])
	{
	int sleep_time = atoi( argv[1] );
	map<int, virDomainPtr> ID_to_DomainPtr; 
	map<int, cpu_time*> ID_to_CPUtime;
	int i, j;

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

    vector<unsigned long long int> CPU_loading (nodeinfo.cores,0);

    cout << endl;
    
    cout << "======== List of active domains' ID ========" << endl << endl;

    
	int numDomains, ncpumaps, maplen;
	unsigned char *cpumaps;
	int *activeDomains;

	numDomains = virConnectNumOfDomains(conn);

	activeDomains = new int [numDomains];
	numDomains = virConnectListDomains(conn, activeDomains, numDomains);

	virDomainPtr temp_DomainPtr;
	virDomainInfo info;
	
	virTypedParameter *params = NULL; 

	cpu_time* CPUtime_obj_ptr;
	cpu_time_and_domain* obj_for_sorting;
	cpu_time_and_domain* temp_node;
	double_linked_list* double_linked_list_obj;

	int nparams; 

	maplen = 2;
	ncpumaps = 1;
	

	while(true)
		{
		double_linked_list_obj = new double_linked_list;
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
			
			/*******************/
			nparams = virDomainGetCPUStats(temp_DomainPtr, NULL, 0, -1, 1, 0); // nparams
			params = (virTypedParameter*)calloc(nparams, sizeof(virTypedParameter));
			virDomainGetCPUStats(temp_DomainPtr, params, nparams, -1, 1, 0); // total stats.
			j = 0;
			while(j < nparams)
				{
				cout << "field:" << setw(13) << params[j].field 
	        		 << ", type:" << params[j].type 
	        		 << ", value:" << params[j].value.ul << endl; 
				++j;
				}
			cout << endl;
			free( params );

	    	if( ID_to_CPUtime.count( activeDomains[i] ) > 0 )
				{
				CPUtime_obj_ptr = ID_to_CPUtime[ activeDomains[i] ];
				if(CPUtime_obj_ptr->old_cpu_time > info.cpuTime) // A new VM with old id
					{
					CPUtime_obj_ptr->duration = info.cpuTime;
					CPUtime_obj_ptr->old_cpu_time = info.cpuTime;
					}
				else // An old VM
					{
					CPUtime_obj_ptr->duration = info.cpuTime - CPUtime_obj_ptr->old_cpu_time;
					CPUtime_obj_ptr->old_cpu_time = info.cpuTime;
					}
				}
			else // A new VM appear
				{	
				CPUtime_obj_ptr = new cpu_time;
				CPUtime_obj_ptr->old_cpu_time = info.cpuTime;
				CPUtime_obj_ptr->duration = info.cpuTime;
				ID_to_CPUtime[ activeDomains[i] ] = CPUtime_obj_ptr;
				}
			// Put the obj for sorting into my linked list
			obj_for_sorting = new cpu_time_and_domain;
			obj_for_sorting->duration = CPUtime_obj_ptr->duration;
			obj_for_sorting->domainID = activeDomains[i];
			obj_for_sorting->DomainPtr = temp_DomainPtr;
			double_linked_list_obj->insert( obj_for_sorting );
	    	
	    	cout << "----------" << endl;
			}
		cpu_time_and_domain* current_node;
		int index_of_min;
		unsigned long long min_loading;

		// Initialize CPU loading vector
		j = 0;
		while(j < CPU_loading.size())
			{
			CPU_loading[j] = 0;
			++j;
			}

		current_node = double_linked_list_obj->head->next;
		while( current_node != NULL )
			{
			// Get the least busy CPU
			min_loading = CPU_loading[0];
			index_of_min = 0;
			j = 1;
			while( j < CPU_loading.size() )
				{
				if( min_loading > CPU_loading[j] )
					{
					min_loading = CPU_loading[j];
					index_of_min = j;
					}
				++j;
				}
			// Assign a CPU for current node;
			CPU_loading[index_of_min] = CPU_loading[index_of_min] + current_node->duration;
			cpumaps = (unsigned char*)calloc(ncpumaps, maplen);
			// cpumaps[0] = 15;
			cpumaps[0] = 1 << index_of_min;
			virDomainPinVcpuFlags(current_node->DomainPtr, 0, cpumaps, maplen, VIR_DOMAIN_AFFECT_CURRENT);
			virDomainGetVcpuPinInfo(current_node->DomainPtr, ncpumaps, cpumaps, maplen, 0);
			printf("Domain %d, cpumaps: %u\n", current_node->domainID, cpumaps[0]);
			virDomainFree( current_node->DomainPtr );
			current_node = current_node->next;
			free( cpumaps );
			}
		cout << "======================================" << endl;

		obj_for_sorting = double_linked_list_obj->head;
		while( obj_for_sorting->next != NULL)
			{
			temp_node = obj_for_sorting->next;
			// cout << temp_node->domainID << "Deleted" << endl;
			obj_for_sorting = obj_for_sorting->next;
			delete temp_node;
			}

		delete double_linked_list_obj;
		// break;
		sleep(sleep_time);
		}
	for (map<int,cpu_time*>::iterator it=ID_to_CPUtime.begin(); it!=ID_to_CPUtime.end(); ++it)
    	delete it->second;
    virConnectClose(conn);
    delete[] activeDomains;
    
    return 0;
	}