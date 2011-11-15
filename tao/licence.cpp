// ****************************************************************************
//  licence.cpp                                                     Tao project
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "licence.h"
#include "scanner.h"
#include "main.h"
#include "flight_recorder.h"
#include "tao_utf8.h"
#include "public_key.h"
#ifdef KEYGEN
#include "private_key.h"
#endif
#include "cryptopp/dsa.h"
#include "cryptopp/osrng.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdio>

#ifndef KEYGEN
#include <QMessageBox>
#endif

using namespace CryptoPP;

namespace Tao
{

Licences &Licences::LM()
// ----------------------------------------------------------------------------
//   Return a singleton for the Licences class
// ----------------------------------------------------------------------------
{
    static Licences lm;
    return lm;
}


Licences::Licences()
// ----------------------------------------------------------------------------
//   Initialize the licence manager by loading the licence file
// ----------------------------------------------------------------------------
    : licences()
{}


Licences::~Licences()
// ----------------------------------------------------------------------------
//   Destructor declared to be private
// ----------------------------------------------------------------------------
{}


void Licences::addLicenceFile(kstring licfname)
// ----------------------------------------------------------------------------
//   Add a licence file and the licences in it
// ----------------------------------------------------------------------------
//   The format of the licence file is something like:
//
//      name "Christophe de Dinechin"
//      address "777, Home Sweet Home Street, CA99701 Birds les Plages"
//      features "foo|bar|zoo.*|gloo[a-z]+"
//      expires 5 dec 1968
{
    RECORD(ALWAYS, "Adding licence file", licfname);

    // Licences we want to add here
    std::vector<Licence> additional;

    // Now analyze what we got from the input text
    XL::Syntax syntax;
    XL::Positions positions;
    XL::Errors errors;
    XL::Scanner scanner(licfname, syntax, positions, errors);
    Licence licence;
    bool had_features = false;
    int day = 0, month = 0, year = 0;
    enum
    {
        // REVISIT: We may want to add host, ip, MAC, display type, ...
        START, DIGEST, DONE, TAG,
        NAME, COMPANY, ADDRESS, EMAIL, FEATURES,
        EXPIRY_DAY, EXPIRY_MONTH, EXPIRY_YEAR
    } state = START;

    while (state != DONE)
    {
        XL::token_t tok = scanner.NextToken();
        text item;

        if (tok == XL::tokNEWLINE)
            continue;

        switch (state)
        {
        case DONE:
            break;              // Keep compiler happy

        case START:
            licence = Licence();
            had_features = false;
            day = 5; month = 12; year = 1968;
            licence.expiry = QDate(year, month, day);
            state = TAG;
            // Fall through on purpose

        case TAG:
            state = DONE;       // For error / EOF cases
            switch (tok)
            {
            case XL::tokEOF:
                break;
            case XL::tokNAME:
                item = scanner.TokenText();
                if (item == "name")
                    state = NAME;
                else if (item == "company")
                    state = COMPANY;
                else if (item == "address")
                    state = ADDRESS;
                else if (item == "email")
                    state = EMAIL;
                else if (item == "features")
                    state = FEATURES;
                else if (item == "expires")
                    state = EXPIRY_DAY;
                else if (item == "digest")
                    state = DIGEST;
                else
                    licenceError(licfname, tr("Invalid tag"));
                break;
            default:
                licenceError(licfname, tr("Invalid token"));
                break;
            } // switch(token for TAG state)
            break;

        case DIGEST:
            // This is normally the final state
            if (tok == XL::tokSTRING || tok == XL::tokQUOTE)
            {
                item = scanner.TextValue();
                if (verify(additional, item))
                {
                    licences.insert(licences.end(),
                                    additional.begin(), additional.end());
                    state = TAG;
                }
                else
                {
                    licenceError(licfname, tr("Digest verification failed"));
                    state = DONE;
                }
                additional.clear();                    
            }
            else
            {
                licenceError(licfname, tr("Invalid digest"));
                state = DONE;
            }
            break;

#define PARSE_IDENTITY(PTAG, PVAR)                                      \
        case PTAG:                                                      \
            if (tok == XL::tokSTRING || tok == XL::tokQUOTE)            \
            {                                                           \
                /* Check consistency of supplied information */         \
                item = scanner.TextValue();                             \
                state = TAG;                                            \
                if (PVAR == "")                                         \
                    PVAR = item;                                        \
                else if (item != PVAR)                                  \
                    licenceError(licfname, tr("Inconsistent %1").arg(#PVAR)); \
            }                                                           \
            else                                                        \
            {                                                           \
                licenceError(licfname, tr("Invalid %1").arg(#PVAR));    \
                state = DONE;                                           \
            }                                                           \
            break;

#define PARSE_ATTRIBUTE(PTAG, PVAR)                                     \
        case PTAG:                                                      \
            if (tok == XL::tokSTRING || tok == XL::tokQUOTE)            \
            {                                                           \
                /* Check consistency of supplied information */         \
                item = scanner.TextValue();                             \
                state = TAG;                                            \
                licence.PVAR = iten;                                    \
            }                                                           \
            else                                                        \
            {                                                           \
                licenceError(licfname, tr("Invalid %1").arg(#PVAR));    \
                state = DONE;                                           \
            }                                                           \
            break;

            PARSE_IDENTITY(NAME, name);
            PARSE_IDENTITY(COMPANY, company);
            PARSE_IDENTITY(ADDRESS, address);
            PARSE_IDENTITY(EMAIL, email);

        case FEATURES:
            if (tok == XL::tokSTRING || tok == XL::tokQUOTE)
            {
                item = scanner.TextValue();
                state = TAG;
                licence.features = QRegExp(+item);
                had_features = true;
            }
            else
            {
                licenceError(licfname, tr("Invalid features pattern"));
                state = DONE;
            }
            break;
            
        case EXPIRY_DAY:
            if (tok == XL::tokINTEGER)
            {
                day = scanner.IntegerValue();
                state = EXPIRY_MONTH;
                if (day < 1 || day > 31)
                {
                    licenceError(licfname, tr("Invalid day"));
                    state = DONE;
                }
            }
            else
             {
                licenceError(licfname, tr("Invalid expiry day"));
                state = DONE;
            }
            break;

        case EXPIRY_MONTH:
            if (tok == XL::tokNAME || tok == XL::tokSYMBOL)
            {
                item = scanner.TokenText();
                if (item == "/" || item == "-")
                {
                    // Skip
                }
                else
                {
                    // Accept named months
                    kstring months[] =
                    {
                        "jan", "feb", "mar", "apr", "may", "jun",
                        "jul", "aug", "sep", "oct", "nov", "dec"
                    };
                    for (uint m = 0; m < 12; m++)
                    {
                        if (item.substr(0, 3) == months[m])
                        {
                            month = m+1;
                            state = EXPIRY_YEAR;
                            break;
                        }
                    }
                    if (state != EXPIRY_YEAR)
                    {
                        licenceError(licfname, tr("Invalid month name"));
                        state = DONE;
                    }
                }
            }
            else if (tok == XL::tokINTEGER)
            {
                month = scanner.IntegerValue();
                state = EXPIRY_YEAR;
                if (month < 1 || month > 12)
                {
                    licenceError(licfname, tr("Invalid expiry month"));
                    state = DONE;
                }
            }
            break;

        case EXPIRY_YEAR:
            if (tok == XL::tokSYMBOL && (scanner.TokenText() == "/" ||
                                         scanner.TokenText() == "-"))
            {
                // Skip
            }
            else if (tok == XL::tokINTEGER)
            {
                year = scanner.IntegerValue();
                if (year < 2011 || year > 2099)
                {
                    licenceError(licfname, tr("Invalid year"));
                    state = DONE;
                }
                licence.expiry = QDate(year, month, day);
                state = TAG;
            }
            else
            {
                licenceError(licfname, tr("Invalid expiry year"));
                state = DONE;
            }
            break;
            
        } // switch(state)

        // Check if we have a complete licence, if so enter it
        if (had_features)
        {
            additional.push_back(licence);
            state = START;
        }
    } // while (state != DONE)

#ifdef KEYGEN
    if (additional.size())
    {
        text digested = sign(additional);
        FILE *file = fopen(licfname, "a");
        fprintf(file, "digest \"%s\"\n", digested.c_str());
        fclose(file);
    }
#endif // KEYGEN
}


text Licences::toText(std::vector<Licence> &licences)
// ----------------------------------------------------------------------------
//    Create a stream for the licence contents
// ----------------------------------------------------------------------------
{
    std::ostringstream os;
    os << "name \"" << name << "\"\n";
    if (company.length())
        os << "company \"" << company << "\"\n";
    if (address.length())
        os << "address \"" << address << "\"\n";
    if (email.length())
        os << "email \"" << email << "\"\n";

    // Loop on all data blocks
    uint i, max = licences.size();
    for (i = 0; i < max; i++)
    {
        Licence &l = licences[i];
        os << "expires "
           << l.expiry.day() << "/"
           << l.expiry.month() << "/"
           << l.expiry.year() << "\n";
        os << "features \"" << +l.features.pattern() << "\"\n";
    }
    return os.str();
}


#ifdef KEYGEN

text Licences::sign(std::vector<Licence> &licences)
// ----------------------------------------------------------------------------
//    Sign input data with private key
// ----------------------------------------------------------------------------
{
    // Create a string for the licence contents
    text lic = toText(licences);

    // Sign data
    byte key[] = TAO_PRIVATE_KEY;
    DSA::Signer signer;
    PrivateKey &privateKey = signer.AccessPrivateKey();
    privateKey.Load(StringSource(key, sizeof(key), true).Ref());

    AutoSeededRandomPool prng;
    byte * signature = new byte[signer.MaxSignatureLength()];
    byte * data = (byte *) lic.data();
    size_t data_length = lic.length();
    size_t length = signer.SignMessage( prng, data, data_length, signature );

    // Compute the result text
    char hex[] = "0123456789ABCDEF";
    text result;
    result.reserve(2*length);
    for (uint i = 0; i < length; i++)
    {
        byte m = (byte) signature[i];
        result.push_back(hex[m >> 4]);
        result.push_back(hex[m & 15]);
    }
    delete[] signature;
    return result;
}

#endif


bool Licences::verify(std::vector<Licence> &licences, text signature)
// ----------------------------------------------------------------------------
//    Verify if signature is valid for input data, using public key
// ----------------------------------------------------------------------------
{
    size_t length = signature.length();
    if (length % 2)
        return false;

    // Convert hex signature to binary
    text sig;
    size_t size = length/2;
    sig.reserve(size);
    std::stringstream sstr(signature);
    for (uint i = 0; i < size; i++)
    {
        char ch1, ch2;
        sstr >> ch1 >> ch2;
        int dig1, dig2;
        if (isdigit(ch1))
            dig1 = ch1 - '0';
        else if (ch1>='A' && ch1<='F')
            dig1 = ch1 - 'A' + 10;
        else if (ch1>='a' && ch1<='f')
            dig1 = ch1 - 'a' + 10;
        else
            return false;
        if (isdigit(ch2))
            dig2 = ch2 - '0';
        else if (ch2>='A' && ch2<='F')
            dig2 = ch2 - 'A' + 10;
        else if (ch2>='a' && ch2<='f')
            dig2 = ch2 - 'a' + 10;
        else
            return false;
        sig[i] = dig1*16 + dig2;
    }

    // Create a string for the licence contents
    text lic = toText(licences);

    // Load public key
    byte key[] = TAO_PUBLIC_KEY;
    DSA::Verifier verifier;
    PublicKey &publicKey = verifier.AccessPublicKey();
    publicKey.Load(StringSource(key, sizeof(key), true).Ref());

    // Verify signature
    byte * data = (byte *) lic.data();
    size_t data_length = lic.length();
    byte * bsig = (byte *) sig.data();
    bool result = verifier.VerifyMessage(data, data_length, bsig, size);

    return result;
}

int Licences::licenceRemainingDays(text feature)
// ----------------------------------------------------------------------------
//   Check if any of the licences grants us the given feature
// ----------------------------------------------------------------------------
{
    QDate today = QDate::currentDate();
    QString qfun = +feature;
    int result = 0;

    // Loop over all licences
    std::vector<Licence>::iterator l;
    for (l = licences.begin(); l != licences.end(); l++)
    {
        Licence &licence = *l;
        if (licence.features.exactMatch(qfun))
        {
            result = today.daysTo(licence.expiry);
            if (result >= 0)
                return result + 1; // If licence expires today, it's still OK
        }
    }

    // No licence matches, or they all expired.
    return result;
}


void Licences::licenceError(kstring file, QString reason)
// ----------------------------------------------------------------------------
//   We had a problem with the licences - Quick exit
// ----------------------------------------------------------------------------
{
    RECORD(ALWAYS, "Licence error", (+reason).c_str(), 0, file, 0);
    licences.clear();

#ifdef KEYGEN
    std::cerr << "Error reading licence file " << file
              << ": " << +reason << "\n";
#else
    QMessageBox oops;
    oops.setIcon(QMessageBox::Critical);
    oops.setWindowTitle(tr("Error reading licence files"));
    oops.setText(tr("There is a problem with licence file '%1'.\n"
                    "The following error was detected: %2\n"
                    "The program will now terminate. "
                    "You need to remove the offending licence file "
                    "before trying to run the application again. "
                    "Please contact Taodyne to obtain valid "
                    "licence files.")
                 .arg(file).arg(reason));
    oops.addButton(QMessageBox::Close);
    oops.exec();
#endif // KEYGEN
    exit(15);    
}


#ifndef KEYGEN
void Licences::WarnUnlicenced(text feature, int days, bool critical)
// ----------------------------------------------------------------------------
//   Remind user that the application is not licenced
// ----------------------------------------------------------------------------
{
    if (days <= 0)
    {
        QMessageBox oops;
        oops.setIcon(critical ? QMessageBox::Critical : QMessageBox::Warning);
        oops.setWindowTitle(tr("Not licenced"));
        if (days == 0)
            oops.setText(tr("You do not have a valid licence for %1. "
                            "Please contact Taodyne to obtain valid "
                            "licence files.").arg(+feature));
        else
            oops.setText(tr("You no longer have a valid licence for %1. "
                            "The licence you had expired %2 days ago. "
                            "Please contact Taodyne to obtain valid "
                            "licence files.").arg(+feature).arg(-days));
        oops.addButton(QMessageBox::Close);
        oops.exec();
    }
}
#endif // KEYGEN


}