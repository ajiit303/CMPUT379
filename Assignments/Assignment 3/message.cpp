// The implementation of packet is identical in most of the aspect 
// to the one in lab experiment.

#include "message.h"


Frame rcvFrame (int fd) {
    int len;
    Frame frame;

    assert( fd >= 0 );
    memset( (char *)&frame, 0, sizeof(frame) );

    len = read( fd, (char *)&frame, sizeof(frame) );
    if ( len != sizeof(frame) )
        warning( "Received from has length = %d (expected = %d), fd: %d \n", len, sizeof(frame), fd );
    
    return frame;
}

Packet composeADD ( int srcIP_lo, int srcIP_hi, int destIP_lo, int destIP_hi, 
    int actionType, int actionVal, int pkCount ) {
    Packet pk;

    memset( (char *)&pk, 0, sizeof(pk) );
    
    pk.addPacket.srcIP_lo = srcIP_lo;
    pk.addPacket.srcIP_hi = srcIP_hi;
    pk.addPacket.destIP_lo = destIP_lo;
    pk.addPacket.destIP_hi = destIP_hi;
    pk.addPacket.actionType = actionType;
    pk.addPacket.actionVal = actionVal;
    pk.addPacket.pkCount = pkCount;

    return pk;
}

Packet composeASK ( int srcIP, int destIP ) {
    Packet pk;

    memset( (char *)&pk, 0, sizeof(pk) );

    pk.askPacket.srcIP = srcIP;
    pk.askPacket.destIP = destIP;

    return pk;
}

Packet composeHELLO ( int switchNum, int prev, int next, int srcIP_lo, int srcIP_high ) {
    Packet pk;

    memset( (char *)&pk, 0, sizeof(pk) );

    pk.helloPacket.switchNum = switchNum;
    pk.helloPacket.prev = prev;
    pk.helloPacket.next = next;
    pk.helloPacket.srcIP_lo = srcIP_lo;
    pk.helloPacket.srcIP_hi = srcIP_high;

    return pk;
}

Packet composeHELLO_ACK () {
    Packet pk;

    memset( (char *)&pk, 0, sizeof(pk) );

    return pk;
}

Packet composeRELAY ( int srcIP, int destIP ) {
    Packet pk;

    memset( (char *)&pk, 0, sizeof(pk) );

    pk.replayPacket.srcIP = srcIP;
    pk.replayPacket.destIP = destIP;

    return pk;
}

void printFrame ( const char *prefix, Frame *frame ) {
    Packet pk = frame->packet;

    printf( "%s [%s]:", prefix, PacketTypeName[frame->packetType] );

    switch(frame->packetType) {
        case ADD:
            printf( " \n(srcIP = %d-%d, destIP = %d-%d, action = %s:%d, pkCount = %d)\n\n",
            pk.addPacket.srcIP_lo, pk.addPacket.srcIP_hi,
            pk.addPacket.destIP_lo, pk.addPacket.destIP_hi,
            actionNames[pk.addPacket.actionType], pk.addPacket.actionVal, 
            pk.addPacket.pkCount );
            break;
        case ASK:
            printf( "  header = (srcIP = %d, destIP = %d)\n\n", 
            pk.askPacket.srcIP, pk.askPacket.destIP );
            break;
        case HELLO:
            printf( " \n(port0 = master, port1 = %d, port2 = %d, port3 = %d-%d)\n\n",
             pk.helloPacket.prev, pk.helloPacket.next,
             pk.helloPacket.srcIP_lo, pk.helloPacket.srcIP_hi );
             break;
        case HELLO_ACK:
            printf( "\n\n" );
            break;
        case RELAY:
            printf( "  header = (srcIP = %d, destIP = %d)\n\n",
            pk.replayPacket.srcIP, pk.replayPacket.destIP );
            break;
        case UNKNOWN:
            printf(" \nUnknown packet");
            break;
    }
}

void sendFrame ( const char *prefix, int fd, PacketType packetType, Packet *packet ) {
    Frame frame;

    assert( fd >= 0 );
    memset( (char *)&frame, 0, sizeof(frame) );

    frame.packetType = packetType;
    frame.packet = *packet;

    write( fd, (char *)&frame, sizeof(frame) ); 

    printFrame( prefix, &frame );
}