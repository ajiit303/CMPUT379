#include "main.h"

ostream& operator<< ( ostream& out, const Rules& r ) {
    string line = 
        "(srcIP = " + to_string(r.srcIP_lo) + "-" + to_string(r.srcIP_hi) + 
        ", destIP = " + to_string(r.destIP_lo) + "-" + to_string(r.destIP_hi) +
        ", action = " + actionNames[r.actionType] + ":" + to_string(r.actionVal) + 
        ", pkCount = " + to_string(r.pkCount) + ")";
    
    out << line << endl;

    return out;
}