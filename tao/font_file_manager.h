#ifndef FONT_FILE_MANAGER_H
#define FONT_FILE_MANAGER_H
// ****************************************************************************
//  font_file_manager.h                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//     Tools to embbed fonts into Tao documents.
//
//
//
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"

#include <QFont>
#include <QString>
#include <QStringList>
#include <QMap>

TAO_BEGIN

struct FontFileManager
// ----------------------------------------------------------------------------
//   Manage fonts (import/export font files)
// ----------------------------------------------------------------------------
{
    FontFileManager();
    ~FontFileManager() {};

    void               AddFontFiles(const QFont &font);
    QList<int>         LoadEmbeddedFonts(const QString &docPath);

    static void        loadApplicationFonts();
    static void        UnloadEmbeddedFonts(const QList<int> &ids);
    static QString     FontPathFor(const QString &docPath);

    QStringList        fontFiles;
    QStringList        errors;

protected:

    QStringList        FilesForFontFamily(const QString &family);
    bool               IsLoadable(QString fileName);

#ifdef CONFIG_MINGW

private:
    QMap<QString, QString> fontFaceToFile;

#endif

private:
    static QStringList fontFileFilter;
};

TAO_END

#endif // FONT_FILE_MANAGER_H