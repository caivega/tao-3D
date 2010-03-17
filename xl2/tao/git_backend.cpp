// ****************************************************************************
//  git_backend.cpp                                                Tao project
// ****************************************************************************
//
//   File Description:
//
//     Storing Tao documents in a Git repository.
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "git_backend.h"
#include "renderer.h"
#include <QDir>
#include <QtGlobal>
#include <iostream>

TAO_BEGIN


static inline QString operator +(text s)
// ----------------------------------------------------------------------------
//   An implicit conversion that should be there
// ----------------------------------------------------------------------------
{
    return QString::fromStdString(s);
}


QString GitRepository::command()
// ----------------------------------------------------------------------------
//   Return the command for 'git'
// ----------------------------------------------------------------------------
{
    return "git";
}


text GitRepository::styleSheet()
// ----------------------------------------------------------------------------
//   Return the XL style sheet to be used for rendering files in git
// ----------------------------------------------------------------------------
{
    return "git.stylesheet";
}


bool GitRepository::valid()
// ----------------------------------------------------------------------------
//   Check if the repository exists and is a valid git repository
// ----------------------------------------------------------------------------
//   We simply use 'git branch' and check for errors
//   First, I tried with 'git status', but the return code is 1 after init
{
    if (!QDir(path).exists())
        return false;
    Process cmd(command(), QStringList("branch"), path);
    return cmd.done();
}


bool GitRepository::initialize()
// ----------------------------------------------------------------------------
//    Initialize a git repository if we need to
// ----------------------------------------------------------------------------
{
    if (!QDir(path).mkpath("."))
        return false;
    Process cmd(command(), QStringList("init"), path);
    return cmd.done();
}


bool GitRepository::checkout(text branch)
// ----------------------------------------------------------------------------
//    Checkout a given branch
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("checkout") << +branch, path);
    return cmd.done();
}


bool GitRepository::branch(text name)
// ----------------------------------------------------------------------------
//    Create a new branch
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("branch") << +name, path);
    return cmd.done();
}


bool GitRepository::add(text name)
// ----------------------------------------------------------------------------
//   Add a new file to the repository
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("add") << +name, path);
    return cmd.done();
}


bool GitRepository::change(text name)
// ----------------------------------------------------------------------------
//   Signal that a file in the repository changed
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("add") << +name, path);
    return cmd.done();
}


bool GitRepository::rename(text from, text to)
// ----------------------------------------------------------------------------
//   Rename a file in the repository
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("mv") << +from << +to, path);
    return cmd.done();
}


bool GitRepository::commit(text message)
// ----------------------------------------------------------------------------
//   Rename a file in the repository
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("commit") << "-m" << +message, path);
    return cmd.done();
}

TAO_END
