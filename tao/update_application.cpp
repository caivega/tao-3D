// ****************************************************************************
//  update_application.cpp                                         Tao project
// ****************************************************************************
//
//   File Description:
//
//     Update Tao application.
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
//  (C) 2012 Baptiste Soulisse <baptiste.soulisse@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include "update_application.h"
#include "tree.h"
#include "tao_tree.h"
#include "main.h"
#include "context.h"
#include "version.h"
#include "application.h"
#include "tao_utf8.h"
#include <QDir>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTextStream>
#include <QDesktopServices>
#include <QMessageBox>
#include <sstream>

namespace Tao {


static void setBoxMinimumWidth(QMessageBox &box, int width)
// ----------------------------------------------------------------------------
//    Set minimum width of a message box
// ----------------------------------------------------------------------------
{
    QSpacerItem* spacer = new QSpacerItem(width, 0, QSizePolicy::Minimum,
                                          QSizePolicy::Expanding);
    QGridLayout* layout = dynamic_cast<QGridLayout*>(box.layout());
    if (!layout)
        return;
    layout->addItem(spacer, layout->rowCount(), 0, 1,
                    layout->columnCount());
}


static QPixmap loadIcon(QString path)
// ----------------------------------------------------------------------------
//    Load image file for use as a message box icon
// ----------------------------------------------------------------------------
{
    return QPixmap(path).scaled(64, 64,
                                Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation);
}


// ============================================================================
//
//                      Update Application
//
// ============================================================================

UpdateApplication::UpdateApplication()
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
  : state(Idle), file(NULL), progress(NULL),
    dialogTitle(QString(tr("Tao Presentations update"))),
    downloadIcon(loadIcon(":/images/download.png")),
    checkmarkIcon(loadIcon(":images/checkmark.png")),
    reply(NULL), manager(NULL), code(-1)
{
    // download.png from http://www.iconfinder.com/icondetails/2085/128
    // Author: Alexandre Moore
    // License: LGPL
    //
    // checkmark.png from http://www.iconfinder.com/icondetails/3207/128
    // Author: Everaldo Coelho
    // License: LGPL

#ifdef TAO_EDITION
   // Get current edition
   edition = TAO_EDITION;
#endif

#if defined(Q_OS_MACX)
   target = "MacOSX";
#elif defined(Q_OS_WIN)
   target = "Windows";
#else
   // Check if we are on Debian or Ubuntu distribution to get .deb package
   QString cmd("uname");
   QStringList args;
   args << "-a";
   Process cp(cmd, args);
   text errors, output;
   if(cp.done(&errors, &output))
   {
       // Check OS name
       if(output.find("Ubuntu") != output.npos ||
          output.find("Debian") != output.npos)
           target = "Linux .deb";
       else
           target = "Linux .tar.bz2";

       // Check architecture
       if(output.find("x86_64") != output.npos)
           target += " x86_64";
       else
           target += " x86";
   }
#endif

   // Get current version of Tao
   QString ver = GITREV;
   QRegExp rxp("([^-]*)");
   rxp.indexIn(ver);
   version = rxp.cap(1).toDouble();

   resetRequest();

   progress = new QProgressDialog(TaoApp->windowWidget());
   progress->setFixedSize(500, 100);
   connect(progress, SIGNAL(canceled()),
           this, SLOT(cancel()));
   progress->setWindowTitle(dialogTitle);

   IFTRACE(update)
       debug() << "Current version: edition='" << +edition
               << "' version=" << version
               << " target='" << +target << "'\n";
}


UpdateApplication::~UpdateApplication()
// ----------------------------------------------------------------------------
//    Destructor
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
        debug() << "Destructor\n";
    // FIXME crash on MacOSX
    // delete progress;
}


void UpdateApplication::resetRequest()
// ----------------------------------------------------------------------------
//    Initialize HTTP request
// ----------------------------------------------------------------------------
{
    request = QNetworkRequest();
    QString ua("Tao Presentations %1");
    request.setRawHeader("User-Agent", (ua.arg(version)).toAscii());
}


QString UpdateApplication::appName()
// ----------------------------------------------------------------------------
//    Return "Tao Presentations <EditionName>" or "Tao Presentations"
// ----------------------------------------------------------------------------
{
    QString ed;
    if (!edition.isEmpty())
        ed = " " + edition;
    return QString("Tao Presentations%1").arg(ed);
}


void UpdateApplication::check(bool show)
// ----------------------------------------------------------------------------
//    Check for update
// ----------------------------------------------------------------------------
{
    // This method starts the "check for update" algorithm which is
    // asynchronous.
    // If 'show' is true, a progress dialog may be shown during the check, and
    // a dialog is shown at the end to tell if update is available or not.
    // If 'show' is false, no interaction with the user occurs
    // until the algorithm detects that an update is available.

    this->show = show;
    if (state != Idle)
    {
        // Already running. Just show the progress dialog.
        progress->show();
        return;
    }

    if (edition.isEmpty())
    {
        if (show)
            showNoUpdateAvailable();
        return;
    }

    if (!manager)
        manager = new QNetworkAccessManager();

    progress->setLabelText(tr("Checking for update"));
    if (show)
        progress->show();

    // The URL where to get update information from. Redirects to a .ini file.
    QUrl url("http://www.taodyne.com/taopresentations/1.0/update");

    IFTRACE(update)
        debug() << "Downloading: '" << +url.toString() << "'\n";

    state = WaitingForUpdate;

    // Create and send HTTP request
    request.setUrl(url);
    reply = manager->get(request);
    connect(reply, SIGNAL(metaDataChanged()), this, SLOT(processReply()));
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(downloadProgress(qint64,qint64)));
}


void UpdateApplication::startDownload()
// ----------------------------------------------------------------------------
//    Start downloading the URL of the Tao file
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
        debug() << "Downloading: '" << +url.toString() << "'\n";

    Q_ASSERT(!reply);
    Q_ASSERT(!url.isEmpty());

    QString msg = tr("Downloading Tao Presentations %1...");
    progress->setLabelText(msg.arg(remoteVersion));
    progress->show();

    state = Downloading;
    downloadTime.start();

    request.setUrl(url);
    reply = manager->get(request);
    connect(reply, SIGNAL(metaDataChanged()), this, SLOT(processReply()));
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(downloadProgress(qint64,qint64)));
}


void UpdateApplication::readIniFile()
// ----------------------------------------------------------------------------
//    Get version, name and path of latest update from received .ini file
// ----------------------------------------------------------------------------
{
    Q_ASSERT(file);

    IFTRACE(update)
    {
        file->open(QIODevice::ReadOnly | QIODevice::Text);
        while (file->bytesAvailable())
            debug() << " " << +QString(file->readLine());
        file->close();
    }

    QSettings settings(file->fileName(), QSettings::IniFormat);
    remoteVersion = settings.value("version", "").toDouble();
    settings.beginGroup(target);
    url.setUrl(settings.value(edition, "").toString());
    settings.endGroup();

    IFTRACE(update)
        debug() << "Remote version is " << +remoteVersion
                << " download URL: '" << +url.toString() << "'\n";
}


void UpdateApplication::showNoUpdateAvailable()
// ----------------------------------------------------------------------------
//    Show message box
// ----------------------------------------------------------------------------
{
    QString msg = tr("%1 is up-to-date.").arg(appName());
    QString info = tr("<p>There is no new version available for download.<p>");
    QMessageBox box(TaoApp->windowWidget());
    box.setIconPixmap(checkmarkIcon);
    box.setWindowTitle(dialogTitle);
    box.setText(msg);
    box.setInformativeText(info);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}


void UpdateApplication::downloadProgress(qint64 bytesRcvd, qint64 bytesTotal)
// ----------------------------------------------------------------------------
//    Update progress bar
// ----------------------------------------------------------------------------
{
    if (state == Downloading)
    {
        // Calculate the download speed
        // REVISIT: compute speed for the last couple of seconds only?
        double speed = bytesRcvd * 1000.0 / (downloadTime.elapsed() + 1.0);
        QString unit;
        if (speed < 1024) {
            unit = tr("B/s");
        } else if (speed < 1024*1024) {
            speed /= 1024;
            unit = tr("KB/s");
        } else {
            speed /= 1024*1024;
            unit = tr("MB/s");
        }
        QString speedTxt;
        if (speed < 1000)
            speedTxt = QString(" (%1 %2)").arg(speed, 3, 'f', 1).arg(unit);

        // Update progress dialog
        QString msg = tr("Downloading Tao Presentations %1%2");
        progress->setLabelText(msg.arg(remoteVersion).arg(speedTxt));
    }
    progress->setMaximum(bytesTotal);
    progress->setValue(bytesRcvd);
}


void UpdateApplication::cancel()
// ----------------------------------------------------------------------------
//    Cancel check for update or download
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
        debug() << "Cancel\n";
    if (file)
    {
        file->remove();
        delete file;
        file = NULL;
    }
    state = Idle;
    if (reply)
    {
        reply->abort();
        delete reply;
        reply = NULL;
    }
    resetRequest();
}


void UpdateApplication::processReply()
// ----------------------------------------------------------------------------
//    Called as we receive new headers
// ----------------------------------------------------------------------------
{
    // Hande redirections
    code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (code >= 300 && code < 400)
    {
        // Get redirect URL
        url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (!url.isValid())
        {
            // Not yet received
            return;
        }

        // In case URL is relative
        url = reply->url().resolved(url);

        IFTRACE(update)
                debug() << "Redirected to: '" << +url.toString() <<  "'\n";

        request.setUrl(url);
        reply->deleteLater();
        reply = manager->get(request);
        connect(reply, SIGNAL(metaDataChanged()), this, SLOT(processReply()));
        connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                this, SLOT(downloadProgress(qint64,qint64)));
    }
    else if (code == 200 && state == Downloading)
    {
        if (!createFile())
        {
            progress->hide();
            cancel();
        }
    }
}


void UpdateApplication::downloadFinished()
// ----------------------------------------------------------------------------
//    Process downloaded data reply
// ----------------------------------------------------------------------------
{
    if (code != 200)
        return;

    bool updateAvailable = false;
    qint64 pid = 0;
    QString name;
    switch (state)
    {
    case WaitingForUpdate:

        // Save and process the update information we have just received

        progress->hide();

        Q_ASSERT(!file);
        pid = QCoreApplication::applicationPid();
        name = QString("tao_update.%1").arg(pid);
        file = new QFile(QDir::temp().filePath(name));
        if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            IFTRACE(update)
                debug() << "Failed to open: " << +file->fileName() << "\n";
            delete file;
            file = NULL;
            return;
        }
        file->write(reply->readAll());
        file->close();

        reply->deleteLater();
        reply = NULL;

        readIniFile();

        file->remove();
        delete file;
        file = NULL;

        // Propose to update if current version is older than the remote one
        updateAvailable = (version < remoteVersion) && !url.isEmpty();
        if (updateAvailable)
        {
            QString msg = tr("<h3>Update available</h3>");
            QString info = tr("<p>%1 version %2 is available."
                             " Do you want to download it now?</p>")
                            .arg(appName()).arg(remoteVersion);
            QMessageBox box(TaoApp->windowWidget());
            setBoxMinimumWidth(box, 400);
            box.setIconPixmap(downloadIcon);
            box.setWindowTitle(dialogTitle);
            box.setText(msg);
            box.setInformativeText(info);
            box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            if (box.exec() == QMessageBox::Yes)
            {
                startDownload();
                break;
            }
        }
        else
        {
            if (show)
                showNoUpdateAvailable();
        }

        progress->close();
        break;

    case Downloading:

        IFTRACE(update)
                debug() << "Download finished\n";

        progress->hide();

        Q_ASSERT(file);
        file->write(reply->readAll());
        file->close();

        QMessageBox::information(NULL, dialogTitle,
                                 tr("%1 downloaded sucessfully")
                                     .arg(file->fileName()));

        delete file;
        file = NULL;

        reply->deleteLater();
        reply = NULL;

        progress->close();
        state = Idle;
        break;

    default:
        // state == Idle if download is aborted by Application calling cancel
        break;
    }
}


bool UpdateApplication::createFile()
// ----------------------------------------------------------------------------
//    Create the update file
// ----------------------------------------------------------------------------
{
    Q_ASSERT(!file);

    // Keep the name of the remote file in the URL
    QString fileName = QFileInfo(url.path()).fileName();

    // Choose folder
    QString desktop = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
    QString folder = QFileDialog::getExistingDirectory(NULL,
                                                       tr("Select destination folder"),
                                                       desktop);

    if (!folder.isEmpty())
    {
        // Set complete filename
        QString completeFileName = folder + "/" + fileName;

        file = new QFile(completeFileName);

        if (file->exists())
        {
            int ret = QMessageBox::question(NULL, tr("File existing"),
                                            tr("There already exists a file called %1 in "
                                               "the specified directory. Overwrite it?").arg(fileName),
                                            QMessageBox::Yes|QMessageBox::No,
                                            QMessageBox::No);
            if (ret == QMessageBox::No)
            {
                delete file;
                file = NULL;
                return false;
            }
        }

        if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            IFTRACE(update)
                debug() << "Could not open file for writing: '"
                        << +completeFileName
                        << "' error: " << +file->errorString() << "\n";

            QMessageBox::information(NULL, "Update failed",
                                     tr("Unable to save to %1: %2.")
                                     .arg(completeFileName)
                                     .arg(file->errorString()));

            delete file;
            file = NULL;
            return false;
        }

        IFTRACE(update)
            debug() << "Save to: '" << +completeFileName << "'\n";

        return true;
    }

    return false;
}


std::ostream & UpdateApplication::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Update] ";
    return std::cerr;
}

}
