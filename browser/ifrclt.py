#! /usr/bin/env python

import sys, os, os.path
import getopt
import string
import omniORB
from omniORB import CORBA, PortableServer
import CosNaming

omniORB.importIRStubs()

##
## Global variables
##

#global orb,ifr
#orb=None
#ifr=None

wrapThreshold=2

def reference(tc):
  """Render a reference to type tc (?? - ignores context)."""
  kind=tc.kind()
  if(kind==CORBA.tk_objref or
     kind==CORBA.tk_struct or
     kind==CORBA.tk_union  or
     kind==CORBA.tk_enum   or
     kind==CORBA.tk_alias  or
     kind==CORBA.tk_except
  ):
    return tc.name()
  elif(kind==CORBA.tk_void or
       kind==CORBA.tk_short or
       kind==CORBA.tk_long or
       kind==CORBA.tk_float or
       kind==CORBA.tk_double or
       kind==CORBA.tk_boolean or
       kind==CORBA.tk_char or
       kind==CORBA.tk_octet or
       kind==CORBA.tk_any or
       kind==CORBA.tk_TypeCode or
       kind==CORBA.tk_string or
       kind==CORBA.tk_wstring
  ):
    return str(kind)[9:]
  elif(kind==CORBA.tk_ushort or
       kind==CORBA.tk_ulong
  ):
    return 'unsigned '+str(kind)[10:]
  elif(kind==CORBA.tk_longlong):
    return 'long long'
  elif(kind==CORBA.tk_ulonglong):
    return 'unsigned long long'
  elif(kind==CORBA.tk_sequence):
    return 'sequence<'+reference(tc.content_type())+'>'
  else:
    return str(kind)

def caseLabel(any):
  """Renders a CORBA.Any to a case label string. This is identical to
     literal() except that Octet(0) is rendered as the keyword 'default'.
  """
  if(any.typecode().kind()==CORBA.tk_octet and any.value()==0):
    return 'default:'
  else:
    return 'case '+literal(any)+':'

def literal(any):
  """Renders a CORBA.Any to a literal value string."""
  tc=any.typecode()
  kind=tc.kind()
  while(kind==CORBA.tk_alias):
    tc=tc.content_type()
    kind=tc.kind()

  if(
    kind==CORBA.tk_enum   or
    kind==CORBA.tk_short or
    kind==CORBA.tk_long or
    kind==CORBA.tk_float or
    kind==CORBA.tk_double or
    kind==CORBA.tk_boolean or
    kind==CORBA.tk_octet or
    kind==CORBA.tk_ushort or
    kind==CORBA.tk_ulong or
    kind==CORBA.tk_longlong or
    kind==CORBA.tk_ulonglong
  ):
    return str(any.value())
  elif(
    kind==CORBA.tk_string or
    kind==CORBA.tk_wstring
  ):
    return '"'+str(any.value())+'"'
  elif(
    kind==CORBA.tk_char or
    kind==CORBA.tk_wchar
  ):
    return '"'+str(any.value())+'"'
  else:
    return 'Any:'+str(kind)


class Output:
  def __init__(self,levels,detail):
      self.indent=0
      self.levels=levels
      self.detail=detail

  def listContents(self,object):
      self.contents(object)

  def showContained(self,object):
      dk=object._get_def_kind()
      if(self.__class__.__dict__.has_key(str(dk))):
        self.__class__.__dict__[str(dk)](self,object)
      else:
        self.dk_all(seq[object])

  def contents(self,object):
      """List the contents of Container object. Iterate 'self.levels' times.
      """
      self.indent=self.indent+1
      levels=self.levels
      if(levels>0):
        self.levels=levels-1
      container=object._narrow(CORBA.Container)
      if(container):
        seq=container.contents(CORBA.dk_all,1)
        for i in range(0,len(seq)):
          self.showContained(seq[i])
      self.levels=levels # Restore
      self.indent=self.indent-1

  def narrow(self,contained,to):
      narrowed=contained._narrow(to)
      if(narrowed):
        return narrowed
      else:
        print "ERROR object doesn't match def_kind:",to
        self.dk_all(contained)
        return None

  def dk_all(self,contained):
      print '  '*self.indent, \
        contained._get_name(),contained._get_def_kind(),contained._get_id()
      if(self.levels):
        self.contents(contained)

  def dk_Interface(self,contained):
      interface=self.narrow(contained,CORBA.InterfaceDef)
      if(interface):
        idl='  '*self.indent+' interface '+interface._get_name()
        if(self.levels):
          baseInterfaces=interface._get_base_interfaces()
          if(len(baseInterfaces)>0):
            idl+=' :'
            for i in range(len(baseInterfaces)):
              if(i>0):
                idl+=','
              idl+='\n     '+'  '*self.indent+baseInterfaces[i]._get_name()
          print idl,'{'
          self.contents(contained)
          print '  '*self.indent,'};'
        else:
          print idl,'... ;'

  def dk_Operation(self,contained):
      op=self.narrow(contained,CORBA.OperationDef)
      if(op):
        idl=' '+'  '*self.indent
        if(op._get_mode()==CORBA.OP_ONEWAY):
          idl+='oneway '
        idl+=reference(op._get_result())+' '
        idl+=op._get_name()+'('
        if(self.detail):
          # Params
          params=op._get_params()
          for p in range(0,len(params)):
            if(p>0):
              idl+=', '
            if(len(params)>wrapThreshold):
              idl+='\n '+'  '*(2+self.indent)
            if(params[p].mode==CORBA.PARAM_IN):
              idl+='in '
            elif(params[p].mode==CORBA.PARAM_OUT):
              idl+='out '
            else:
              idl+='in out '
            idl+=reference(params[p].type)+' '+params[p].name
          # Exceptions
          exceptions=op._get_exceptions()
          if(len(exceptions)):
            idl+=')\n '+'  '*self.indent+'  raises('
          for e in range(0,len(exceptions)):
            if(e>0):
              idl+=', '
            if(len(exceptions)>wrapThreshold):
              idl+='\n '+'  '*(2+self.indent)
            idl+=reference(exceptions[e]._get_type())
        else:
          idl+=' ... '
        idl+=');'
        print idl

  def dk_Enum(self,contained):
      enum=self.narrow(contained,CORBA.EnumDef)
      if(enum):
        idl=' '+'  '*self.indent+'enum '+enum._get_name()+' { '
        if(self.detail):
          members=enum._get_members()
          for m in range(0,len(members)):
            if(m>0):
              idl+=', '
            if(len(members)>wrapThreshold):
              idl+='\n '+'  '*(2+self.indent)
            idl+=members[m]
        else:
          idl+='...'
        idl+=' };'
        print idl

  def dk_Exception(self,contained):
      ex=self.narrow(contained,CORBA.ExceptionDef)
      if(ex):
        if(self.detail):
          print '  '*self.indent,'exception',ex._get_name(),'{'
          if(self.levels):
            self.contents(ex)
          members=ex._get_members()
          for m in range(0,len(members)):
            print '  '*(1+self.indent),reference(members[m].type),members[m].name+';'
          print '  '*self.indent,'};'
        elif(self.levels):
          print '  '*self.indent,'exception',ex._get_name(),'{'
          self.contents(ex)
          print '  '*self.indent,' ... };'
        else:
          print '  '*self.indent,'exception',ex._get_name(),'{ ... };'

  def dk_Struct(self,contained):
      struct=self.narrow(contained,CORBA.StructDef)
      if(struct):
        if(self.detail):
          print '  '*self.indent,'struct',struct._get_name(),'{'
          if(self.levels):
            self.contents(struct)
          members=struct._get_members()
          for m in range(0,len(members)):
            print '  '*(1+self.indent),reference(members[m].type),members[m].name+';'
          print '  '*self.indent,'};'
        elif(self.levels):
          print '  '*self.indent,'struct',struct._get_name(),'{'
          self.contents(struct)
          print '  '*self.indent,' ... };'
        else:
          print '  '*self.indent,'struct',struct._get_name(),'{ ... };'

  def dk_Union(self,contained):
      union=self.narrow(contained,CORBA.UnionDef)
      if(union):
        if(self.detail):
          print '  '*self.indent,'union',union._get_name()
          print '  '*self.indent,'  switch(',reference(union._get_discriminator_type()),')'
          print '  '*self.indent,'{'
          self.contents(union)
          members=union._get_members()
          for m in range(0,len(members)):
            print '  '*(1+self.indent),caseLabel(members[m].label)
            print '  '*(2+self.indent),reference(members[m].type_def._get_type()),members[m].name+';'
          print '  '*self.indent,'};'
        elif(self.levels):
          print '  '*self.indent,'union',union._get_name(),'... {'
          self.contents(union)
          print '  '*self.indent,' ... };'
        else:
          print '  '*self.indent,'union',union._get_name(),'... { ... };'

  def dk_Module(self,contained):
      module=self.narrow(contained,CORBA.ModuleDef)
      if(module):
        if(self.levels):
          print '  '*self.indent,'module',module._get_name(),'{'
          self.contents(module)
          print '  '*self.indent,'};'
        else:
          print '  '*self.indent,'module',module._get_name(),'{ ... };'

  def dk_Alias(self,contained):
      alias=self.narrow(contained,CORBA.AliasDef)
      if(alias):
        print '  '*self.indent,'typedef', \
          reference(alias._get_original_type_def()._get_type()), \
          alias._get_name()+';'
 
  def dk_Attribute(self,contained):
      attr=self.narrow(contained,CORBA.AttributeDef)
      if(attr):
        if(attr._get_mode()==CORBA.ATTR_READONLY):
          readonly='readonly '
        else:
          readonly=''
        print '  '*self.indent,readonly+'attribute', \
          reference(attr._get_type()),attr._get_name()+';'

  def dk_Constant(self,contained):
      const=self.narrow(contained,CORBA.ConstantDef)
      if(const):
        print '  '*self.indent,'const', \
          reference(const._get_type()), \
          const._get_name(),'=',literal(const._get_value()),';'
 
# end class Output


class Current:
  def __init__(self,levels,detail):
      self.levels=levels # How many levels to recurse.
      self.detail=detail # If true -> output full info on types.
      if(os.environ.has_key('IFRBROWSERSTATE')):
        self.fname=os.environ['IFRBROWSERSTATE']
      else:
        self.fname=os.path.join(os.environ['HOME'],'.ifrbrowserstate')
      self.rid,self.container =self._initContainer()
  
  def _execute(self,argv):
      if(not argv):
        sys.stderr.write('No command.\n')
        sys.exit(1)
      command=argv[0]
      if(self.__class__.__dict__.has_key(command)):
        self.__class__.__dict__[command](self,argv)
      else:
        sys.stderr.write('Unknown command: %s\n'%command)

  def _initContainer(self):
      rid=''
      object=None
      try:
        ifrbrowserstate=open(self.fname)
        rid=string.strip(ifrbrowserstate.readline())
        ifrbrowserstate.close()
        object=ifr.lookup_id(rid)
      except:
        pass
      if(object):
        object=object._narrow(CORBA.Container)
      if(not object):
        rid=''
        object=ifr
      return rid,object

  def _toContained(self):
      """Narrows self.container to a Contained, if possible."""
      contained=self.container._narrow(CORBA.Contained)
      if(not contained):
        sys.stderr.write('Not contained.\n')
        sys.exit(1)
      else:
        return contained
  
  def _save(self,rid=None):
      if(rid is None):
        rid=self.rid
      ifrbrowserstate=open(self.fname,'w')
      ifrbrowserstate.write(self.rid+'\n')
      ifrbrowserstate.close()

  def cd(self,argv):
      if(len(argv)<2 or argv[1]=='::'):
        self.rid=''
        self._save()
        return

      if(argv[1]=='..'):
        contained=self._toContained()
        self.container=contained._get_defined_in()
      else:
        searchName=argv[1]
        contained=self.container.lookup(searchName)
        if(not contained):
          sys.stderr.write('%s - no such Contained.\n'%searchName)
          sys.exit(1)
        self.container=contained._narrow(CORBA.Container)
        if(not self.container):
          sys.stderr.write('%s - not a Container.\n'%searchName)
          sys.exit(1)
      try:
        self.rid=self.container._get_id()
      except:
        self.rid=''
      self._save()
  
  def ls(self,argv):
      if(len(argv)<2):
        Output(self.levels,self.detail).listContents(self.container)
      else:
        searchName=argv[1]
        contained=self.container.lookup(searchName)
        if(not contained):
          sys.stderr.write('%s - no such Contained.\n'%searchName)
          sys.exit(1)
        Output(self.levels+1,self.detail).showContained(contained)

  def pwd(self,argv):
      if(not self.rid):
        print '::'
      else:
        contained=self._toContained()
        print contained._get_absolute_name()

def usage():
  print """
syntax: %s [OPTIONS] [COMMAND] [ARGS]

  A very simple interface repository browser. Modelled on
  shell directory listing commands: cd, ls, pwd.

  OPTIONS
   -l       --long            ls: detailed output.
   -rLEVELS --recurse=LEVELS  ls: Drill down LEVELS.               [default: 0]
                              Set to -1 to recurse without limit.
  COMMANDS
   pwd              Show the current scope
   cd SCOPED_NAME   Change the current scope, relative to the current scope.
   cd ..            Change the current scope, up one level.
   cd               Change to the root scope (the Interface Repository itself).
   ls               List the contents of the current scope.
   ls SCOPED_NAME   List the contents of the named definition.

  The current scope is stored in the file pointed to by environment
  variable IFRBROWSERSTATE [default: $HOME/.ifrbrowserstate].
"""%sys.argv[0]
  sys.exit(1)


###############################################################################
if __name__=='__main__':
  global orb,ifr
  orb=CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
  obj=orb.resolve_initial_references("InterfaceRepository")
  ifr=obj._narrow(CORBA.Repository)

  detailedOutput=0
  recurseLevels=0
  try:
    opts,argv=getopt.getopt(sys.argv[1:],'hlr:',['help','long','recurse='])
  except getopt.error:
    # print help information and exit:
    usage()
    sys.exit(2)
  for option, optarg in opts:
    if option in ("-h", "--help"):
      usage()
      sys.exit(0)
    elif option in ("-l", "--long"):
      detailedOutput=1
    elif option in ("-r", "--recurse"):
      recurseLevels=int(optarg)
    else:
      usage()
      sys.exit(1)
  current=Current(recurseLevels,detailedOutput)
  current._execute(argv)
  
