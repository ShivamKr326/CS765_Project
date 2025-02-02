#include "descriptions.h"

using namespace std;

Link::Link(Peer* p, bool isfast, int light_delay){
    pij = light_delay;
    peer = p;
    if(peer->isfast && isfast) cij = 12500;
    else cij = 625;
    dij = exponential_distribution<ld>(cij/12);
}

int Link::delay(int msg_length){
    ld total_delay = (ld)pij + (ld)msg_length/cij + dij(gen);
    return (int)total_delay;
}