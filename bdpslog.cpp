#include "bdpslog.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

BDPSlog::BDPSlog()
{
    logs = NULL; notOpen = false;
    swaplogtype = 2;
}
BDPSlog::~ BDPSlog()
{
    closeLogFile();
}
void BDPSlog::closeLogFile()
{
    if (datafile) datafile->close();
    logs = NULL;  datafile = NULL;
}
void BDPSlog::setLogFilePath(QString path)
{
    setLogFilePath(path,true);
}
void BDPSlog::setLogFilePath(QString path, bool apd)
{
    if (path.compare(filepath)) {
        closeLogFile();
        filepath = path;
        append   = apd;
    }
}
void BDPSlog::setLogFilePath(QString path, bool apd, bool notOpen)
{
    setLogFilePath(path,apd);
    this->notOpen = notOpen;
}
void BDPSlog::setSwapLogType(bool byday)
{
    int pretype = swaplogtype;
    if (byday)  swaplogtype = 2;
    else        swaplogtype = 1;
    if (pretype!=swaplogtype)	closeLogFile();
}

void BDPSlog::log(QString msg)
{
    addLog(msg,true);
}
void BDPSlog::logt(QString msg)
{
    addLog(msg,false);
}
void BDPSlog::addLog(QString msg,bool notime)
{
    swapLogFile();
    if (logs) {
        (*logs) <<(notime?"":timestampstr)<<msg<<endl;   logs->flush();
    }
}

void BDPSlog::filepath2logfile()
{
   /*
    * %Y -> yyyy : year (4 digits) (1900-2099)
    * %y -> yy   : year (2 digits) (00-99)
    * %m -> mm   : month           (01-12)
    * %d -> dd   : day of month    (01-31)
    * %h -> hh   : hours           (00-23)
    * %M -> mm   : minutes         (00-59)
    * %S -> ss   : seconds         (00-59)
    */

    double  ep[6];

    logfile = filepath;
    timestamp.timeGet().time2epoch(ep);

    logfile.replace("%Y",QString("%1").arg((int)ep[0],4,10,QLatin1Char('0')));
    logfile.replace("%y",QString("%1").arg(((int)ep[0])%100,2,10,QLatin1Char('0')));
    logfile.replace("%m",QString("%1").arg((int)ep[1],2,10,QLatin1Char('0')));
    logfile.replace("%d",QString("%1").arg((int)ep[2],2,10,QLatin1Char('0')));

    logfile.replace("%h",QString("%1").arg((int)ep[3],2,10,QLatin1Char('0')));
    logfile.replace("%M",QString("%1").arg((int)ep[4],2,10,QLatin1Char('0')));
    logfile.replace("%S",QString("%1").arg((int)ep[5],2,10,QLatin1Char('0')));
}
void BDPSlog::openLogFile()
{
    double ep[6];

    if (this->notOpen) return;

    filepath2logfile();
    datafile = new QFile(logfile);
    if (datafile->open((append?(QFile::ReadWrite|QFile::Append):(QFile::WriteOnly))|QFile::Text)) {
        logs = new QTextStream(datafile);
        logs->setCodec("UTF-8");
        timestamp.timeGet().time2epoch(ep);
        monthlogfile = (int)ep[swaplogtype];
    }
}
void BDPSlog::swapLogFile()
{
    double ep[6];  bool pre_append=append;
    timestamp.timeGet().time2epoch(ep);
    if (monthlogfile!=(int)ep[swaplogtype] || !logs) {
        if (logs) append = false;
        closeLogFile();
        openLogFile();
        append = pre_append;
    }
    timestampstr = QString("%1-%2-%3 %4:%5:%6.%7 ")
            .arg((int)ep[0],4,10,QLatin1Char('0'))
            .arg((int)ep[1],2,10,QLatin1Char('0'))
            .arg((int)ep[2],2,10,QLatin1Char('0'))
            .arg((int)ep[3],2,10,QLatin1Char('0'))
            .arg((int)ep[4],2,10,QLatin1Char('0'))
            .arg((int)ep[5],2,10,QLatin1Char('0'))
            .arg((int)((ep[5]-(int)ep[5])*1000),3,10,QLatin1Char('0'));
}


/*
 *  时间类及操作
 */
BDPStime::BDPStime()
{
}
BDPStime::BDPStime(time_t time, double sec)
{
    init(time,sec);
}
void BDPStime::init(time_t time, double sec)
{
    this->time = time;
    this->sec  = sec;
}
void BDPStime::init(int yr,int mo,int da,int mi,double sec)
{
    time_t t; double s,ep[6]={0.0};
    ep[0] = yr; ep[1] = mo; ep[2] = da; ep[4] = mi; ep[5] = sec;
    if (epoch2timesec(ep,&t,&s)) {
        init(t,s);
    }
}

void BDPStime::str2epoch(QString &ymd,QString &hms,double *ep)
{
    QStringList ymdlist = ymd.split("/",QString::SkipEmptyParts);
    if (ymdlist.size()>2) {
        ep[0] = ymdlist[0].toDouble();
        ep[1] = ymdlist[1].toDouble();
        ep[2] = ymdlist[2].toDouble();
    } else {
        ep[0]=ep[1]=ep[2]=0.0;
    }

    QStringList hmslist = hms.split(":",QString::SkipEmptyParts);
    if (hmslist.size()>2) {
        ep[3] = hmslist[0].toDouble();
        ep[4] = hmslist[1].toDouble();
        ep[5] = hmslist[2].toDouble();
    } else {
        ep[3]=ep[4]=ep[5]=0.0;
    }
}
BDPStime BDPStime::timeGet()
{
    double ep[6]={0};
#ifdef WIN32
    SYSTEMTIME ts;

    /*GetSystemTime(&ts);  utc */
    GetLocalTime(&ts); /* local time */
    ep[0]=ts.wYear; ep[1]=ts.wMonth;  ep[2]=ts.wDay;
    ep[3]=ts.wHour; ep[4]=ts.wMinute; ep[5]=ts.wSecond+ts.wMilliseconds*1E-3;
#else
    struct timeval tv;
    struct tm *tt;

    /*if (!gettimeofday(&tv,NULL)&&(tt=gmtime(&tv.tv_sec))) {*/
    if (!gettimeofday(&tv,NULL)&&(tt=localtime(&tv.tv_sec))) { /* local time */
        ep[0]=tt->tm_year+1900; ep[1]=tt->tm_mon+1; ep[2]=tt->tm_mday;
        ep[3]=tt->tm_hour; ep[4]=tt->tm_min; ep[5]=tt->tm_sec+tv.tv_usec*1E-6;
    }
#endif
    return epoch2time(ep);
}
void BDPStime::sleepms(int ms)
{
#ifdef WIN32
    if (ms<5) Sleep(1); else Sleep(ms);
#else
    struct timespec ts;
    if (ms<=0) return;
    ts.tv_sec=(time_t)(ms/1000);
    ts.tv_nsec=(long)(ms%1000*1000000);
    nanosleep(&ts,NULL);
#endif
}

BDPStime BDPStime::epoch2time(const double *ep)
{
    time_t t; double secs;
    BDPStime tm;

    if (epoch2timesec(ep,&t,&secs)) {
        tm.init(t,secs);
    } else {
        tm.init(0,0.0);
    }

    return tm;
}
int BDPStime::epoch2timesec(const double *ep,time_t *t,double *s)
{
    const int doy[]={1,32,60,91,121,152,182,213,244,274,305,335};
    int days,secs,year=(int)ep[0],mon=(int)ep[1],day=(int)ep[2];

    if (year<1970||2099<year||mon<1||12<mon) return 0;

    /* leap year if year%4==0 in 1901-2099 */
    days=(year-1970)*365+(year-1969)/4+doy[mon-1]+day-2+(year%4==0&&mon>=3?1:0);
    secs=(int)floor(ep[5]);
    if (t) *t = (time_t)days*86400+(int)ep[3]*3600+(int)ep[4]*60+secs;
    if (s) *s = ep[5]-secs;
    return 1;
}
void BDPStime::timesec2epoch(time_t t,double s, double *ep)
{
    const int mday[]={ /* # of days in a month */
        31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,
        31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31
    };
    int days,secs,mon,day;

    /* leap year if year%4==0 in 1901-2099 */
    days=(int)(t/86400);
    secs=(int)(t-(time_t)days*86400);
    for (day=days%1461,mon=0;mon<48;mon++) {
        if (day>=mday[mon]) day-=mday[mon]; else break;
    }
    ep[0]=1970+days/1461*4+mon/12; ep[1]=mon%12+1; ep[2]=day+1;
    ep[3]=secs/3600; ep[4]=secs%3600/60; ep[5]=secs%60+s;
}
void BDPStime::time2epoch(double *ep)
{
    timesec2epoch(gettime(),getsec(),ep);
}
void BDPStime::time2epoch(BDPStime t, double *ep)
{
    timesec2epoch(t.gettime(),t.getsec(),ep);
}
double BDPStime::timeDiff(BDPStime t)
{
    return t.gettime()-this->time + t.getsec()-this->sec;
}
double BDPStime::timeDiff(BDPStime t1, BDPStime t2)
{
    return t1.gettime()-t2.gettime()+t1.getsec()-t2.getsec();
}
int BDPStime::isTimeout(int timeout)
{
    return (timeDiff(timeGet())-timeout)>0.0;
}

