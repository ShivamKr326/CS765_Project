#include "descriptions.h"

using namespace std;

random_device rd;
mt19937 gen(rd());

/* Constructor */
Simulator::Simulator(int n_,ld z0_,ld z1_,ld Ttx_){
    n = n_;
    slowPeers = (int)z0_*n;
    lowPeers = (int)z1_*n;
    Ttx = Ttx_;
    current_timestamp = START_TIME;
    simulation_ended = false;

    Transaction::counter = 0;
    Block::max_size = MAX_BLOCK_SIZE;  // 1000 KB
    Block::counter = 0;
    Blockchain::global_genesis = new Block(NULL);
    Blockchain::global_genesis->set_parent(NULL);
    Peer::counter = 0;
    Peer::totalPeers = n;
    Peer::Ttx = Ttx;
}

/* Destructor */
Simulator::~Simulator() {
    for (int i = 0; i < n; i++) {
        delete peers[i];
    }
}

/* initialize peers*/
void Simulator::get_peers(){

    peers.resize(n);

    for(int i=0;i<n;i++){
        peers[i] = new Peer;
        peers[i]->id = Peer::counter++;
    }
    vector<int> participants(n);
    for(int i=0;i<n;i++) participants[i]=i;

    // random_device rd;
    // mt19937 g(rd());
    shuffle(participants.begin(),participants.end(),gen);

    for(int i=0;i<slowPeers;i++){
        peers[participants[i]]->isfast = false;
    }
    for(int i=slowPeers;i<n;i++){
        peers[participants[i]]->isfast = true;
    }

    shuffle(participants.begin(),participants.end(),gen);

    for(int i=0;i<lowPeers;i++){
        peers[participants[i]]->ishigh = false;
    }
    for(int i=lowPeers;i<n;i++){
        peers[participants[i]]->ishigh = true;
    }

    int total_hashing_power = lowPeers + (n-lowPeers)*10;
    for(int i=0;i<n;i++){
        if(peers[i]->islow) peers[i]->frac_hashingPow = (ld)1/total_hashing_power;
        else peers[i]->frac_hashingPow = (ld)10/total_hashing_power; 
    }
    return;
}   

void createRandomConnections(){
    for(int i=0;i<n;i++){
        peers[i]->adjList.clear(); //adjList is an unordered_set of pair of pair of int(neighbour),bool(whether neighbour is fast)
        // and pij for the edge between them
    }
    // random_device rd;
    // mt19937 gen(rd());
    uniform_int_distribution<int> degreeDist(3, 6); // Random degree between 3 and 6

    for (int i = 0; i < n; i++) {
        while (peers[i]->adjList.size() < degreeDist(gen)) {
            int peer = rand() % n; // Random peer
            if (peer != i && peers[i]->adjList.size() < 6 && peers[peer]->adjList.size() < 6) {
                Peer::add_edge(peers[i],peers[peer]); // here have to add the adj node, its parity for fast and pij.
            }
        }
    }
    return;
}

bool isConnected(){
    vector<bool> vis(n,0);
    queue<int> q;
    q.push(0);
    vis[0] = 1;
    while(!q.empty()){
        int peer = q.front();
        q.pop();
        for(auto it:peers[peer]->adjList){
            if(!vis[it.first]){
                vis[it.first] = 1;
                q.push(it.first);
            }
        }
    }
    for(int i=0;i<n;i++){
        if(!vis[i]) return false;
    }
    return true;
}

void printTopology(){
    for(int i=0;i<n;i++){
        cout<<"Neighbours of "<<(i+1)<<"th node are: ";
        for(auto it:peers[i]->adjList){
            cout<<"{"<<it.first<<" "<<it.second<<"}";
        }
        cout<<endl;
    }
    return;
}

/* Constructing network Topology */
void Simulator::construct_network(){
    assert(n>=4);
    do{
        createRandomConnections();
    }while(!isConnected());
    printTopology();
    return;
}

/* Event generator */
void Simulator::events_init(){
    for(Peer* peer: peers){
        peer->schedule_next_txn(this);
        peer->schedul_next_block(this);
    }
    return;
}

void Simulator::add_event(Event* event){
    event_timestamp += current_timestamp;
    events.insert(event);
    return;
}

void Simulator::delete_event(Event* event){
    assert(event != NULL);
    auto it = events.find(event);
    assert(it != events.end());
    events.erase(it);
    delete event;
    return;
}

void Simulator::reset(const fs::path& dir_path) {
    fs::remove_all(dir_path);
    fs::create_directories(dir_path);
}

void Simulator::run(int timeout_,int max_txns_,int max_blocks_){
    get_peers();
    reset("output");
    
    construct_network();
    events_init();
    simulation_ended = false;

    int max_txns = max_txns_ <= 0 ? INT_MAX : max_txns_;
    int max_blocks = max_blocks_ <= 0 ? INT_MAX : max_blocks_;
    ld timeout = timeout_ <= 0 ? DBL_MAX : timeout_;

    while (!events.empty()) {
        current_event = *events.begin();
        current_timestamp = current_event->timestamp;

        if (current_event->timestamp > end_time)
            break;
        if (Transaction::counter >= max_txns)
            break;
        if (Block::counter >= max_blocks)
            break;

        current_event->run(this);

        delete_event(current_event);
    }

    reset("output/termination_blockchains");

    for (Peer* p : peers) {
        string filename = "output/termination_blockchains/" + p->get_name() + ".txt";
        ofstream outfile(filename);
        p->export_blockchain(outfile);
        outfile.close();
    }

    complete_non_generate_events();

    reset("output/block_arrivals");
    for (Peer* p : peers) {
        string filename = "output/block_arrivals/" + p->get_name() + ".txt";
        ofstream outfile(filename);
        p->export_arrival_times(outfile);
        outfile.close();
    }

    reset("output/peer_stats");
    for (Peer* p : peers) {
        string filename = "output/peer_stats/" + p->get_name() + ".txt";
        ofstream outfile(filename);
        p->export_stats(this, outfile);
        outfile.close();
    }

    cout << "Total Transactions: " << (Transaction::counter) << '\n';
    cout << "Total Blocks: " << (Block::counter) << '\n';
    return;
}

void Simulator::complete_non_generate_events() {
    simulation_ended = true;

    for (Peer* p : peers) {
        p->next_mining_event = NULL;
        p->next_mining_block = NULL;
    }

    while (!events.empty()) {
        current_event = *events.begin();
        current_timestamp = current_event->timestamp;

        if (!current_event->is_generate_type_event)
            current_event->run(this);

        delete_event(current_event);
    }

    reset("output/final_blockchains");
    for (Peer* p : peers) {
        string filename = "output/final_blockchains/" + p->get_name() + ".txt";
        ofstream outfile(filename);
        p->export_blockchain(outfile);
        outfile.close();
    }
}