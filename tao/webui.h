#ifndef WEBUI_H
#define WEBUI_H
// ****************************************************************************
//  webui.h                                                        Tao project
// ****************************************************************************
//
//   File Description:
//
//    Interface with the web-based document editor.
//
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************

#include "process.h"
#include <QByteArray>
#include <QObject>
#include <QString>
#include <iostream>

namespace Tao {

class WebUI : public QObject
// ----------------------------------------------------------------------------
//   1 server instance (1 document) and possibly several web browser instances
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    WebUI(QObject *parent = 0);
    virtual ~WebUI();

public:
    void           launch(QString path);

public slots:
    void           stop();

protected:
    std::ostream & debug();
    QString        nodePath();
    QString        serverDir();

protected slots:
    void           serverStartError();
    void           readServerOut(QByteArray newOut);
    void           launchBrowser(unsigned port);

protected:
    Process        server;
    QString        docPath;
};

} // namespace Tao

#endif // WEBUI_H
