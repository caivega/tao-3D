// ****************************************************************************
//  chooser.cpp                                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//    The chooser is a way to select a command or item in a large set.
//    It works by showing a list, and then allowing you to restrict that
//    list by typing keystrokes.
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "chooser.h"
#include "widget.h"
#include "context.h"
#include "runtime.h"
#include "utf8.h"
#include "tao_utf8.h"
#include "gl_keepers.h"
#include <GL/glew.h>
#include <wctype.h>
#include <stdio.h>
#include <QFont>
#include <QFontMetricsF>


TAO_BEGIN

using namespace XL;


Chooser::Chooser(text name, Widget *w)
// ----------------------------------------------------------------------------
//   Chooser constructor
// ----------------------------------------------------------------------------
    : Activity(name, w), keystrokes(""), item(0), firstItem(0), selected(NULL)
{}


Chooser::~Chooser()
// ----------------------------------------------------------------------------
//    Chooser destructor
// ----------------------------------------------------------------------------
{}


struct utf8position
// ----------------------------------------------------------------------------
//   Depending on what we do, we may need byte or char index
// ----------------------------------------------------------------------------
{
    int byteOffset;
    int charCount;
    utf8position(int b, int c): byteOffset(b), charCount(c) {}
};


static utf8position KeystrokesFind(text where, text what)
// ----------------------------------------------------------------------------
//   Find 'what' in 'where', ignoring capitalization and stuff...
// ----------------------------------------------------------------------------
{
    uint i, maxi = where.length();
    uint j, maxj = what.length();
    kstring di = where.data();
    kstring dj = what.data();
    int c = 0;

    for (i = 0; i + maxj <= maxi; i = Utf8Next(di, i))
    {
        bool found = true;
        for (j = 0; found && j < maxj; j = Utf8Next(dj, j))
        {
            wchar_t ref, test;
            mbtowc(&ref, di + i + j, maxi - i - j);
            mbtowc(&test, dj + j, maxj - j);
            found = (towlower(ref) == towlower(test) ||
                     ((iswspace(test) || test == '_') &&
                      (iswspace(ref) || ref == '_')));
        }
        if (found && j >= maxj)
            return utf8position(i, c);
        c++;
    }
    return utf8position(-1, -1);
}


Activity *Chooser::Display(void)
// ----------------------------------------------------------------------------
//    Display the chooser
// ----------------------------------------------------------------------------
{
    // Symbols where we execute stuff
    Symbols *symbols = widget->currentSymbols();

    // Select the chooser font
    XLCall("chooser_title_font") (symbols);
    QFont titleFont = widget->currentFont();
    QFontMetricsF titleFM(titleFont);
    coord mtlh = titleFM.height();
    coord mtla = titleFM.ascent();

    // Dimensions data for the chooser
    coord ww = widget->width();
    coord wh = widget->height();
    coord mw = ww / 2;
    coord mh = mtlh * 3 / 2;

    int i, found = 0, displayed = 0;

    // We start with nothing selected
    selected = NULL;

    // Select the chooser item font
    XLCall("chooser_item_font")(symbols);
    QFont itemFont = widget->currentFont();
    QFontMetricsF itemFM(itemFont);
    coord milh = itemFM.height();
    coord mild = itemFM.descent();
    coord maxmh = wh - 3 * milh;

    // List the elements in the chooser that match the keystrokes
    ChooserItems remaining;
    ChooserItems::iterator it;
    for (it = items.begin(); it != items.end(); it++)
    {
        text caption = (*it).caption;
        int pos = KeystrokesFind (caption, keystrokes).byteOffset;
        if (pos >= 0)
        {
            // Add items found to the list
            remaining.push_back(*it);
            found++;

            // Compute the width of the chooser
            coord mw1 = itemFM.width(+caption);
            if (mw < mw1)
                mw = mw1;

            // Compute the height of the chooser
            if (mh <= maxmh)
            {
                mh += milh;
                displayed++;
            }
        }
    }
    coord mx = (ww - mw) / 2;
    coord my = (wh - mh) / 2;
    coord ix = mx;
    coord iy = my + mh - mtla;

    // Initialize an "overlay" projection mode
    GLAllStateKeeper saveGL(GL_ALL_ATTRIB_BITS, true, true, false);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, ww, 0, wh);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw the chooser box
    widget->drawSelection(NULL, Box3(mx, my, 0, mw, mh, 0), "chooser_box");

    // Show what the user has typed
    if (keystrokes.length())
    {
        XLCall call(found ? "draw_chooser_choice" : "draw_chooser_error");
        call, keystrokes, ix, iy;
        widget->drawCall(NULL, call);
    }
    else
    {
        XLCall call("draw_chooser_title");
        call, name, ix, iy;
        widget->drawCall(NULL, call);
    }
    iy -= mtlh;

    // Adjust the chooser selection
    if (item < 0)
        item = found - 1;
    else if (item >= found)
        item = 0;
    if (displayed == found)
        firstItem = 0;
    if (firstItem > item)
        firstItem = item;
    else if (firstItem + displayed <= item)
        firstItem = item - displayed + 1;

    // List all the chooser items
    for (i = 0; i < displayed; i++)
    {
        text caption = remaining[i + firstItem].caption;

        if (found > displayed)
        {
            // Indicate when there's more above in the list...
            if (i == 0 && firstItem > 0)
                widget->drawSelection(NULL, Box3(ix+mw/2, iy+mild, 0,
                                                 milh, milh, 0),
                                      "chooser_more_above");

            // Indicate when there's more below in the list...
            if (i == displayed-1 && firstItem + i != found - 1)
                widget->drawSelection(NULL, Box3(ix+mw/2, iy+mild, 0,
                                                 milh, milh, 0),
                                      "chooser_more_below");
        }

        // Draw the selection box if we are on selected line
        if (i + firstItem == item)
        {
            widget->drawSelection(NULL,  Box3(mx, iy - mild, 0, mw, milh, 0),
                                  "chooser_selection");
            selected = remaining[i + firstItem].function;
        }

        // Render command selection
        if (keystrokes.length())
        {
            utf8position pos = KeystrokesFind(caption, keystrokes);
            text ca1 = caption.substr(0, pos.byteOffset);
            text ca2 = caption.substr(pos.byteOffset, keystrokes.length());
            text ca3 = caption.substr(pos.byteOffset + keystrokes.length());
            XLCall call("draw_chooser_match");
            call, ca1, ca2, ca3, ix, iy;
            widget->drawCall(NULL, call);
        }
        else
        {
            XLCall call("draw_chooser_item");
            call, caption, ix, iy;
            widget->drawCall(NULL, call);
        }

        // Move to the next line
        iy -= milh;
    }

    // Display next activity
    return next;
}


Activity *Chooser::Idle(void)
// ----------------------------------------------------------------------------
//   Process remaining idle activities
// ----------------------------------------------------------------------------
{
    return next;
}


Activity *Chooser::Key(text key)
// ----------------------------------------------------------------------------
//    Pressing a key while the chooser is active
// ----------------------------------------------------------------------------
{
    uint position = keystrokes.length();
    if (key == "Return" || key == "Enter")
    {
        if (selected)
        {
            xl_evaluate(selected);
            delete this;
        }
    }
    else if (key == "Escape")
    {
        delete this;
    }
    else if (key == "Down" || key == "Right")
    {
        item++;
    }
    else if (key == "Up" || key == "Left")
    {
        item--;
    }
    else if (key == "PageUp")
    {
        item -= 8;
    }
    else if (key == "PageDown")
    {
        item += 8;
    }
    else if (key == "Backspace")
    {
        if (position > 0)
        {
            uint prev = Utf8Previous(keystrokes, position);
            keystrokes.erase(prev, position - prev);
        }
    }
    else if (Utf8Length(key) == 1)
    {
        keystrokes += key;
    }

    // Force an immediate widget refresh
    widget->refresh(0);

    // Notify caller that we intercept all keystrokes
    return NULL;
}


Activity *Chooser::Click(uint button, uint count, int x, int y)
// ----------------------------------------------------------------------------
//   When we click in a chooser, it clicks...
// ----------------------------------------------------------------------------
{
    (void) button; (void) count; (void) x; (void) y;
    Activity *n = next;
    delete this;
    return n;
}


Activity *Chooser::MouseMove(int x, int y, bool active)
// ----------------------------------------------------------------------------
//   Mouse movements...
// ----------------------------------------------------------------------------
{
    (void) x; (void) y; (void) active;
    return next;
}


void Chooser::AddItem(text caption, Tree *function)
// ----------------------------------------------------------------------------
//    Add an item to the list
// ----------------------------------------------------------------------------
{
    items.push_back(ChooserItem(caption, function));
}


void Chooser::AddCommands(text prefix, Symbols *symbols)
// ----------------------------------------------------------------------------
//   Add chooser commands from the symbols table
// ----------------------------------------------------------------------------
{
    // Add all the commands that begin with the prefix in the current context
    symbol_table &syms = symbols->names;
    symbol_table::iterator elem;
    uint first = prefix.length();
    for (elem = syms.begin(); elem != syms.end(); elem++)
    {
        text symname = (*elem).first;
        if (symname.find(prefix) == 0)
        {
            text caption = "";
            kstring data = symname.data();
            uint c, maxc = symname.length();
            for (c = first; c < maxc; c = Utf8Next(data, c))
            {
                wchar_t wc;
                char wcbuf[MB_LEN_MAX];
                mbtowc(&wc, data + c, maxc - c);
                if (wc == '_')
                    wc = ' ';
                else if (c == first)
                    wc = towupper(wc);
                int sz = wctomb(wcbuf, wc);
                if (sz > 0)
                    caption.insert(caption.end(), wcbuf, wcbuf + sz);
            }
            AddItem(caption, (*elem).second);
        }
    }
}

TAO_END