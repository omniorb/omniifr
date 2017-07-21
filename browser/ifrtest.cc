//
// Really simple test client that gets the IFR's own InterfaceDef.
// Just outputs the ID to prove it's worked.
// You must load `ir.idl' into the repository for this to work:
//
//  % cd /usr/local/share/idl/omniORB
//  % omniidl -bifr -I. ir.idl
//

#define ENABLE_CLIENT_IR_SUPPORT 1
#include <iostream>
#include "IfrConnection.h"

int main(int argc, char** argv)
{
  using namespace CORBA;
  ORB_var orb =ORB_init(argc,argv);
  IfrConnection ifrc(orb);
  InterfaceDef_var def =ifrc.get_interface(ifrc.ifr);
  String_var rid=def->id();
  cout<<rid.in()<<endl;
  return 0;
}
