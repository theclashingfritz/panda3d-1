#begin bin_target
  #define TARGET stitch-command
  #define LOCAL_LIBS \
    stitchbase
  #define OTHER_LIBS \
    progbase \
    pnmimage:c mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m pystub dtoolconfig dtool

  #define SOURCES \
    stitchCommandProgram.cxx stitchCommandProgram.h

  #define INSTALL_HEADERS \

#end bin_target

#begin bin_target
  #define TARGET stitch-image
  #define LOCAL_LIBS \
    stitchbase
  #define OTHER_LIBS \
    progbase \
    pnmimagetypes:c pnmimage:c linmath:c putil:c panda:m \
    express:c pandaexpress:m pystub dtoolconfig dtool

  #define SOURCES \
    stitchImageProgram.cxx stitchImageProgram.h

#end bin_target

#begin bin_target
  #define TARGET stitch-viewer
  #define LOCAL_LIBS \
    stitchviewer stitchbase
  #define OTHER_LIBS \
    progbase \
    device:c tform:c graph:c dgraph:c sgraph:c gobj:c sgattrib:c \
    event:c chancfg:c display:c sgraphutil:c light:c \
    pnmimagetypes:c pnmimage:c putil:c express:c \
    panda:m pandaexpress:m \
    dtoolutil:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    stitchViewerProgram.cxx stitchViewerProgram.h

#end bin_target

