// https://docs.fileformat.com/programming/h/ (Header Guards)
#ifndef CONST_H
#define CONST_H
#define MAX_NSW 7
#define MAXMSFD (MAX_NSW + 1)
#define MAXIP 1000
#define MAXPKFD 4
#define DROP 0
#define FORWARD 1
#define MAXBUF 80

const char actionNames[2][16] = { "DROP", "FORWARD" };


#endif
