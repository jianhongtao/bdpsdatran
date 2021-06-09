#ifndef BDPSDATA_H
#define BDPSDATA_H

#include "rtklib.h"
#include "bdpslog.h"
#include <QList>
#include <QVector>

#define MAX_STREAM      200
#define MAX_STREAM_TASK 500
#define MAX_BUFFER_BYTE 8192

class StrEntity
{
public:

    int      type;                          // type of stream
    int      index;                         // index of stream

    QString  path;                          // path for stream
    QString  port;                          // port for stream only in NGCC data
    QString  mpnt;                          // mount point
    QString  mpntx;                         // mount point for input/output streams name
    QString  url;                           // path + mpnt
    bool     filter;                        // the filter: 0/1=open/close

    unsigned char buff[MAX_BUFFER_BYTE];    // stream data buffer
    int      nb;                            // datalength of buff
    int      ntrip2rtcm;                    // dataformat: 0/1/2/3=NONE/NTRIP2RTCM/RTCM2NTRIP/NGCCDATA
    int      opened;                        // opened: 0/1=FALSE/TRUE
    bool     status;                        // status: 0/1=FALSE/TRUE
    bool     sign;                          // sign: 0/1=FALSE/TRUE

    stream_t stream;                        // stream handle
    BDPStime timestamp;                     // timestamp for stream activity(received data)
    BDPStime sendstamp;                     // timestamp for stream senddata
    QString  filterType;                    // rtcm type for pass filter
    QStringList strListFilter;              // rtcm type StringList

    StrEntity()  {strinit(&stream);}
    ~StrEntity() {Close();}

    void Close() {strclose(&stream);nb=opened=0;}
    int  Open()  {Close();opened=stropen(&stream,type,STR_MODE_W|STR_MODE_R,qPrintable(url));return opened;}

};

typedef QList<StrEntity> StrEntityList;

class StrTask
{
public:
    int             inum;                   // number of stream incoming
    StrEntityList   istreams;               // list of stream incoming
    int             onum;                   // number of stream outing
    StrEntityList   ostreams;               // list of stream outing

    StrTask()       {init();}

    void init()     {inum=onum=0;istreams.clear();ostreams.clear();}
};

class BDPSdata
{
public:
    BDPSdata();

    void init();
    void run();
    void close();

    StrTask strTask[MAX_STREAM_TASK];

    BDPStime t0;
    void sleep(BDPStime tt){t0.sleepms(svrcycle-t0.timeDiff(tt)*1000);}

    QString Tn;                                 // base name of exe image
    int showlog;                                // show log on sreen
    int nstream;                                // number of stream groups
    int timeout;
    int svrcycle;
    int svrbuffsize;
    int rtcmtype;                               // used rtcm-data-type when NTRIP2RTCM=1,2
    int getSvrcycle()       {return svrcycle;}
    int getSvrbuffsize()    {return svrbuffsize;}
    int getNStream()        {return nstream;}

    int hasData(unsigned char *obuff,StrEntity *psi,int *type,int *n0);
};

#endif // BDPSDATA_H
