CC_FILES_LIB := \
  AliasDef.cc \
  ArrayDef.cc \
  AttributeDef.cc \
  ConstantDef.cc \
  Contained.cc \
  Container.cc \
  EnumDef.cc \
  ExceptionDef.cc \
  FixedDef.cc \
  IRObject.cc \
  InterfaceDef.cc \
  ModuleDef.cc \
  OperationDef.cc \
  Persist.cc \
  PersistNode.cc \
  PrimitiveDef.cc \
  Repository.cc \
  SequenceDef.cc \
  StringDef.cc \
  StructDef.cc \
  TypedefDef.cc \
  UnionDef.cc \
  WstringDef.cc \

CC_FILES_EXE := \
  main.cc \
  usage.cc \

# Files required to use OmniIfr::Repository from a library.
H_FILES_LIB := \
 config.h \
 Container.h \
 IRObject.h \
 Persist.h \
 Repository.h \
 scour.h \

ifneq ($(BUILD_FOR_WINDOWS),)
  CC_FILES_EXE += daemon_windows.cc
else
  CC_FILES_EXE += daemon_unix.cc
endif

OBJ_FILES_LIB := $(patsubst %.cc,repository/%.$(OBJEXT),$(CC_FILES_LIB))
OBJ_FILES_EXE := $(patsubst %.cc,repository/%.$(OBJEXT),$(CC_FILES_EXE))

OBJ_FILES     += $(OBJ_FILES_LIB) $(OBJ_FILES_EXE)
