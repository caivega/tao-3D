#ifndef TAO_MODULE_API_H
#define TAO_MODULE_API_H
// ****************************************************************************
//  module_api.h                                                   Tao project
// ****************************************************************************
//
//   File Description:
//
//    Interface between the Tao runtime and native modules
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao/module_info.h"

// ========================================================================
//
//   Interface versioning
//
// ========================================================================

// The Tao module loader uses a versioning system similar to libtool's:
// http://www.gnu.org/software/libtool/manual/html_node/Libtool-versioning.html
// The loader will refuse to load a module library that does not pass the
// version compatibility check.
//
// Here are the rules when changing these numbers:
//
// - Update the version information only immediately before a public release
// - [ANY CHANGE] If any interfaces have been added, removed, or changed
//   since the last update, increment current.
// - [COMPATIBLE CHANGE] If any interfaces have been added since the last
//   public release, then increment age.
// - [INCOMPATIBLE CHANGE] If any interfaces have been removed or changed
//   since the last public release, then set age to 0.

#define TAO_MODULE_API_CURRENT   2
#define TAO_MODULE_API_AGE       2

// ========================================================================
//
//   Tao runtime interface
//
// ========================================================================

namespace Tao {


struct ModuleApi
// ------------------------------------------------------------------------
//   API exported by the Tao runtime for use by modules
// ------------------------------------------------------------------------
{
    typedef void (*render_fn)(void *arg);

    // Add a rendering callback to the current layout. Callback will be
    // invoked when it's time to draw the layout.
    // This function is typically used by XL primitives that need to draw
    // something using OpenGL calls.
    bool (*scheduleRender)(render_fn callback, void *arg);
};

}

// ========================================================================
//
//   Module interface
//
// ========================================================================

namespace XL
{
    struct Context;
}

namespace Tao
{
    typedef int (*module_init_fn)   (const Tao::ModuleApi *,
                                     const Tao::ModuleInfo *);
    typedef int (*enter_symbols_fn) (XL::Context *);
    typedef int (*delete_symbols_fn)(XL::Context *);
    typedef int (*module_exit_fn)   ();
}

extern "C"
// ------------------------------------------------------------------------
//   API exported by modules for use by the Tao runtime
// ------------------------------------------------------------------------
{
    // Called once immediately after the module library is loaded
    // [Optional]
    int module_init(const Tao::ModuleApi *a, const Tao::ModuleInfo *m);

    // Called once after module_init to let the module extend the XL symbol
    // table (for instance, add new XL commands) in a given context
    // [Mandatory]
    // [May be automatically generated by the tbl_gen script]
    int enter_symbols(XL::Context *c);

    // Called once before module is unloaded to let module remove its
    // primitives from the XL symbol table
    // [Optional]
    // [May be automatically generated by the tbl_gen script]
    int delete_symbols(XL::Context *c);

    // Called when the module library is about to be unloaded
    // [Optional]
    int module_exit();
}

#endif // TAO_MODULE_API_H
