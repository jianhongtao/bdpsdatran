#ifndef BDPSINI_H
#define BDPSINI_H
#include <QFileInfo>
#include <QSettings>

class BDPSini
{
public:
    BDPSini();

    bool iniUpdated(bool isFirst);
    QString get();
    void set(QString inifile);
    QString logfile();
    QString taskName();

    QStringList search(QString fn);

private:
    QString   TaskName;// base name of exe image
    QString   IniFile; // initialization file
    QString   LogFile; // default logfile
    QFileInfo IniFileInfo;
};

// 初始化参数对象
// extern BDPSini bdpsini;


#endif // BDPSINI_H
