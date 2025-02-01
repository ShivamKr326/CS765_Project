#include "descriptions.h"

using namespace std;

int Block::counter;

/* Constructor */
Block::Block(Peer* miner_){
    miner = miner_;
    parent = NULL;
    parent_id = -1;
    size = 1;
    depth = -1;
}

void Block::set_id(){
    id = counter++;
    return;
}

void Block::add_txn(Transaction* txn){
    txns.push_back(txn);
    size += 1;
    return;
}

void Block::set_parent(Block* blk){
    parent = blk;
    parent_id = (blk==NULL)?-2:blk->id;
    depth = (blk == NULL)?0:blk->depth+1;
    return;
}

void Block::reset_parent(){
    parent = NULL;
    parent_id = -1;
    return;
}

string Block::get_name(){
    return "BlkID:"+to_string(id+1);
}

/* could use a clone function if req */