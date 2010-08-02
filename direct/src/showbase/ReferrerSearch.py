import inspect
import sys
import gc
from direct.showbase.PythonUtil import _getSafeReprNotify
from direct.showbase.Job import Job

class ReferrerSearch(Job):
    def __init__(self, obj, maxRefs = 100):
        Job.__init__(self, 'ReferrerSearch')
        self.obj = obj
        self.maxRefs = maxRefs
        self.found = set()
        self.depth = 0
        self.shouldPrintStats = False
    
    def __call__(self):
        safeReprNotify = _getSafeReprNotify()
        info = safeReprNotify.getInfo()
        safeReprNotify.setInfo(0)

        self.found = set()
        try:
            self.step(0, [self.obj])
        finally:
            self.obj = None
            pass
        
        safeReprNotify.setInfo(info)
        pass

    def run(self):
        safeReprNotify = _getSafeReprNotify()
        self.info = safeReprNotify.getInfo()
        safeReprNotify.setInfo(0)

        print 'RefPath(%s): Beginning ReferrerSearch for %s' %(self._id, fastRepr(self.obj))

        self.found = set()
        for x in self.stepGenerator(0, [self.obj]):
            yield None
            pass
        
        yield Job.Done
        pass

    def finished(self):
        print 'RefPath(%s): Finished ReferrerSearch for %s' %(self._id, fastRepr(self.obj))
        self.obj = None

        safeReprNotify = _getSafeReprNotify()
        safeReprNotify.setInfo(self.info)
        pass

    def __del__(self):
        print 'ReferrerSearch garbage collected'
        
    def truncateAtNewLine(self, s):
        if s.find('\n') == -1:
            return s
        else:
            return s[:s.find('\n')]

    def printStatsWhenAble(self):
        self.shouldPrintStats = True
        pass
        
    def myrepr(self, referrer, refersTo):
        pre = ''
        if (isinstance(referrer, dict)):
            for k,v in referrer.iteritems():
                if v is refersTo:
                    pre = self.truncateAtNewLine(fastRepr(k)) + ']-> '
                    break
        elif (isinstance(referrer, (list, tuple))):
            for x in xrange(len(referrer)):
                if referrer[x] is refersTo:
                    pre = '%s]-> ' % (x)
                    break

        if (isinstance(refersTo, dict)):
            post = 'dict['
        elif (isinstance(refersTo, list)):
            post = 'list['
        elif (isinstance(refersTo, tuple)):
            post = 'tuple['
        elif (isinstance(refersTo, set)):
            post = 'set->'
        else:
            post = self.truncateAtNewLine(fastRepr(refersTo)) + "-> "

        return '%s%s' % (pre, post)
        
    def step(self, depth, path):
        if self.shouldPrintStats:
            self.printStats(path)
            self.shouldPrintStats = False
        
        at = path[-1]

        if id(at) in self.found:
               # don't continue down this path
               return

        # check for success
        if (self.isAtRoot(at, path)):
            return

        # mark our progress after checking goal
        self.found.add(id(at))

        referrers = [ref for ref in gc.get_referrers(at) \
                     if not (ref is path or \
                       inspect.isframe(ref) or \
                       (isinstance(ref, dict) and \
                        ref.keys() == locals().keys()) or \
                       ref is self.__dict__ or \
                       id(ref) in self.found) ]

        if (self.isManyRef(at, path, referrers)):
            return
            
        while(referrers):
            ref = referrers.pop()
            self.depth+=1
            for x in self.stepGenerator(depth + 1, path + [ref]):
                pass
            self.depth-=1
            pass
        pass

    def stepGenerator(self, depth, path):
        if self.shouldPrintStats:
            self.printStats(path)
            
            self.shouldPrintStats = False

        at = path[-1]

        if id(at) in self.found:
               # don't continue down this path
               raise StopIteration 

        # check for success
        if (self.isAtRoot(at, path)):
            raise StopIteration 

        # mark our progress after checking goal
        self.found.add(id(at))
        
        referrers = [ref for ref in gc.get_referrers(at) \
                     if not (ref is path or \
                       inspect.isframe(ref) or \
                       (isinstance(ref, dict) and \
                        ref.keys() == locals().keys()) or \
                       ref is self.__dict__ or \
                       id(ref) in self.found) ]

        if (self.isManyRef(at, path, referrers)):
            raise StopIteration 
            
        while(referrers):
            ref = referrers.pop()
            self.depth+=1
            for x in self.stepGenerator(depth + 1, path + [ref]):
                yield None
                pass
            self.depth-=1
            pass

        yield None
        pass
    
    def printStats(self, path):
        path = list(reversed(path))
        path.insert(0,0)
        print 'RefPath(%s) - Stats - found(%s) | depth(%s) | CurrentPath(%s)' % \
              (self._id, len(self.found), self.depth, ''.join(self.myrepr(path[x], path[x+1]) for x in xrange(len(path)-1)))
        pass
    
    def isAtRoot(self, at, path):
        # Now we define our 'roots'
        
        # __builtins__
        if at is __builtins__:
            sys.stdout.write("RefPath(%s): __builtins__-> " % self._id)
            path = list(reversed(path))
            path.insert(0,0)
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return True
        
        # any module scope
        if inspect.ismodule(at):
            sys.stdout.write("RefPath(%s): Module(%s)-> " % (self._id, at.__name__))
            path = list(reversed(path))
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return True

        # any class scope
        if inspect.isclass(at):
            sys.stdout.write("RefPath(%s): Class(%s)-> " % (self._id, at.__name__))
            path = list(reversed(path))
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return True

        # simbase
        if at is simbase:
            sys.stdout.write("RefPath(%s): simbase-> " % self._id)
            path = list(reversed(path))
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return True

        # simbase.air
        if at is simbase.air:
            sys.stdout.write("RefPath(%s): simbase.air-> " % self._id)
            path = list(reversed(path))
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return True
        
        # messenger
        if at is messenger:
            sys.stdout.write("RefPath(%s): messenger-> " % self._id)
            path = list(reversed(path))
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return True

        # taskMgr
        if at is taskMgr:
            sys.stdout.write("RefPath(%s): taskMgr-> " % self._id)
            path = list(reversed(path))
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return True

        # world
        if hasattr(simbase.air, 'mainWorld') and at is simbase.air.mainWorld:
            sys.stdout.write("RefPath(%s): mainWorld-> " % self._id)
            path = list(reversed(path))
            for x in xrange(len(path)-1):
                sys.stdout.write(self.myrepr(path[x], path[x+1]))
                pass
            print
            return True
        pass

        return False

    def isManyRef(self, at, path, referrers):
        if (len(referrers) > self.maxRefs and \
            at is not self.obj):
            if not isinstance(at, (list, tuple, dict, set)):
                sys.stdout.write("RefPath(%s): ManyRefs(%s)[%s]-> " % (self._id, len(referrers), fastRepr(at)))
                path = list(reversed(path))
                path.insert(0,0)
                for x in xrange(len(path)-1):
                    sys.stdout.write(self.myrepr(path[x], path[x+1]))
                    pass
                print
                return True
            else:
                sys.stdout.write("RefPath(%s): ManyRefsAllowed(%s)[%s]-> " % (self._id, len(referrers), fastRepr(at, maxLen = 1, strFactor = 30)))
                print                
                pass
            pass
        return False
    pass



"""
from direct.showbase.ReferrerSearch import ReferrerSearch
door = simbase.air.doFind("DistributedBuildingDoorAI")
class A: pass
door = A()
ReferrerSearch()(door)
reload(ReferrerSearch); from direct.showbase.PythonUtil import ReferrerSearch
"""
