from omniidl import idlast,idltype,idlutil,idlvisitor
import omniORB
from omniORB import CORBA
import sys
import string

omniORB.importIRStubs()

"""
If resultType is not used, then any new anonymous types made for it (array,
sequence, bound string etc.) need to be destroyed.

Need to worry about overriding inherited members.

Generally, what behaviour is correct when an object is already found in the
repository? Destroy and make a new one? Use the exiting one but iterate down
into it (certainly for ModuleDefs)? Leave the existing one unchanged? Exit
with an error?

It will be difficult or impossible to remove what is already there piecemeal.
Better then to add definitions when we find extra ones, but not to remove
those that already exist.

Or perhaps there should be a check first phase. Could use diff.

We could back-out the entire change if anything goes wrong. Keep a stack of
objects that we've created, and iterate backward through them to kill them all.

Need a blatt-everything mode.

Modules need to be treated differently, as they are reopenable.
Find out what else is reopenable. - Pretty sure it's just modules.
"""

##
## Global variables
##

visitor=None
ifr=None

##
## Global methods
##

def DB(debugLevel,message):
  """Print a debug message."""
  if(5>=debugLevel):
    print message

def obtainversion(node):
  """Calculate the version string, from node's Repository ID."""
  rid=node.repoId()
  pos=string.rfind(rid,":")
  return rid[pos+1:]

def expecting(irobj,expectedDefKind):
  """Checks that irobj.def_kind()==expectedDefKind"""
  defKind=irobj._get_def_kind()
  if(defKind!=expectedDefKind):
    sys.stderr.write(
      'Found %s, but expected %s.\n'%(defKind,expectedDefKind))
    sys.exit(1)

def nameToEnumItem(tc,name):
    """This function will get you a named enum item given a TypeCode.
    
    Inside a TypeCode is a type descriptor, which is a tuple (or a single
    integer in the case of basic types like long). In the case of enums,
    item 3 in that tuple is another tuple containing the enum items. You
    can test the enum items by name. 

    This clearly relies on the internal structures of omniORBpy, so in
    theory it could break with a new major release of omniORBpy. I don't
    see why this bit of it would ever change, though. You might want to
    raise an exception if the item is not found instead or returning None.
    """
    assert(tc.kind()==CORBA.tk_enum)
    descriptor=tc._d
    enums=descriptor[3]
    for e in enums:
      if(e._n==name):
        return e
    return None

## #############################################################################
## Helper classes
## #############################################################################

class ContainedWrapper:
  def __init__(self,node):
      self.ref=None
      self.id=node.repoId()
      self.name=node.identifier()
      self.version=obtainversion(node)

  def addToContainer(self,container):
      contained=container.lookup(self.name)
      if(contained):
        self.setRef(contained)
      else:
        self.create(container)
## end class ContainedWrapper


class ModuleWrapper(ContainedWrapper):
  """Converts an idllast.Module into CORBA.Repository parameter,
     ready for calling create_module()."""

  def __init__(self,node):
      ContainedWrapper.__init__(self,node)

  def create(self,container):
      assert(self.ref is None)
      DB(5,"Creating Module: "+self.id)
      self.ref=container.create_module(self.id,self.name,self.version)

  def setRef(self,contained):
      """Modules may be re-opened so we should never be surprised to find that
         a module already exists in the repository."""
      expecting(contained,CORBA.dk_Module)
      self.ref=contained._narrow(CORBA.ModuleDef)
      self.ref._set_id(self.id)
      self.ref._set_version(self.version)
## end class ModuleWrapper


class InterfaceWrapper(ContainedWrapper):
  """Converts an idllast.Interface into CORBA.Repository parameter,
     ready for calling create_interface().
     ?? Deal with abstract operations."""

  def __init__(self,node):
      ContainedWrapper.__init__(self,node)
      self.baseInterfaces=self.initBaseInterfaces(node)

  def initBaseInterfaces(self,node):
      self.baseInterfaces=[] # InterfaceDefSeq
      for i in node.inherits():
        rid=i.repoId()
        DB(10,"base+="+rid)
        baseInterface=ifr.lookup_id(rid)
        if(baseInterface):
          expecting(baseInterface,CORBA.dk_Interface) # ?? Perhaps local?
          self.baseInterfaces.append(baseInterface)
        else:
          sys.stderr.write(
            "Can't find base interface %s in Repository.\n"%rid)
          sys.exit(1)
      return self.baseInterfaces

  def create(self,container):
      assert(self.ref is None)
      DB(5,"Creating Interface: "+self.id)
      self.ref=container.create_interface(
          self.id, 
          self.name, 
          self.version,
          self.baseInterfaces)

  def setRef(self,contained):
      expecting(contained,CORBA.dk_Interface)
      self.ref=contained._narrow(CORBA.InterfaceDef)
      self.ref._set_id(self.id)
      self.ref._set_version(self.version)
      self.ref._set_base_interfaces(self.baseInterfaces)
## end class InterfaceWrapper


class OperationWrapper(ContainedWrapper):
  """Converts an idllast.Operation into CORBA.Repository parameters,
     ready for calling create_operation()."""

  def __init__(self,node):
      ContainedWrapper.__init__(self,node)
      self.result=self.initResult(node)
      self.mode=self.initMode(node)
      self.params=self.initParams(node)
      self.exceptions=self.initExceptions(node)
      self.contexts=node.contexts() # ?? May need to copy then one by one.

  def initResult(self,node):
      node.returnType().accept(visitor)
      self.result=visitor.resultType
      return self.result

  def initMode(self,node):
      if node.oneway():
        self.mode=CORBA.OP_ONEWAY
      else:
        self.mode=CORBA.OP_NORMAL
      return self.mode

  def initParams(self,node): 
      self.params=[]
      for p in node.parameters():
        if p.is_in() and p.is_out():
          paramMode=CORBA.PARAM_INOUT
        elif p.is_in():
          paramMode=CORBA.PARAM_IN
        else:
          paramMode=CORBA.PARAM_OUT             
        p.paramType().accept(visitor) # sets visitor.resultType
        self.params.append( CORBA.ParameterDescription( 
            p.identifier(), # name
            visitor.resultType._get_type(),
            visitor.resultType,
            paramMode )
        )
      return self.params

  def initExceptions(self,node):
      self.exceptions=[]
      for ex in node.raises():
        obj=ifr.lookup_id(ex.repoId())
        expecting(obj,CORBA.dk_Exception)
        self.exceptions.append(obj._narrow(CORBA.ExceptionDef))
      return self.exceptions

  def create(self,interface):
      assert(self.ref is None)
      DB(5,"Creating Operation: "+self.id)
      self.ref=interface.create_operation(
          self.id, 
          self.name,
          self.version,
          self.result,
          self.mode,
          self.params,
          self.exceptions,
          self.contexts)

  def setRef(self,contained):
      expecting(contained,CORBA.dk_Operation)
      self.ref=contained._narrow(CORBA.OperationDef)
      self.ref._set_id(self.id)
      self.ref._set_version(self.version)
      self.ref._set_result_def(self.result)
      self.ref._set_mode(self.mode)
      self.ref._set_params(self.params)
      self.ref._set_exceptions(self.exceptions)
      self.ref._set_contexts(self.contexts)
## end class OperationWrapper

class AttributesWrapper(ContainedWrapper):
  """Converts idllast.Attributes into CORBA.Repository parameters,
     ready for calling create_attribute()."""

  def __init__(self,node):
      self.ref=None
      self.id=None
      self.name=None
      self.version=None
      self.typeDef=self.initTypeDef(node)
      self.mode=self.initMode(node)

  def initTypeDef(self,node):
      """ ?? Call this once for each name, to avoid duplicate use
         of anonymous types."""
      node.attrType().accept(visitor)
      self.typeDef=visitor.resultType
      return self.typeDef

  def initMode(self,node):
      if node.readonly():
        self.mode=CORBA.ATTR_READONLY
      else:
        self.mode=CORBA.ATTR_NORMAL
      return self.mode
  
  def setDeclarator(self,declarator):
      self.id=declarator.repoId()
      self.name=declarator.identifier()
      self.version=obtainversion(declarator)

  def create(self,interface):
      assert(self.ref is None)
      DB(5,"Creating Attribute: "+self.id)
      self.ref=interface.create_attribute(
          self.id, 
          self.name,
          self.version,
          self.typeDef,
          self.mode)

  def setRef(self,contained):
      expecting(contained,CORBA.dk_Attribute)
      self.ref=contained._narrow(CORBA.AttributeDef)
      self.ref._set_id(self.id)
      self.ref._set_version(self.version)
      self.ref._set_type_def(self.typeDef)
      self.ref._set_mode(self.mode)
## end class AttributesWrapper


class EnumWrapper(ContainedWrapper):
  """Converts an idllast.Enum into CORBA.Repository parameter,
     ready for calling create_enum()."""

  def __init__(self,node):
      ContainedWrapper.__init__(self,node)
      self.members=[]
      for e in node.enumerators():
        self.members.append(e.identifier())

  def create(self,container):
      assert(self.ref is None)
      DB(5,"Creating Enum: "+self.id)
      self.ref=container.create_enum(
          self.id, 
          self.name, 
          self.version,
          self.members)

  def setRef(self,contained):
      expecting(contained,CORBA.dk_Enum)
      self.ref=contained._narrow(CORBA.EnumDef)
      self.ref._set_id(self.id)
      self.ref._set_version(self.version)
      self.ref._set_members(self.members)
## end class EnumWrapper


class UnionWrapper(ContainedWrapper):
  """Converts an idllast.Union into CORBA.Repository parameter,
     ready for calling create_union()."""

  def __init__(self,node):
      ContainedWrapper.__init__(self,node)
      self.discriminatorTypeDef=self.initDiscriminatorType(node)

  def initDiscriminatorType(self,node):
      if node.constrType():
        node.switchType().decl().accept(visitor)
      else:
        node.switchType().accept(visitor)
      self.discriminatorTypeDef=visitor.resultType
      return self.discriminatorTypeDef

  def setMembers(self,node):
      """Cannot set members until the definition itself has been created.
         This is because members might refer to contained definitions, which
         can only be created AFTER the union."""
      visitor.push(self.ref)
      members=[] # UnionMemberSeq
      for c in node.cases():
        if c.constrType():
          c.caseType().decl().accept(visitor)
        for l in c.labels():
          discriminatorTc=self.discriminatorTypeDef._get_type()
          if(l.default()):
            label=CORBA.Any(CORBA._tc_octet,0)
          elif(discriminatorTc.kind()==CORBA.tk_enum):
            # ?? Sleight of hand using omniORBpy internals instead of DynEnum.
            enumItem=nameToEnumItem(discriminatorTc,l.value().identifier())
            label=CORBA.Any(discriminatorTc,enumItem)
          else:
            label=CORBA.Any(discriminatorTc,l.value())
          c.caseType().accept(visitor)   # Inside loop to make a new copy of
          c.declarator().accept(visitor) # anonymous type for each member.
          members.append( CORBA.UnionMember(
              c.declarator().identifier(), 
              label,
              CORBA._tc_void,    # type (must be void)
              visitor.resultType # type_def
          ))
      visitor.pop()
      self.ref._set_members(members)

  def create(self,container):
      assert(self.ref is None)
      DB(5,"Creating Union: "+self.id)
      self.ref=container.create_union(
          self.id, 
          self.name, 
          self.version,
          self.discriminatorTypeDef,
          [] # members
      )

  def setRef(self,contained):
      expecting(contained,CORBA.dk_Union)
      self.ref=contained._narrow(CORBA.UnionDef)
      self.ref._set_id(self.id)
      self.ref._set_version(self.version)
      self.ref._set_discriminator_type_def(self.discriminatorTypeDef)
## end class UnionWrapper


class ConstWrapper(ContainedWrapper):
  """Converts an idllast.Const into CORBA.Repository parameter,
     ready for calling create_constant()."""

  def __init__(self,node):
      ContainedWrapper.__init__(self,node)
      self.typeDef=self.initTypeDef(node)
      self.value=self.initValue(node)

  def initTypeDef(self,node):
      node.constType().accept(visitor)
      self.typeDef=visitor.resultType
      return self.typeDef

  def initValue(self,node):
      self.value=CORBA.Any(self.typeDef._get_type(),node.value())
      return self.value

  def create(self,container):
      assert(self.ref is None)
      DB(5,"Creating Const: "+self.id)
      self.ref=container.create_constant(
          self.id, 
          self.name, 
          self.version,
          self.typeDef,
          self.value)

  def setRef(self,contained):
      expecting(contained,CORBA.dk_Constant)
      self.ref=contained._narrow(CORBA.ConstantDef)
      self.ref._set_id(self.id)
      self.ref._set_version(self.version)
      self.ref._set_type_def(self.typeDef)
      self.ref._set_value(self.value)
## end class ConstWrapper


class ExceptionWrapper(ContainedWrapper):
  """Converts an idllast.Exception into CORBA.Repository parameter,
     ready for calling create_exception()."""

  def __init__(self,node):
      ContainedWrapper.__init__(self,node)

  def setMembers(self,node):
      """Cannot set members until the definition itself has been created.
         This is because members might refer to contained definitions, which
         can only be created AFTER the exception."""
      visitor.push(self.ref)
      members=[]
      for m in node.members():
        for d in m.declarators():
          if m.constrType():
            m.memberType().decl().accept(visitor)
          m.memberType().accept(visitor) # sets visitor.resultType
          d.accept(visitor) # May create array(s)
          members.append(
            CORBA.StructMember(
              d.identifier(),    # name
              CORBA._tc_void,    # type (must be void)
              visitor.resultType # type_def
          ) )
      visitor.pop()
      self.ref._set_members(members)

  def create(self,container):
      assert(self.ref is None)
      DB(5,"Creating Exception: "+self.id)
      self.ref=container.create_exception(
          self.id, 
          self.name, 
          self.version,
          [] # members
      )

  def setRef(self,contained):
      expecting(contained,CORBA.dk_Exception)
      self.ref=contained._narrow(CORBA.ExceptionDef)
      self.ref._set_id(self.id)
      self.ref._set_version(self.version)
## end class ExceptionWrapper


class StructWrapper(ContainedWrapper):
  """Converts an idllast.Struct into CORBA.Repository parameter,
     ready for calling create_struct()."""

  def __init__(self,node):
      ContainedWrapper.__init__(self,node)

  def setMembers(self,node):
      """Cannot set members until the definition itself has been created.
         This is because members might refer to contained definitions, which
         can only be created AFTER the struct."""
      visitor.push(self.ref)
      members=[]
      for m in node.members():
        for d in m.declarators():
          if m.constrType():
            m.memberType().decl().accept(visitor)
          m.memberType().accept(visitor) # sets visitor.resultType
          d.accept(visitor) # May create array(s)
          members.append(
            CORBA.StructMember(
              d.identifier(),    # name
              CORBA._tc_void,    # type (must be void)
              visitor.resultType # type_def
          ) )
      visitor.pop()
      self.ref._set_members(members)

  def create(self,container):
      assert(self.ref is None)
      DB(5,"Creating Struct: "+self.id)
      self.ref=container.create_struct(
          self.id, 
          self.name, 
          self.version,
          [] # members
      )

  def setRef(self,contained):
      expecting(contained,CORBA.dk_Struct)
      self.ref=contained._narrow(CORBA.StructDef)
      self.ref._set_id(self.id)
      self.ref._set_version(self.version)
## end class StructWrapper


## #############################################################################
## Main visitor class
## #############################################################################

class IfrVisitor:
  def __init__(self,args):
      self.orb=CORBA.ORB_init(args)
      global ifr
      ifr=self.resolveIfr()
      self.stack=[ifr] # Current containment stack
      self.forwards={} # Map of forward declared objects, indexed by RID.
      self.resultType=None

  def resolveIfr(self):
      """Helper function resolves InterfaceRepository initial ref."""
      try:
        obj=self.orb.resolve_initial_references("InterfaceRepository")
        ifr=obj._narrow(CORBA.Repository)
        return ifr
      except CORBA.ORB.InvalidName, ex:
        sys.stderr.write('InitRef InterfaceRepository not defined.\n')
        sys.exit(1)

  def push(self,container):
      """Appends container to the back of the containment stack."""
      assert(container)
      self.stack.append(container)

  def pop(self):
      """Pops the back of the containment stack."""
      assert(self.stack)
      del self.stack[-1]
      assert(self.stack) # IFR should always be left of the stack.

  def container(self):
      """Obtains the last container on the containment stack."""
      assert(self.stack)
      return self.stack[-1]
        

  ## Visit methods #############################################################
  
  def visitAST(self,node):
      for n in node.declarations():
        n.accept(self)

  def visitModule(self,node):
      moduleWrapper=ModuleWrapper(node)
      moduleWrapper.addToContainer(self.container())
      self.push(moduleWrapper.ref)
      for n in node.definitions():
        n.accept(self)
      self.pop()

  def visitInterface(self,node):
      interfaceWrapper=InterfaceWrapper(node)
      interfaceWrapper.addToContainer(self.container())
      # Add contained objects.
      self.push(interfaceWrapper.ref)
      for n in node.contents():
        n.accept(self)
      self.pop()

  def visitForward(self,node):
      """??? Doesn't yet deal with abstract interfaces."""
      contained=self.container().lookup(node.identifier())
      if(not contained):
        DB(7,"Creating forward Interface: "+node.repoId())
        self.container().create_interface(
            node.repoId(),node.identifier(),obtainversion(node),[])

  def visitConst(self,node):
      constWrapper=ConstWrapper(node)
      constWrapper.addToContainer(self.container())

  def visitTypedef(self,node):
      """Make one alias for each declarator."""
      for d in node.declarators():
        contained=self.container().lookup(d.identifier())
        if(contained): # this name already exists.
          expecting(contained,CORBA.dk_Alias)
        else:
          if node.constrType():                   # ?? Should this bit be
            node.aliasType().decl().accept(self)  #    outside the loop??
          node.aliasType().accept(self)
          d.accept(self)
          self.container().create_alias(
            d.repoId(),
            d.identifier(),
            obtainversion(d),
            self.resultType
          )

  def visitStruct(self, node):
      structWrapper=StructWrapper(node)
      structWrapper.addToContainer(self.container())
      structWrapper.setMembers(node)

  def visitStructForward(self, node):
      contained=self.container().lookup(node.identifier())
      if(not contained):
        DB(7,"Creating forward Struct: "+node.repoId())
        self.container().create_struct(
            node.repoId(),node.identifier(),obtainversion(node),[])

  def visitException(self, node):
      exceptionWrapper=ExceptionWrapper(node)
      exceptionWrapper.addToContainer(self.container())
      exceptionWrapper.setMembers(node)

  def visitUnion(self, node):
      unionWrapper=UnionWrapper(node)
      unionWrapper.addToContainer(self.container())
      unionWrapper.setMembers(node)

  def visitUnionForward(self, node):
      contained=self.container().lookup(node.identifier())
      if(not contained):
        DB(7,"Creating forward Union: "+node.repoId())
        self.container().create_struct(
            node.repoId(),node.identifier(),obtainversion(node),
            ifr.get_primitive(CORBA.pk_char),[]) # dummy values

  def visitEnum(self, node):
      enumWrapper=EnumWrapper(node)
      enumWrapper.addToContainer(self.container())

  def visitAttribute(self, node):
      attributesWrapper=AttributesWrapper(node)
      # Make the attribute, if required.
      expecting(self.container(),CORBA.dk_Interface)
      interface=self.container()._narrow(CORBA.InterfaceDef)
      for d in node.declarators():
        attributesWrapper.setDeclarator(d)
        attributesWrapper.addToContainer(interface)

  def visitOperation(self, node):
      operationWrapper=OperationWrapper(node)
      # Make the operation, if required.
      expecting(self.container(),CORBA.dk_Interface)
      interface=self.container()._narrow(CORBA.InterfaceDef)
      operationWrapper.addToContainer(interface)

  def visitNative(self, node):
      """Not yet supported by omniIFR."""
      contained=self.container().lookup(node.identifier())
      if(not contained):
        DB(5,"Creating Native: "+node.repoId())
        self.container().create_native(
            node.repoId(),node.identifier(),obtainversion(node))

  def visitDeclarator(self,node):
      """Wraps self.resultType in an array, if necessary."""
      for s in node.sizes():
        self.resultType=ifr.create_array(s,self.resultType)

  ## Type visitors #############################################################

  ttsMap={
    idltype.tk_void:       CORBA.pk_void,
    idltype.tk_short:      CORBA.pk_short,
    idltype.tk_long:       CORBA.pk_long,
    idltype.tk_ushort:     CORBA.pk_ushort,
    idltype.tk_ulong:      CORBA.pk_ulong,
    idltype.tk_float:      CORBA.pk_float,
    idltype.tk_double:     CORBA.pk_double,
    idltype.tk_boolean:    CORBA.pk_boolean,
    idltype.tk_char:       CORBA.pk_char,
    idltype.tk_octet:      CORBA.pk_octet,
    idltype.tk_any:        CORBA.pk_any,
    idltype.tk_TypeCode:   CORBA.pk_TypeCode,
    idltype.tk_Principal:  CORBA.pk_Principal,
    idltype.tk_longlong:   CORBA.pk_longlong,
    idltype.tk_ulonglong:  CORBA.pk_ulonglong,
    idltype.tk_longdouble: CORBA.pk_longdouble,
    idltype.tk_wchar:      CORBA.pk_wchar,
    idltype.tk_objref:     CORBA.pk_objref
  }

  def visitBaseType(self,typeNode):
      primitiveKind=self.ttsMap[typeNode.kind()]
      self.resultType=ifr.get_primitive(primitiveKind)

  def visitStringType(self,typeNode):
      bound=typeNode.bound()
      if(bound):
        self.resultType=ifr.create_string(bound)
      else:
        self.resultType=ifr.get_primitive(CORBA.pk_string)

  def visitWStringType(self,typeNode):
      bound=typeNode.bound()
      if(bound):
        self.resultType=ifr.create_wstring(bound)
      else:
        self.resultType=ifr.get_primitive(CORBA.pk_wstring)

  def visitSequenceType(self,typeNode):
      typeNode.seqType().accept(self)
      bound=typeNode.bound()
      self.resultType=ifr.create_sequence(bound,self.resultType)

  def visitFixedType(self,typeNode):
      self.resultType=\
        ifr.create_fixed(typeNode.digits(),typeNode.scale())

  def visitDeclaredType(self,typeNode):
      if(self.ttsMap.has_key(typeNode.kind())):
        # omniidl incorrectly treats Object (&Value?) as Declared types
        # rather than base types, so redirect...
        self.visitBaseType(typeNode)
      else:
        absoluteName='::'+idlutil.ccolonName(typeNode.decl().scopedName())
        DB(10,'IFR.lookup("%s")'%absoluteName)
        self.resultType=ifr.lookup(absoluteName)
        if(not self.resultType):
          sys.stderr.write("Can't find %s in the Repository.\n"%absoluteName)
          sys.exit(1)
        # Expecting an IDLType here.
        idltype=self.resultType._narrow(CORBA.IDLType)
        if(not idltype):
          sys.stderr.write('Expected %s to be an IDLType, got a %s.\n'%
            (absoluteName,self.resultType._get_def_kind()))
          sys.exit(1)

## end class IfrVisitor

## #############################################################################
## Entry point, called by omniidl
## #############################################################################

def run(tree, args):
  """Entry point, called by omniidl."""
  global visitor
  visitor=IfrVisitor(args)
  tree.accept(visitor)
