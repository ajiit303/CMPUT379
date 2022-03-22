#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
		     
#define MAXLINE   132
#define MAXWORD    32

#define NF 3		 // number of fields in each message

#define MSG_KINDS 5
typedef enum {UNKNOWN, ADD, ASK, DISCONNECT, HELLO, HELLO_ACK, RELAY} KIND;	  // Message kinds
char KINDNAME[][MAXWORD]= {"UNKNOWN", "ADD", "ASK", "DISCONNECT", "HELLO", "HELLO_ACK", "RELAY"};

typedef struct {int prev; int next; int switchNumber;
    int IP_low; int IP_high;} helloPacket;

typedef struct {
    int srcIP_lo; int srcIP_hi;
    int destIP_lo; int destIP_hi;
    int actionType; int actionVal;
    int pkCount;
} addPacket;

typedef struct
{    // EMPTY
} helloACKPacket;

typedef struct {
    int srcIP;
    int destIP;
} askPacket;

typedef struct {
    int switchNum;
} disconnectPacket;

typedef struct {
    int srcIP;
    int destIP;
} relayPacket;

typedef union {
    helloPacket  hello;
    helloACKPacket acknowledge;
    askPacket askPckt;
    disconnectPacket disconnectPckt;
    relayPacket relayPckt;
    addPacket addPckt;
} MSG;

typedef struct { KIND kind; MSG msg; } FRAME;


// void FATAL (const char *fmt, ... )
// {
//     va_list  ap;
//     fflush (stdout);
//     va_start (ap, fmt);  vfprintf (stderr, fmt, ap);  va_end(ap);
//     fflush (NULL);
//     exit(1);
// }

MSG composeAsk(int srcIP, int destIP) {
    MSG msg;

    memset((char *)&msg, 0, sizeof(msg));

    msg.askPckt.srcIP = srcIP;
    msg.askPckt.destIP = destIP;

    return msg;
}

MSG composeAdd(int srcIP_lo, int srcIP_hi, int destIP_lo, int destIP_hi, 
    int actionType, int actionVal, int pkCount) {
        MSG msg;

        memset((char *)&msg, 0, sizeof(msg));

        msg.addPckt.srcIP_hi = srcIP_hi;
        msg.addPckt.srcIP_lo = srcIP_lo;
        msg.addPckt.actionType = actionType;
        msg.addPckt.actionVal = actionVal;
        msg.addPckt.destIP_lo = destIP_lo;
        msg.addPckt.destIP_hi = destIP_hi;
        msg.addPckt.pkCount = pkCount;

        return msg;
    }

MSG composeRelay(int srcIP, int destIP) {
    MSG msg;

    memset((char *)&msg, 0, sizeof(msg));

    msg.relayPckt.srcIP = srcIP;
    msg.relayPckt.destIP = destIP;

    return msg;
}

MSG composeHello(int prev, int next, int switchNumber,
    int IP_low, int IP_high) {
        MSG msg;

        memset((char *)&msg, 0, sizeof(msg));

        msg.hello.prev = prev;
        msg.hello.next = next;
        msg.hello.switchNumber = switchNumber;
        msg.hello.IP_low = IP_low;
        msg.hello.IP_high = IP_high;

        return msg;
    }

MSG composeHelloAcknowledge() {
    MSG msg;

    memset((char *)&msg, 0, sizeof(msg));

    return msg;
}

MSG composeDisconnect(int switchNum) {
    MSG msg;

    memset((char *)&msg, 0, sizeof(msg));

    msg.disconnectPckt.switchNum = switchNum;

    return msg;
}

void WARNING (const char *fmt, ... )
{
    va_list  ap;
    fflush (stdout);
    va_start (ap, fmt);  vfprintf (stderr, fmt, ap);  va_end(ap);
}

// ------------------------------
void printFrame (const char *prefix, FRAME *frame) {
    MSG  msg= frame->msg;
    
    printf ("%s [%s] ", prefix, KINDNAME[frame->kind]);
    switch (frame->kind) {
    case UNKNOWN:
        printf ("UNKNOWN");
        break;
    case ADD:
        printf ("ADD");
        break;
    case ASK:
        printf ("ASK");
        break;		   
    case DISCONNECT:
        printf("DISCONNECT"); 
        break;
    case HELLO:
        printf("HELLO");
        break;
    case HELLO_ACK:
        printf("HELLO_ACK");
        break;
    case RELAY:
        printf("RELAY");
        break;
      default:
          WARNING ("Unknown frame type (%d)\n", frame->kind);
	  break;
      }
      printf("\n");
}

void sendFrame (const char *prefix, int fd, KIND kind, MSG *msg)
{
    FRAME  frame;

    assert (fd >= 0);
    memset( (char *) &frame, 0, sizeof(frame) );
    frame.kind= kind;
    frame.msg=  *msg;
    write (fd, (char *) &frame, sizeof(frame));

    printFrame(prefix, &frame);
}

FRAME rcvFrame (int fd, int *len)
{
    FRAME  frame;

    assert (fd >= 0);
    memset( (char *) &frame, 0, sizeof(frame) );
    * len= read (fd, (char *) &frame, sizeof(frame));
    if (* len != sizeof(frame))
        WARNING ("Received frame has length= %d (expected= %d)\n",
		  len, sizeof(frame));
    return frame;		  
}

