#                            Package   : omniIFR
#  ifrconnection.py          Created   : 2004/03/21
#                            Author    : Alex Tingle
#
#    This work is hereby released into the Public Domain. To view a copy of
#    the public domain dedication, visit
#    http:#creativecommons.org/licenses/publicdomain/ or send a letter to
#    Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
# 

#
# Helful little python module that connects us to the Interface Repository.
#

import sys
import omniORB
from omniORB import CORBA

omniORB.importIRStubs()

class IfrConnection:
  def __init__(self,orb=None,ifr=None,argv=None):
    """Supply AT MOST ONE of the three optional arguments."""
    self.orb=orb
    self.ifr=ifr
    if(not self.ifr):
      if(not self.orb):
        if(argv):
          self.orb=CORBA.ORB_init(argv, CORBA.ORB_ID)
        else:
          self.orb=CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
      obj=self.orb.resolve_initial_references("InterfaceRepository")
      self.ifr=obj._narrow(CORBA.Repository)

  def get_interface(self,obj):
    """Use this method while omniORB's Object._get_interface() method is broken.
       It's not reliable though, sometimes _NP_RepositoryId is empty.
    """
    # _NP_RepositoryId is non-portable: omniORB specific.
    contained=self.ifr.lookup_id(obj._NP_RepositoryId)
    if contained:
      return contained._narrow(CORBA.InterfaceDef)
    else:
      return None

# end class IfrConnection
