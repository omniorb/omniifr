#! /bin/env python
#                            Package   : omniEvents
# Makefile                   Created   : 2004/08/04
#                            Author    : Alex Tingle
#
#    Copyright (C) 2004 Alex Tingle.
#
#    This file is part of the omniEvents application.
#
#    omniEvents is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    omniEvents is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

import sys
import re
import os.path
import string
import getopt

class MakeDep:
  """Calculates a 'make' dependency rule for a single file. Sends the result
     to standard out."""

  def __init__(self):
      self.targetExt=['.o']
      self.objDir=''
      # found & visited are used as sets. The value is not really important.
      # However, we use the value to store the name of the file that contained
      # the #incliude - useful for diagnostics.
      self.found={}
      self.visited={}
      self.includedirs=[]
      # Only search for includes
      self.reInclude=re.compile('^\s*\#\s*include\s+"([^"]+)"')
      self.reExclude=None
        
  def process(self,srcfile):
      """Scans srcfile for #includes, and then follows all the links it finds.
         Builds a make format dependency list for the srcfile, and sends it to
         stdout."""
      self.found={}
      self.visited={}
      self.readfile(srcfile)
      while(len(self.found)>0):
        fname=self.found.keys()[0] # set fname to first key
        self.visited[fname]=self.found[fname]
        del self.found[fname]
        self.readfile(fname)
      # Display the result
      srcDir,srcBase=os.path.split(srcfile)
      if not self.objDir:
        self.objDir=srcDir
      srcRoot,unused=os.path.splitext(srcBase)
      targets=[]
      for ext in self.targetExt:
        targets.append(os.path.join(self.objDir,srcRoot+ext))
      print string.join(targets,' ')+': \\\n  '+srcfile+' \\'
      for fname in self.visited.keys():
        print ' ',fname,'\\'
      print ''
      # Add useful diagnostics to the end of the file.
      print '## Diagnostic information starts here'
      for fname in self.visited.keys():
        print '# %s  from  %s'%(fname,self.visited[fname])
      print ''

  def readfile(self,fname):
      """Scans fname for local #includes (system includes with <> are ignored).
         Searches for each included file in the includedirs. If a found file
         has not yet been visited, then it is added to the self.found set (to
         be scanned later). Missing files with simple names (no '/') are
         assumed to be generated files in the source directory - they are just
         added to the self.visited set, without being scanned."""
      try:
        file=open(fname,'r')
      except:
        sys.stderr.write('Cannot open file %s\n'%fname)
        sys.exit(1)
      while 1:
        line=file.readline()
        if(not line): break ## <======================= loop exit point is here
        match=self.reInclude.match(line)
        if(match):
          found=match.group(1) # 'found' is now the filename: #include "found"
          if(self.reExclude and self.reExclude.search(found)):
            continue # skip #includes that match the exclude pattern
          incfile=self.findfile(found,os.path.dirname(fname))
          if(incfile):
            if(not self.visited.has_key(incfile)):
              self.found[incfile]=fname
          elif(string.find(found,'/')==-1): # found does NOT contain '/'.
            self.visited[os.path.join(os.path.dirname(fname),found)]=fname
          else:
            self.visited[found]=fname
      file.close()

  def findfile(self,fname,sourcedir):
      """Searches self.includedirs and sourcedir for fname, and returns the
         full pathname if a file is found. Otherwise returns None."""
      for includedir in [sourcedir]+self.includedirs:
        fullpath=os.path.join(includedir,fname)
        if(os.path.isfile(fullpath)):
          return os.path.normpath(fullpath) # strip off initial './'
      return None

def usage():
  """Emit help text."""
  print sys.argv[0],"""[OPTIONS] [FILE]
%s
OPTIONS
 -I<path> --include=<path>          add <path> to the search list.
 -o<ext>  --objsuffix=<ext>[,<ext>] set targets' extensions. [.o]
 -d<name> --objdir=<name>           set target to be in directory <name>.
 -x<expr> --exclude=<expr>          ignore files that match regular expression.
 -h       --help                    show this help text.
"""%MakeDep.__doc__


if(__name__=='__main__'):
  try:
    opts,args=getopt.getopt(
      sys.argv[1:],
      'hI:o:d:x:',
      ['help','include=','objsuffix=','objdir=','exclude=']
    )
  except getopt.error:
    # print help information and exit:
    usage()
    sys.exit(2)
  dep=MakeDep()
  for option, optarg in opts:
    if option in ("-h", "--help"):
      usage()
      sys.exit(0)
    elif option in ("-I", "--include"):
      dep.includedirs.append(optarg)
    elif option in ("-o", "--objsuffix"):
      dep.targetExt=string.split(optarg,',')
    elif option in ("-d", "--objdir"):
      dep.objDir=optarg
    elif option in ("-x", "--exclude"):
      dep.reExclude=re.compile(optarg)
    else:
      sys.stderr.write('Ignoring unknown option: %s\n'%option)
      sys.stderr.flush()
  for fname in args:
    dep.process(fname)

