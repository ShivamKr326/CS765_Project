#include "descriptions.h"

using namespace std;

Event::Event(ld timestamp_){
    timestamp = timestamp_;
    is_generate_type_event = false;
}

bool Event::operator<(const Event& other){
    if(timestamp!=other.timestamp){
        return timestamp < other.timestamp;
    }
    else return this < &other;
}
