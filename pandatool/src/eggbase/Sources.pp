#begin ss_lib_target
  #define TARGET eggbase
  #define LOCAL_LIBS \
    progbase converter
  #define OTHER_LIBS \
    egg:c linmath:c panda:m
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     

  #define SOURCES \
     eggBase.h eggConverter.h eggFilter.h eggMultiBase.h \
     eggMultiFilter.h eggReader.h eggToSomething.h eggWriter.h \
     somethingToEgg.h 
    
  #define INCLUDED_SOURCES \
     eggBase.cxx eggConverter.cxx eggFilter.cxx eggMultiBase.cxx \
     eggMultiFilter.cxx eggReader.cxx eggToSomething.cxx \
     eggWriter.cxx somethingToEgg.cxx 

  #define INSTALL_HEADERS \
    eggBase.h eggConverter.h eggFilter.h \
    eggMultiBase.h eggMultiFilter.h \
    eggReader.h eggToSomething.h eggWriter.h somethingToEgg.h

#end ss_lib_target

