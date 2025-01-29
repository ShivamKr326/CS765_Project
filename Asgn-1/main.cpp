#include "descriptions.h"

using namespace std;

// All the measured times are in milliseconds.


int main(){
    int n;
    ld z0,z1,Ttx;
    cout<<"Enter the number of nodes in the network: "<<endl;
    cin>>n;
    cout<<"Enter the percentage of slow nodes: "<<endl;
    cin>>z0;
    cout<<"Enter the percentage of low CPU nodes: "<<endl;
    cin>>z1;
    cout<<"Enter the mean time for the expo dist for txn generation: "<<endl;
    cin>>Ttx;
    int timeout,max_txns,max_blocks;
    cin>>timeout>>max_txns>>max_blocks;
    Simulator simulator(n,z0,z1,Ttx);
    simulator.run(timeout,max_txns,max_blocks);

    return 0;
}