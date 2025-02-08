#include "descriptions.h"

using namespace std;

int Peer::counter;
int Peer::totalPeers;
ld Peer::Ttx;
vector<Peer*> Simulator::peers;

random_device rd;
mt19937 gen(rd());

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

void Peer::add_blk(Block* blk, bool upd_balances){
    blockchain.add(blk);
    chain_blocks[block->id] = blk;
    if (upd_balances) {
        for (Transaction* txn : blk->txns) {
            balances[txn->sender->id] -= txn->amount;
            balances[txn->receiver->id] += txn->amount;
            auto it = txn_pool.find(txn);
            if (it != txn_pool.end())
                txn_pool.erase(it);
        }
        balances[block->owner->id] += MINING_FEE;
        blockchain.current_block = blk;
    }
}

void Peer::schedule_next_txn(Simulator* simulator){
    ld next_txn_time = txn_interarrival(gen);
    Event* ev = new Generate_txn(next_txn_time, this);
    simulator->add_event(ev);
    return;
}

void Peer::schedule_next_blk(Simulator* simulator){
    next_mining_block = generate_blk(simulator);
    ld mining_time = ;
    Event* ev = new Broadcast_mined_blk(mining_time, this);
    simulator->add_event(ev);
    return;
}

Transaction* Peer::generate_txn(Simulator* simulator){
    int cur_bal = balances[id];
    int coins=-1;
    if(cur_bal > 0){
        coin_dist = uniform_int_distribution<int>(1,cur_bal);
        coins = coin_dist(gen);
    }
    Transaction* txn = NULL;
    if(coins > 0){
        int receiver = rand_peer(gen);
        while(receiver == if) receiver = rand_peer(gen);
        txn = new Transaction(simulator->current_timestamp, this, simulator->peers[receiver], coins);
        recv_pool.insert(txn->id);
        txn_pool.insert(txn);

        Event* ev = new Forward_txn(0, this, this, txn);
        simulator->add_event(ev);
    }
    schedule_next_txn(simulator);
    return txn;
}

Block* Peer::generate_blk(Simulator* simulator){
    Block* blk = new Block(this);
    blk->set_parent(blkchain.current_block);
    vector<int> bal_copy = balances;

    for(Transaction* txn: txn_pool){
        if(blk->size + TRANSACTION_SIZE > MAX_BLOCK_SIZE) break;
        if(validate_txn(txn, bal_copy)){
            blk->add_txn(txn);
            bal_copy[txn->sender->id] -= txn->amount;
            bal_copy[txn->receiver->id] += txn->amount;
        }
    }
    return blk;
}

bool Peer::validate_txn(Transaction* txn, vector<int>& balances){
    int bal = balances[txn->sender->id];
    return txn->amount <= bal;
}

bool Peer::validate_blk(Block* blk, vector<int>& balances){
    if(blk->size > MAX_BLOCK_SIZE) return false;

    vector<int> bal_copy = balances;
    for(Transaction* txn: blk->txns){
        if(!validate_txn(txn, bal_copy)) return false;
        bal_copy[txn->sender->id] -= txn->amount;
        bal_copy[txn->receiver->id] += txn->amount;
    }
    return true;
}

void Peer::forward_txn(Simulator* simulator, Peer* src, Transaction* txn){
    for(link& link: adjList){
        if(link.peer->id == src->id) continue;
        ld delay = link.delay(TRANSACTION_SIZE);
        Event* ev = new Receive_txn(delay, this, link.peer, txn);
        simulator->add_event(ev);
    }
}
void Peer::receive_txn(Simulator* simulator, Peer* sender, Transaction* txn){
    if (recv_pool.find(txn->id) != recv_pool.end())
        return;

    recv_pool.insert(txn->id);
    txn_pool.insert(txn);
    Event* ev = new Forward_txn(0, this, sender, txn);
    simulator->add_event(ev);
}

void Peer::forward_blk(Simulator* simulator, Peer* src, Block* blk){
    // this block is a copy, memory needs to be freed
    for (Link& link : adjList) {
        if (link.peer->id == source->id) continue;  // source already has the blpck, loop-less forwarding
        Block* new_blk = blk->clone();
        ld delay = link.delay(new_blk->size);
        Event* ev = new Receive_blk(delay, this, link.peer, new_blk);
        simulator->add_event(ev);
    }
    delete blk;
}

void Peer::receive_blk(Simulator* simulator, Peer* sender, Block* blk){

    auto chain_it = chain_blks.find(blk->id);
    auto free_it = free_blks.find(blk->id);

    // already received this block
    if (chain_it != chain_blks.end() || free_it != free_blks.end()) 
        return;

    block_arrival_times.push_back(make_pair(blk, simulator->current_timestamp));
    // forward every received block regardless of validity
    Event* ev = new Forward_Blk(0, this, sender, blk->clone());
    simulator->add_event(ev);
    
    chain_it = chain_blks.find(blk->parent_id);

    // block parent not in our blockchain
    if (chain_it == chain_blocks.end()) {
        free_blks[blk->id] = blk;
        free_blk_parents[blk->parent_id].push_back(blk);
        return;
    }

    blk->set_parent(chain_it->second);

    Block* current_blk = blockchain.current_block; // last block in the blockchain
    Block* branch_blk = blk->parent; // add the new block as a child of branch block

    // balances to update in case longest chain changes
    vector<int> current_balance_change(total_peers, 0); 
    // txns to add to the txn pool in case longest chain changes
    vector<Transaction*> txns_to_add; 
    // find lca
    while (current_blk->depth > branch_blk->depth)
        current_blk = Blockchain::backward(current_blk, current_balance_change, txns_to_add);
    
    // balances to update in case longest chain changes
    vector<int> branch_balance_change(total_peers, 0);
    // txns to remove from the txn pool in case longest chain changes
    vector<Transaction*> txns_to_remove;
    while (branch_blk->depth > current_blk->depth)
        branch_blk = Blockchain::backward(branch_blk, branch_balance_change, txns_to_remove);

    while (branch_blk->id != current_blk->id) {
        current_blk = Blockchain::backward(current_blk, current_balance_change, txns_to_add);
        branch_blk = Blockchain::backward(branch_blk, branch_balance_change, txns_to_remove);
    }

    // current_balance_change = balances just before block insertion point
    for (int i = 0; i < total_peers; i++)
        current_balance_change[i] += balances[i] - branch_balance_change[i];

    unordered_set<Block*> blks_to_add;
    Block* deepest_blk = NULL;

    free_blks_dfs(blk, current_balance_change, blocks_to_add, deepest_blk, simulator);

    // block is invalid
    if (deepest_blk == NULL)
        return;

    // now block gets added to blockchain
    // balances will be updated only if branch was changed
    if (deepest_blk->depth > blockchain.current_block->depth) {

        // change peer state to just before block insertion
        balances = current_balance_change;
        for (Transaction* txn : txns_to_add)
            txn_pool.insert(txn);
        for (Transaction* txn : txns_to_remove)
            txn_pool.erase(txn);

        stack<Block*> order;
        order.push(deepest_blk);

        while (order.top() != block)
            order.push(order.top()->parent);

        while (!order.empty()) {
            Block* b = order.top();
            order.pop();
            add_blk(b, true);
            blocks_to_add.erase(b);
        }

        for (Block* b : blks_to_add)
            add_blk(b, false);

        if (next_mining_event != NULL) {
            simulator->delete_event(next_mining_event);
            delete next_mining_block;
            schedule_next_block(simulator);
        }
    } else {
        for (Block* b : blks_to_add)
            add_blk(b, false);
    }
}

void Peer::delete_invalid_free_blks(Block* blk, Simulator* simulator){

    auto it = free_blk_parents.find(blk->id);
    
    // add block to rejected_blocks
    rejected_blks.insert(blk->id);
    simulator->log(cout, get_name() + " REJECTS block " + blk->get_name());
    
    // dont delete block to maintain block arrivals consistency
    // delete block;

    if (it == free_blk_parents.end()) 
        return;

    // recursive call to delete child blocks
    for (Block* child : it->second) {
        assert(child->parent == NULL);
        child->set_parent(blk);
        free_blks.erase(child->id);
        delete_invalid_free_blocks(child, simulator);
    }
    free_blk_parents.erase(it);
}

void Peer::free_blks_dfs(Block* blk, vector<int>& cur_balances, unordered_set<Block*>& blks_to_add, Block*& deepest_blk, Simulator* simulator){
    if (!validate_blk(blk, cur_balances)) {
        delete_invalid_free_blks(blk, simulator);
        return;
    }

    blks_to_add.insert(blk);
    if (deepest_blk == NULL || blk->depth > deepest_blk->depth)
        deepest_blk = blk;

    auto it = free_blk_parents.find(blk->id);
    if (it == free_blk_parents.end()) 
        return;

    // update balance array
    for (Transaction* txn : blk->txns) {
        cur_balances[txn->sender->id] -= txn->amount;
        cur_balances[txn->receiver->id] += txn->amount;
    }
    cur_balances[blk->owner->id] += MINING_FEE; 

    // recursive call to chld blocks
    for (Block* child : it->second) {
        assert(child->parent == NULL);
        child->set_parent(blk);
        free_blks.erase(child->id);
        free_blks_dfs(child, cur_balances, blks_to_add, deepest_blk, simulator);
    }
    free_blk_parents.erase(it);

    // reset balance array
    for (Transaction* txn : blk->txns) {
        cur_balances[txn->sender->id] += txn->amount;
        cur_balances[txn->receiver->id] -= txn->amount;
    }
    cur_balances[blk->owner->id] -= MINING_FEE;
}

void Peer::broadcast_mined_blk(Simulator* simulator){
    Block* blk = next_mining_block;
	blk->set_id();
	
	assert(blkchain.current_blk->id == blk->parent->id);
	bool is_valid = validate_block(blk, balances);

	// do not add invalid block, only transmit it to other peers
	string validity = "INVALID";
	if (is_valid) {
		add_block(blk, true);
		validity = "VALID";
	}
	simulator->log(cout, get_name() + " mines and broadcasts " + validity + " block " + blk->get_name());
    blk_arrival_times.push_back(make_pair(blk, simulator->current_timestamp));

	Event* ev = new Forward_Blk(0, this, this, blk->clone());
	simulator->add_event(ev);

	schedule_next_blk(simulator);
}

/* output the edges in blockchain in os and update deepest_block */
void Peer::traverse_blockchain(Block* blk, ostream& os, Block*& deepest_blk, vector<int>& total_blks) {
    // for canonicalization
    sort(all(blk->next), [](Block* a1, Block* a2) {
        return (a1->id) < (a2->id);
    });
    if (blk->depth > deepest_blk->depth || (blk->depth == deepest_blk->depth && blk->id < deepest_blk->id))
        deepest_blk = blk;

    // genesis id is -2
    if (blk->parent_id >= -1) 
        total_blks[blk->owner->id]++;
    
    for (Block* c : blk->next) {
        os << (blk->id + 1) << ' ' << (c->id + 1) << '\n';
        traverse_blockchain(c, os, deepest_blk, total_blks);
    }
}

/* output the blockchain to a stream */
void Peer::export_blockchain(ostream& os){
    vector<int> total_blks(total_peers, 0);
    Block* deepest_blk = blkchain.genesis;
    traverse_blockchain(blkchain.genesis, os, deepest_blk, total_blks);
}

/* output the arrival times of blocks to os */
void Peer::export_arrival_times(ostream& os){
    sort(all(blk_arrival_times), [](pair<Block*, ld> a1, pair<Block*, ld> a2) {
        return (a1.first->id) < (a2.first->id);
    });


    os << get_name() << '\n';
    for (pair<Block*, ld>& p : blk_arrival_times) {
        Block* b = p.first;
        ld timestamp = p.second;
        os << b->get_name() << ", ";
        os << (b->depth) << ", ";
        os << fixed << setprecision(5) << timestamp << ", ";
        if (b->parent == NULL)
            os << "NO_PARENT" << '\n';
        else
            os << b->parent->get_name() << '\n';
    }
    os << '\n';
}

void Peer::export_stats(Simulator* simulator, ostream& os){
    ostream fake(0);
    Block* deepest_blk = blkchain.genesis;
    vector<int> total_blks(total_peers, 0);
    traverse_blockchain(blkchain.genesis, fake, deepest_blk, total_blks);

    vector<int> blks_in_chain(total_peers, 0);
    while (deepest_blk->id != blkchain.genesis->id) {
        blks_in_chain[deepest_blk->owner->id]++;
        deepest_blk = deepest_blk->parent;
    }

    os << "id,chain_blocks,generated_blocks,is_fast,frac_hashing_power\n";
    for (int i = 0; i < total_peers; i++) {
        os << (i + 1) << ',';
        os << blks_in_chain[i] << ',';
        os << total_blks[i] << ',';
        os << (int)simulator->peers[i]->is_fast << ',';
        os << simulator->peers[i]->frac_hashingPow << '\n';
    }
}


