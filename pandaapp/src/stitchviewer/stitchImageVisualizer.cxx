// Filename: stitchImageVisualizer.cxx
// Created by:  drose (05Nov99)
// 
////////////////////////////////////////////////////////////////////

#include "stitchImageVisualizer.h"
#include "config_stitch.h"
#include "triangleMesh.h"
#include "stitchLens.h"

#include <luse.h>
#include <chancfg.h>
#include <transform2sg.h>
#include <geomTristrip.h>
#include <geomNode.h>
#include <interactiveGraphicsPipe.h>
#include <noninteractiveGraphicsPipe.h>
#include <buttonThrower.h>
#include <dataGraphTraversal.h>
#include <eventQueue.h>
#include <texture.h>
#include <textureTransition.h>
#include <renderModeTransition.h>
#include <cullFaceTransition.h>
#include <cullFaceAttribute.h>
#include <clockObject.h>
#include <config_gobj.h>
#include <allAttributesWrapper.h>
#include <renderRelation.h>
#include <dataRelation.h>

#include <algorithm>

StitchImageVisualizer *StitchImageVisualizer::_static_siv;

// Returns the largest power of 2 less than or equal to value.
static int
to_power_2(int value) {
  int x = 1;
  while ((x << 1) <= value) {
    x = (x << 1);
  }
  return x;
}

StitchImageVisualizer::Image::
Image(StitchImage *image, int index, bool scale) :
  _image(image),
  _index(index)
{
  _arc = NULL;
  _viz = true;

  if (!image->read_file()) {
    nout << "Unable to read image.\n";
  }

  if (scale && image->_data != NULL) {
    // We have to make it a power of 2.
    int old_xsize = image->_data->get_x_size();
    int old_ysize = image->_data->get_y_size();
    int new_xsize = old_xsize;
    int new_ysize = old_ysize;
    if (max_texture_dimension > 0) {
      new_xsize = min(new_xsize, max_texture_dimension);
      new_ysize = min(new_ysize, max_texture_dimension);
    }
    new_xsize = to_power_2(new_xsize);
    new_ysize = to_power_2(new_ysize);

    if (new_xsize != old_xsize || new_ysize != old_ysize) {
      nout << "Scaling " << image->get_name() << " from " 
	   << old_xsize << " " << old_ysize << " to "
	   << new_xsize << " " << new_ysize << "\n";
      
      PNMImage *n = new PNMImage(new_xsize, new_ysize);
      n->quick_filter_from(*image->_data);
      delete image->_data;
      image->_data = n;
    }
  }
}

StitchImageVisualizer::Image::
Image(const Image &copy) :
  _image(copy._image),
  _arc(copy._arc),
  _tex(copy._tex),
  _viz(copy._viz),
  _index(copy._index)
{
}
 
void StitchImageVisualizer::Image::
operator = (const Image &copy) {
  _image = copy._image;
  _arc = copy._arc;
  _tex = copy._tex;
  _viz = copy._viz;
  _index = copy._index;
}

StitchImageVisualizer::
StitchImageVisualizer() :
  _event_handler(EventQueue::get_global_event_queue())
{
  _event_handler.add_hook("q", static_handle_event);
  _event_handler.add_hook("z", static_handle_event);
}


void StitchImageVisualizer::
add_input_image(StitchImage *image) {
  int index = _images.size();
  char letter = index + 'a';
  _images.push_back(Image(image, index, true));
  
  string event_name(1, letter);
  _event_handler.add_hook(event_name, static_handle_event);
}

void StitchImageVisualizer::
add_output_image(StitchImage *) {
}

void StitchImageVisualizer::
add_stitcher(Stitcher *) {
}

void StitchImageVisualizer::
execute() {
  setup();

  if (is_interactive()) {
    _main_win->set_draw_callback(this);
    _main_win->set_idle_callback(this);
    _running = true;
    while (_running) {
      _main_win->update();
    }
  } else {
    nout << "Drawing frame\n";
    draw(true);
    nout << "Done drawing frame\n";
  }
}
  
void StitchImageVisualizer::
setup() {
  ChanCfgOverrides override;

  override_chan_cfg(override);

  // load display modules
  GraphicsPipe::resolve_modules();
  
  // Create a window
  TypeHandle want_pipe_type = InteractiveGraphicsPipe::get_class_type();
  if (!is_interactive()) {
    want_pipe_type = NoninteractiveGraphicsPipe::get_class_type();
  }

  _main_pipe = GraphicsPipe::get_factory().make_instance(want_pipe_type);

  if (_main_pipe == (GraphicsPipe*)0L) {
    nout << "No suitable pipe is available!  Check your Configrc!\n";
    exit(1);
  }

  nout << "Opened a '" << _main_pipe->get_type().get_name()
       << "' graphics pipe." << endl;

  // Create the render node
  _render = new NamedNode("render");

  // make a node for the cameras to live under
  _cameras = new NamedNode("cameras");
  RenderRelation *cam_trans = new RenderRelation(_render, _cameras);

  _main_win = ChanConfig(_main_pipe, chan_cfg, _cameras, _render, override);
  assert(_main_win != (GraphicsWindow*)0L);

  // Turn on backface culling.
  CullFaceAttribute *cfa = new CullFaceAttribute;
  cfa->set_mode(CullFaceProperty::M_cull_clockwise);
  _initial_state.set_attribute(CullFaceTransition::get_class_type(), cfa);

  // Create the data graph root.
  _data_root = new NamedNode( "data" );

  // Create a mouse and put it in the data graph.
  _mak = new MouseAndKeyboard(_main_win, 0);
  new DataRelation(_data_root, _mak);

  // Create a trackball to handle the mouse input.
  _trackball = new Trackball("trackball");

  new DataRelation(_mak, _trackball);

  // Connect the trackball output to the camera's transform.
  PT(Transform2SG) tball2cam = new Transform2SG("tball2cam");
  tball2cam->set_arc(cam_trans);
  new DataRelation(_trackball, tball2cam);
  
  // Create an ButtonThrower to throw events from the keyboard.
  PT(ButtonThrower) et = new ButtonThrower("kb-events");
  new DataRelation(_mak, et);

  // Create all the images.
  Images::iterator ii;
  for (ii = _images.begin(); ii != _images.end(); ++ii) {
    create_image_geometry(*ii);
  }
}


void StitchImageVisualizer::
override_chan_cfg(ChanCfgOverrides &override) {
  override.setField(ChanCfgOverrides::Mask,
		    ((unsigned int)(W_DOUBLE|W_DEPTH|W_MULTISAMPLE)));
  override.setField(ChanCfgOverrides::Title, "Stitch");
}

void StitchImageVisualizer::
setup_camera(const RenderRelation &) {
}

bool StitchImageVisualizer::
is_interactive() const {
  return true;
}

void StitchImageVisualizer::
toggle_viz(StitchImageVisualizer::Image &im) {
  im._viz = !im._viz;
  if (im._viz) {
    im._arc->set_transition(new RenderModeTransition(RenderModeProperty::M_filled));
    im._arc->set_transition(new CullFaceTransition(CullFaceProperty::M_cull_clockwise));
    if (im._tex != (Texture *)NULL) {
      im._arc->set_transition(new TextureTransition(im._tex));
    }
  } else {
    im._arc->set_transition(new RenderModeTransition(RenderModeProperty::M_wireframe));
    im._arc->set_transition(new CullFaceTransition(CullFaceProperty::M_cull_none));
    im._arc->set_transition(new TextureTransition);
  }
}

void StitchImageVisualizer::
create_image_geometry(StitchImageVisualizer::Image &im) {
  /*
  int x_verts = im._image->get_x_verts();
  int y_verts = im._image->get_y_verts();
  */
  int x_verts = 2;
  int y_verts = 2;
  TriangleMesh mesh(x_verts, y_verts);

  StitchLens *lens = im._image->_lens;
  LVector3d center = 
    lens->extrude(LPoint2d(0.0, 0.0), im._image->_size_mm[0]);
  double scale = 10.0 / length(center);

  for (int xi = 0; xi < x_verts; xi++) {
    for (int yi = 0; yi < y_verts; yi++) {
      LPoint2d uv = LPoint2d((double)xi / (double)(x_verts - 1),
			     1.0 - (double)yi / (double)(y_verts - 1));
      LVector3d p = im._image->extrude(uv);

      mesh._coords.push_back(LCAST(float, p) * scale);
      mesh._texcoords.push_back(LCAST(float, uv));
    }
  }

  PT(GeomTristrip) geom = mesh.build_mesh();

  PT(GeomNode) node = new GeomNode;
  node->add_geom(geom.p());

  im._arc = new RenderRelation(_render, node);

  if (im._image->_data != NULL) {
    im._tex = new Texture;
    im._tex->set_name(im._image->get_filename());
    im._tex->load(*im._image->_data);
    im._arc->set_transition(new TextureTransition(im._tex));
  }
}


void StitchImageVisualizer::
draw(bool) {
  int num_windows = _main_pipe->get_num_windows();
  for (int w = 0; w < num_windows; w++) {
    GraphicsWindow *win = _main_pipe->get_window(w);
    win->get_gsg()->render_frame(_initial_state);
  }
  ClockObject::get_global_clock()->tick();
}

void StitchImageVisualizer::
idle() {
  // Initiate the data traversal, to send device data down its
  // respective pipelines.
  traverse_data_graph(_data_root);
  
  // Throw any events generated recently.
  _static_siv = this;
  _event_handler.process_events();
}

void StitchImageVisualizer::
static_handle_event(CPT(Event) event) {
  _static_siv->handle_event(event);
}


void StitchImageVisualizer::
handle_event(CPT(Event) event) {
  string name = event->get_name();

  if (name.size() == 1 && isalpha(name[0])) {
    int index = tolower(name[0]) - 'a';
    if (index >= 0 && index < (int)_images.size()) {
      toggle_viz(_images[index]);
      return;
    }
  }
  if (name == "q") {
    _running = false;

  } else if (name == "z") {
    _trackball->reset();
  }
}
