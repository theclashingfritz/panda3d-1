// Filename: fmodAudioManager.cxx
// Created by:  cort (January 22, 2003)
// Prior system by: cary
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

#include "pandabase.h"
#ifdef HAVE_FMOD //[

#include "config_audio.h"
#include "config_util.h"
#include "config_express.h"
#include "filename.h"
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "nullAudioSound.h"
#include "virtualFileSystem.h"
#include "string_utils.h"

#include <algorithm>
#include <cctype>
#include <fmod.h>

PT(AudioManager) Create_AudioManager() {
  audio_debug("Create_AudioManager() Fmod.");
  return new FmodAudioManager;
}

int FmodAudioManager::_active_managers = 0;

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::FmodAudioManager
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
FmodAudioManager::
FmodAudioManager() {
  audio_debug("FmodAudioManager::FmodAudioManager()");
  audio_debug("  audio_active="<<audio_active);
  audio_debug("  audio_volume="<<audio_volume);

  _active = audio_active;
  _cache_limit = audio_cache_limit;
  _concurrent_sound_limit = 0;

  // Initialize FMOD, if this is the first manager created.
  _is_valid = true;
  if (_active_managers == 0) {
    do {
      audio_debug("Initializing FMOD for real.");
      float fmod_dll_version = FSOUND_GetVersion();
      if (fmod_dll_version < FMOD_VERSION) {
        audio_error("Wrong FMOD DLL version.  You have "<<fmod_dll_version
            <<".  You need "<<FMOD_VERSION);
        _is_valid = false;
        break;
      }
      
      if (FSOUND_Init(44100, 32, 0) == 0) {
        audio_error("Fmod initialization failure.");
        _is_valid = false;
        break;
      }
    }
    while(0); // curious -- why is there a non-loop here?
  }

  // increment regardless of whether an error has occured -- the
  // destructor will do the right thing.
  ++_active_managers;
  audio_debug("  _active_managers="<<_active_managers);

  if (_is_valid)  {
    assert(is_valid());    
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::~FmodAudioManager
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FmodAudioManager::
~FmodAudioManager() {
  // Be sure to delete associated sounds before deleting the manager!
  nassertv(_soundsOnLoan.empty());
  clear_cache();
  --_active_managers;
  audio_debug("~FmodAudioManager(): "<<_active_managers<<" still active");
  if (_active_managers == 0) {
    audio_debug("Shutting down FMOD");
    FSOUND_Close();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::is_valid
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
is_valid() {
  bool check=true;
  if (_sounds.size() != _lru.size()) {
    audio_debug("--sizes--");
    check=false;
  } else {
    LRU::const_iterator i=_lru.begin();
    for (; i != _lru.end(); ++i) {
      SoundMap::const_iterator smi=_sounds.find(*i);
      if (smi == _sounds.end()) {
        audio_debug("--"<<*i<<"--");
        check=false;
        break;
      }
    }
  }
  return _is_valid && check;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PT(AudioSound) FmodAudioManager::
get_sound(const string &file_name) {
  audio_debug("FmodAudioManager::get_sound(file_name=\""<<file_name<<"\")");

  if(!is_valid()) {
     audio_debug("invalid FmodAudioManager returning NullSound");
     return get_null_sound();
  }

  assert(is_valid());
  Filename path = file_name;

  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->resolve_filename(path, get_sound_path());
  } else {
    path.resolve_filename(get_sound_path());
  }

  audio_debug("  resolved file_name is '"<<path<<"'");

  // Get the sound, either from the cache or from disk.
  SoundMap::iterator si = _sounds.find(path);
  SoundCacheEntry *entry = NULL;
  if (si != _sounds.end()) {
    // The sound was found in the cache.
    entry = &(*si).second;
    audio_debug("Sound file '"<<path<<"' found in cache.");
  } else {
    // The sound was not found in the cache.  Load it from disk.
    SoundCacheEntry new_entry;
    new_entry.data = load(path, new_entry.size);
    if (!new_entry.data) {
      audio_error("FmodAudioManager::load failed.");
      return get_null_sound();
    }
    new_entry.refcount = 0;
    new_entry.stale = true;

    // Add to the cache
    while (_sounds.size() >= (unsigned int)_cache_limit) {
      uncache_a_sound();
    }

    si = _sounds.insert(SoundMap::value_type(path, new_entry)).first;

    // It's important that we assign entry to the address of the entry
    // we just added to the map, and not to the address of the
    // temporary variable new_entry, which we just defined locally and
    // is about to go out of scope.
    entry = &(*si).second;
  }
  assert(entry != NULL);
  
  // Create an FMOD object from the memory-mapped file.  Here remains
  // one last vestige of special-case MIDI code: apparently, FMOD
  // doesn't like creating streams from memory-mapped MIDI files.
  // They must therefore be streamed from disk every time.  This
  // causes strange things to happen when the same MIDI file is loaded
  // twice, and played simultaneously...so, *don't do that then*.  all
  // I can say is that MIDI support will be significantly improved in
  // FMOD v4.0!
  FSOUND_STREAM *stream = NULL;
  int flags = FSOUND_LOADMEMORY | FSOUND_MPEGACCURATE;
  string os_path = path.to_os_specific();
  string suffix = downcase(path.get_extension());
  
  if (suffix == "mid" || suffix == "rmi" || suffix == "midi") {
    stream = FSOUND_Stream_OpenFile(os_path.c_str(), 0, 0);
  } else {
    stream = FSOUND_Stream_OpenFile((const char*)(entry->data),
                                    flags, entry->size);
  }
  if (stream == NULL) {
    audio_error("FmodAudioManager::get_sound failed.");
    return get_null_sound();
  }
  inc_refcount(path);
  most_recently_used(path);

  // determine length of sound
  float length = (float)FSOUND_Stream_GetLengthMs(stream) * 0.001f;

  // Build a new AudioSound from the audio data.
  PT(AudioSound) audioSound = 0;
  PT(FmodAudioSound) fmodAudioSound = new FmodAudioSound(this, stream, path,
               length);
  fmodAudioSound->set_active(_active);
  _soundsOnLoan.insert(fmodAudioSound);
  audioSound = fmodAudioSound;
  
  audio_debug("  returning 0x" << (void*)audioSound);
  assert(is_valid());
  return audioSound;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::uncache_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
uncache_sound(const string& file_name) {
  audio_debug("FmodAudioManager::uncache_sound(\""<<file_name<<"\")");
  assert(is_valid());
  Filename path = file_name;

  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->resolve_filename(path, get_sound_path());
  } else {
    path.resolve_filename(get_sound_path());
  }
  audio_debug("  path=\""<<path<<"\"");
  SoundMap::iterator itor = _sounds.find(path);
  if (itor == _sounds.end()) {
    audio_error("FmodAudioManager::uncache_sound: no such entry "<<file_name);
    return;
  }

  // Mark the entry as stale -- when its refcount reaches zero, it will
  // be removed from the cache.
  SoundCacheEntry *entry = &(*itor).second;
  if (entry->refcount == 0) {
    // If the refcount is already zero, it can be
    // purged right now!
    audio_debug("FmodAudioManager::uncache_sound: purging "<<path
    << " from the cache.");
    delete [] entry->data;

    // Erase the sound from the LRU list as well.
    assert(_lru.size()>0);
    LRU::iterator lru_i=find(_lru.begin(), _lru.end(), itor->first);
    assert(lru_i != _lru.end());
    _lru.erase(lru_i);
    _sounds.erase(itor);
  } else {
    entry->stale = true;
  }

  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::uncache_a_sound
//       Access: Public
//  Description: Uncaches the least recently used sound.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
uncache_a_sound() {
  audio_debug("FmodAudioManager::uncache_a_sound()");
  assert(is_valid());
  // uncache least recently used:
  assert(_lru.size()>0);
  LRU::reference path=_lru.front();
  SoundMap::iterator i = _sounds.find(path);
  assert(i != _sounds.end());
  _lru.pop_front();

  if (i != _sounds.end()) {
    audio_debug("  uncaching \""<<i->first<<"\"");
    uncache_sound(path);
  }
  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::most_recently_used
//       Access: Public
//  Description: Indicates that the given sound was the most recently used.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
most_recently_used(const string& path) {
  audio_debug("FmodAudioManager::most_recently_used(path=\""
      <<path<<"\")");
  LRU::iterator i=find(_lru.begin(), _lru.end(), path);
  if (i != _lru.end()) {
    _lru.erase(i);
  }
  // At this point, path should not exist in the _lru:
  assert(find(_lru.begin(), _lru.end(), path) == _lru.end());
  _lru.push_back(path);
  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::clear_cache
//       Access: Public
//  Description: Clear out the sound cache.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
clear_cache() {
  audio_debug("FmodAudioManager::clear_cache()");
  // Mark all cache entries as stale.  Delete those which already have 
  // refcounts of zero.

  SoundMap::iterator itor = _sounds.begin();

  // Have to use a while loop, not a for loop, since we don't want to
  // increment itor in the case in which we delete an entry.
  while (itor != _sounds.end()) {
    SoundCacheEntry *entry = &(*itor).second;
    if (entry->refcount == 0) {
      audio_debug("FmodAudioManager::clear_cache: purging "<< (*itor).first
      << " from the cache.");
      delete [] entry->data;

      // Erase the sound from the LRU list as well.
      assert(_lru.size()>0);
      LRU::iterator lru_i=find(_lru.begin(), _lru.end(), itor->first);
      assert(lru_i != _lru.end());
      _lru.erase(lru_i);
      _sounds.erase(itor);

      itor = _sounds.begin();
    } else {
      entry->stale = true;
      ++itor;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_cache_limit
//       Access: Public
//  Description: Set the number of sounds that the cache can hold.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_cache_limit(unsigned int count) {
  audio_debug("FmodAudioManager::set_cache_limit(count="<<count<<")");
  assert(is_valid());
  while (_lru.size() > count) {
    uncache_a_sound();
  }
  _cache_limit = count;
  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_cache_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int FmodAudioManager::
get_cache_limit() {
  audio_debug("FmodAudioManager::get_cache_limit() returning "
        <<_cache_limit);
  return _cache_limit;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::release_sound
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
release_sound(FmodAudioSound* audioSound) {
  audio_debug("FmodAudioManager::release_sound(audioSound=\""
      <<audioSound->get_name()<<"\")");
  dec_refcount(audioSound->get_name());
  _soundsOnLoan.erase(audioSound);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_volume
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_volume(float) {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_volume
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
float FmodAudioManager::
get_volume() {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_active
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_active(bool active) {
  audio_debug("FmodAudioManager::set_active(flag="<<active<<")");
  if (_active!=active) {
    _active=active;
    // Tell our AudioSounds to adjust:
    AudioSet::iterator i=_soundsOnLoan.begin();
    for (; i!=_soundsOnLoan.end(); ++i) {
      (**i).set_active(_active);
    }
  }
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_active
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
get_active() {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_concurrent_sound_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_concurrent_sound_limit(unsigned int limit) {
  _concurrent_sound_limit = limit;
  reduce_sounds_playing_to(_concurrent_sound_limit);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_concurrent_sound_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int FmodAudioManager::
get_concurrent_sound_limit() const {
  return _concurrent_sound_limit;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::reduce_sounds_playing_to
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
reduce_sounds_playing_to(unsigned int count) {
  // This is an example from Miles audio, this should be done for fmod:
  //int limit = _sounds_playing.size() - count;
  //while (limit-- > 0) {
  //  SoundsPlaying::iterator sound = _sounds_playing.begin();
  //  assert(sound != _sounds_playing.end());
  //  (**sound).stop();
  //}
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::stop_all_sounds
//       Access: Public
//  Description: Stop playback on all sounds managed by this manager.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
stop_all_sounds() {
  audio_debug("FmodAudioManager::stop_all_sounds()");
  AudioSet::iterator i=_soundsOnLoan.begin();
  for (; i!=_soundsOnLoan.end(); ++i) {
    if ((**i).status()==AudioSound::PLAYING) {
      (**i).stop();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::inc_refcount
//       Access: Protected
//  Description: Increments the refcount of a file's cache entry.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
inc_refcount(const string& file_name) {
  Filename path = file_name;
  SoundMap::iterator itor = _sounds.find(path);
  if (itor == _sounds.end()) {
    audio_debug("FmodAudioManager::inc_refcount: no such file "<<path);
    return;
  }

  SoundCacheEntry *entry = &(*itor).second;
  entry->refcount++;
  entry->stale = false; // definitely not stale!
  audio_debug("FmodAudioManager: "<<path<<" has a refcount of "
        << entry->refcount);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::dec_refcount
//       Access: Protected
//  Description: Decrements the refcount of a file's cache entry. If
//               the refcount reaches zero and the entry is stale, it
//               will be removed from the cache.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
dec_refcount(const string& file_name) {
  Filename path = file_name;
  SoundMap::iterator itor = _sounds.find(path);
  if (itor != _sounds.end()) {
    SoundCacheEntry *entry = &(*itor).second;
    entry->refcount--;
    audio_debug("FmodAudioManager: "<<path<<" has a refcount of "
    << entry->refcount);
    if (entry->refcount == 0 && entry->stale) {
      audio_debug("FmodAudioManager::dec_refcount: purging "<<path<< " from the cache.");
      delete [] entry->data;

      // Erase the sound from the LRU list as well.
      assert(_lru.size()>0);
      LRU::iterator lru_i=find(_lru.begin(), _lru.end(), itor->first);
      assert(lru_i != _lru.end());
      _lru.erase(lru_i);
      _sounds.erase(itor);
    }
  } else {
    audio_debug("FmodAudioManager::dec_refcount: no such file "<<path);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::load
//       Access: Private
//  Description: Loads the specified file into memory.  Returns a
//               newly-allocated buffer, and stores the size of the
//               buffer in size.  Returns NULL if an error occurs.
////////////////////////////////////////////////////////////////////
char* FmodAudioManager::
load(const Filename& filename, size_t &size) const {
  // Check file type (based on filename suffix
  string suffix = downcase(filename.get_extension());
  bool bSupported = false;
  if (suffix == "wav" || suffix == "mp3" || suffix == "mid"
      || suffix == "rmi" || suffix == "midi") {
    bSupported = true;
  }
  if (!bSupported) {
    audio_error("FmodAudioManager::load: "<<filename
    <<" is not a supported file format.");
    audio_error("Supported formats are: WAV, MP3, MIDI");
    return NULL;
  }

  // open the file.
  istream *audioFile = NULL;

  Filename binary_filename = Filename::binary_filename(filename);
  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

    if (!vfs->exists(filename)) {
      audio_error("File " << filename << " does not exist.");
      return NULL;
    }

    audioFile = vfs->open_read_file(binary_filename);

  } else {
    if (!filename.exists()) {
      audio_error("File " << filename << " does not exist.");
      return NULL;
    }

    audioFile = new ifstream;
    if (!binary_filename.open_read(*(ifstream *)audioFile)) {
      delete audioFile;
      audioFile = NULL;
    }
  }

  if (audioFile == (istream *)NULL) {
    // Unable to open.
    audio_error("Unable to read " << filename << ".");
    return NULL;
  }

  // Determine the file size.
  audioFile->seekg(0, ios::end);
  size = (size_t)audioFile->tellg();
  audioFile->seekg(0, ios::beg);
  
  // Read the entire file into memory.
  char *buffer = new char[size];
  if (buffer == NULL) {
    audio_error("out-of-memory error while loading "<<filename);
    delete audioFile;
    return NULL;
  }
  audioFile->read(buffer, size);
  if (!(*audioFile)) {
    audio_error("Read error while loading "<<filename);
    delete audioFile;
    delete [] buffer;
    return NULL;
  }

  delete audioFile;
  return buffer;
}

#endif //]
