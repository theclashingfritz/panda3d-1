// Filename: cppExpressionParser.cxx
// Created by:  drose (25Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


#include "cppExpressionParser.h"
#include "cppExpression.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPExpressionParser::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpressionParser::
CPPExpressionParser(CPPScope *current_scope, CPPScope *global_scope) :
  _current_scope(current_scope),
  _global_scope(global_scope)
{
  _expr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpressionParser::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpressionParser::
~CPPExpressionParser() {
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpressionParser::parse_expr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPExpressionParser::
parse_expr(const string &expr) {
  if (!init_const_expr(expr)) {
    cerr << "Unable to parse expression\n";
    return false;
  }

  _expr = parse_const_expr(this, _current_scope, _global_scope);

  return get_error_count() == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpressionParser::parse_expr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPExpressionParser::
parse_expr(const string &expr, const CPPPreprocessor &filepos) {
  if (!init_const_expr(expr)) {
    cerr << "Unable to parse expression\n";
    return false;
  }

  copy_filepos(filepos);

  _expr = parse_const_expr(this, _current_scope, _global_scope);

  return get_error_count() == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExpressionParser::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPExpressionParser::
output(ostream &out) const {
  if (_expr == NULL) {
    out << "(null expr)";
  } else {
    out << *_expr;
  }
}
