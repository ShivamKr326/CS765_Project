#include "descriptions.h"

using namespace std;

int Peer::counter;
int Peer::total_peers;
ld Peer::Ttx;
vector<Peer*> Simulator::peers;
// frac_hashingPow is also there along with slow and low boolean and adjList

/* Constructor */
Peer::Peer(){

}

Peer::~Peer(){

}

static void Peer::add_edge(Peer* a, Peer* b){

}

void Peer::add_block(Block* blk, bool upd_balances){

}

void Peer::schedule_next_txn(Simulator* simulator){

}

void Peer::schedule_next_blk(Simulator* simulator){

}

Transaction* Peer::generate_txn(Simulator* simulator){

}

Block* Peer::generate_blk(Simulator* simulator){

}

bool Peer::validate_txn(Transaction* txn, vector<int>& bal){

}

bool Peer::validate_block(Block* blk, vector<int>& bal){

}

void Peer::forward_txn(Simulator* simulator, Peer* dest, Transaction* txn){

}
void Peer::receive_txn(Simulator* simulator, Peer* src, Transaction* txn){

}

void Peer::forward_block(Simulator* simulator, Peer* dest, Block* blk){

}

void Peer::receive_block(Simulator* simulator, Peer* src, Block* blk){

}

void Peer::broadcast_mined_block(Simulator* simulator){

}

void Peer::export_blockchain(ostream& os){

}

void Peer::export_arrival_times(ostream& os){

}

void Peer::export_stats(Simulator* simulator, ostream& os){

}


