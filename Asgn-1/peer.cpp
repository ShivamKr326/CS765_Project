#include "descriptions.h"

using namespace std;

int Peer::counter;
int Peer::totalPeers;
ld Peer::Ttx;
vector<Peer*> Simulator::peers;

/* Constructor */
Peer::Peer(){
    balances.assign(total_peers,0);
    rand_peer = uniform_int_distribution<int>(0,total_peers-1);
    rand_speed_of_light_delay = uniform_int_distribution<int>(10,500);
    txn_interarrival = exponential_distribution<ld>(1/Ttx); // lambda = 1/mean;
    blkchain = Blockchain();
    block_arrival_time.push_back({blkchain.genesis,0});
    chain_blks[blkchain.current_block->id] = blkchain.current_block;
}

static void Peer::add_edge(Peer* a, Peer* b){
    int light_delay = rand_speed_of_light_delay(gen);
    Link ab(b,a->isfast,light_delay);
    a->adjList.push_back(ab);
    Link ba(a,b->isfast,light_delay);
    b->adjList.push_back(ba);
    return;
}

void Peer::add_block(Block* blk, bool upd_balances){


}

void Peer::schedule_next_txn(Simulator* simulator){
    ld next_txn_time = txn_interarrival(gen);
    Event* ev = new GenerateTxn(next_txn_time, this);
    simulator->add_event(ev);
    return;
}

void Peer::schedule_next_blk(Simulator* simulator){
    next_mining_block = generate_blk(simulator);
    ld mining_time = ;
    Event* ev = new BroadcastBlk(mining_time, this);
    simulator->add_event(ev);
}

Transaction* Peer::generate_txn(Simulator* simulator){

}

Block* Peer::generate_blk(Simulator* simulator){

}

bool Peer::validate_txn(Transaction* txn, vector<int>& bal){

}

bool Peer::validate_block(Block* blk, vector<int>& bal){

}

void Peer::forward_txn(Simulator* simulator, Peer* src, Transaction* txn){

}
void Peer::receive_txn(Simulator* simulator, Peer* sender, Transaction* txn){

}

void Peer::forward_block(Simulator* simulator, Peer* src, Block* blk){

}

void Peer::receive_block(Simulator* simulator, Peer* sender, Block* blk){

}

void Peer::broadcast_mined_block(Simulator* simulator){

}

void Peer::export_blockchain(ostream& os){

}

void Peer::export_arrival_times(ostream& os){

}

void Peer::export_stats(Simulator* simulator, ostream& os){

}


