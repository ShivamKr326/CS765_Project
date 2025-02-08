#ifndef DESCRIPTIONS_H
#define DESCRIPTIONS_H

#include "headers.h"
using namespace std;

#define TRANSACTION_SIZE 1 // 1 KB	
#define MAX_BLOCK_SIZE 1000 // 1000 KB
#define START_TIME 0
#define MINING_FEE 50

extern random_device rd;
extern mt19937 gen;

class Block;
class Peer;
class Transaction;
class Simulator;
class Blockchain;
class Link;
class Event;
class Forward_blk;
class Receive_blk;
class Generate_txn;
class Forward_txn;
class Receive_txn;
class Broadcast_mined_blk;

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
        Block* clone();
};

class Blockchain{
    public:
        /* pointer to the genesis block */
        static Block* global_genesis;
        /* pointer to the genesis block in this blockchain*/
        Block* genesis;
        /* pointer to the last block in the blockchain */
        Block* current_block;
        Blockchain();
        void add_block(Block*);
        static Block* backward(Block*, vector<int>&, vector<Transaction*>&);
};

class Peer{
    public:
        static int counter;
        static int total_peers;
        static ld Ttx;
        ld frac_hashingPow;
        int id;
        bool isfast, ishigh;
        
        uniform_int_distribution<int> rand_peer;
        uniform_int_distribution<int> rand_speed_of_light_delay;
        exponential_distribution<ld> txn_interarrival, blk_mining;

        vector<int> balances; // for storing remaining balances at each peer
        vector<Link> adjList; // list of peers adj to this peer

        unordered_set<int> recv_pool; // all txn ids recv so far
        set<Transaction*, txn_comparator> txn_pool; // txn not yet mined
        Blockchain blkchain; // local blockchain copy
        vector<pair<Block*, ld>> blk_arrival_time; // store arrival time for each block
        unordered_set<int> rejected_blks;
        map<int,Block*> chain_blks, free_blks; //chain_blks->blks in chain, free_blks->blks not in chain
        map<int,vector<Block*>> free_blk_parents; // parents of free blks
        unordered_set<int> recv_blk;

        /* pointer to next mining event */
        Broadcast_mined_blk* next_mining_event;
        /* pointer to next mining block */
        Block* next_mining_block;

        Peer();
        virtual ~Peer(){};
        static void add_edge(Peer*, Peer*);
        void add_blk(Block*, bool);
        void schedule_next_txn(Simulator*);
        void schedule_next_blk(Simulator*);
        Transaction* generate_txn(Simulator*);
        Block* generate_blk(Simulator*);
        bool validate_txn(Transaction*, vector<int>&);
        bool validate_blk(Block*,vector<int>&);
        void forward_txn(Simulator*, Peer*, Transaction*);
        void receive_txn(Simulator*, Peer*, Transaction*);
        void forward_blk(Simulator*, Peer*, Block*);
        void receive_blk(Simulator*, Peer*, Block*);
        void delete_invalid_free_blks(Block*, Simulator*);
        void free_blks_dfs(Block*, vector<int>&, unordered_set<Block*>&, Block*&, Simulator*);
        void broadcast_mined_blk(Simulator*);
        void traverse_blockchain(Block*, ostream&, Block*&, vector<int>&);
        void export_blockchain(ostream&);
        void export_arrival_times(ostream&);
        void export_stats(Simulator*, ostream&);
};

class Transaction{
    public:
        static int counter;
        int id;
        Peer* sender, receiver;
        ld timestamp;
        ld amount;

        Transaction(ld,Peer*, Peer*,ld);
        string get_name();
};

struct txn_comparator{
    bool operator()(Transaction* a, Transaction* b){
        return a->timestamp <= b->timestamp;
    }
};

class Simulator{
    public:
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

class Link{
    public:
        int cij,pij; //cij is in mb
        exponential_distribution<ld> dij;
        Peer* peer;
        Link(Peer*, bool, int);
        int delay(int);
};

class Event{
    public:
        ld timestamp;
        bool is_generate_type_event;
        virtual void run(Simulator*);
        bool operator<(const Event&);
        Event(ld);
        virtual ~Event(){};
};

struct blk_comparator{
    bool operator()(Event* a, Event* b){
        return a->timestamp <= b->timestamp;
    }
};

class Forward_txn : public Event {
    public:
        Peer* peer; // peer who'll forward the transaction
        Peer* source; // peer who sent the transaction (won't forward to this peer)
        Transaction* txn; // transaction
        
        Forward_txn(ld, Peer*, Peer*, Transaction*);
        void run(Simulator*);
    };
    
    // ======================================================================= //
    class Receive_txn : public Event {
    public: 
        Peer* sender;
        Peer* receiver; // peer who receives the transaction
        Transaction* txn; // transaction
        
        void run(Simulator*);
        Receive_txn(ld, Peer*, Peer*, Transaction*);
    };
    
    // ======================================================================= //
    class Generate_txn : public Event {
    public:
        Peer* payed_by;
    
        void run(Simulator*);
        Generate_txn(ld, Peer*);
    };
    
    // ======================================================================= //
    class Receive_blk : public Event {
    public:
        Peer* sender;
        Peer* receiver; // peer who receives the block
        Block* block; // block
    
        void run(Simulator*);
        Receive_blk(ld, Peer*, Peer*, Block*);
    };
    
    // ======================================================================= //
    class Forward_blk : public Event {
    public:
        Peer* peer; // peer who'll forward the block
        Peer* source; // peer who sent the block (won't forward to this peer)
        Block* block; // block
        
        Forward_blk(ld, Peer*, Peer*, Block*);
        void run(Simulator*);
    };
    
    // ======================================================================= //
    class Broadcast_mined_blk : public Event {
    public:
        Peer* owner;
    
        Broadcast_mined_blk(ld, Peer*);
        void run(Simulator*);
    };

#endif