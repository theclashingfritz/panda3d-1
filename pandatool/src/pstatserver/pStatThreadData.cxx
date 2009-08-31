// Filename: pStatThreadData.cxx
// Created by:  drose (09Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pStatThreadData.h"

#include "pStatFrameData.h"
#include "pStatCollectorDef.h"
#include "config_pstats.h"


PStatFrameData PStatThreadData::_null_frame;

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatThreadData::
PStatThreadData(const PStatClientData *client_data) :
  _client_data(client_data)
{
  _first_frame_number = 0;
  _history = pstats_history;
  _computed_elapsed_frames = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatThreadData::
~PStatThreadData() {
}


////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::is_empty
//       Access: Public
//  Description: Returns true if the structure contains no frames,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PStatThreadData::
is_empty() const {
  return _frames.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_latest_frame_number
//       Access: Public
//  Description: Returns the frame number of the most recent frame
//               stored in the data.
////////////////////////////////////////////////////////////////////
int PStatThreadData::
get_latest_frame_number() const {
  nassertr(!_frames.empty(), 0);
  return _first_frame_number + _frames.size() - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_oldest_frame_number
//       Access: Public
//  Description: Returns the frame number of the oldest frame still
//               stored in the data.
////////////////////////////////////////////////////////////////////
int PStatThreadData::
get_oldest_frame_number() const {
  nassertr(!_frames.empty(), 0);
  return _first_frame_number;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::has_frame
//       Access: Public
//  Description: Returns true if we have received data for the
//               indicated frame number from the client and we still
//               have it stored, or false otherwise.
////////////////////////////////////////////////////////////////////
bool PStatThreadData::
has_frame(int frame_number) const {
  int rel_frame = frame_number - _first_frame_number;

  return (rel_frame >= 0 && rel_frame < (int)_frames.size() &&
          _frames[rel_frame] != (PStatFrameData *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_frame
//       Access: Public
//  Description: Returns a FrameData structure associated with the
//               indicated frame number.  If the frame data has not
//               yet been received from the client, returns the newest
//               frame older than the requested frame.
////////////////////////////////////////////////////////////////////
const PStatFrameData &PStatThreadData::
get_frame(int frame_number) const {
  int rel_frame = frame_number - _first_frame_number;
  int num_frames = _frames.size();
  if (rel_frame >= num_frames) {
    rel_frame = num_frames - 1;
  }

  while (rel_frame >= 0 && _frames[rel_frame] == (PStatFrameData *)NULL) {
    rel_frame--;
  }
  if (rel_frame < 0) {
    // No frame data that old.  Return the oldest frame we've got.
    rel_frame = 0;
    while (rel_frame < num_frames &&
           _frames[rel_frame] == (PStatFrameData *)NULL) {
      rel_frame++;
    }
  }

  if (rel_frame >= 0 && rel_frame < num_frames) {
    PStatFrameData *frame = _frames[rel_frame];
    nassertr(frame != (PStatFrameData *)NULL, _null_frame);
    nassertr(frame->get_start() >= 0.0, _null_frame);
    return *frame;
  }

  nassertr(_null_frame.get_start() >= 0.0, _null_frame);
  return _null_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_latest_time
//       Access: Public
//  Description: Returns the timestamp (in seconds elapsed since
//               connection) of the latest available frame.
////////////////////////////////////////////////////////////////////
float PStatThreadData::
get_latest_time() const {
  nassertr(!_frames.empty(), 0.0);
  return _frames.back()->get_start();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_oldest_time
//       Access: Public
//  Description: Returns the timestamp (in seconds elapsed since
//               connection) of the oldest available frame.
////////////////////////////////////////////////////////////////////
float PStatThreadData::
get_oldest_time() const {
  nassertr(!_frames.empty(), 0.0);
  return _frames.front()->get_start();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_frame_at_time
//       Access: Public
//  Description: Returns the FrameData structure associated with the
//               latest frame not later than the indicated time.
////////////////////////////////////////////////////////////////////
const PStatFrameData &PStatThreadData::
get_frame_at_time(float time) const {
  return get_frame(get_frame_number_at_time(time));
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_frame_number_at_time
//       Access: Public
//  Description: Returns the frame number of the latest frame not
//               later than the indicated time.
//
//               If the hint is nonnegative, it represents a frame
//               number that we believe the correct answer to be near,
//               which may speed the search for the frame.
////////////////////////////////////////////////////////////////////
int PStatThreadData::
get_frame_number_at_time(float time, int hint) const {
  hint -= _first_frame_number;
  if (hint >= 0 && hint < (int)_frames.size()) {
    if (_frames[hint] != (PStatFrameData *)NULL &&
        _frames[hint]->get_start() <= time) {
      // The hint might be right.  Scan forward from there.
      int i = hint + 1;
      while (i < (int)_frames.size() &&
             (_frames[i] == (PStatFrameData *)NULL ||
              _frames[i]->get_start() <= time)) {
        if (_frames[i] != (PStatFrameData *)NULL) {
          hint = i;
        }
        ++i;
      }
      return _first_frame_number + hint;
    }
  }

  // The hint is totally wrong.  Start from the end and work
  // backwards.

  int i = _frames.size() - 1;
  while (i >= 0) {
    const PStatFrameData *frame = _frames[i];
    if (frame != (PStatFrameData *)NULL && frame->get_start() <= time) {
      break;
    }
    --i;
  }

  return _first_frame_number + i;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_latest_frame
//       Access: Public
//  Description: Returns the FrameData associated with the most recent
//               frame.
////////////////////////////////////////////////////////////////////
const PStatFrameData &PStatThreadData::
get_latest_frame() const {
  nassertr(!_frames.empty(), _null_frame);
  return *_frames.back();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_elapsed_frames
//       Access: Public
//  Description: Computes the oldest frame number not older than
//               pstats_average_time seconds, and the newest frame
//               number.  Handy for computing average frame rate over
//               a time.  Returns true if there is any data in that
//               range, false otherwise.
////////////////////////////////////////////////////////////////////
bool PStatThreadData::
get_elapsed_frames(int &then_i, int &now_i) const {
  if (!_computed_elapsed_frames) {
    ((PStatThreadData *)this)->compute_elapsed_frames();
  }

  now_i = _now_i;
  then_i = _then_i;
  return _got_elapsed_frames;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_frame_rate
//       Access: Public
//  Description: Computes the average frame rate over the past
//               pstats_average_time seconds, by counting up the
//               number of frames elapsed in that time interval.
////////////////////////////////////////////////////////////////////
float PStatThreadData::
get_frame_rate() const {
  int then_i, now_i;
  if (!get_elapsed_frames(then_i, now_i)) {
    return 0.0f;
  }

  int num_frames = now_i - then_i + 1;
  float now = _frames[now_i - _first_frame_number]->get_end();
  float elapsed_time = (now - _frames[then_i - _first_frame_number]->get_start());
  return (float)num_frames / elapsed_time;
}


////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::set_history
//       Access: Public
//  Description: Sets the number of seconds worth of frames that will
//               be retained by the ThreadData structure as each new
//               frame is added.  This affects how old the oldest
//               frame that may be queried is.
////////////////////////////////////////////////////////////////////
void PStatThreadData::
set_history(float time) {
  _history = time;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::get_history
//       Access: Public
//  Description: Returns the number of seconds worth of frames that
//               will be retained by the ThreadData structure as each
//               new frame is added.  This affects how old the oldest
//               frame that may be queried is.
////////////////////////////////////////////////////////////////////
float PStatThreadData::
get_history() const {
  return _history;
}


////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::record_new_frame
//       Access: Public
//  Description: Makes room for and stores a new frame's worth of
//               data.  Calling this function may cause old frame data
//               to be discarded to make room, according to the amount
//               of time set up via set_history().
//
//               The pointer will become owned by the PStatThreadData
//               object and will be freed on destruction.
////////////////////////////////////////////////////////////////////
void PStatThreadData::
record_new_frame(int frame_number, PStatFrameData *frame_data) {
  nassertv(frame_data != (PStatFrameData *)NULL);
  nassertv(!frame_data->is_empty());
  float time = frame_data->get_start();

  // First, remove all the old frames that fall outside of our
  // history window.
  float oldest_allowable_time = time - _history;
  while (!_frames.empty() &&
         (_frames.front() == (PStatFrameData *)NULL ||
          _frames.front()->is_empty() ||
          _frames.front()->get_start() < oldest_allowable_time)) {
    if (_frames.front() != (PStatFrameData *)NULL) {
      delete _frames.front();
    }
    _frames.pop_front();
    _first_frame_number++;
  }

  // Now, add enough empty frame definitions to account for the latest
  // frame number.  This might involve some skips, since we don't
  // guarantee that we get all the frames in order or even at all.
  if (_frames.empty()) {
    _first_frame_number = frame_number;
    _frames.push_back(NULL);

  } else {
    while (_first_frame_number + (int)_frames.size() <= frame_number) {
      _frames.push_back(NULL);
    }
  }

  int index = frame_number - _first_frame_number;
  nassertv(index >= 0 && index < (int)_frames.size());

  if (_frames[index] != (PStatFrameData *)NULL) {
    nout << "Got repeated frame data for frame " << frame_number << "\n";
    delete _frames[index];
  }

  _frames[index] = frame_data;
  _computed_elapsed_frames = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThreadData::compute_elapsed_frames
//       Access: Private
//  Description: Computes the frame numbers returned by
//               get_elapsed_frames().  This is non-const, but only
//               updates cached values, so may safely be called from a
//               const method.
////////////////////////////////////////////////////////////////////
void PStatThreadData::
compute_elapsed_frames() {
  if (_frames.empty()) {
    // No frames in the data at all.
    _got_elapsed_frames = false;
    
  } else {
    _now_i = _frames.size() - 1;
    while (_now_i > 0 && _frames[_now_i] == (PStatFrameData *)NULL) {
      _now_i--;
    }
    if (_now_i < 0) {
      // No frames have any real data.
      _got_elapsed_frames = false;
      
    } else {
      nassertv(_frames[_now_i] != (PStatFrameData *)NULL);
      
      float now = _frames[_now_i]->get_end();
      float then = now - pstats_average_time;
      
      int old_i = _now_i;
      _then_i = _now_i;
      
      while (old_i >= 0) {
        const PStatFrameData *frame = _frames[old_i];
        if (frame != (PStatFrameData *)NULL) {
          if (frame->get_start() > then) {
            _then_i = old_i;
          } else {
            break;
          }
        }
        old_i--;
      }
      
      nassertv(_then_i >= 0);
      nassertv(_frames[_then_i] != (PStatFrameData *)NULL);
      _got_elapsed_frames = true;

      _now_i += _first_frame_number;
      _then_i += _first_frame_number;
    }
  }
  
  _computed_elapsed_frames = true;
}
