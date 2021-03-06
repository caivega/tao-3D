// ****************************************************************************
//  dde_widget.h                                                   Tao project
// ****************************************************************************
//
//   File Description:
//
//   Implementation of the DDEWidget class for Windows.
//
//
//
//
//
//
//
//
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2011 Jérôme Forissier <jerome@taodyne.com>`
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "dde_widget.h"
#include "tao_utf8.h"
#include "application.h"
#include <QFileInfo>
#include <iostream>

namespace Tao {

DDEWidget::DDEWidget()
// ----------------------------------------------------------------------------
//   Create atoms
// ----------------------------------------------------------------------------
{
    QFileInfo fi(qApp->applicationFilePath());
    appAtomName = fi.baseName();
    appAtom = ::GlobalAddAtomW((const wchar_t*)appAtomName.utf16());
    systemTopicAtomName = "system";
    systemTopicAtom = ::GlobalAddAtomW((const wchar_t*)
                                       systemTopicAtomName.utf16());
}


DDEWidget::~DDEWidget()
// ----------------------------------------------------------------------------
//   Delete atoms
// ----------------------------------------------------------------------------
{
    ::GlobalDeleteAtom(appAtom);
    ::GlobalDeleteAtom(systemTopicAtom);
    appAtom = systemTopicAtom = 0;
}


#if QT_VERSION < 0x050000
bool DDEWidget::winEvent(MSG *message, long *result)
// ----------------------------------------------------------------------------
//   Process a Windows DDE message
// ----------------------------------------------------------------------------
{
    switch(message->message)
    {
    case WM_DDE_INITIATE:
        return ddeInitiate(message, result);
    case WM_DDE_EXECUTE:
        return ddeExecute(message, result);
    case WM_DDE_TERMINATE:
        return ddeTerminate(message, result);
    }

    return QWidget::winEvent(message, result);
}


#else
bool DDEWidget::nativeEvent(const QByteArray &event, void *msg, long *result)
// ----------------------------------------------------------------------------
//   Process a Windows DDE message
// ----------------------------------------------------------------------------
{
    if (event == "windows_generic_MSG")
    {
        MSG *message = (MSG *) msg;

        switch(message->message)
        {
        case WM_DDE_INITIATE:
            return ddeInitiate(message, result);
        case WM_DDE_EXECUTE:
            return ddeExecute(message, result);
        case WM_DDE_TERMINATE:
            return ddeTerminate(message, result);
        }
    }
    return QWidget::nativeEvent(event, msg, result);
}
#endif


bool DDEWidget::ddeInitiate(MSG* message, long* result)
// ----------------------------------------------------------------------------
//   Process a Windows WM_DDE_INITIATE message
// ----------------------------------------------------------------------------
{
    if ((0 != LOWORD(message->lParam)) &&
        (0 != HIWORD(message->lParam)) &&
        (LOWORD(message->lParam) == appAtom) &&
        (HIWORD(message->lParam) == systemTopicAtom))
    {
        // make duplicates of the incoming atoms (really adding a reference)
        wchar_t atomName[_MAX_PATH];
        bool ok;
        ok = (::GlobalGetAtomNameW(appAtom, atomName, _MAX_PATH-1) != 0);
        Q_ASSERT(ok);
        if (ok)
            ok = (::GlobalAddAtomW(atomName) == appAtom);
        Q_ASSERT(ok);
        if (ok)
            ok = (::GlobalGetAtomNameW(systemTopicAtom, atomName, _MAX_PATH-1)
                  != 0);
        Q_ASSERT(ok);
        if (ok)
            ok = (::GlobalAddAtomW(atomName) == systemTopicAtom);
        Q_ASSERT(ok);

        if (!ok)
            return false;

        // send the WM_DDE_ACK (caller will delete duplicate atoms)
        ::SendMessage((HWND)message->wParam, WM_DDE_ACK, (WPARAM)winId(),
                      MAKELPARAM(appAtom, systemTopicAtom));
    }
    *result = 0;
    return true;
}


bool DDEWidget::ddeExecute(MSG* message, long* result)
// ----------------------------------------------------------------------------
//   Process a Windows WM_DDE_EXECUTE message
// ----------------------------------------------------------------------------
{
    // unpack the DDE message
    UINT_PTR unused;
    HGLOBAL hData;
    bool ok;
    //IA64: Assume DDE LPARAMs are still 32-bit
    ok = (::UnpackDDElParam(WM_DDE_EXECUTE, message->lParam, &unused,
                            (UINT_PTR*)&hData) != 0);
    Q_ASSERT(ok);
    if (!ok)
        return false;

    QString command = QString::fromWCharArray((LPCWSTR)::GlobalLock(hData));
    ::GlobalUnlock(hData);

    // acknowledge now - before attempting to execute
    ::PostMessage((HWND)message->wParam, WM_DDE_ACK, (WPARAM)winId(),
                  //IA64: Assume DDE LPARAMs are still 32-bit
                  ReuseDDElParam(message->lParam, WM_DDE_EXECUTE, WM_DDE_ACK,
                                 (UINT)0x8000, (UINT_PTR)hData));

    // don't execute the command when the window is disabled
    if (!isEnabled())
    {
        *result = 0;
        return true;
    }

    QRegExp regCommand("^\\[(\\w+)\\((.*)\\)\\]$");
    if(regCommand.exactMatch(command))
    {
        executeDdeCommand(regCommand.cap(1), regCommand.cap(2));
    }

    *result = 0;
    return true;
}

bool DDEWidget::ddeTerminate(MSG* message, long* result)
// ----------------------------------------------------------------------------
//   Process a Windows WM_DDE_TERMINATE message
// ----------------------------------------------------------------------------
{
    // The client or server application should respond by posting a
    // WM_DDE_TERMINATE message.
    ::PostMessageW((HWND)message->wParam, WM_DDE_TERMINATE, (WPARAM)winId(),
                   message->lParam);
    *result = 0;
    return true;
}


void DDEWidget::executeDdeCommand(const QString& command, const QString& params)
// ----------------------------------------------------------------------------
//   Execute DDE command
// ----------------------------------------------------------------------------
{
    // Currently, only "open file/URI" is supported, but we could also
    // implement "print" or "new file"
    QRegExp regCommand("^\"(.*)\"$");
    bool singleCommand = regCommand.exactMatch(params);
    if ((!command.compare("open", Qt::CaseInsensitive)) && singleCommand)
    {
        ddeOpenFile(regCommand.cap(1));
    }
    else
    {
        std::cerr << "Window::executeDdeCommand: unsupported command or "
                << "multiple parameters: " << +command << " " << +params
                << "\n";
    }

}


void DDEWidget::ddeOpenFile(const QString& filePath)
// ----------------------------------------------------------------------------
//   Open a file or URI
// ----------------------------------------------------------------------------
{
    TaoApp->loadUri(filePath);
}

}
