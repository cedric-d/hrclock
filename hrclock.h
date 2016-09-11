#ifndef HRCLOCK_H
#define HRCLOCK_H

#include <QFont>
#include <QGLWidget>
#include <QLabel>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QWidget>

class HRClock : public QObject
{
    Q_OBJECT
public:
    HRClock(const QString &fontFamily, const int fontSize, const int period);

protected:
    static QString getTime();

protected slots:
    virtual void updateText(const QString &newText) = 0;

private slots:
    void timerExpired();

protected:
    QFont font;

private:
    QTimer timer;
};


class StdClock : public HRClock
{
    Q_OBJECT
public:
    StdClock(const QString &fontFamily, const int fontSize, const int period);

protected:
    void updateText(const QString &newText);

private:
    QLabel label;
};


class GLClock;

class GLClockWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLClockWidget(GLClock *clock, QWidget *parent = 0);
    void renderText(int width, int height);

private:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    GLuint textureId;
    quint32 frameCount;
    QTime lastFrameTime;
    GLClock *clock;
};

class GLClock : public HRClock
{
    Q_OBJECT
public:
    GLClock(const QString &fontFamily, const int fontSize, const int period, const bool verticalSync);

protected:
    void updateText(const QString &newText);

private:
    GLClockWidget window;
    QString text;

    friend class GLClockWidget;
};

#endif
