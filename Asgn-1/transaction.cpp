#include "descriptions.h"

using namespace std;

int Transaction::counter;

/* Constructor */
Transaction::Transaction(ld timestamp_, Peer* sender_, Peer* receiver_, ld coins){
    id = counter++;
    sender = sender_;
    receiver = receiver_;
    amount = coins;
    timestamp = timestamp_;
}

string Transaction::get_name(){
    return "TxnID:"+to_string(id+1);
}

