#ifndef MAIN_H
#define MAIN_H


#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "message.h"

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
        
        Master( int numSwitch, int portNumber );

        void setPfd ( int index, int fd );
        void serverListen();
        void startPoll();
        void information();
        void addPkInfo(helloPacket pckt);
        void sendHelloAck(int switchNum, int wfd);
};

class TOR {
    public:
        int sfd =1; // server socket file descriptor
        int portNumber;
        int switchNum;
        int prev, next;
        int ipLow, ipHigh;

        vector<Rules> forwardingTable;

        int fds[MAXPKFD][2];
        // 0 -> stdin, stdout (Non fifos)
        // 1 -> port 0, master (Non fifos, socket)
        // 2 -> port 1, prev
        // 3 -> port 2, next
        struct pollfd polledfds[MAXPKFD]; // pfds is polledfds

        string filename;
        string serverAddr;

        int admitCount = 0, hello_ackCount = 0, addCount = 0, relayinCount = 0;
        int helloCount = 0, askCount = 0, relayoutCount = 0;

        TOR(int switchNum, int portNumber, int prev, int next,
        int ipLow, int ipHigh, string filename, string serverAddr);

        void createSocket();

        void setPfd(int index, int fd);

        void createFIFO();

};

class Rules {
    public:
        int srcIP_lo, srcIP_hi;
        int destIP_lo, destIP_hi;
        int actionType, actionVal;
        int pkCount = 0;

        Rules (int srcIP_lo, int srcIP_hi, int destIP_lo, int destIP_hi, 
        int actionType, int actionVal );
        
        friend ostream& operator<< ( ostream& out, const Rules& r );

        bool isMatch( int srcIP, int destIP );
        bool isReach( int destIP );
        bool isEqual( const Rules& r );
};

Rules::Rules(int srcIP_lo, int srcIP_hi, int destIP_lo, int destIP_hi, 
        int actionType, int actionVal) {
            this->srcIP_hi = srcIP_hi;
            this->srcIP_lo = srcIP_lo;
            this->destIP_hi = destIP_hi;
            this->destIP_lo = destIP_lo;
            this->actionType = actionType;
            this->actionVal = actionVal;
        }

Master::Master(int numSwitch, int portNumber) {
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
        cout << "serverListen: failed to create a socket \n" << endl;

    cout << "create socket" << endl;

    // bind the managing socket to a name
    //
    sin.sin_family= AF_INET;
    sin.sin_addr.s_addr= htonl(INADDR_ANY);
    sin.sin_port= htons(portNumber);

    if (bind (sfd, (sockaddr *) &sin, sizeof(sin)) < 0)
        cout << "serverListen: bind failed \n" << endl;

    // indicate how many connection requests can be queued
  
    cout << "socket created" << endl;

    listen (sfd, MAX_NSW);

    setPfd(0, sfd);
}

void Master::information() {
    cout << "\n Switch Information \n";

    for ( auto it = pkInfos.begin(); it != pkInfos.end(); it++ ) {
        cout << "[psw" << to_string(it->swNum) << "]" << 
        " port1 = " << to_string(it->prev) << 
        ", port2 = " << to_string(it->next) << 
        ", port3 = " << to_string(it->ipLow) << "-" << to_string(it->ipHigh) << endl;
    }

    cout << "\n";

    cout << "Packet Stats:" << endl;
    
    cout << "\tReceived:\t" << "HELLO:" << to_string(helloCount) << 
    ", ASK:" << to_string(askCount) << endl;

    cout << "\tTransmitted: " << "HELLO_ACK:" << to_string(hello_ackCount) << 
    ", ADD:" << to_string(addCount) << endl << endl;
}

void Master::addPkInfo(helloPacket pckt) {
    pkInfo pkinfo;

    memset((char *)&pkinfo, 0, sizeof(pkinfo));
    
    pkinfo.ipHigh = pckt.IP_high;
    pkinfo.ipLow = pckt.IP_low;
    pkinfo.next = pckt.next;
    pkinfo.prev = pckt.prev;
    pkinfo.swNum = pckt.switchNumber;

    auto position = pkInfos.begin() + pkinfo.swNum - 1;

    pkInfos.insert(position, pkinfo);

    cout << "Package Switch " << pkinfo.swNum << " is connected \n" << endl;
}

void Master::sendHelloAck(int switchNum, int wfd) {
    MSG helloAckPk;

    memset(&helloAckPk, 0, sizeof(helloAckPk));

    helloAckPk = composeHelloAcknowledge();

    string prefix = "Transmitted (src = master, dest = " + to_string(switchNum) + ")";

    sendFrame(wfd, HELLO_ACK, &helloAckPk);

    hello_ackCount++;
}

void Master::startPoll() {
    int i = 0; // index for looping polled file descriptors
    int length = 0; // measure number of bytes read from socket
    int rval = 0;

    char buf[MAXBUF];
    string prefix;

    FRAME frame;
    MSG msg;

    cout << "start polling" << endl << endl;


    while (1) {
    rval= poll (polledfds, MAXMSFD, 0);
    for (i= 0; i < MAXMSFD; i++) {
        if ((polledfds[i].revents & POLLIN)) {
            if (i == 0) {
                length = read(polledfds[i].fd, buf, MAXBUF);
            }

            if (strcmp(buf, "info\n") == 0) {
                information();
            }

            else if (strcmp(buf, "exit\n") == 0) {
                information();
                return;
            }
            else {
                cout << "Command not found!" << endl;
            }

            memset(&buf, 0, sizeof(buf));
        }

        else {
            frame = rcvFrame(polledfds[i].fd, &length);

            msg = frame.msg;

            prefix = "\nReceived (src = psw" + to_string(i) + ", dest = master)";

            printFrame(prefix.c_str(), &frame);

            switch (frame.kind)
            {
            case HELLO:
                helloCount++;
                addPkInfo(frame.msg.hello);
                sendHelloAck(i, polledfds[i].fd);
                break;
            case ASK:
                break;
            default:
                break;
            }
        }
    }
}
}

TOR::TOR(int switchNum, int portNumber, int prev, int next,
        int ipLow, int ipHigh, string filename, string serverAddr) {

            this->switchNum = switchNum;
            this->portNumber = portNumber;
            this->prev = prev;
            this->next = next;
            this->ipHigh = ipHigh;
            this->ipLow = ipLow;
            this->filename = filename;
            this->serverAddr = serverAddr;

            this->forwardingTable.push_back(Rules(0, MAXIP, this->ipLow, this->ipHigh, FORWARD, 3));

            memset(&fds, 0, sizeof(fds));
            memset(&polledfds, 0, sizeof(polledfds));

            fds[0][0] = STDIN_FILENO;
            fds[0][1] = STDOUT_FILENO;
            setPfd(0, fds[0][0]);
}

void TOR::createSocket() {
    struct sockaddr_in  server;
    struct hostent      *hp;                    // host entity

    // lookup the specified host
    //
    hp= gethostbyname(serverAddr.c_str());
    if (hp == (struct hostent *) NULL)
        cout << "clientConnect: failed gethostbyname" << endl;

    // put the host's address, and type into a socket structure
    //
    memset ((char *) &server, 0, sizeof server);
    memcpy ((char *) &server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_family= AF_INET;
    server.sin_port= htons(portNumber);

    // create a socket, and initiate a connection
    if ( (sfd= socket(AF_INET, SOCK_STREAM, 0)) < 0)
        cout << "clientConnect: failed to create a socket" << endl;

    if (connect(sfd, (sockaddr *) &server, sizeof(server)) < 0)
	cout << "clientConnect: failed to connect \n";

    // sokcet file descriptor accecpt both read and write
    fds[1][0] = sfd;
    fds[1][1] = sfd;

    setPfd( 1, sfd);
}

void TOR::setPfd(int index, int fd) {
    polledfds[index].fd = fd;
    polledfds[index].events = POLLIN;
    polledfds[index].revents = 0;
}

void TOR::createFIFO() {
    if (prev != -1) {
        string name = "fifo-" + to_string(prev) + "-" + to_string(switchNum);

        cout << "making " + name + "..." << endl;
        if ( mkfifo( name.c_str(), S_IRWXU ) < 0 ) {
            if ( errno != EEXIST ) {
                cout <<  "mkfifo error()" << endl;
                exit(1);
            }
        }
        cout << name + " is made" << endl << endl;

        cout << "opening " + name + " for read..." << endl;
        if ( ( fds[2][0] = open( name.c_str(), O_RDWR ) ) < 0 ) {
            cout << "open error()" << endl;
            exit(1);
        }
        cout << name + " is open" << endl << endl;

        string name = "fifo-" + to_string(switchNum) + "-" + to_string(prev);

        cout << "making " + name + "..." << endl;
        if ( mkfifo( name.c_str(), S_IRWXU ) < 0 ) {
            if ( errno != EEXIST ) {
                cout << "mkfifo error()" << endl;
                exit(1);
            }
        }
        cout << name + " is made" << endl << endl;

        cout << "opening " + name + " for write..." << endl;
        if ( ( fds[2][1] = open( name.c_str(), O_RDWR ) ) < 0 ) {
            cout << "open error()" << endl;
            exit(1);
        }

        cout << name + " is open" << endl << endl;
    }

    if (next != -1) {
        string name = "fifo-" + to_string(next) + "-" + to_string(switchNum);

        cout << "making " + name + "..." << endl;
        if ( mkfifo( name.c_str(), S_IRWXU ) < 0 ) {
            if ( errno != EEXIST ) {
                cout <<  "mkfifo error()" << endl;
                exit(1);
            }
        }
        cout << name + " is made" << endl << endl;

        cout << "opening " + name + " for read..." << endl;
        if ( ( fds[3][0] = open( name.c_str(), O_RDWR ) ) < 0 ) {
            cout << "open error()" << endl;
            exit(1);
        }
        cout << name + " is open" << endl << endl;

        string name = "fifo-" + to_string(switchNum) + "-" + to_string(next);

        cout << "making " + name + "..." << endl;
        if ( mkfifo( name.c_str(), S_IRWXU ) < 0 ) {
            if ( errno != EEXIST ) {
                cout << "mkfifo error()" << endl;
                exit(1);
            }
        }
        cout << name + " is made" << endl << endl;

        cout << "opening " + name + " for write..." << endl;
        if ( ( fds[3][1] = open( name.c_str(), O_RDWR ) ) < 0 ) {
            cout << "open error()" << endl;
            exit(1);
        }

        cout << name + " is open" << endl << endl;
    }
    
}

bool Rules::isMatch(int srcIP, int destIP) {
    return ( srcIP >= srcIP_lo ) && ( srcIP <= srcIP_hi ) && ( destIP >= destIP_lo ) && ( destIP <= destIP_hi ) ;
}

bool Rules::isReach(int destIP) {
    return actionVal == 3 && destIP <= destIP_hi && destIP >= destIP_lo;
}

bool Rules::isEqual(const Rules& r) {
    return srcIP_lo == r.srcIP_lo && 
           srcIP_hi == r.srcIP_hi && 
           destIP_lo == r.destIP_lo && 
           destIP_hi == r.destIP_hi && 
           actionType == r.actionType && 
           actionVal == r.actionVal;
}

#endif