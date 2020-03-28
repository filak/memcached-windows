#ifndef RESOURCE_H_INCLUDED
#define RESOURCE_H_INCLUDED

#define RUSAGE_SELF 0

struct rusage   {
    struct timeval ru_utime;    /* Total amount of user time used.  */
    struct timeval ru_stime;    /* Total amount of system time used.  */
};

int getrusage(int who, struct rusage *usage);

#endif // RESOURCE_H_INCLUDED

