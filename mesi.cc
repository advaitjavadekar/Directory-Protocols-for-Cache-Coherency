/************************************************************
 Course		: 	CSC/ECE506
 Source 		:	mesi.cc
 Owner		:	Ed Gehringer
 Email Id	:	efg@ncsu.edu
 ------------------------------------------------------------*
 © Please do not replicate this code without consulting
 the owner & instructor! Thanks!
 *************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "cache.h"

#include "main.h"
#include "mesi.h"
//


void MESI::PrRd(ulong addr, int processor_number) {
	// FIXME: YOUR CODE HERE
	// The below comments are for reference and might not be
	// sufficient to match the debug runs. 
	// 
	// Update the Per-cache global counter to maintain LRU 
	// order among cache ways, updated on every cache access
	// Increment the global read counter.
	// Check whether the line is cached in the processor cache. 
	// If not cached, allocate a fresh line and update the state. (M,E,S,I) 
	// Check whether the directory entry is updated. If not updated,
	// create a fresh entry in the directory, update the sharer vector or list.
	// Update the directory state (U, EM, S_). 
	// Increment the directory operation counter like signalrds, 
	// response_replies etc... Invoke the relevant directory
	// signal functions like signalRd or signalRdX etc... 
	// If the line is cached in the processor cache, do not forget
	// to update the LRU
	// Do not forget to update miss/hit counter

	dir_state dirState;
	cache_state state;	
	current_cycle++; // per cache global counter
	reads++;	// increment global read counter
	
	cache_line * line = find_line(addr);
	
	if(line == NULL || line->get_state() == I){
		// this is a miss, allocate line
		read_misses++;
		cache_line *newline = allocate_line(addr);
		memory_transactions++;
		signalRd(addr, processor_number);
	
		ulong tag = tag_field(addr);
		dir_entry* dirEntry = directory->find_dir_line(tag);
		dirState = dirEntry -> get_state();		
		
		if(dirState==EM){
			// state I --> E, since no sharers (also reset Dir -> U)
			I2E++;
			newline->set_state(E);
		}
		else if(dirState==S_){
			// state I --> S
			I2S++;
			newline->set_state(S);
		}	
	
	}
	else{
		// Block is cached, get current state
		state = line->get_state();
		if(state == M || state == E || state == S){
			// remains in current state & no bus transactions generated
			update_LRU(line);
		}
	}	
}

void MESI::PrWr(ulong addr, int processor_number) {
	// FIXME: YOUR CODE HERE
	// Refer comments for PrRd
	
	//dir_state dirState;
	cache_state state;
	current_cycle++;
	writes++;

	cache_line * line = find_line(addr);
	if(line == NULL || line->get_state() == I){
		// this is a miss, allocate line
		write_misses++;
		cache_line *newline = allocate_line(addr);
		memory_transactions++;
	
		// state I --> M
		I2M++;
		newline->set_state(M);
		signalRdX(addr, processor_number);
		
	}
	else{
		state = line->get_state();
		if(state == E){
			// state E --> M
			E2M++;
			line->set_state(M);			

		}
		else if(state == S){
			// state S --> M
			S2M++;
			line->set_state(M);
			signalUpgr(addr, processor_number);
		}
	
		update_LRU(line);
	}
	
}

cache_line * MESI::allocate_line(ulong addr) {
	ulong tag;
	cache_state state;

	cache_line *victim = find_line_to_replace(addr);
	assert(victim != 0);
	state = victim->get_state();
	if (state == M)
		write_back(addr);

	ulong victim_tag = victim->get_tag();
	dir_entry* dir_line = directory->find_dir_line(victim_tag);
	if (dir_line != NULL) {
		dir_line->remove_sharer_entry(cache_num);
		int present = 0;
		present = dir_line->check_sharer_entry(num_processors);
		if (!present)
			dir_line->state = U;
	}

	tag = tag_field(addr);
	victim->set_tag(tag);
	victim->set_state(I);
	return victim;
}
//
void MESI::signalRd(ulong addr, int processor_number) {
	// FIXME: YOUR CODE HERE
	// The below comments are for reference and might not be
	// sufficient to match the debug runs.
	//
	// Check the directory state and update the cache2cache counter
	// Update the directory state
	// Update the vector/list
	// Send Intervention or Invalidation

	
	signal_rds++;

	dir_state dirState;
	ulong tag = tag_field(addr);
	dir_entry* dirEntry = directory->find_dir_line(tag);
	
	if(dirEntry == NULL){
		dirEntry = directory->find_empty_line(tag);
		dirEntry->tag = tag;
		dirEntry->set_dir_state(U);
	}

	//if(!dirEntry->check_sharer_entry(num_processors)){
		// if all bits are 0 then set dir state -> U
	//	dirEntry->set_dir_state(U);		
	//}

	dirState = dirEntry->get_state();
	
	if(dirState == U){
	 	// dir state U --> EM for Rd
		dirEntry -> set_dir_state(EM);
		dirEntry->update_sharer_entry(processor_number);
	}

	if(dirState == EM){
		// dir state EM --> S for Rd
	        cache2cache++; 
		dir_EM2S++;
		dirEntry -> set_dir_state(S_);
		dirEntry->update_sharer_entry(processor_number);
		dirEntry->sendInt_sharer(addr, num_processors, processor_number);
		
	} 
	else if(dirState == S_){
		//dirEntry->set_dir_state(S_);
		dirEntry->update_sharer_entry(processor_number);
	}


}

void MESI::signalRdX(ulong addr, int processor_number) {
	// FIXME: YOUR CODE HERE
	// Refer signalRd

	signal_rdxs++;
	
	dir_state dirState;
	ulong tag = tag_field(addr);
	dir_entry* dirEntry = directory->find_dir_line(tag);
	
	if(dirEntry == NULL){
		dirEntry = directory->find_empty_line(tag);
	}

	//if(!dirEntry->check_sharer_entry(num_processors)){
		// if all bits are 0 then set dir state -> U
	//	dirEntry->set_dir_state(U);		
	//}
	
	dirState = dirEntry -> get_state();
	if(dirState == U){
		// dir state U --> EM for Rdx
		dirEntry -> set_dir_state(EM);
		dirEntry->update_sharer_entry(processor_number);
	}
	else if(dirState == EM){
		// dir state EM --> EM for Rdx
		// no need to update the dir shared BV/list but need to send Inv
		dirEntry->update_sharer_entry(processor_number);
		dirEntry->sendInv_sharer(addr, num_processors, processor_number);
		cache2cache++;
	} 
	else if(dirState == S_){
	        // dir state S_ --> EM for Rdx
		dir_S2EM++;
		//cache2cache++;
		dirEntry->set_dir_state(EM);

		// remove sharer entry on PrWr when leaving the Shared Dir state
		dirEntry->update_sharer_entry(processor_number);
		dirEntry->sendInv_sharer(addr, num_processors, processor_number);
	}
	
}

void MESI::signalUpgr(ulong addr, int processor_number) {
	// FIXME: YOUR CODE HERE
	// Refer signalRd

	signal_upgrs++;	
	
	dir_state dirState;
	ulong tag = tag_field(addr);
	dir_entry* dirEntry = directory->find_dir_line(tag);

	if(dirEntry == NULL){
		dirEntry = directory->find_empty_line(tag);
	}
	
	//if(!dirEntry->check_sharer_entry(num_processors)){
		// if all bits are 0 then set dir state -> U
	//	dirEntry->set_dir_state(U);		
	//}
	
	dirState = dirEntry -> get_state();
	if(dirState == S_){
	        // dir state S_ --> EM for Upgr
		dir_S2EM++;
		dirEntry->set_dir_state(EM);
		dirEntry->update_sharer_entry(processor_number);
		dirEntry->sendInv_sharer(addr, num_processors, processor_number);
	}
	
}

void MESI::Int(ulong addr) {
	// FIXME: YOUR CODE HERE
	// The below comments are for reference and might not be
	// sufficient to match the debug runs.
	//
	// Update the relevant counter, if the cache copy is dirty,
	// same needs to be written back to main memory. This is 
	// achieved by simply updating the writeback counter
	interventions++;
	cache_line *line = find_line(addr);
	if(line != NULL){
		if(line->get_state() == M){
			line->set_state(S);
			memory_transactions++;
			write_back(addr);
			M2S++;
	}
		if(line->get_state()==E){
			line->set_state(S);
			write_back(addr);
			E2S++;
	}
	
	}
	
}

void MESI::Inv(ulong addr) {
	// FIXME: YOUR CODE HERE
	// Refer Int
	
	// invalidate cache line if not null
	
	cache_line *line = find_line(addr);
	if(line != NULL){
		line->set_state(I);
		invalidations++;
		
	}
	if(line->get_state()==M){
		line->set_state(I);
		write_back(addr);
	}
	if(line->get_state()==E){
		line->set_state(I);
		write_back(addr);
	}
	if(line->get_state()==S){
		line->set_state(I);
		write_back(addr);
	}
}
