
import FFIConstants
import TypedObject

WrapperClassMap = {}

# For testing, you can turn verbose and debug on
# FFIConstants.notify.setVerbose(1)
# FFIConstants.notify.setDebug(1)


# Register a python class in the type map if it is a typed object
# The type map is used for upcasting and downcasting through
# the panda inheritance chain
def registerInTypeMap(pythonClass):
    if issubclass(pythonClass, TypedObject.TypedObject):
        typeIndex = pythonClass.getClassType().getIndex()
        WrapperClassMap[typeIndex] = pythonClass



class FFIExternalObject:
    def __init__(self, *_args):
        # By default, we do not manage our own memory
        self.userManagesMemory = 0
        # Start with a null this pointer
        self.this = 0

    def destructor(self):
        # Base destructor in case you do not define one
        pass

    def getDowncastFunctions(self, thisClass, baseClass, chain):
        if (thisClass == baseClass):
            # Found it, return true
            return 1
        elif (len(thisClass.__bases__) == 0):
            # Not here, return 0
            return 0
        else:
            # Look recursively in the classes thisClass inherits from
            for base in thisClass.__bases__:
                # If it finds it, append the base class's downcast function
                # to the chain if it has one
                if self.getDowncastFunctions(base, baseClass, chain):
                    downcastFuncName = ('downcastTo' + thisClass.__name__
                                        + 'From' + base.__name__)
                    # Look over this classes global modules dictionaries
                    # for the downcast function name
                    for globmod in self.__class__.__CModuleDowncasts__:
                        if globmod.__dict__.has_key(downcastFuncName):
                            func = globmod.__dict__[downcastFuncName]
                            FFIConstants.notify.debug('Found downcast function %s in %s'
                                                      % (downcastFuncName, globmod.__name__))
                            chain.append(func)
                            return chain
                        else:
                            FFIConstants.notify.debug('Did not find downcast function %s in %s'
                                                      % (downcastFuncName, globmod.__name__))
                    # In any case, return the chain
                    return chain
                # Probably went up the wrong tree and did not find the rootClass
                else:
                    return []

        
    def setPointer(self):
        if (self.this == 0):
            # Null pointer, return None
            return None
        # If it is not a typed object, our work is done, just return the object
        if (not isinstance(self, TypedObject.TypedObject)): 
            return self
        # Ok, it is a typed object. See what type it really is and downcast
        # to that type (if necessary)
        exactWrapperClass = self.wrapperClassForTypeHandle(self.getType())
        # We do not need to downcast if we already have the same class
        if (exactWrapperClass and (exactWrapperClass != self.__class__)):
            # Create a new wrapper class instance
            exactObject = exactWrapperClass(None)
            # Get the downcast pointer that has had all the downcast funcs called
            downcastObject = self.downcast(exactWrapperClass)
            exactObject.this = downcastObject.this
            exactObject.userManagesMemory = downcastObject.userManagesMemory
            # Make sure the original downcast object does not get garbage collected
            # so that the exactObject will not get gc'd thereby transferring ownership
            # of the object to this new exactObject
            downcastObject.userManagesMemory = 0
            return exactObject
        else:
            return self
 
    def wrapperClassForTypeHandle(self, aTypeHandle):
        if WrapperClassMap.has_key(aTypeHandle.getIndex()):
            return WrapperClassMap[aTypeHandle.getIndex()]
        else:
            return None
        
    def downcast(self, specificClass):
        FFIConstants.notify.debug('downcasting from %s to %s' % \
            (self.__class__.__name__, specificClass.__name__))
        downcastChain = self.getDowncastFunctions(specificClass, self.__class__, [])
        FFIConstants.notify.debug('downcast chain: ' + `downcastChain`)
        newObject = self
        if (downcastChain == None):
            return newObject
        elif (downcastChain == 1):
            return newObject
        elif (downcastChain == 0):
            return newObject
        else:
            for downcastFunc in downcastChain:
                FFIConstants.notify.debug('downcasting %s using %s' % \
                    (newObject.__class__.__name__, downcastFunc))
                newObject = downcastFunc(newObject)
            return newObject

    def compareTo(self, other):
        # By default, we compare the C++ pointers
        # Some classes will override the compareTo operator with their own
        # logic in C++ (like vectors and matrices for instance)
        try:
            if self.this < other.this:
                return -1
            if self.this > other.this:
                return 1
            else:
                return 0
        except:
            return 1

    def __cmp__(self, other):
        # Only use the C++ compareTo if they are the same class
        if isinstance(other, self.__class__):
            return self.compareTo(other)
        # Otherwise, they must not be the same
        else:
            return 1

    def __hash__(self):
        return self.this




    
