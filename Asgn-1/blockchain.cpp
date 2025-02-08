#include "descriptions.h"

using namespace std;

Block* Blockchain::global_genesis;

Blockchain::Blockchain(){
    genesis = global_genesis->clone();
    current_block = genesis;
}

/* add block to the children of its parent in the blockchain */
void Blockchain::add_block(Block* blk){
    assert(blk->parent!=NULL);
    block->parent->next.push_back(blk);
    return;
}


/* returns the parent while updating the balances array and storing the transactions */
static Block* Blockchain::backward(Block* blk,vector<int>& balances, vector<Transaction*> txns){
    for(Transaction* txn:blk->txns){
        balances[txn->sender->id]+=txn->amount;
        balances[txn->receiver->id]-=txn->amount;
        txns.push_back(txn);
    }
    balances[blk->miner->id]-=MINING_FEE;
    return blk->parent;
}

/// please work
// do please