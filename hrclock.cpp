#include "hrclock.h"

#include <QtGlobal>
#include <QtDebug>
#include <QApplication>
#include <QBoxLayout>
#include <QCoreApplication>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QTimer>

static const char *DEFAULT_FONT_FAMILY = "courier";
static const int   DEFAULT_FONT_SIZE   = 48;

static const int DEFAULT_PERIOD = 0;
static const char *NO_TIME = "XXX-XX:XX:XX.XXX";

HRClock::HRClock(const QString &fontFamily, const int fontSize, const int period)
    : font(fontFamily, fontSize), timer(this)
{
    connect(&this->timer, SIGNAL(timeout()), this, SLOT(timerExpired()));
    this->timer.start(period);
}

QString HRClock::getTime()
{
    const QDateTime &now = QDateTime::currentDateTime();
    return QString::number(now.date().dayOfYear()) + now.toString("-hh:mm:ss.zzz");
}

void HRClock::timerExpired()
{
    const QString &s = HRClock::getTime();

    this->updateText(s);
}


StdClock::StdClock(const QString &fontFamily, const int fontSize, const int period)
    : HRClock(fontFamily, fontSize, period)
{
    this->label = new QLabel(NO_TIME, this);
    this->label->setFont(this->font);

    QLayout *layout = new QVBoxLayout;
    layout->addWidget(this->label);
    this->setLayout(layout);
}

void StdClock::updateText(const QString &newText)
{
    this->label->setText(newText);
}


GLClock::GLClock(const QString &fontFamily, const int fontSize, const int period)
    : HRClock(fontFamily, fontSize, period)
{
}

void GLClock::updateText(const QString &newText)
{
}


int main(int argc, char *argv[])
{
    QString fontFamily = DEFAULT_FONT_FAMILY;
    int fontSize = DEFAULT_FONT_SIZE;
    int period = DEFAULT_PERIOD;
    bool useOpengl = false;

    QApplication app(argc, argv);

    // parse arguments
    QStringList args = QCoreApplication::arguments();
    args.removeFirst();
    while (!args.isEmpty()) {
        const QString &arg = args.takeFirst();

        if (arg == "-help") {
            qDebug() << "Usage:"
                     << QCoreApplication::applicationName()
                     << QString("[-period <msecs (default=%1)>]").arg(DEFAULT_PERIOD)
                     << "[-opengl]"
                     << QString("[-font <family (default=%1)>]").arg(DEFAULT_FONT_FAMILY)
                     << QString("[-fontsize <size (default=%1)>]").arg(DEFAULT_FONT_SIZE);
        } else if (arg == "-period") {
            if (args.isEmpty()) {
                qCritical() << "You must specify the period in milliseconds";
                app.quit();
            } else {
                bool ok;
                const QString &arg1 = args.takeFirst();
                period = arg1.toInt(&ok);
                if (!ok) {
                    qCritical() << arg1 << "is not a valid period";
                    app.quit();
                }
            }
        } else if (arg == "-opengl") {
            useOpengl = true;
        } else {
            qCritical() << "Unknown argument:" << arg;
            app.quit();
        }
    }

    HRClock *clock;
    if (useOpengl)
        clock = new GLClock(fontFamily, fontSize, period);
    else
        clock = new StdClock(fontFamily, fontSize, period);
    clock->show();

    return app.exec();
}
