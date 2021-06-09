#include <QCoreApplication>
#include "bdpsdata.h"
#include "bdpslog.h"

// show message in message area ---------------------------------------------
#if 0
extern "C" {
extern int showmsg(char *format,...) {return 0;}
extern void settspan(gtime_t ts, gtime_t te){}
extern void settime(gtime_t time){}
}
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    BDPStime tt;
    BDPSdata datran;

    datran.t0 = tt.timeGet();
    datran.init();

    do {
        /*heartbeat();*/
        datran.t0 = tt.timeGet();
        datran.run();
        datran.sleep(tt.timeGet());
    } while(datran.getSvrcycle()>0);

    datran.close();
    return 0;
}
