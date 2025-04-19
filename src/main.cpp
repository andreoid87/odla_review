// SingleApplication is a Library that makes impossible to run more than one process
#include "singleapplication/SingleApplication"
#include "panel.h"
#include <QString>
#include <QDebug>

#ifdef Q_OS_MAC
#include "appnap.h"
QString currentOS = "MAC";
#elif defined(Q_OS_WIN)
QString currentOS = "WIN";
#elif defined(Q_OS_LINUX)
QString currentOS = "LINUX";
#endif


void getOptions(QApplication &app);
bool isDebug = false;

/*!
 * \brief main
 * \param argc
 * \param argv
 * \return
 */
int main(int argc, char *argv[])
{
    // Enable HI-DPI support
    SingleApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    SingleApplication app(argc, argv);

    // This program runs as a daemon only tray-icon
    app.setQuitOnLastWindowClosed(false);
    // Set app information
    app.setOrganizationName("Kemonia River s.r.l.");
    app.setOrganizationDomain("kemoniariver.com");
    app.setApplicationName("ODLA Keyboard");
    app.setApplicationVersion(VERSION);

    // Get command line options
    getOptions(app);
#ifdef Q_OS_MACX
    // App Nap subtracts resouces to a window-less program
    disableAppNap();
#endif

    // Init Database
    if(!Database::instance()->initDb())
        return 1;

    Panel::instance();

    int return_value = app.exec();
    return return_value;
}

/*!
 * \brief getOptions
 *
 * This function parse the input paramter
 * and return a map with couple key - value
 *
 * Possibile values:
 * lang: language for first running
 *
 * \retur CommandMap (QMap<QString, QString>)
 *
 */
void getOptions(QApplication &app)
{
    // command line options
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("ODLA keybord"));

    //Creating help option
    parser.addHelpOption();

    //Creating version option
    parser.addVersionOption();

    //Creating language option
    QCommandLineOption debugOption("d", "Run printing debug info");
    parser.addOption(debugOption);

    //Creating reset option
    QCommandLineOption resetOption("r", "Reset all settings");
    parser.addOption(resetOption);

    //Process all input arguments
    parser.process(app);

    // Reset all settings
    if(parser.isSet("r"))
        Database::instance()->forceReplace();

    // Set debug state
    isDebug = parser.isSet("d");
}
