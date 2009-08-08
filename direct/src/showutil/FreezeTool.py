""" This module contains code to freeze a number of Python modules
into a single (mostly) standalone DLL or EXE. """

import modulefinder
import sys
import os
import marshal
import imp
import platform
from distutils.sysconfig import PREFIX, get_python_inc, get_python_version

import direct
from pandac.PandaModules import *

# Check to see if we are running python_d, which implies we have a
# debug build, and we have to build the module with debug options.
# This is only relevant on Windows.

# I wonder if there's a better way to determine this?
python = os.path.splitext(os.path.split(sys.executable)[1])[0]
isDebugBuild = (python.lower().endswith('_d'))

# These are modules that Python always tries to import up-front.  They
# must be frozen in any main.exe.
startupModules = [
    'site', 'sitecustomize', 'os', 'encodings.cp1252',
    'org',
    ]

# Our own Python source trees to watch out for.
sourceTrees = ['direct']

# The command to compile a c to an object file.  Replace %(basename)s
# with the basename of the source file, and an implicit .c extension.
compileObj = 'error'

# The command to link a single object file into an executable.  As
# above, replace $(basename)s with the basename of the original source
# file, and of the target executable.
linkExe = 'error'

# The command to link a single object file into a shared library.
linkDll = 'error'

# Paths to Python stuff.
Python = None
PythonIPath = get_python_inc()
PythonVersion = get_python_version()

# The VC directory of Microsoft Visual Studio (if relevant)
MSVC = None
# Directory to Windows Platform SDK (if relevant)
PSDK = None

# The setting to control release vs. debug builds.  Only relevant on
# Windows.
MD = None

# The _d extension to add to dll filenames on Windows in debug builds.
dllext = ''


if sys.platform == 'win32':
    Python = PREFIX
    
    if ('VCINSTALLDIR' in os.environ):
        MSVC = os.environ['VCINSTALLDIR']
    elif (Filename('/c/Program Files/Microsoft Visual Studio 9.0/VC').exists()):
        MSVC = Filename('/c/Program Files/Microsoft Visual Studio 9.0/VC').toOsSpecific()
    elif (Filename('/c/Program Files/Microsoft Visual Studio .NET 2003/Vc7').exists()):
        MSVC = Filename('/c/Program Files/Microsoft Visual Studio .NET 2003/Vc7').toOsSpecific()
    else:
        print 'Could not locate Microsoft Visual C++ Compiler! Try running from the Visual Studio Command Prompt.'
        sys.exit(1)
    
    if ('WindowsSdkDir' in os.environ):
        PSDK = os.environ['WindowsSdkDir']
    elif (platform.architecture()[0] == '32bit' and Filename('/c/Program Files/Microsoft Platform SDK for Windows Server 2003 R2').exists()):
        PSDK = Filename('/c/Program Files/Microsoft Platform SDK for Windows Server 2003 R2').toOsSpecific()
    elif (os.path.exists(os.path.join(MSVC, 'PlatformSDK'))):
        PSDK = os.path.join(MSVC, 'PlatformSDK')
    else:
        print 'Could not locate the Microsoft Windows Platform SDK! Try running from the Visual Studio Command Prompt.'
        sys.exit(1)

    # We need to use the correct compiler setting for debug vs. release builds.
    MD = '/MD'
    if isDebugBuild:
        MD = '/MDd'
        dllext = '_d'
    
    # If it is run by makepanda, it handles the MSVC and PlatformSDK paths itself.
    if ('MAKEPANDA' in os.environ):
        compileObj = 'cl /wd4996 /Fo%(basename)s.obj /nologo /c %(MD)s /Zi /O2 /Ob2 /EHsc /Zm300 /W3 /I"%(pythonIPath)s" %(filename)s'
        linkExe = 'link /nologo /MAP:NUL /FIXED:NO /OPT:REF /STACK:4194304 /INCREMENTAL:NO /LIBPATH:"%(python)s\libs"  /out:%(basename)s.exe %(basename)s.obj'
        linkDll = 'link /nologo /DLL /MAP:NUL /FIXED:NO /OPT:REF /INCREMENTAL:NO /LIBPATH:"%(python)s\libs"  /out:%(basename)s%(dllext)s.pyd %(basename)s.obj'
    else:
        os.environ['PATH'] += ';' + MSVC + '\\bin;' + MSVC + '\\Common7\\IDE;' + PSDK + '\\bin'
        
        compileObj = 'cl /wd4996 /Fo%(basename)s.obj /nologo /c %(MD)s /Zi /O2 /Ob2 /EHsc /Zm300 /W3 /I"%(pythonIPath)s" /I"%(PSDK)s\include" /I"%(MSVC)s\include" %(filename)s'
        linkExe = 'link /nologo /MAP:NUL /FIXED:NO /OPT:REF /STACK:4194304 /INCREMENTAL:NO /LIBPATH:"%(PSDK)s\lib" /LIBPATH:"%(MSVC)s\lib" /LIBPATH:"%(python)s\libs"  /out:%(basename)s.exe %(basename)s.obj'
        linkDll = 'link /nologo /DLL /MAP:NUL /FIXED:NO /OPT:REF /INCREMENTAL:NO /LIBPATH:"%(PSDK)s\lib" /LIBPATH:"%(MSVC)s\lib" /LIBPATH:"%(python)s\libs"  /out:%(basename)s%(dllext)s.pyd %(basename)s.obj'

elif sys.platform == 'darwin':
    # OSX
    compileObj = "gcc -fPIC -c -o %(basename)s.o -O2 -I%(pythonIPath)s %(filename)s"
    linkExe = "gcc -o %(basename)s %(basename)s.o -framework Python"
    linkDll = "gcc -undefined dynamic_lookup -bundle -o %(basename)s.so %(basename)s.o"

else:
    # Linux
    compileObj = "gcc -fPIC -c -o %(basename)s.o -O2 %(filename)s -I %(pythonIPath)s"
    linkExe = "gcc -o %(basename)s %(basename)s.o -lpython%(pythonVersion)s"
    linkDll = "gcc -shared -o %(basename)s.so %(basename)s.o -lpython%(pythonVersion)s"

# The code from frozenmain.c in the Python source repository.
frozenMainCode = """
/* Python interpreter main program for frozen scripts */

#include "Python.h"

#ifdef MS_WINDOWS
extern void PyWinFreeze_ExeInit(void);
extern void PyWinFreeze_ExeTerm(void);
extern int PyInitFrozenExtensions(void);
#endif

/* Main program */

int
Py_FrozenMain(int argc, char **argv)
{
    char *p;
    int n, sts;
    int inspect = 0;
    int unbuffered = 0;

    Py_FrozenFlag = 1; /* Suppress errors from getpath.c */

    if ((p = Py_GETENV("PYTHONINSPECT")) && *p != '\\0')
        inspect = 1;
    if ((p = Py_GETENV("PYTHONUNBUFFERED")) && *p != '\\0')
        unbuffered = 1;

    if (unbuffered) {
        setbuf(stdin, (char *)NULL);
        setbuf(stdout, (char *)NULL);
        setbuf(stderr, (char *)NULL);
    }

#ifdef MS_WINDOWS
    PyInitFrozenExtensions();
#endif /* MS_WINDOWS */
    Py_SetProgramName(argv[0]);
    Py_Initialize();
#ifdef MS_WINDOWS
    PyWinFreeze_ExeInit();
#endif

    if (Py_VerboseFlag)
        fprintf(stderr, "Python %s\\n%s\\n",
            Py_GetVersion(), Py_GetCopyright());

    PySys_SetArgv(argc, argv);

    n = PyImport_ImportFrozenModule("__main__");
    if (n == 0)
        Py_FatalError("__main__ not frozen");
    if (n < 0) {
        PyErr_Print();
        sts = 1;
    }
    else
        sts = 0;

    if (inspect && isatty((int)fileno(stdin)))
        sts = PyRun_AnyFile(stdin, "<stdin>") != 0;

#ifdef MS_WINDOWS
    PyWinFreeze_ExeTerm();
#endif
    Py_Finalize();
    return sts;
}
"""

# The code from frozen_dllmain.c in the Python source repository.
# Windows only.
frozenDllMainCode = """
#include "windows.h"

static char *possibleModules[] = {
    "pywintypes",
    "pythoncom",
    "win32ui",
    NULL,
};

BOOL CallModuleDllMain(char *modName, DWORD dwReason);


/*
  Called by a frozen .EXE only, so that built-in extension
  modules are initialized correctly
*/
void PyWinFreeze_ExeInit(void)
{
    char **modName;
    for (modName = possibleModules;*modName;*modName++) {
/*      printf("Initialising '%s'\\n", *modName); */
        CallModuleDllMain(*modName, DLL_PROCESS_ATTACH);
    }
}

/*
  Called by a frozen .EXE only, so that built-in extension
  modules are cleaned up 
*/
void PyWinFreeze_ExeTerm(void)
{
    // Must go backwards
    char **modName;
    for (modName = possibleModules+(sizeof(possibleModules) / sizeof(char *))-2;
         modName >= possibleModules;
         *modName--) {
/*      printf("Terminating '%s'\\n", *modName);*/
        CallModuleDllMain(*modName, DLL_PROCESS_DETACH);
    }
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    BOOL ret = TRUE;
    switch (dwReason) {
        case DLL_PROCESS_ATTACH: 
        {
            char **modName;
            for (modName = possibleModules;*modName;*modName++) {
                BOOL ok = CallModuleDllMain(*modName, dwReason);
                if (!ok)
                    ret = FALSE;
            }
            break;
        }
        case DLL_PROCESS_DETACH: 
        {
            // Must go backwards
            char **modName;
            for (modName = possibleModules+(sizeof(possibleModules) / sizeof(char *))-2;
                 modName >= possibleModules;
                 *modName--)
                CallModuleDllMain(*modName, DLL_PROCESS_DETACH);
            break;
        }
    }
    return ret;
}

BOOL CallModuleDllMain(char *modName, DWORD dwReason)
{
    BOOL (WINAPI * pfndllmain)(HINSTANCE, DWORD, LPVOID);

    char funcName[255];
    HMODULE hmod = GetModuleHandle(NULL);
    strcpy(funcName, "_DllMain");
    strcat(funcName, modName);
    strcat(funcName, "@12"); // stdcall convention.
    pfndllmain = (BOOL (WINAPI *)(HINSTANCE, DWORD, LPVOID))GetProcAddress(hmod, funcName);
    if (pfndllmain==NULL) {
        /* No function by that name exported - then that module does
           not appear in our frozen program - return OK
                */
        return TRUE;
    }
    return (*pfndllmain)(hmod, dwReason, NULL);
}
"""

# Our own glue code to start up a Python executable.
mainInitCode = """
%(frozenMainCode)s

int
main(int argc, char *argv[]) {
  PyImport_FrozenModules = _PyImport_FrozenModules;
  return Py_FrozenMain(argc, argv);
}
"""

# Our own glue code to start up a Python shared library.
dllInitCode = """
static PyMethodDef nullMethods[] = {
  {NULL, NULL}
};

%(dllexport)svoid init%(moduleName)s() {
  int count;
  struct _frozen *new_FrozenModules;

  count = 0;
  while (PyImport_FrozenModules[count].name != NULL) {
    ++count;
  }
  new_FrozenModules = (struct _frozen *)malloc((count + %(newcount)s + 1) * sizeof(struct _frozen));
  memcpy(new_FrozenModules, _PyImport_FrozenModules, %(newcount)s * sizeof(struct _frozen));
  memcpy(new_FrozenModules + %(newcount)s, PyImport_FrozenModules, count * sizeof(struct _frozen));
  memset(new_FrozenModules + count + %(newcount)s, 0, sizeof(struct _frozen));

  PyImport_FrozenModules = new_FrozenModules;

  Py_InitModule("%(moduleName)s", nullMethods);
}
"""

programFile = """
#include "Python.h"

%(moduleDefs)s

static struct _frozen _PyImport_FrozenModules[] = {
%(moduleList)s
  {NULL, NULL, 0}
};

%(initCode)s
"""

# Windows needs this bit.
frozenExtensions = """

static struct _inittab extensions[] = {
        /* Sentinel */
        {0, 0}
};
extern DL_IMPORT(int) PyImport_ExtendInittab(struct _inittab *newtab);

int PyInitFrozenExtensions()
{
        return PyImport_ExtendInittab(extensions);
}
"""

okMissing = [
    'Carbon.Folder', 'Carbon.Folders', 'HouseGlobals', 'Carbon.File',
    'MacOS', '_emx_link', 'ce', 'mac', 'org.python.core', 'os.path',
    'os2', 'posix', 'pwd', 'readline', 'riscos', 'riscosenviron',
    'riscospath', 'dbm', 'fcntl', 'win32api',
    '_winreg', 'ctypes', 'ctypes.wintypes', 'nt','msvcrt',
    'EasyDialogs', 'SOCKS', 'ic', 'rourl2path', 'termios',
    'OverrideFrom23._Res', 'email', 'email.Utils', 'email.Generator',
    'email.Iterators', '_subprocess', 'gestalt',
    'direct.extensions_native.extensions_darwin',
    ]

class Freezer:
    # Module tokens:
    MTAuto = 0
    MTInclude = 1
    MTExclude = 2
    MTForbid = 3

    class ModuleDef:
        def __init__(self, token, moduleName, filename = None):
            self.token = token
            self.moduleName = moduleName
            self.filename = filename

    def __init__(self, previous = None, debugLevel = 0):
        # Normally, we are freezing for our own platform.  Change this
        # if untrue.
        self.platform = sys.platform

        # You will also need to change these for a cross-compiler
        # situation.
        self.compileObj = compileObj
        self.linkExe = linkExe
        self.linkDll = linkDll

        # The filename extension to append to the source file before
        # compiling.
        self.sourceExtension = '.c'

        # The filename extension to append to the object file.
        self.objectExtension = '.o'
        if self.platform == 'win32':
            self.objectExtension = '.obj'

        # Change any of these to change the generated startup and glue
        # code.
        self.frozenMainCode = frozenMainCode
        self.frozenDllMainCode = frozenDllMainCode
        self.mainInitCode = mainInitCode
        self.frozenExtensions = frozenExtensions

        # Set this true to encode Python files in a Multifile as their
        # original source if possible, or false to encode them as
        # compiled pyc or pyo files.  This has no effect on frozen exe
        # or dll's; those are always stored with compiled code.
        self.storePythonSource = False

        # End of public interface.  These remaining members should not
        # be directly manipulated by callers.
        self.previousModules = {}
        self.modules = {}

        self.virtualModules = {}

        if previous:
            self.previousModules = dict(previous.modules)
            self.modules = dict(previous.modules)

        self.mf = None

        # Make sure we know how to find "direct".
        if direct.__path__:
            modulefinder.AddPackagePath('direct', direct.__path__[0])

    def excludeFrom(self, freezer):
        """ Excludes all modules that have already been processed by
        the indicated FreezeTool.  This is equivalent to passing the
        indicated FreezeTool object as previous to this object's
        constructor, but it may be called at any point during
        processing. """

        for key, value in freezer.modules.items():
            self.previousModules[key] = value
            self.modules[key] = value

    def excludeModule(self, moduleName, forbid = False):
        """ Adds a module to the list of modules not to be exported by
        this tool.  If forbid is true, the module is furthermore
        forbidden to be imported, even if it exists on disk. """

        assert self.mf == None

        if forbid:
            self.modules[moduleName] = self.ModuleDef(self.MTForbid, moduleName)
        else:
            self.modules[moduleName] = self.ModuleDef(self.MTExclude, moduleName)

    def handleCustomPath(self, moduleName):
        """ Indicates a module that may perform runtime manipulation
        of its __path__ variable, and which must therefore be actually
        imported at runtime in order to determine the true value of
        __path__. """

        str = 'import %s' % (moduleName)
        exec str

        module = sys.modules[moduleName]
        for path in module.__path__:
            modulefinder.AddPackagePath(moduleName, path)

    def getModulePath(self, moduleName):
        """ Looks for the indicated directory module and returns its
        __path__ member: the list of directories in which its python
        files can be found.  If the module is a .py file and not a
        directory, returns None. """

        # First, try to import the module directly.  That's the most
        # reliable answer, if it works.
        try:
            module = __import__(moduleName)
        except:
            module = None

        if module != None:
            for symbol in moduleName.split('.')[1:]:
                module = getattr(module, symbol)
            return module.__path__
        
        # If it didn't work--maybe the module is unimportable because
        # it makes certain assumptions about the builtins, or
        # whatever--then just look for file on disk.  That's usually
        # good enough.
        path = None
        baseName = moduleName
        if '.' in baseName:
            parentName, baseName = moduleName.rsplit('.', 1)
            path = self.getModulePath(parentName)
            if path == None:
                return None
            
        file, pathname, description = imp.find_module(baseName, path)

        if os.path.isdir(pathname):
            return [pathname]
        else:
            return None
            
    def addModule(self, moduleName, implicit = False, newName = None,
                  filename = None):
        """ Adds a module to the list of modules to be exported by
        this tool.  If implicit is true, it is OK if the module does
        not actually exist.

        newName is the name to call the module when it appears in the
        output.  The default is the same name it had in the original.
        Use caution when renaming a module; if another module imports
        this module by its original name, the module will need to be
        duplicated in the output, a copy for each name.

        The module name may end in ".*", which means to add all of the
        .py files (other than __init__.py) in a particular directory.
        It may also end in ".*.*", which means to cycle through all
        directories within a particular directory.
        """

        assert self.mf == None

        if not newName:
            newName = moduleName
        moduleName = moduleName.replace("/", ".").replace("direct.src", "direct")
        newName = newName.replace("/", ".").replace("direct.src", "direct")
        if implicit:
            token = self.MTAuto
        else:
            token = self.MTInclude

        if moduleName.endswith('.*'):
            assert(newName.endswith('.*'))
            # Find the parent module, so we can get its directory.
            parentName = moduleName[:-2]
            newParentName = newName[:-2]
            parentNames = [(parentName, newParentName)]

            if parentName.endswith('.*'):
                assert(newParentName.endswith('.*'))
                # Another special case.  The parent name "*" means to
                # return all possible directories within a particular
                # directory.

                topName = parentName[:-2]
                newTopName = newParentName[:-2]
                parentNames = []
                for dirname in self.getModulePath(topName):
                    for basename in os.listdir(dirname):
                        if os.path.exists(os.path.join(dirname, basename, '__init__.py')):
                            parentName = '%s.%s' % (topName, basename)
                            newParentName = '%s.%s' % (newTopName, basename)
                            if self.getModulePath(parentName):
                                parentNames.append((parentName, newParentName))

            for parentName, newParentName in parentNames:
                path = self.getModulePath(parentName)

                if path == None:
                    # It's actually a regular module.
                    self.modules[newParentName] = self.ModuleDef(token, parentName)

                else:
                    # Now get all the py files in the parent directory.
                    for dirname in path:
                        for basename in os.listdir(dirname):
                            if '-' in basename:
                                continue
                            if basename.endswith('.py') and basename != '__init__.py':
                                moduleName = '%s.%s' % (parentName, basename[:-3])
                                newName = '%s.%s' % (newParentName, basename[:-3])
                                self.modules[newName] = self.ModuleDef(token, moduleName)
        else:
            # A normal, explicit module name.
            self.modules[newName] = self.ModuleDef(token, moduleName, filename = filename)

    def done(self):
        """ Call this method after you have added all modules with
        addModule().  You may then call generateCode() or
        writeMultifile() to dump the resulting output.  After a call
        to done(), you may not add any more modules until you call
        reset(). """
        
        assert self.mf == None

        # If we have a __main__ module, we also need to implicitly
        # bring in Python's startup modules.
        if '__main__' in self.modules:
            for moduleName in startupModules:
                if moduleName not in self.modules:
                    self.modules[moduleName] = self.ModuleDef(self.MTAuto, moduleName)

        # Excluding a parent module also excludes all its children.
        # Walk through the list in sorted order, so we reach children
        # before parents.
        names = self.modules.items()
        names.sort()

        excludes = []
        excludeDict = {}
        includes = []
        autoIncludes = []
        origToNewName = {}
        for newName, mdef in names:
            token = mdef.token
            origToNewName[mdef.moduleName] = newName
            if '.' in newName:
                parentName, baseName = newName.rsplit('.', 1)
                if parentName in excludeDict:
                    token = excludeDict[parentName]
            
            if token == self.MTInclude:
                includes.append(mdef)
            elif token == self.MTAuto:
                autoIncludes.append(mdef)
            elif token == self.MTExclude or token == self.MTForbid:
                excludes.append(mdef.moduleName)
                excludeDict[mdef.moduleName] = token

        self.mf = modulefinder.ModuleFinder(excludes = excludes)

        # Attempt to import the explicit modules into the modulefinder.
        for mdef in includes:
            self.__loadModule(mdef)

        # Also attempt to import any implicit modules.  If any of
        # these fail to import, we don't really care.
        for mdef in autoIncludes:
            try:
                self.__loadModule(mdef)
            except ImportError:
                pass

        # Now, any new modules we found get added to the export list.
        for origName in self.mf.modules.keys():
            if origName not in origToNewName:
                self.modules[origName] = self.ModuleDef(self.MTAuto, origName)
                            
            elif origName not in self.modules:
                print "Module %s renamed to %s, but imported directly with its original name" % (
                    origName, origToNewName[origName])
                self.modules[origName] = self.ModuleDef(self.MTAuto, origName)

        missing = []
        for origName in self.mf.any_missing():
            if origName in startupModules:
                continue
            if origName in self.previousModules:
                continue

            # This module is missing.  Let it be missing in the
            # runtime also.
            self.modules[origName] = self.ModuleDef(self.MTExclude, origName)

            if origName in okMissing:
                # If it's listed in okMissing, don't even report it.
                continue

            prefix = origName.split('.')[0]
            if prefix not in sourceTrees:
                # If it's in not one of our standard source trees, assume
                # it's some wacky system file we don't need.
                continue
                
            missing.append(origName)
                
        if missing:
            error = "There are some missing modules: %r" % missing
            print error
            raise StandardError, error

    def __loadModule(self, mdef):
        """ Adds the indicated module to the modulefinder. """
        
        if mdef.filename:
            # If it has a filename, then we found it as a file on
            # disk.  In this case, the moduleName may not be accurate
            # and useful, so load it as a file instead.

            pathname = mdef.filename.toOsSpecific()
            fp = open(pathname, modulefinder.READ_MODE)
            stuff = ("", "r", imp.PY_SOURCE)
            self.mf.load_module(mdef.moduleName, fp, pathname, stuff)

        else:
            # Otherwise, we can just import it normally.
            self.mf.import_hook(mdef.moduleName)

    def reset(self):
        """ After a previous call to done(), this resets the
        FreezeTool object for a new pass.  More modules may be added
        and dumped to a new target.  Previously-added modules are
        remembered and will not be dumped again. """
        
        self.mf = None
        self.previousModules = dict(self.modules)

    def mangleName(self, moduleName):
        return 'M_' + moduleName.replace('.', '__').replace('-', '_')

    def getAllModuleNames(self):
        """ Return a list of all module names that have been included
        or forbidden, either in this current pass or in a previous
        pass.  Module names that have been excluded are not included
        in this list. """

        moduleNames = []

        for newName, mdef in self.modules.items():
            if mdef.token != self.MTExclude:
                moduleNames.append(newName)

        moduleNames.sort()
        return moduleNames

    def getModuleDefs(self):
        """ Return a list of all of the modules we will be explicitly
        or implicitly including.  The return value is actually a list
        of tuples: (moduleName, moduleDef)."""
        
        moduleDefs = []

        for newName, mdef in self.modules.items():
            token = mdef.token
            prev = self.previousModules.get(newName, None)
            if token == self.MTInclude or token == self.MTAuto:
                # Include this module (even if a previous pass
                # excluded it).  But don't bother if we exported it
                # previously.
                if prev and \
                   (prev.token == self.MTInclude or prev.token == self.MTAuto):
                    # Previously exported.
                    pass
                else:
                    if newName in self.mf.modules or \
                       newName in startupModules or \
                       mdef.filename:
                        moduleDefs.append((newName, mdef))
            elif token == self.MTForbid:
                if not prev or prev.token != self.MTForbid:
                    moduleDefs.append((newName, mdef))

        moduleDefs.sort()
        return moduleDefs

    def __replacePaths(self):
        # Build up the replacement pathname table, so we can eliminate
        # the personal information in the frozen pathnames.  The
        # actual filename we put in there is meaningful only for stack
        # traces, so we'll just use the module name.
        replace_paths = []
        for moduleName, module in self.mf.modules.items():
            if module.__code__:
                origPathname = module.__code__.co_filename
                replace_paths.append((origPathname, moduleName))
        self.mf.replace_paths = replace_paths

        # Now that we have built up the replacement mapping, go back
        # through and actually replace the paths.
        for moduleName, module in self.mf.modules.items():
            if module.__code__:
                co = self.mf.replace_paths_in_code(module.__code__)
                module.__code__ = co;

    def __addPyc(self, multifile, filename, code):
        if code:
            data = imp.get_magic() + '\0\0\0\0' + \
                   marshal.dumps(code)

            stream = StringStream(data)
            multifile.addSubfile(filename, stream, 0)
            multifile.flush()

    def __addPythonDirs(self, multifile, moduleDirs, dirnames):
        """ Adds all of the names on dirnames as a module directory. """
        if not dirnames:
            return
        
        str = '.'.join(dirnames)
        if str not in moduleDirs:
            # Add an implicit __init__.py file.
            moduleName = '.'.join(dirnames)
            filename = '/'.join(dirnames) + '/__init__'

            if self.storePythonSource:
                filename += '.py'
                stream = StringStream('')
                multifile.addSubfile(filename, stream, 0)
                multifile.flush()
            else:
                if __debug__:
                    filename += '.pyc'
                else:
                    filename += '.pyo'
                code = compile('', moduleName, 'exec')
                self.__addPyc(multifile, filename, code)

            moduleDirs[str] = True
            self.__addPythonDirs(multifile, moduleDirs, dirnames[:-1])

    def __addPythonFile(self, multifile, moduleDirs, moduleName, mdef):
        """ Adds the named module to the multifile as a .pyc file. """

        # First, split the module into its subdirectory names.
        dirnames = moduleName.split('.')
        self.__addPythonDirs(multifile, moduleDirs, dirnames[:-1])

        filename = '/'.join(dirnames)

        module = self.mf.modules.get(moduleName, None)
        if getattr(module, '__path__', None) is not None:
            # It's actually a package.  In this case, we really write
            # the file moduleName/__init__.py.
            filename += '/__init__'

            # Ensure we don't have an implicit filename from above.
            multifile.removeSubfile(filename + '.py')
            if __debug__:
                multifile.removeSubfile(filename + '.pyc')
            else:
                multifile.removeSubfile(filename + '.pyo')

        # Attempt to add the original source file if we can.
        if self.storePythonSource:
            sourceFilename = None
            if mdef.filename and mdef.filename.getExtension() == "py":
                sourceFilename = mdef.filename
            elif getattr(module, '__file__', None):
                sourceFilename = Filename.fromOsSpecific(module.__file__)
                sourceFilename.setExtension("py")
            
            if sourceFilename and sourceFilename.exists():
                filename += '.py'
                multifile.addSubfile(filename, sourceFilename, 0)
                return

        # If we can't find the source file, add the compiled pyc instead.
        if __debug__:
            filename += '.pyc'
        else:
            filename += '.pyo'

        if module:
            # Get the compiled code directly from the module object.
            code = getattr(module, "__code__", None)
        else:
            # Read the code from the source file and compile it on-the-fly.
            sourceFilename = None
            if mdef.filename and mdef.filename.getExtension() == "py":
                sourceFilename = mdef.filename
                source = open(sourceFilename.toOsSpecific(), 'r').read()
                if source and source[-1] != '\n':
                    source = source + '\n'
                code = compile(source, sourceFilename.cStr(), 'exec')

        self.__addPyc(multifile, filename, code)

    def addToMultifile(self, multifile):
        """ After a call to done(), this stores all of the accumulated
        python code into the indicated Multifile. """

        moduleDirs = {}
        for moduleName, mdef in self.getModuleDefs():
            if mdef.token != self.MTForbid:
                self.__addPythonFile(multifile, moduleDirs, moduleName, mdef)
    
    def writeMultifile(self, mfname):
        """ After a call to done(), this stores all of the accumulated
        python code into a Multifile with the indicated filename,
        including the extension. """

        self.__replacePaths()

        Filename(mfname).unlink()
        multifile = Multifile()
        if not multifile.openReadWrite(mfname):
            raise StandardError

        self.addToMultifile(multifile)

        multifile.flush()
        multifile.repack()

    def generateCode(self, basename, compileToExe = False):
        """ After a call to done(), this freezes all of the
        accumulated python code into either an executable program (if
        compileToExe is true) or a dynamic library (if compileToExe is
        false).  The basename is the name of the file to write,
        without the extension.

        The return value is the newly-generated filename, including
        the extension. """
        
        if compileToExe:
            # We must have a __main__ module to make an exe file.
            if not self.__writingModule('__main__'):
                message = "Can't generate an executable without a __main__ module."
                raise StandardError, message

        self.__replacePaths()

        # Now generate the actual export table.
        moduleDefs = []
        moduleList = []
        
        for moduleName, mdef in self.getModuleDefs():
            token = mdef.token
            origName = mdef.moduleName
            if token == self.MTForbid:
                # Explicitly disallow importing this module.
                moduleList.append(self.makeForbiddenModuleListEntry(moduleName))
            else:
                assert token != self.MTExclude
                # Allow importing this module.
                module = self.mf.modules.get(origName, None)
                code = getattr(module, "__code__", None)
                if not code and moduleName in startupModules:
                    # Forbid the loading of this startup module.
                    moduleList.append(self.makeForbiddenModuleListEntry(moduleName))
                else:
                    if origName in sourceTrees:
                        # This is one of our Python source trees.
                        # These are a special case: we don't compile
                        # the __init__.py files within them, since
                        # their only purpose is to munge the __path__
                        # variable anyway.  Instead, we pretend the
                        # __init__.py files are empty.
                        code = compile('', moduleName, 'exec')

                    if code:
                        code = marshal.dumps(code)

                        mangledName = self.mangleName(moduleName)
                        moduleDefs.append(self.makeModuleDef(mangledName, code))
                        moduleList.append(self.makeModuleListEntry(mangledName, code, moduleName, module))

        filename = basename + self.sourceExtension

        if compileToExe:
            code = self.frozenMainCode
            if self.platform == 'win32':
                code += self.frozenDllMainCode
            initCode = self.mainInitCode % {
                'frozenMainCode' : code,
                'programName' : os.path.basename(basename),
                }
            if self.platform == 'win32':
                initCode += self.frozenExtensions
                target = basename + '.exe'
            else:
                target = basename

            compileFunc = self.compileExe
            
        else:
            dllexport = ''
            if self.platform == 'win32':
                dllexport = '__declspec(dllexport) '
                target = basename + dllext + '.pyd'
            else:
                target = basename + '.so'
            
            initCode = dllInitCode % {
                'dllexport' : dllexport,
                'moduleName' : os.path.basename(basename),
                'newcount' : len(moduleList),
                }
            compileFunc = self.compileDll

        text = programFile % {
            'moduleDefs' : '\n'.join(moduleDefs),
            'moduleList' : '\n'.join(moduleList),
            'initCode' : initCode,
            }

        file = open(filename, 'w')
        file.write(text)
        file.close()

        try:
            compileFunc(filename, basename)
        finally:
            if (os.path.exists(filename)):
                os.unlink(filename)
            if (os.path.exists(basename + self.objectExtension)):
                os.unlink(basename + self.objectExtension)
        
        return target

    def compileExe(self, filename, basename):
        compile = self.compileObj % {
            'python' : Python,
            'MSVC' : MSVC,
            'PSDK' : PSDK,
            'MD' : MD,
            'pythonIPath' : PythonIPath,
            'pythonVersion' : PythonVersion,
            'filename' : filename,
            'basename' : basename,
            }
        print >> sys.stderr, compile
        if os.system(compile) != 0:
            raise StandardError

        link = self.linkExe % {
            'python' : Python,
            'MSVC' : MSVC,
            'PSDK' : PSDK,
            'pythonIPath' : PythonIPath,
            'pythonVersion' : PythonVersion,
            'filename' : filename,
            'basename' : basename,
            }
        print >> sys.stderr, link
        if os.system(link) != 0:
            raise StandardError

    def compileDll(self, filename, basename):
        compile = self.compileObj % {
            'python' : Python,
            'MSVC' : MSVC,
            'PSDK' : PSDK,
            'MD' : MD,
            'pythonIPath' : PythonIPath,
            'pythonVersion' : PythonVersion,
            'filename' : filename,
            'basename' : basename,
            }
        print >> sys.stderr, compile
        if os.system(compile) != 0:
            raise StandardError

        link = self.linkDll % {
            'python' : Python,
            'MSVC' : MSVC,
            'PSDK' : PSDK,
            'pythonIPath' : PythonIPath,
            'pythonVersion' : PythonVersion,
            'filename' : filename,
            'basename' : basename,
            'dllext' : dllext,
            }
        print >> sys.stderr, link
        if os.system(link) != 0:
            raise StandardError

    def makeModuleDef(self, mangledName, code):
        result = ''
        result += 'static unsigned char %s[] = {' % (mangledName)
        for i in range(0, len(code), 16):
            result += '\n  '
            for c in code[i:i+16]:
                result += ('%d,' % ord(c))
        result += '\n};\n'
        return result

    def makeModuleListEntry(self, mangledName, code, moduleName, module):
        size = len(code)
        if getattr(module, "__path__", None):
            # Indicate package by negative size
            size = -size
        return '  {"%s", %s, %s},' % (moduleName, mangledName, size)

    def makeForbiddenModuleListEntry(self, moduleName):
        return '  {"%s", NULL, 0},' % (moduleName)


    def __writingModule(self, moduleName):
        """ Returns true if we are outputting the named module in this
        pass, false if we have already output in a previous pass, or
        if it is not yet on the output table. """
        
        mdef = self.modules.get(moduleName, (None, None))
        token = mdef.token
        if token != self.MTAuto and token != self.MTInclude:
            return False

        if moduleName in self.previousModules:
            return False

        return True