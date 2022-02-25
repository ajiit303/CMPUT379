// https://docs.fileformat.com/programming/h/ (Header Guards)
#ifndef MASTER_H
#define MASTER_H


#include "message.h"

#include <vector>

#include <poll.h>

using namespace std;


typedef struct {
    int swNum;
    int next, prev;
    int ipHigh, ipLow;
} Switch;

class MasterSwitch {

    private:
        int swNums;

        int askCount, helloCount;
        int addCount, hello_ackCount;

        // 0 -> stdin, stdout
        // 1 ~ MAXNSW all packets switch
        int fifos[MAXMSFD][2];
        struct pollfd pfds[MAXMSFD];
        
        vector<Switch> switch;
        
        int forkSwitches();
        
        void setPfd ( int i, int rfd );
    
    public:
        MasterSwitch(int swNums);
        void initFIFO();
        void info();
        
        void startPoll();

        void sendHELLO_ACK(int switchNum, int fd);
        
        void addSwitchInfo(HELLOPacket pk);
        
};

#endif