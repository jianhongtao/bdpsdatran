#include "bdpsini.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>

BDPSini::BDPSini()
{
    // ini file name
    QString file=QCoreApplication::applicationFilePath();
    QFileInfo fi(file);
    TaskName=fi.baseName();
//    IniFile=fi.baseName()+".ini";
//    LogFile=fi.baseName()+".log";
    IniFile="../cfg/bdpsdatran/"+fi.baseName()+".ini";
    LogFile="../log/bdpsdatran/"+fi.baseName()+".log";
}

bool BDPSini::iniUpdated(bool isFirst)
{
    QFileInfo fi(IniFile);

    if ((!isFirst)&&(fi.size()==IniFileInfo.size())
        &&(fi.lastModified()==IniFileInfo.lastModified()))
            return false;

    IniFileInfo.setFile(IniFile);

    return true;
}
QString BDPSini::get()
{
    return IniFile;
}
void BDPSini::set(QString inifile)
{
    IniFile = inifile;
}
QString BDPSini::logfile()
{
    return LogFile;
}
QString BDPSini::taskName()
{
    return TaskName;
}
QStringList BDPSini::search(QString fn)
{
    QStringList fl;
    if (fn.isEmpty()) return fl;

    QFileInfo fi(fn);

    QString pathname=fi.absolutePath();
    QString basename=fi.fileName();

    if (pathname.isEmpty()) {
        pathname=QCoreApplication::applicationDirPath();
        basename=fn;
    }

    if (basename.isEmpty()) return fl;

    QStringList filters;
    filters.append("*" + basename + "*");

    QDir dir(pathname);

    int m;

    //搜索当前目录符合条件的文件(返回路径及文件名)
    foreach (QString file, dir.entryList(filters, QDir::Files, QDir::Time)){
        if((m=file.indexOf(basename))>0) {
            fl += pathname + '/' + file.left(m) + basename;
            break;
        }
    }

    return fl;
}
