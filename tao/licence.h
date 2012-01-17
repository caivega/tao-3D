#ifndef LICENCE_H
#define LICENCE_H
// ****************************************************************************
//  licence.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//     Licence check for Tao Presentation
//
//     Sources for information:
//     - http://www.sentientfood.com/display_story.php?articleid=3
//     - http://sigpipe.macromates.com/2004/09/05/using-openssl-for-license-keys
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "base.h"

#include <vector>
#include <QRegExp>
#include <QDateTime>
#include <QFileInfoList>

// Features id for CheckOnce
#define NO_FEATURE 0x00000000
#define WEB        0x00000001
#define GUI        0x00000002

namespace Tao
{

struct Licences : public QObject
// ----------------------------------------------------------------------------
//   Class checking the available licence for a given module ID or function
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    static void AddLicenceFile(kstring lfile)
    {
        return LM().addLicenceFile(lfile);
    }

    static void AddLicenceFiles(const QFileInfoList &files)
    {
        return LM().addLicenceFiles(files);
    }

#ifndef KEYGEN
    static bool Has(text feature)
    {
        return LM().licenceRemainingDays(feature) > 0;
    }

    static int RemainingDays(text feature)
    {
        return LM().licenceRemainingDays(feature);
    }

    static void WarnUnlicenced(text feature, int days, bool critical);

    static bool Check(text feature, bool critical = false)
    {
        int days = RemainingDays(feature);
        if (days <= 0)
            WarnUnlicenced(feature, days, critical);
        return days > 0;
    }

    static bool CheckOnce(quint64 feature, bool silent = true,
                          bool critical = false);
    static text hostID();

#endif

    static text Name()
    {
        return LM().name;
    }

    static text Company()
    {
        return LM().company;
    }

private:
    struct Licence
    {
        QRegExp         features;
        QDate           expiry;
    };
    std::vector<Licence>licences;
    text                name;
    text                company;
    text                address;
    text                email;
    struct LicenceFile
    {
        std::vector<Licence>licences;
        text                name;
        text                company;
        text                address;
        text                email;
        text                hostid;
    };

    // Used for CheckOnce : store if the feature has already been checked and
    // the result of the check.
    static quint64 alreadyChecked;
    static quint64 licencesOK;
    static std::map<quint64, text> featuresName;

private:
    Licences();
    ~Licences();
    static Licences &LM();

    void addLicenceFile(kstring licfname);
    void addLicenceFiles(const QFileInfoList &files);
    int  licenceRemainingDays(text feature);
    void licenceError(kstring file, QString reason);
    text toText(LicenceFile &lf);
#ifdef KEYGEN
    text sign(LicenceFile &lf);
#endif
    bool verify(LicenceFile &lf, text signature);
    std::ostream & debug();
};

}

#endif // LICENCE_H
