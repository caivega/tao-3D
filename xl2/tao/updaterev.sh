#!/bin/sh
#
# Create or update file version.h with current Tao version,
# retrieved from Git
#
# Version string examples:
#
# "7ad3489"           // HEAD commit for non-tagged build
# "7ad3489-dirty"     // Same as previous, with unspecified modifications (work area not clean)
# "1.0 alpha"         // Tagged build (version is tag name)
# "1.0 alpha-dirty"   // Tagged build, work area not clean 
# "1.0 alpha-14-g8d1b01e"       // Tagged build plus 14 commits, leading to commit 8d1b01e
# "1.0 alpha-14-g8d1b01e-dirty" // Same as previous, work area not clean

gitrev() {
    if ! which git >/dev/null 2>&1 ||
       ! cd "$(dirname "$0")" ||
       ! git rev-parse --git-dir >/dev/null 2>&1
    then
        :
    else    
        v=$(git describe --tags --always --dirty=-dirty 2>/dev/null)
    fi
    [ -z "$v" ] && v="Unknown"
    echo "$v"
}

GITREV=$(gitrev)

# -n option to display version info without touching version.h
if [ "$1" = "-n" ] ; then
    echo $GITREV
    exit 0;
fi

echo "// Generated by $0 - do not edit!" > .version.new 
echo "#define GITREV \"$GITREV\"" >> .version.new
#diff .version.new version.h > .version.diff 2>&1
#if [ -s .version.diff ] ; then
    mv -f .version.new version.h
#fi
rm -f .version.new .version.diff