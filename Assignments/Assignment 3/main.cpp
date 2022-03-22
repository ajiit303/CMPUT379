#include "main.h"


struct sigaction oldAct, newAct;

static Master *master;
// static PacketSwitch *packetSwitch;
static TOR *tor;

static int masterNumber = 0;

void infoHandler (int sigNumber) {
    if (masterNumber) {
        master->information();
    }
    else {
        tor->information();
    }
}


int main ( int argc, char *args[] ) {
    newAct.sa_handler = &infoHandler;
    newAct.sa_flags |= SA_SIGINFO;

    sigaction(SIGUSR1, &newAct, &oldAct);

    if (argc == 4) {
        int numSwitch = atoi(args[2]);
        int portNumber = atoi(args[3]);

        if ( numSwitch > 0 && numSwitch <= MAX_NSW ) {
            masterNumber = 1;
            master = new Master( numSwitch, portNumber );
            
            master->serverListen();
            master->startPoll();

            delete master;
        }
    }
    else if ( argc == 8 ) {
        int switchNum = tor->stringToInt( string(args[1]) );
        string filename = string(args[2]);
        int prev = tor->stringToInt(string(args[3]));
        int next = tor->stringToInt(string(args[4]));

        vector<string> ipRange;

        tor->split( string(args[5]), "-", ipRange );

        int ipLow = stoi(ipRange.front());
        int ipHigh = stoi(ipRange.back());

        string serverName = string(args[6]);
        int portNumber = atoi(args[7]);

        tor = new TOR(switchNum, portNumber, prev, next, ipLow, ipHigh, 
            filename, serverName);

        tor->createSocket();
        tor->createFIFO();

        int rval;
        pthread_t fileThreadId, pollThreadId;

        rval = pthread_create( &fileThreadId, NULL, (THREADFUNCPTR)&TOR::fileReading, tor);

        if (rval) {
            cout << "pthread_create() error" << endl;
            exit(1);
        }

        rval = pthread_create( &pollThreadId, NULL, (THREADFUNCPTR)&TOR::startPoll, tor);

        if (rval) {
            cout << "pthread_create() error" << endl;
            exit(1);
        }

        rval = pthread_join( fileThreadId, NULL );
        if (rval) {
            cout << "pthread_join() error " << rval << endl;  
        }

        rval = pthread_join( pollThreadId, NULL );
        if (rval) {
            cout << "pthread_join() error " << rval << endl;  
        }

        delete tor;
    }
    
    return 0;
}