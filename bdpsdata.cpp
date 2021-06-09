#include <QDebug>
#include <QSettings>
#include "bdpsini.h"
#include "bdpslog.h"
#include "rtklib.h"
#include "bdpsdata.h"

#define MIN(x,y)    ((x)<=(y)?(x):(y))

BDPSlog logbk;

BDPSdata::BDPSdata()
{
    nstream     = 0;
    svrcycle    = 0;
    timeout     = 10;
    svrbuffsize = 32768;
    rtcmtype    = 4000;
    showlog     = 0;
    strinitcom();
}

void BDPSdata::init()
{
    BDPSini bdpsini;

    Tn = bdpsini.taskName();
    QSettings settings(bdpsini.get(),QSettings::IniFormat);
    QString logfile=settings.value("system/logfile",bdpsini.logfile()).toString();
    logbk.setLogFilePath(logfile);
    logbk.logt("\r\n"+Tn+QString("::init Loadini %1").arg(bdpsini.get()));
    logbk.log(Tn+QString("::logfile %1"     ).arg(logfile));

    // [system]
    svrcycle    =settings.value("system/svrcycle",0).toInt();
    logbk.log(Tn+QString("::svrcycle %1 ms"  ).arg(svrcycle));
    svrbuffsize =settings.value("system/svrbuffsize",32768).toInt();
    logbk.log(Tn+QString("::svrbuffsize %1 B").arg(svrbuffsize));
    timeout     =settings.value("system/timeout",10).toInt();
    logbk.log(Tn+QString("::timeout %1 sec"  ).arg(timeout));
    rtcmtype    =settings.value("system/rtcmtype",4000).toInt();
    logbk.log(Tn+QString("::rtcmtype %1"     ).arg(rtcmtype));
    showlog     =settings.value("system/showlog",1).toInt();
    logbk.log(Tn+QString("::showlog  %1"     ).arg(showlog));

    nstream     =settings.value("system/nstream",0).toInt();
    if(nstream>MAX_STREAM)nstream=MAX_STREAM;
    logbk.log(Tn+QString("::nstream %1").arg(nstream));

    // [stream***]
    for (int i=0;i<nstream;i++) {
        strTask[i].init();
        QString sect = QString("stream%1").arg(i+1,3,10,QLatin1Char('0'));
        // [stream***] in-stream
        int innum    = settings.value(sect+QString("/innum"),0).toInt();
        logbk.log(Tn+QString("::%1/innum=%2").arg(sect).arg(innum));
        for (int j=0;j<innum;j++) {
            StrEntity *istream = new StrEntity ;

            QString keynt = QString("%1/inote%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            QString inote = settings.value(keynt,"... ...").toString();
            logbk.log(Tn+QString("::%1=%2").arg(keynt).arg(inote));
            QString keyt  = QString("%1/itype%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            istream->type = settings.value(keyt,0).toInt();
            logbk.log(Tn+QString("::%1=%2").arg(keyt).arg(istream->type));
            QString keyp  = QString("%1/ipath%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            istream->path = settings.value(keyp,"").toString();
            logbk.log(Tn+QString("::%1=%2").arg(keyp).arg(istream->path));
            QString keynr = QString("%1/intrip2rtcm%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            istream->ntrip2rtcm = settings.value(keynr,0).toInt();
            logbk.log(Tn+QString("::%1=%2").arg(keynr).arg(istream->ntrip2rtcm));
            QString kmp = QString("%1/imp%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            istream->mpnt = settings.value(kmp,"").toString();
            kmp = QString("%1/impx%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            istream->mpntx = settings.value(kmp,istream->mpnt).toString();
            logbk.log(Tn+QString("::imp/%1 impx/%2...").arg(istream->mpnt).arg(istream->mpntx));
            QString kfilter = QString("%1/ifilter%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            istream->filter = settings.value(kfilter,0).toBool();
            logbk.log(Tn+QString("::%1=%2").arg(kfilter).arg(istream->filter));
            QString kfilterType = QString("%1/ifiltertype%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            istream->filterType = settings.value(kfilterType,"").toString();//
            logbk.log(Tn+QString("::%1=%2").arg(kfilterType).arg(istream->filterType));

            istream->strListFilter = istream->filterType.split(",",QString::SkipEmptyParts);
            QStringList strlist=istream->strListFilter;
            for(int i=0;i<istream->strListFilter.size();i++){  //00
                logbk.log(Tn+QString("::%1-%2=%3").arg(kfilterType).arg(i+1,3,10,QLatin1Char('0')).arg(strlist.at(i)));
            }


            if (!istream->type) {
                delete istream;
                continue;
            }
            else if (istream->type==STR_NTRIPCLI) {
                QString keym  = QString("%1/impnt%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
                int mpnt = settings.value(keym,0).toInt();
                logbk.log(Tn+QString("::%1=%2").arg(keym).arg(mpnt));
                for (int k=0;k<mpnt;k++) {
                    QString keymp = QString("%1/imp%2_%3").arg(sect)
                            .arg(j+1,3,10,QLatin1Char('0')).arg(k+1,3,10,QLatin1Char('0'));
                    istream->mpnt = settings.value(keymp,"").toString();
                    keymp = QString("%1/impx%2_%3").arg(sect)
                            .arg(j+1,3,10,QLatin1Char('0')).arg(k+1,3,10,QLatin1Char('0'));
                    istream->mpntx= settings.value(keymp,istream->mpnt).toString();
                    if (istream->mpnt.length()<1) istream->mpnt = istream->mpntx;
                    if (istream->mpntx.length()<1) continue;
                    StrEntity *istream_ = new StrEntity ;
                    istream_->type  = istream->type;
                    istream_->index = ++strTask[i].inum;
                    istream_->path  = istream->path;
                    istream_->mpnt  = istream->mpnt;
                    istream_->mpntx = istream->mpntx;
                    istream_->filter= istream->filter;
                    istream_->ntrip2rtcm = istream->ntrip2rtcm;
                    istream_->filterType = istream->filterType;
                    istream_->strListFilter = istream->strListFilter;
                    istream_->url   = istream_->path + "/" + istream_->mpnt;
                    logbk.log(Tn+QString("::openI(%1:%2,%3) mpx %4 n2r %5 type %6 stat %7")
                         .arg(i+1).arg(istream_->index).arg(istream_->url)
                         .arg(istream_->mpntx).arg(istream_->ntrip2rtcm)
                         .arg(istream_->type).arg(istream_->Open()));
                    istream_->timestamp = t0;
                    strTask[i].istreams.append(*istream_);
                }
                delete istream;
            }
            else if(istream->ntrip2rtcm==STR_NGCCDATA){
                QString keym  = QString("%1/impnt%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
                int mpnt = settings.value(keym,0).toInt();
                logbk.log(Tn+QString("::%1=%2").arg(keym).arg(mpnt));
                for (int k=0;k<mpnt;k++) {
                    QString keymp = QString("%1/imp%2_%3").arg(sect)
                            .arg(j+1,3,10,QLatin1Char('0')).arg(k+1,3,10,QLatin1Char('0'));
                    QStringList mpnts = settings.value(keymp,"").toString().split(",",QString::SkipEmptyParts);
                    istream->mpnt = mpnts[0];
                    istream->port = mpnts[1];
                    keymp = QString("%1/impx%2_%3").arg(sect)
                            .arg(j+1,3,10,QLatin1Char('0')).arg(k+1,3,10,QLatin1Char('0'));
                    QStringList mpntxs = settings.value(keymp,istream->mpnt).toString().split(",",QString::SkipEmptyParts);
                    istream->mpntx = mpntxs[0];//mpntx=mpnt
                    if (istream->mpnt.length()<1) istream->mpnt = istream->mpntx;
                    if (istream->mpntx.length()<1) continue;
                    StrEntity *istream_ = new StrEntity ;
                    istream_->type  = istream->type;
                    istream_->index = ++strTask[i].inum;
                    istream_->path  = istream->path;
                    istream_->mpnt  = istream->mpnt;
                    istream_->mpntx = istream->mpntx;
                    istream_->filter= istream->filter;
                    istream_->ntrip2rtcm = istream->ntrip2rtcm;
                    istream_->filterType = istream->filterType;
                    istream_->strListFilter = istream->strListFilter;
                    istream_->url   = istream->path + ":" + istream->port;
                    logbk.log(Tn+QString("::openI(%1:%2,%3) mpx %4 n2r %5 type %6 stat %7")
                         .arg(i+1).arg(istream_->index).arg(istream_->url).arg(istream_->mpntx)
                         .arg(istream_->ntrip2rtcm).arg(istream_->type)
                         .arg(istream_->Open()));
                    istream_->timestamp = t0;
                    strTask[i].istreams.append(*istream_);
                }
                delete istream;
            }else {
                istream->url   = istream->path;
                istream->index = ++strTask[i].inum;
                logbk.log(Tn+QString("::openI(%1:%2,%3) n2r %4 type %5 stat %6")
                     .arg(i+1).arg(istream->index).arg(istream->url)
                     .arg(istream->ntrip2rtcm).arg(istream->type)
                     .arg(istream->Open()));
                istream->timestamp = t0;
                strTask[i].istreams.append(*istream);
            }
        } // end for(j-in)
        logbk.log(Tn+QString("::%1/inum=%2").arg(sect).arg(strTask[i].inum));

        // [stream***] out-stream
        int outnum        = settings.value(sect+QString("/outnum"),0).toInt();
        logbk.log(Tn+QString("::%1/outnum=%2").arg(sect).arg(outnum));
        for (int j=0;j<outnum;j++) {
            StrEntity *ostream = new StrEntity ;

            QString keynt = QString("%1/onote%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            QString onote = settings.value(keynt,"... ...").toString();
            logbk.log(Tn+QString("::%1=%2").arg(keynt).arg(onote));
            QString keyt  = QString("%1/otype%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            ostream->type = settings.value(keyt,0).toInt();
            logbk.log(Tn+QString("::%1=%2").arg(keyt).arg(ostream->type));
            QString keyp  = QString("%1/opath%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
            ostream->path = settings.value(keyp,"").toString();
            logbk.log(Tn+QString("::%1=%2").arg(keyp).arg(ostream->path));
            if (!ostream->type) {
                delete ostream;
                continue;
            }
            else if (ostream->type==STR_NTRIPSVR) {
                QString keym  = QString("%1/ompnt%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
                int mpnt = settings.value(keym,0).toInt();
                logbk.log(Tn+QString("::%1=%2").arg(keym).arg(mpnt));
                for (int k=0;k<mpnt;k++) {
                    QString keymp = QString("%1/omp%2_%3").arg(sect)
                            .arg(j+1,3,10,QLatin1Char('0')).arg(k+1,3,10,QLatin1Char('0'));
                    ostream->mpnt = settings.value(keymp,"").toString();
                    keymp = QString("%1/ompx%2_%3").arg(sect)
                            .arg(j+1,3,10,QLatin1Char('0')).arg(k+1,3,10,QLatin1Char('0'));
                    ostream->mpntx= settings.value(keymp,ostream->mpnt).toString();
                    if (ostream->mpnt.length()<1) ostream->mpnt = ostream->mpntx;
                    if (ostream->mpntx.length()<1) continue;
                    StrEntity *ostream_ = new StrEntity ;
                    ostream_->type  = ostream->type;
                    ostream_->index = ++strTask[i].onum;
                    ostream_->path  = ostream->path;
                    ostream_->mpnt  = ostream->mpnt;
                    ostream_->mpntx = ostream->mpntx;
                    ostream_->status= 0;
                    ostream_->url   = ostream_->path + "/" + ostream_->mpnt;
                    logbk.log(Tn+QString("::openO(%1:%2,%3) mpx %4 type %5 stat %6")
                         .arg(i+1).arg(ostream_->index).arg(ostream_->url)
                         .arg(ostream_->mpntx).arg(ostream_->type)
                         .arg(ostream_->Open()));
                    strTask[i].ostreams.append(*ostream_);
                }
                delete ostream;
            }else if (ostream->type==STR_NCONVERTION) {
                QString keym  = QString("%1/ompnt%2").arg(sect).arg(j+1,3,10,QLatin1Char('0'));
                int mpnt = settings.value(keym,0).toInt();
                logbk.log(Tn+QString("::%1=%2").arg(keym).arg(mpnt));
                for (int k=0;k<mpnt;k++) {
                    QString keymp = QString("%1/omp%2_%3").arg(sect)
                            .arg(j+1,3,10,QLatin1Char('0')).arg(k+1,3,10,QLatin1Char('0'));
                    QStringList mpnts = settings.value(keymp,"").toString().split(",",QString::SkipEmptyParts);
                    ostream->mpnt = mpnts[0];
                    ostream->port = mpnts[1];
                    keymp = QString("%1/ompx%2_%3").arg(sect)
                            .arg(j+1,3,10,QLatin1Char('0')).arg(k+1,3,10,QLatin1Char('0'));
                    QStringList mpntxs= settings.value(keymp,ostream->mpnt).toString().split(",",QString::SkipEmptyParts);
                    ostream->mpntx = mpntxs[0];
                    if (ostream->mpnt.length()<1) ostream->mpnt = ostream->mpntx;
                    if (ostream->mpntx.length()<1) continue;
                    StrEntity *ostream_ = new StrEntity ;
                    ostream_->type  = ostream->type;
                    ostream_->index = ++strTask[i].onum;
                    ostream_->path  = ostream->path;
                    ostream_->mpnt  = ostream->mpnt;
                    ostream_->mpntx = ostream->mpntx;
                    ostream_->status= 0;
                    ostream_->url   = ostream->path + ":" + ostream->port;
                    logbk.log(Tn+QString("::openO(%1:%2,%3) mpx %4 type %5 stat %6")
                         .arg(i+1).arg(ostream_->index).arg(ostream_->url)
                         .arg(ostream_->mpntx).arg(ostream_->type)
                         .arg(ostream_->Open()));
                    strTask[i].ostreams.append(*ostream_);
                }
                delete ostream;
            }else {
                ostream->url   = ostream->path;
                ostream->index = ++strTask[i].onum;
                ostream->status= 0;
                logbk.log(Tn+QString("::openO(%1:%2,%3) type %4 stat %5")
                     .arg(i+1).arg(ostream->index)
                     .arg(ostream->url).arg(ostream->type)
                     .arg(ostream->Open()));
                strTask[i].ostreams.append(*ostream);
            }
        } // end for(j-out)
        logbk.log(Tn+QString("::%1/onum=%2").arg(sect).arg(strTask[i].onum));
    } // end for(i)
}

void BDPSdata::run()
{
    unsigned char buff[MAX_BUFFER_BYTE],mpntx[1024]; int n,n0,stat,type;
    QString conStr="connecting...";
    StrEntity *psi,*pso;
    /*printf("nstream:%d\r\n",nstream);*/
    for (int i=0;i<nstream;i++) {
        for (int j=0;j<strTask[i].inum&&j<strTask[i].istreams.size();j++) {
            psi= &strTask[i].istreams[j];
            n  = strread(&psi->stream,buff,4096);

            if(n) {
                if(showlog)printf("strread(%d,%d)=%4d(B) Left%4dB Tot %d\r\n",
                                   i+1,psi->index,n,psi->nb,n+psi->nb);
                memcpy(&psi->buff[psi->nb],buff,MIN(n,MAX_BUFFER_BYTE-psi->nb));
                psi->nb += MIN(n,MAX_BUFFER_BYTE-psi->nb);
                psi->timestamp = t0;
                if(psi->sign==0){
                    logbk.logt(Tn+QString(" %1 get data from sourcetable success !").arg(psi->mpntx));
                    psi->sign=1;
                }
            } else if (psi->type==STR_TCPSVR&&psi->opened) {// TCPSVR
                psi->timestamp = t0;
            }



            while ((n=hasData(buff,psi,&type,&n0))>0 || psi->sendstamp.timeDiff(t0)>1.0) {
                psi->sendstamp = t0;
                if(psi->type==STR_NTRIPCLI||psi->ntrip2rtcm==STR_NGCCDATA) { // [withMPi,VMP]
                    strcpy((char*)mpntx,qPrintable(psi->mpntx));
                }
                else if (type==rtcmtype) {    // RTCM2NTRIP
                    decode_rtcmtype(mpntx,buff,n,type); continue;
                }

                //Filter decode rtcm type
                if (psi->filter==1){
                    if(psi->strListFilter.length()>0){
                        if ( ! psi->strListFilter.contains(QString("%1").arg(type))) {
                        continue;
                        }
                    }
                }

                //转发广播星历需求-于滨，发送到rbpb
                char header[64];
                int hdlen = 0;
                hdlen =sprintf(header, "%s:%x\r\n", "ZDNE01", n+n0);

                for (int k=0;k<strTask[i].onum&&k<strTask[i].ostreams.size();k++) {
                    pso=&strTask[i].ostreams[k];
                    if (!pso->type) continue;
                    if (n<1) {
                        if (pso->type!=STR_TCPSVR)        continue;
                        strread(&pso->stream,buff,1024);  continue;
                    }
                    if ((pso->type==STR_NTRIPSVR)&&pso->mpntx.compare(QString((char*)mpntx)) )
                        continue;

                    if ((pso->type==STR_NCONVERTION)&&pso->mpntx.compare(QString((char*)mpntx)) )
                        continue;

                    stat = strwrite(&pso->stream,(unsigned char*)header,hdlen);
                    stat = strwrite(&pso->stream,buff,n+n0);

                    if(showlog)printf("  strwrite(%d:%d->%d)%s%4dB %4d url %s type %d stat %d msg:%s\r\n",
                               i+1,psi->index,pso->index,stat?"OK":"NOK",n,type,qPrintable(pso->url),
                               pso->type,pso->stream.state,pso->stream.msg);
                    if (!stat&&(pso->stream.state!=1)&&(pso->type!=STR_TCPSVR||((pso->type==STR_TCPSVR)&&(!pso->opened)))){
                        logbk.logt(Tn+QString("::reOpenO(%1:%2,%3) mpx %4 type %5 stat %6")
                             .arg(i+1).arg(pso->index).arg(pso->url).arg(pso->mpntx)
                             .arg(pso->type).arg(pso->Open()));
                    }


                    if(pso->sign==0){
                        if(stat){
                           logbk.logt(Tn+QString(" %1 put data to destination success !").arg(pso->url));
                           pso->status=1;
                        }else{
//                           logbk.logt(Tn+QString(" %1 put data to destination error !").arg(QString((char*)mpntx)));
                           pso->status=0;
                        }
                        pso->sign=1;
                    }else{
                        if(stat){
                            if(!pso->status){
                                logbk.logt(Tn+QString(" %1 put data to destination success !").arg(pso->url));
                                pso->status=1;
                            }
                        }else{
                            if(pso->status){
                                logbk.logt(Tn+QString(" %1 put data to destination error !").arg(pso->url));pso->status=0;
                            }
                        }
                    }

                 } // end for(k-out)
            } // end while()

            if (psi->type && ((psi->timestamp.timeDiff(t0)>timeout)||!psi->opened)){
                logbk.logt(Tn+QString("::reOpenI(%1:%2,%3) mpx %4 n2r %5 type %6 stat %7")
                     .arg(i+1).arg(psi->index).arg(psi->url).arg(psi->mpntx)
                     .arg(psi->ntrip2rtcm).arg(psi->type).arg(psi->Open()));
                psi->timestamp = t0;
            }
        } // end for(j-in)
     } // end for(i)
}

void BDPSdata::close()
{
    for (int i=0;i<nstream;i++) {
        for (int j=0;j<strTask[i].inum&&j<strTask[i].istreams.size();j++) {
            strTask[i].istreams[j].Close();
        }
        strTask[i].istreams.clear();
        strTask[i].inum = 0;

        for (int j=0;j<strTask[i].onum&&j<strTask[i].ostreams.size();j++) {
            strTask[i].ostreams[j].Close();
        }
        strTask[i].ostreams.clear();
        strTask[i].onum = 0;
    }
}

int BDPSdata::hasData(unsigned char *obuff,StrEntity *psi,int *type,int *n0)
{
    int n=0;  *n0=0;  *type=0;
    switch (psi->ntrip2rtcm) {


    case 1://ntrip2rtcm,add seperator(rtcmtype)
        *n0=pack_rtcm3(obuff, (unsigned char *)qPrintable(psi->mpntx) ,psi->mpntx.length(), rtcmtype);


    case 2://rtcm2ntrip[withVMP],cut seperator(rtcmtype)
        n  =unpack_rtcm3(psi->buff,&psi->nb,obuff+*n0,type);
        break;

    case 3:
    case 0:

    default :
        if (psi->nb>0) {
            memcpy(obuff,psi->buff,psi->nb);
            n = psi->nb; psi->nb = 0;
        }
    }

    return n;
}


