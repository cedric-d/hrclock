#include "hrclock.h"

#include <QtGlobal>
#include <QtDebug>
#include <QApplication>
#include <QBoxLayout>
#include <QCoreApplication>
#include <QDateTime>
#include <QGLFormat>
#include <QPainter>
#include <QPixmap>
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
    : HRClock(fontFamily, fontSize, period), label(NO_TIME)
{
    this->label.setFont(this->font);
    this->label.show();
}

void StdClock::updateText(const QString &newText)
{
    this->label.setText(newText);
}


GLClock::GLClock(const QString &fontFamily, const int fontSize, const int period, const bool verticalSync)
    : HRClock(fontFamily, fontSize, period), window(this)
{
    if (!this->window.isValid()) {
        qCritical() << "OpenGL window is not valid";
        QCoreApplication::quit();
    } else {
        QGLFormat format;
        format.setDoubleBuffer(true);
        format.setSwapInterval(verticalSync ? 1 : 0);
        this->window.setFormat(format);
        this->window.show();
    }
}

void GLClock::updateText(const QString &newText)
{
    this->text = newText;
    this->window.renderText(this->window.width(), this->window.height());
    this->window.updateGL();
}

GLClockWidget::GLClockWidget(GLClock *clock, QWidget *parent)
    : QGLWidget(parent), frameCount(0), clock(clock)
{
    this->lastFrameTime.start();

    QPixmap pixmap(1, 1);
    pixmap.fill(Qt::darkGray);
    this->textureId = this->bindTexture(pixmap);
}

void GLClockWidget::initializeGL()
{
    if (!this->doubleBuffer())
        qWarning() << "Double buffering is not enabled";

    qglClearColor(Qt::darkGray);
    qglColor(Qt::white);
    glEnable(GL_TEXTURE_2D);
}

void GLClockWidget::resizeGL(int width, int height)
{
    this->renderText(width, height);
}

void GLClockWidget::paintGL()
{
    if (this->lastFrameTime.elapsed() >= 1000) {
        qDebug() << this->frameCount << "FPS";
        this->frameCount = 0;
        this->lastFrameTime.restart();
    }
    this->frameCount++;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, this->textureId);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
    glEnd();
}

void GLClockWidget::renderText(int width, int height)
{
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::black);

    QPainter painter(&pixmap);
    painter.setFont(this->clock->font);
    painter.setPen(Qt::white);
    painter.drawText(pixmap.rect(), Qt::AlignVCenter | Qt::AlignLeft, this->clock->text);

    this->deleteTexture(this->textureId);
    this->textureId = this->bindTexture(pixmap);
}


int main(int argc, char *argv[])
{
    QString fontFamily = DEFAULT_FONT_FAMILY;
    int fontSize = DEFAULT_FONT_SIZE;
    int period = DEFAULT_PERIOD;
    bool useOpengl = false;
    bool verticalSync = false;

    QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

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
                     << "[-opengl]" << "[-vsync]"
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
        } else if (arg == "-vsync") {
            verticalSync = true;
        } else if (arg == "-font") {
            if (args.isEmpty()) {
                qCritical() << "You must specify the font family";
                app.quit();
            } else {
                fontFamily = args.takeFirst();
            }
        } else if (arg == "-fontsize") {
            if (args.isEmpty()) {
                qCritical() << "You must specified the font size";
                app.quit();
            } else {
                bool ok;
                const QString &arg1 = args.takeFirst();
                fontSize = arg1.toInt(&ok);
                if (!ok) {
                    qCritical() << arg1 << "is not a valid font size";
                    app.quit();
                }
            }
        } else {
            qCritical() << "Unknown argument:" << arg;
            app.quit();
        }
    }

    HRClock *clock;
    if (useOpengl)
        clock = new GLClock(fontFamily, fontSize, period, verticalSync);
    else
        clock = new StdClock(fontFamily, fontSize, period);

    return app.exec();
}
