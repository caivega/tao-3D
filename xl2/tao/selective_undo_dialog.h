#ifndef SELECTIVE_UNDO_DIALOG_H
#define SELECTIVE_UNDO_DIALOG_H
// ****************************************************************************
//  selective_undo_dialog.cpp                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Selective undo" dialog box. This is a modeless
//    dialog which enables to pick any past change and revert it in the current
//    document.
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

#include "history_dialog.h"
#include "repository.h"
#include <QWidget>
#include <QPushButton>

namespace Tao {

struct Repository;

class SelectiveUndoDialog : public HistoryDialog
{
    Q_OBJECT

public:
    SelectiveUndoDialog(Repository *repo, QWidget *parent = 0);

private slots:
    void    undoButton_clicked();
};

}

#endif // SELECTIVE_UNDO_DIALOG_H