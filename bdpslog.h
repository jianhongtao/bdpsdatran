#ifndef BDPSLOG_H
#define BDPSLOG_H

#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <time.h>
#include <math.h>


/*
 *  时间类及操作
 */
class BDPStimebase
{
public:
    time_t time;    /* time (s) expressed by standard time_t */
    double sec;     /* fraction of second under 1 s */

    BDPStimebase() {time=0;sec=0.0;}
};

class BDPStime : public BDPStimebase
{

public:
    BDPStime();
    BDPStime(time_t time,double sec);

public:
    void     init(time_t time=0,double sec=0.0);
    void     init(int yr,int mo,int da,int hr,double sec);
    time_t   gettime(){return time;}
    double   getsec() {return sec;}
    void     str2epoch(QString &ymd,QString &hms,double *ep);

    BDPStime timeGet();
    BDPStime epoch2time(const double *ep);
    void     sleepms(int ms);
    void     time2epoch(BDPStime t, double *ep);
    void     time2epoch(double *ep);
    double   timeDiff(BDPStime t1,BDPStime t2);
    double   timeDiff(BDPStime time);

    int      isTimeout(int timeout=1);

private:
    int  epoch2timesec(const double *ep,time_t *t=NULL, double *s=NULL);
    void timesec2epoch(time_t t,double s, double *ep);
};

class BDPSlog
{
public:
    BDPSlog();
    ~ BDPSlog();

    // 每次运行记录一个新的日志文件,跨月换文件
    void log(QString msg);
    void logt(QString msg);

    void setLogFilePath(QString path);
    void setLogFilePath(QString path, bool append);
    void setLogFilePath(QString path, bool append, bool notOpen);
    void setSwapLogType(bool byday);

    void closeLogFile();

private:
    void filepath2logfile();
    void openLogFile();
    void swapLogFile();
    void addLog(QString msg,bool notime);

private:
    QString filepath;
    QString logfile;
    QFile * datafile;
    QTextStream * logs;

    bool     notOpen;
    bool     append;
    BDPStime timestamp;
    int		 swaplogtype;	// 1/2=by month/day
    int      monthlogfile;
    QString  timestampstr;
};

// 日志对象
extern BDPSlog logbk;


#endif // BDPSLOG_H
