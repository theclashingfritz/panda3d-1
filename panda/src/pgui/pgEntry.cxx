// Filename: pgEntry.cxx
// Created by:  drose (10Jul01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pgEntry.h"
#include "pgMouseWatcherParameter.h"

#include "throw_event.h"
#include "renderRelation.h"
#include "mouseWatcherParameter.h"
#include "directRenderTraverser.h"
#include "allTransitionsWrapper.h"
#include "transformTransition.h"
#include "pruneTransition.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "lineSegs.h"

#include "math.h"

TypeHandle PGEntry::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGEntry::
PGEntry(const string &name) : PGItem(name)
{
  _cursor_position = 0;
  _cursor_stale = true;
  _max_chars = 0;
  _max_width = 0.0;
  _last_text_def = (TextNode *)NULL;
  _text_geom_stale = true;
  _blink_start = 0.0;
  _blink_rate = 1.0;

  _text_render_root = new NamedNode("text_root");
  _current_text_arc = (NodeRelation *)NULL;
  Node *cursor = new NamedNode("cursor");
  _cursor_def = new RenderRelation(_text_render_root, cursor);
  _cursor_visible = true;

  update_state();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGEntry::
~PGEntry() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PGEntry::
PGEntry(const PGEntry &copy) :
  PGItem(copy),
  _text(copy._text),
  _cursor_position(copy._cursor_position),
  _max_chars(copy._max_chars),
  _max_width(copy._max_width),
  _text_defs(copy._text_defs),
  _blink_start(copy._blink_start),
  _blink_rate(copy._blink_rate)
{
  _cursor_stale = true;
  _last_text_def = (TextNode *)NULL;
  _text_geom_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PGEntry::
operator = (const PGEntry &copy) {
  _text = copy._text;
  _cursor_position = copy._cursor_position;
  _max_chars = copy._max_chars;
  _max_width = copy._max_width;
  _text_defs = copy._text_defs;
  _blink_start = copy._blink_start;
  _blink_rate = copy._blink_rate;

  _cursor_stale = true;
  _text_geom_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *PGEntry::
make_copy() const {
  return new PGEntry(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::draw_item
//       Access: Public, Virtual
//  Description: Called by the PGTop's traversal to draw this
//               particular item.
////////////////////////////////////////////////////////////////////
void PGEntry::
draw_item(PGTop *top, GraphicsStateGuardian *gsg, 
          const AllAttributesWrapper &attrib) {
  PGItem::draw_item(top, gsg, attrib);
  update_text();
  update_cursor();

  nassertv(_text_render_root != (Node *)NULL);

  // We'll use a normal DirectRenderTraverser to do the rendering
  // of the text.
  DirectRenderTraverser drt(gsg, RenderRelation::get_class_type());
  drt.set_view_frustum_cull(false);
  drt.traverse(_text_render_root, attrib, AllTransitionsWrapper());
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::press
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard entry is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void PGEntry::
press(const MouseWatcherParameter &param) {
  if (get_active()) {
    if (param.has_button()) {
      ButtonHandle button = param.get_button();
      
      if (button == MouseButton::one() ||
          button == MouseButton::two() ||
          button == MouseButton::three()) {
        // Mouse button; set focus.
        set_focus(true);
        
      } else if (get_focus()) {
        // Keyboard button.
        _cursor_position = min(_cursor_position, (int)_text.length());
        _blink_start = ClockObject::get_global_clock()->get_frame_time();
        if (button == KeyboardButton::enter()) {
          // Enter.  Accept the entry.
          accept(param);
          
        } else if (button == KeyboardButton::backspace()) {
          // Backspace.  Remove the character to the left of the cursor.
          if (_cursor_position > 0) {
            _text.erase(_text.begin() + _cursor_position - 1);
            _cursor_position--;
            _cursor_stale = true;
            _text_geom_stale = true;
          }
          
        } else if (button == KeyboardButton::del()) {
          // Delete.  Remove the character to the right of the cursor.
          if (_cursor_position < (int)_text.length()) {
            _text.erase(_text.begin() + _cursor_position);
            _text_geom_stale = true;
          }
          
        } else if (button == KeyboardButton::left()) {
          // Left arrow.  Move the cursor position to the left.
          _cursor_position = max(_cursor_position - 1, 0);
          _cursor_stale = true;
          
        } else if (button == KeyboardButton::right()) {
          // Right arrow.  Move the cursor position to the right.
          _cursor_position = min(_cursor_position + 1, (int)_text.length());
          _cursor_stale = true;
          
        } else if (button == KeyboardButton::home()) {
          // Home.  Move the cursor position to the beginning.
          _cursor_position = 0;
          _cursor_stale = true;
          
        } else if (button == KeyboardButton::end()) {
          // End.  Move the cursor position to the end.
          _cursor_position = _text.length();
          _cursor_stale = true;
          
        } else if (button.has_ascii_equivalent()) {
          char key = button.get_ascii_equivalent();
          if (isprint(key)) {
            // A normal visible character.  Add a new character to the
            // text entry, if there's room.
            
            if (get_max_chars() > 0 && (int)_text.length() >= get_max_chars()) {
              overflow(param);
            } else {
              string new_text = 
                _text.substr(0, _cursor_position) + key +
                _text.substr(_cursor_position);
              
              // Check the width.
              bool too_wide = false;
              if (get_max_width() > 0.0) {
                TextNode *text_node = get_text_def(S_focus);
                too_wide = (text_node->calc_width(new_text) > get_max_width());
              }
              
              if (too_wide) {
                overflow(param);
                
              } else {
                _text = new_text;
                
                _cursor_position++;
                _cursor_stale = true;
                _text_geom_stale = true;
              }
            }
          }
        }
      }
    }
  }
  PGItem::press(param);
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::accept
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               entry is accepted by the user pressing Enter normally.
////////////////////////////////////////////////////////////////////
void PGEntry::
accept(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  throw_event(get_accept_event(param.get_button()), 
              EventParameter(ep));
  set_focus(false);
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::overflow
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               entry is overflowed because the user attempts to type
//               too many characters, exceeding either set_max_chars()
//               or set_max_width().
////////////////////////////////////////////////////////////////////
void PGEntry::
overflow(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  throw_event(get_overflow_event(param.get_button()), 
              EventParameter(ep));
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::setup
//       Access: Published
//  Description: Sets up the entry for normal use.  The width is the
//               maximum width of characters that will be typed, and
//               determines the size of the entry, based on the
//               TextNode in effect.
////////////////////////////////////////////////////////////////////
void PGEntry::
setup(float width) {
  set_text(string());
  _cursor_position = 0;
  set_max_chars(0);
  set_max_width(width);

  TextNode *text_node = get_text_def(S_focus);
  float height = text_node->get_line_height();

  LVecBase4f frame(0.0, width, -0.3 * height, height);
  switch (text_node->get_align()) {
  case TM_ALIGN_LEFT:
    // The default case.
    break;

  case TM_ALIGN_CENTER:
    frame[0] = -width / 2.0;
    frame[1] = width / 2.0;
    break;

  case TM_ALIGN_RIGHT:
    frame[0] = -width;
    frame[1] = 0.0;
    break;
  }

  set_frame(frame[0] - 0.15, frame[1] + 0.15, frame[2], frame[3]);

  PGFrameStyle style;
  style.set_width(0.1, 0.1);
  style.set_type(PGFrameStyle::T_bevel_in);
  style.set_color(0.8, 0.8, 0.8, 1.0);

  set_frame_style(S_no_focus, style);

  style.set_color(0.9, 0.9, 0.9, 1.0);
  set_frame_style(S_focus, style);

  style.set_color(0.6, 0.6, 0.6, 1.0);
  set_frame_style(S_inactive, style);

  // Set up a default cursor: a vertical bar.
  clear_cursor_def();
  LineSegs ls;
  ls.set_color(0.0, 0.0, 0.0, 1.0);
  ls.move_to(0.0, 0.0, -0.15 * height);
  ls.draw_to(0.0, 0.0, 0.85 * height);
  new RenderRelation(get_cursor_def(), ls.create());

  // An underscore cursor would work too.
  //  text_node->set_text("_");
  //  new RenderRelation(get_cursor_def(), text_node->generate());
}


////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_text_def
//       Access: Published
//  Description: Changes the TextNode that will be used to render the
//               text within the entry when the entry is in the
//               indicated state.  The default if nothing is specified
//               is the same TextNode returned by
//               PGItem::get_text_node().
//
//               It is the responsibility of the user to ensure that
//               this TextNode has been frozen by a call to freeze().
//               Passing in an unfrozen TextNode will result in
//               needless work.
////////////////////////////////////////////////////////////////////
void PGEntry::
set_text_def(int state, TextNode *node) {
  nassertv(state >= 0 && state < 1000);  // Sanity check.
  if (node == (TextNode *)NULL && state >= (int)_text_defs.size()) {
    // If we're setting it to NULL, we don't need to slot a new one.
    return;
  }
  slot_text_def(state);

  _text_defs[state] = node;
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::get_text_def
//       Access: Published
//  Description: Returns the TextNode that will be used to render the
//               text within the entry when the entry is in the
//               indicated state.  See set_text_def().
////////////////////////////////////////////////////////////////////
TextNode *PGEntry:: 
get_text_def(int state) const {
  if (state < 0 || state >= (int)_text_defs.size()) {
    // If we don't have a definition, use the global one.
    return get_text_node();
  }
  if (_text_defs[state] == (TextNode *)NULL) {
    return get_text_node();
  }
  return _text_defs[state];
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_active
//       Access: Published, Virtual
//  Description: Toggles the active/inactive state of the entry.  In
//               the case of a PGEntry, this also changes its visual
//               appearance.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
set_active(bool active) {
  PGItem::set_active(active);
  update_state();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::set_focus
//       Access: Published, Virtual
//  Description: Toggles the focus state of the entry.  In the case of
//               a PGEntry, this also changes its visual appearance.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
set_focus(bool focus) {
  PGItem::set_focus(focus);
  _blink_start = ClockObject::get_global_clock()->get_frame_time();
  update_state();
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::slot_text_def
//       Access: Private
//  Description: Ensures there is a slot in the array for the given
//               text definition.
////////////////////////////////////////////////////////////////////
void PGEntry::
slot_text_def(int state) {
  while (state >= (int)_text_defs.size()) {
    _text_defs.push_back((TextNode *)NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::update_text
//       Access: Private
//  Description: Causes the PGEntry to recompute its text, if
//               necessary.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
update_text() {
  TextNode *node = get_text_def(get_state());
  nassertv(node != (TextNode *)NULL);

  if (_text_geom_stale || node != _last_text_def) {
    // We need to regenerate.
    _last_text_def = node;
    _last_text_def->set_text(_text);

    if (_current_text_arc != (NodeRelation *)NULL) {
      remove_arc(_current_text_arc);
    }
    PT_Node text = _last_text_def->generate();
    _current_text_arc = new RenderRelation(_text_render_root, text);
    _text_geom_stale = false;
    _text_left = _last_text_def->get_left();
    _cursor_stale = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::update_cursor
//       Access: Private
//  Description: Moves the cursor to its correct position.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
update_cursor() {
  TextNode *node = get_text_def(get_state());
  nassertv(node != (TextNode *)NULL);

  if (_cursor_stale || node != _last_text_def) {
    update_text();

    _cursor_position = min(_cursor_position, (int)_text.length());

    float width = 
      _last_text_def->calc_width(_text.substr(0, _cursor_position));

    LVecBase3f trans(_text_left + width, 0.0, 0.0);
    LMatrix4f pos = LMatrix4f::translate_mat(trans);
    _cursor_def->set_transition(new TransformTransition(pos));

    _cursor_stale = false;
  }

  // Should the cursor be visible?
  if (!get_focus()) {
    show_hide_cursor(false);
  } else {
    double elapsed_time = 
      ClockObject::get_global_clock()->get_frame_time() - _blink_start;
    int cycle = (int)floor(elapsed_time * _blink_rate * 2.0);
    bool visible = ((cycle & 1) == 0);
    show_hide_cursor(visible);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::show_hide_cursor
//       Access: Private
//  Description: Makes the cursor visible or invisible, e.g. during a
//               blink cycle.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
show_hide_cursor(bool visible) {
  if (visible != _cursor_visible) {
    if (visible) {
      // Reveal the cursor.
      _cursor_def->clear_transition(PruneTransition::get_class_type());
    } else {
      // Hide the cursor.
      _cursor_def->set_transition(new PruneTransition());
    }
    _cursor_visible = visible;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGEntry::update_state
//       Access: Private
//  Description: Determines what the correct state for the PGEntry
//               should be.
////////////////////////////////////////////////////////////////////
void PGEntry:: 
update_state() {
  if (get_active()) {
    if (get_focus()) {
      set_state(S_focus);
    } else {
      set_state(S_no_focus);
    }
  } else {
    set_state(S_inactive);
  }
}
