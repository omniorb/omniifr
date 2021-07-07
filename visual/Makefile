# Makefile for Visual 8.0 (2005)

PRG = omniifr

DEFINES = \
	-D WIN32 \
	-D NDEBUG \
	-D _CONSOLE \
	-D _CRT_SECURE_NO_WARNINGS \
	-D HAVE_CONFIG_H \
	-D __x86__ \
	-D __WIN32__ \
	-D ENABLE_CLIENT_IR_SUPPORT \
	# end of list

INCLUDES = -I $(ORB_INC)

CFLAGS = -nologo -W3 -Od -EHa -GR -MD -Zi $(DEFINES) $(INCLUDES)

LDFLAGS = -nologo -nodefaultlib -incremental:no -debug

LIBS = kernel32.lib advapi32.lib msvcrt.lib msvcprt.lib -libpath:$(ORB_LIB) omnithread_rt.lib omniORB4_rt.lib omniDynamic4_rt.lib

HDRS = \
	AliasDef.h \
	ArrayDef.h \
	AttributeDef.h \
	config.h \
	ConstantDef.h \
	Contained.h \
	Container.h \
	Creche.h \
	daemon.h \
	daemon_windows.h \
	Dependency.h \
	EnumDef.h \
	ExceptionDef.h \
	FixedDef.h \
	IdentifierUtil.h \
	idltype.h \
	InterfaceDef.h \
	IRObject.h \
	main.h \
	ModuleDef.h \
	NativeDef.h \
	OperationDef.h \
	Persist.h \
	PersistNode.h \
	PrimitiveDef.h \
	Repository.h \
	scour.h \
	SequenceDef.h \
	string_to.h \
	StringDef.h \
	StructDef.h \
	TypedefDef.h \
	UnionDef.h \
	WstringDef.h \
	# end of list

SRCS = \
	AliasDef.cc \
	ArrayDef.cc \
	AttributeDef.cc \
	ConstantDef.cc \
	Contained.cc \
	Container.cc \
	daemon_windows.cc \
	EnumDef.cc \
	ExceptionDef.cc \
	FixedDef.cc \
	InterfaceDef.cc \
	IRObject.cc \
	main.cc \
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
	usage.cc \
	WstringDef.cc \
	# end of list

OBJS = $(SRCS:.cc=.obj)

EXE = $(PRG).exe

{..\..\repository\}.cc.obj:
	$(CC) $(CFLAGS) -c $<

.PHONY: all

all: $(EXE)

# Link with embedded manifest
$(EXE): $(OBJS)
	link.exe $(LDFLAGS) $(LIBS) $(OBJS) -out:$(EXE) -pdb:$(PRG).pdb
	mt.exe -nologo -manifest $(EXE).manifest -outputresource:$(EXE);1
