#ifndef TEXT_DRAWING_H
#define TEXT_DRAWING_H
// ****************************************************************************
//  text_drawing.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//     Rendering of text
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "attributes.h"
#include "shapes.h"
#include "selection.h"
#include "coords3d.h"
#include "tree.h"
#include <QFont>
#include <QTextCursor>

TAO_BEGIN

struct TextSpan : Shape
// ----------------------------------------------------------------------------
//    A contiguous run of glyphs
// ----------------------------------------------------------------------------
{
    TextSpan(Text *source, uint start = 0, uint end = ~0)
        : Shape(), source(source), start(start), end(end) {}
    virtual void        Draw(Layout *where);
    virtual void        DrawCached(Layout *where);
    virtual void        Identify(Layout *where);
    virtual Box3        Bounds(Layout *where);
    virtual Box3        Space(Layout *where);
    virtual TextSpan *  Break(BreakOrder &order, uint &sz);
    virtual scale       TrailingSpaceSize(Layout *where);
    virtual void        Draw(GraphicPath &path, Layout *where);

protected:
    void                DrawDirect(Layout *where);
    void                DrawSelection(Layout *where);
    uint                PerformEditOperation(Widget *w, uint i, uint next);
    void                PerformInsertOperation(Layout * l,
                                               Widget * widget,
                                               uint     position);
public:
    Text_p              source;
    uint                start, end;
};


struct TextFormulaEditInfo : XL::Info
// ----------------------------------------------------------------------------
//    Record the text format for a text formula while editing it
// ----------------------------------------------------------------------------
{
    TextFormulaEditInfo(XL::Text *s, uint id): source(s), order(id) {}
    Text_p              source;
    uint                order;
};


struct TextFormula : TextSpan
// ----------------------------------------------------------------------------
//   Like a text span, but for an evaluated value
// ----------------------------------------------------------------------------
{
    TextFormula(XL::Prefix *self, Widget *wid, uint start = 0, uint end = ~0)
        : TextSpan(NULL, start, end), self(self), widget(wid)
    {
        source = Format(self);
    }
    Text *              Format(Prefix *value);
    bool                Validate(Text *source, Widget *widget);

    virtual void        DrawSelection(Layout *where);
    virtual void        Identify(Layout *where);

public:
    Prefix_p            self;
    Widget *            widget;
    static uint         formulas, shows;
};


struct TextSelect : Identify
// ----------------------------------------------------------------------------
//   A text selection (contiguous range of characters)
// ----------------------------------------------------------------------------
{
    TextSelect(Widget *w);

    virtual Activity *  Display(void);
    virtual Activity *  Idle(void);
    virtual Activity *  Key(text key);
    virtual Activity *  Edit(text key);
    virtual Activity *  Click(uint button, uint count, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);

    // Mark and point have roughly the same meaning as in GNU Emacs
    void                moveTo(uint pos)        { mark = point = pos; }
    void                select(uint mk, uint pt){ mark = mk; point = pt; }
    uint                start() { return mark < point ? mark : point; }
    uint                end()   { return mark < point ? point : mark; }
    bool                hasSelection()          { return mark != point; }
    void                updateSelection();
    bool                needsPositions()        { return direction >= Up; }
    void                processLineBreak();
    void                processChar(uint id, coord x, bool selected, uint code);

    enum Direction      { None, Mark, Left, Right, Up, Down };
    uint                mark, point, previous, last, textBoxId;
    Direction           direction;
    coord               targetX;
    Box3                selBox;
    Box3                formulaBox;
    text                replacement;
    bool                replace;
    bool                replaceInProgress;
    bool                textMode;
    bool                pickingLineEnds;
    bool                pickingUpDown;
    bool                movePointOnly;
    uint                formulaMode;

    QTextCursor         cursor;
    Tree *              replacement_tree;
    bool                inSelection;
};


TAO_END

#endif // TEXT_DRAWING_H