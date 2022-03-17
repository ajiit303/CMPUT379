#ifndef MAIN_H
#define MAIN_H


#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>


#define DROP 0
#define FORWARD 1
#define MAXBUF 80
#define MAX_NSW 7
#define MAXMSFD MAX_NSW + 2
#define MAXIP 1000
#define MAXPKFD 4


const char actionNames[2][16] = { "DROP", "FORWARD" };


using namespace std;

typedef struct {
    int swNum;
    int prev, next;
    int ipLow, ipHigh;
} pkInfo;


class Master {
    public:
        int acceptConnection = 0;
        int numSwitch;
        int portNumber;
        int sfd = -1; // server socket file descriptor

        int helloCount = 0, askCount = 0;
        int hello_ackCount = 0, addCount = 0;

        // 0 -> server socket
        // 1 -> stdin
        // 2 ~ 8 -> packet switch connection
        // file descriptors being polled
        
        struct pollfd polledfds[MAXMSFD]; // pfds is polledfds
        
        vector<pkInfo> pkInfos;
        
        int forkSwitches();
        
        void setPfd ( int index, int fd );
        void serverListen();
};

void Master::Master(int numSwitch, int portNumber) {
    this->numSwitch = numSwitch;
    this->portNumber = portNumber;

    memset(polledfds, 0, sizeof(polledfds));
    setPfd(1, STDIN_FILENO);
}

void Master::setPfd(int index, int fd) {
    polledfds[index].fd = fd;
    polledfds[index].events = POLLIN;
    polledfds[index].revents = 0;
}

void Master::serverListen() {
    struct sockaddr_in  sin;

    memset ((char *) &sin, 0, sizeof(sin));

    // create a managing socket
    //
    if ( (sfd= socket (AF_INET, SOCK_STREAM, 0)) < 0)
        FATAL ("serverListen: failed to create a socket \n");

    // bind the managing socket to a name
    //
    sin.sin_family= AF_INET;
    sin.sin_addr.s_addr= htonl(INADDR_ANY);
    sin.sin_port= htons(portNumber);

    if (bind (sfd, (SA *) &sin, sizeof(sin)) < 0)
        FATAL ("serverListen: bind failed \n");

    // indicate how many connection requests can be queued
  
    listen (sfd, MAX_NSW);

    setPfd(0, sfd); 
}


#endif