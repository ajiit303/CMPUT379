#include "main.h"


struct sigaction oldAct, newAct;

static Master *master;
// static PacketSwitch *packetSwitch;


int main ( int argc, char *args[] ) {

    if ( argc == 4 ) {
        int numSwitch = atoi(args[2]);
        int portNumber = atoi(args[3]);

        if ( numSwitch > 0 && numSwitch <= MAX_NSW ) {
            master = new Master( numSwitch, portNumber );
            
            master->serverListen();
            master->

            delete master;
        }
    } /* else if ( argc == 8 ) {

        int switchNum = stoSwNum( string(args[1]) );
        string filename = string(args[2]);
        int prev = stoSwNum( string(args[3]) );
        int next = stoSwNum( string(args[4]) );

        vector<string> ipRange;

        split( string(args[5]), "-", ipRange );

        int ipLow = stoi(ipRange.front());
        int ipHigh = stoi(ipRange.back());

        string serverName = string(args[6]);
        int portNumber = atoi(args[7]);

        packetSwitch = new PacketSwitch( switchNum, prev, next, ipLow, ipHigh, 
            filename, serverName, portNumber );

        packetSwitch->connectMaster();
        packetSwitch->createFIFO();

        int rval;
        pthread_t fileThreadId, pollThreadId;

        rval = pthread_create( &fileThreadId, NULL, (THREADFUNCPTR)&PacketSwitch::readFile, packetSwitch );

        if (rval) {
            fatal( "pthread_create() error" );
            exit(1);
        }

        rval = pthread_create( &pollThreadId, NULL, (THREADFUNCPTR)&PacketSwitch::startPoll, packetSwitch );

        if (rval) {
            fatal( "pthread_create() error" );
            exit(1);
        }

        rval = pthread_join( fileThreadId, NULL );
        if (rval) {
            fatal( "pthread_join() error %d", rval );  
        }

        rval = pthread_join( pollThreadId, NULL );
        if (rval) {
            fatal( "pthread_join() error %d", rval );  
        }

        delete packetSwitch;
    }
    */

    return 0;
}