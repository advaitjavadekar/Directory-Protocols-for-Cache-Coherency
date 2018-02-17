/************************************************************
 Course		: 	CSC/ECE506
 Source 		:	fbv.cc
 Owner		:	Ed Gehringer
 Email Id	:	efg@ncsu.edu
 ------------------------------------------------------------*
 © Please do not replicate this code without consulting
 the owner & instructor! Thanks!
 *************************************************************/
#include "fbv.h"

void FBV::update_sharer_entry(int proc_num) {
	bit[proc_num] = true;
}

void FBV::remove_sharer_entry(int proc_num) {
	// FIXME: YOUR CODE HERE
	// Reset the bit vector entry
	bit[proc_num] = false;
	
	if(!check_sharer_entry(num_processors)){
		// if all bits are 0 then set dir state -> U
		set_dir_state(U);		
	}
}

int FBV::check_sharer_entry(int num_proc) {
	// FIXME: YOUR CODE HERE
	// Check bit vector for any set bit. 
	// If set, return 1, else send 0
	
	for(int i = 0; i < num_proc; i++){
		if(bit[i]){
			return 1;
		}
	}
	
	return 0; // Returning 0 to avoid compilation errors.
}

void FBV::sendInt_sharer(ulong addr, int num_proc, int proc_num) {
	// FIXME: YOUR CODE HERE
	//
	// Invoke the sendInt function defined in main 
	// for all the processors except for proc_num
	// Make sure that you check the FBV to see if the 
	// bit is set
	
	for(int i = 0; i < num_proc; i++){
		if(i != proc_num && bit[i]){
			sendInt(addr, i);
		}
	}
	
}

void FBV::sendInv_sharer(ulong addr, int num_proc, int proc_num) {
	// FIXME: YOUR CODE HERE
	//
	// Invoke the sendInv function defined in main 
	// for all the processors except for proc_num
	// Make sure that you check the FBV to see if the 
	// bit is set

	for(int i = 0; i < num_proc; i++){
		if(i != proc_num && bit[i]){
			sendInv(addr, i);
			remove_sharer_entry(i);	
		}
	}

}

