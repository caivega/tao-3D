// ****************************************************************************
//  xl.semantics.functions.xs       (C) 1992-2003 Christophe de Dinechin (ddd) 
//                                                                 XL2 project 
// ****************************************************************************
// 
//   File Description:
// 
//     Implementation of functions and function overloading
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
// See http://www.gnu.org/copyleft/gpl.html for details
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

import DCL = XL.SEMANTICS.DECLARATIONS
import FT = XL.SEMANTICS.TYPES.FUNCTIONS
import TY = XL.SEMANTICS.TYPES
import SYM = XL.SYMBOLS


module XL.SEMANTICS.FUNCTIONS with
// ----------------------------------------------------------------------------
//    Interface of the semantics of basic C++-style functions
// ----------------------------------------------------------------------------

    type declaration            is DCL.declaration
    type declaration_list       is FT.declaration_list
    type function_type          is FT.function_type
    type any_type               is TY.any_type 


    type function_data is DCL.declaration_data with
    // ------------------------------------------------------------------------
    //   The function holds the parameters and return type
    // ------------------------------------------------------------------------
    //   It is expected that the 'type' field be a function type, and
    //   the type_source is empty

        parameters       : declaration_list
        return_type      : any_type
        outputs_count    : integer
        inputs_count     : integer
        result_mname     : PT.name_tree
        symbols          : SYM.symbol_table

    type function is access to function_data

    function FunctionInfo (input : PT.tree) return function
