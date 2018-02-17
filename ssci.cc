/************************************************************
 Course		: 	CSC/ECE506
 Source 		:	ssci.cc
 Owner		:	Ed Gehringer
 Email Id	:	efg@ncsu.edu
 ------------------------------------------------------------*
 � Please do not replicate this code without consulting
 the owner & instructor! Thanks!
 *************************************************************/
#include "ssci.h"

void SSCI::update_sharer_entry(int proc_no) {
	cache_list.push_back(proc_no);
}

void SSCI::remove_sharer_entry(int proc_num) {
	// FIXME: YOUR CODE HERE
	//
	// Remove the entry from the linked list
	cache_list.remove(proc_num);
	if(!check_sharer_entry(num_processors)){
		// if all bits are 0 then set dir state -> U
		set_dir_state(U);		
	}

}

int SSCI::check_sharer_entry(int proc_num) {
	// Return 1 if the linked list is not empty
	// else return 0
	
	if(!cache_list.empty()){
		return 1;
	}
	
	return 0; // Returning 0 to avoid compilation error
}

void SSCI::sendInv_sharer(ulong addr, int num_proc, int proc_num) {
	// FIXME: YOUR CODE HERE
	//
	// Erase the entry from the list except for the latest entry
	// The latest entry will be for the processor which is invoking
	// this function
	// Invoke the sendInv function defined in the main function
	// for all the entries in the list except for proc_num.
	
	
	list<int>::iterator iter;
	for(int i=0;i<num_proc;i++){
		if(i!=proc_num){
			iter=std::find(cache_list.begin(),cache_list.end(),i);
			if(iter!=cache_list.end()){
				sendInv(addr,i);
				remove_sharer_entry(i);
			}
			
		}
	}
	
}

void SSCI::sendInt_sharer(ulong addr, int num_proc, int proc_num) {
	// FIXME: YOUR CODE HERE
	//
	// Invoke the sendInv function defined in the main function
	// for all the entries in the list except for proc_num.
	
	
	list<int>::iterator iter;
	for(int i=0;i<num_proc;i++){
		if(i!=proc_num){
			iter=std::find(cache_list.begin(),cache_list.end(),i);
			if(iter!=cache_list.end()){
				sendInt(addr,i);
			
			}
			
		}
	}

}
