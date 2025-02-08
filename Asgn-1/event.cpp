#include "descriptions.h"

using namespace std;

Event::Event(ld timestamp_){
    timestamp = timestamp_;
    is_generate_type_event = false;
}

bool Event::operator<(const Event& other){
    if(timestamp!=other.timestamp){
        return timestamp < other.timestamp;
    }else return this < &other;
}

void Event::run(Simulator* simulator){
    assert(false);
}

// ======================================================================= //
Generate_txn::Generate_txn(ld timestamp_, Peer* p) : Event(timestamp_) {
    payed_by = p;
    is_generate_type_event = true;
}

void Generate_txn::run(Simulator* simulator) {
    Transaction* txn = payed_by->generate_txn(simulator);
}

// ======================================================================= //
Forward_txn::Forward_txn(ld timestamp_, Peer* peer_, Peer* source_, Transaction* txn_) : Event(timestamp_) {
    peer = peer_;
    source = source_;
    txn = txn_;
}

void Forward_txn::run(Simulator* simulator) {
    peer->forward_txn(simulator, source, txn);
}

// ======================================================================= //
Receive_txn::Receive_txn(ld timestamp_, Peer* sender_, Peer* receiver_, Transaction* txn_) : Event(timestamp_) {
    sender = sender_;
    receiver = receiver_;
    txn = txn_;
}

void Receive_txn::run(Simulator* simulator) {
    receiver->receive_txn(sim, sender, txn);
}

// ======================================================================= //
Broadcast_mined_blk::Broadcast_mined_blk(ld timestamp, Peer* p) : Event(timestamp) {
    owner = p;
    is_generate_type_event = true;
}

void Broadcast_mined_blk::run(Simulator* simulator) {
    owner->broadcast_mined_blk(simulator);
}

// ======================================================================= //
Forward_blk::Forward_blk(ld timestamp_, Peer* peer_, Peer* source_, Block* block_) : Event(timestamp_) {
    peer = peer_;
    source = source_;
    block = block_;
}

void Forward_blk::run(Simulator* simulator) {
    peer->forward_blk(simulator, source, block);
}

// ======================================================================= //
Receive_blk::Receive_blk(ld timestamp_, Peer* sender_, Peer* receiver_, Block* block_) : Event(timestamp_) {
    sender = sender_;
    receiver = receiver_;
    block = block_;
}

void Receive_blk::run(Simulator* simulator) {
    receiver->receive_blk(simulator, sender, block);
}