#ifndef FILE_MONITOR_H
#define FILE_MONITOR_H
// ****************************************************************************
//  file_monitor.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//     Be notified when files or directories are created, modified or deleted.
//     Monitoring occurs in its own thread, therefore it never blocks the main
//     thread.
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2012 Jerome Forissier <jerome@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include <QDateTime>
#include <QFileInfo>
#include <QMap>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QThread>
#include <iostream>



namespace Tao {

class FileMonitorThread;

class FileMonitor : public QObject
// ----------------------------------------------------------------------------
//    Monitor a set of file and directory paths for changes.
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    FileMonitor();
    virtual ~FileMonitor();

    void           removeAllPaths();

public slots:
    void           addPath(const QString &path);
    void           removePath(const QString &path);

signals:
    void           created(const QString &path);
    void           changed(const QString &path);
    void           deleted(const QString &path);



protected slots:
    void           onCreated(const QString &path);
    void           onChanged(const QString &path);
    void           onDeleted(const QString &path);

protected:
    std::ostream&  debug();

protected:
    QStringList    paths;

private:
    QSharedPointer<FileMonitorThread> thread;
};


class FileMonitorThread : public QThread
// ----------------------------------------------------------------------------
//    A singleton that implements path monitoring in its own thread.
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    static QSharedPointer<FileMonitorThread> instance();

public:
    FileMonitorThread();
    virtual ~FileMonitorThread();

public:
    void           addPath(const QString &path);
    void           removePath(const QString &path);

signals:
    void           created(const QString &path);
    void           changed(const QString &path);
    void           deleted(const QString &path);

protected:
    enum NotificationKind
    {
        None, Created, Changed, Deleted
    };

    class FileInfo : public QFileInfo
    {
    public:
        FileInfo(const QString &path)
            : QFileInfo(path),
              refs(1), ignore(false), lastNotification(None) {}
        FileInfo()
            : refs(1), ignore(false), lastNotification(None) {}

    public:
        QDateTime        cachedModified;
        int              refs;
        bool             ignore;
        NotificationKind lastNotification;
    };

protected:
    std::ostream&  debug();

protected slots:
    void           checkFiles();

protected:
    QMutex                   mutex;
    QMap<QString, FileInfo>  files;
    QTimer                   pollingTimer;
    int                      pollInterval;
    bool                     dontPollReadOnlyFiles;

private:
    static QWeakPointer<FileMonitorThread> inst;
};

}

#endif // file_monitor.h
