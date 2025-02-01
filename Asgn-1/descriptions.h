#ifndef DESCRIPTIONS_H
#define DESCRIPTIONS_H

#include "headers.h"
using namespace std;

#define TRANSACTION_SIZE 1 // 1 KB	
#define MAX_BLOCK_SIZE 1000 // 1000 KB
#define START_TIME 0
#define MINING_FEE 50

class Peer;
class Block;
class Transaction;
class Simulator;
class Blockchain;

class Block{
    public:
        static int counter;
        static int max_size;
        int size,id,depth,parent_id;
        Peer* miner;
        Block* parent;

        vector<Transaction*> txns;
        vector<Block*> next;

        Block(Peer*);
        void set_id();
        void add_txn(Transaction*);
        void set_parent(Block*);
        void reset_parent();
        string get_name();
};

struct txn_comparator{
    bool operator(Transaction* a, Transaction* b){
        return a->timestamp <= b->timestamp;
    }
};

class Peer{
    static int counter;
    static int total_peers;
    static ld Ttx;
    ld frac_hashingPow;
    int id;
    bool isfast, ishigh;
    
    unifor_int_distribution<int> rand_peer;
    exponential_distribution<ld> txn_interarrival, blk_mining;

    vector<int> balances; // for storing remaining balances at each peer
    vector<Link> adjList; // list of peers adj to this peer

    unordered_set<int> recv_pool; // all txn ids recv so far
    set<Transaction*, txn_comparator> txn_pool; // txn not yet mined
    Blockchain blkchain; // local blockchain copy
    vector<pair<Block*, ld>> block_arrival_time; // store arrival time for each block

    Peer();
};

class Transaction{
    static int counter;
    int id;
    Peer* sender, receiver;
    ld timestamp;
    ld amount;

    Transaction(ld,Peer*, Peer*,ld);
    string get_name();
};

struct blk_comparator{
    bool operator(Event* a, Event* b){
        return a->timestamp <= b->timestamp;
    }
};

class Simulator{
    int n, slowPeers, lowPeers;
    ld Ttx, current_timestamp;
    bool simulation_ended;
    Event* current_event;

    static vector<Peer*> peers;
    set<Event*, blk_comparator> events; 

    Simulator(int, ld, ld, ld);
    ~Simulator();
    void get_peers();
    void construct_network();
    void events_init();
    void add_event(Event*);
    void delete_event(Event*);
    void reset(const fs::path& dir_path);
    void run(int, int, int);
    void complete_non_generate_events();
};

class Blockchain{

};

#endif