//                            Package   : omniIFR
// PersistNode.cc             Created   : 2004/04/29
//                            Author    : Alex Tingle
//
//    Copyright (C) 2004 Alex Tingle.
//
//    This file is part of the omniIFR application.
//
//    omniIFR is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    omniIFR is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "PersistNode.h"

#include "Repository.h"

#include <stdlib.h>
#include <stdio.h>

namespace Omniifr {

const char* PersistNode::_separator ="::";

PersistNode::PersistNode(istream& is)
{
  while( readnode(is) ){}
}

PersistNode::~PersistNode()
{
  for(map<string,PersistNode*>::iterator i=_child.begin(); i!=_child.end(); ++i)
      delete i->second;
}

void PersistNode::output(ostream& os,string name) const
{
  if(!name.empty()) // Don't output root node.
  {
    os<<name<<'\n';
    for(map<string,string>::const_iterator i=_attr.begin();
        i!=_attr.end();
        ++i)
    {
      os<<" "<<i->first<<"="<<i->second<<'\n';
    }
    os<<" ;;\n";
    name+=_separator;
  }
  for(map<string,PersistNode*>::const_iterator i=_child.begin();
      i!=_child.end();
     ++i)
  {
    i->second->output(os,name+i->first);
  }
}


inline bool PersistNode::readnode(istream& is)
{
  PersistNode* node =NULL;
  string tok;
  while(true)
  {
    if(!readtoken(is,tok) || tok==";;")
        return bool(node);
    else if(node)
        node->addattr(tok);
    else if(tok[0]=='-')
        delnode(tok.substr(1));
    else
        node=addnode(tok);
  }
}

inline bool PersistNode::readtoken(istream& is, string& tok)
{
  while(is)
  {
    is>>tok;
    if(tok.empty())
        break;
    if(tok[0]!='#')
        return true;
    is.ignore(INT_MAX,'\n');
  }
  return false;
}

PersistNode* PersistNode::addnode(const string& name)
{
  string::size_type pos =name.find(_separator);
  // get reference to Next node in the path.
  PersistNode*& newchild =_child[name.substr(0,pos)];

  if(pos==string::npos) // leaf: add new leaf.
  {
    if(newchild)
        delete newchild; // overwrite old leaf (and its children)
    newchild=new PersistNode();
    return newchild;
  }
  else // branch: just add the branch if it's missing, and then recurse.
  {
    if(!newchild)
        newchild=new PersistNode();
    return newchild->addnode(name.substr(pos+2));
  }
}

void PersistNode::delnode(const string& name)
{
  string::size_type pos =name.find(_separator);
  // get reference to Next node in the path.
  map<string,PersistNode*>::iterator childpos =_child.find(name.substr(0,pos));
  if(childpos!=_child.end())
  {
    if(pos==string::npos) // leaf: delete leaf.
    {
      delete childpos->second;
      _child.erase(childpos);
    }
    else // branch: recurse
    {
      childpos->second->delnode(name.substr(pos+2));
    }
  }
}

void PersistNode::addattr(const string& keyvalue)
{
  string::size_type pos =keyvalue.find('=');
  _attr[keyvalue.substr(0,pos)]=(pos==string::npos?"":keyvalue.substr(pos+1));
}

void PersistNode::addattr(const string& key, long value)
{
  char buf[64];
  sprintf(buf,"%i",value);
  _attr[key]=string(buf);
}

bool PersistNode::hasAttr(const string& key) const
{
  return( _attr.find(key)!=_attr.end() );
}
string PersistNode::attrString(const string& key, const string& fallback) const
{
  map<string,string>::const_iterator pos=_attr.find(key);
  if(pos!=_attr.end())
      return pos->second;
  DB(20,"PersistNode failed to find key: "<<key.c_str()<<" (string)")
  return fallback;
}
long PersistNode::attrLong(const string& key, long fallback) const
{
  map<string,string>::const_iterator pos=_attr.find(key);
  if(pos!=_attr.end())
      return ::atol(pos->second.c_str());
  DB(20,"PersistNode failed to find key: "<<key.c_str()<<" (long)")
  return fallback;
}
cdrMemoryStream PersistNode::attrCdrStream(const string& key) const
{
  map<string,string>::const_iterator pos=_attr.find(key);
  if(pos==_attr.end())
  {
    DB(1,"ERROR, missing cdrStream attribute: "<<key.c_str())
    return cdrMemoryStream(); // eek! bad input data.
  }
  if(1==pos->second.size()%2)
  {
    DB(1,"ERROR, cdrStream attribute should have even-number of chars: "
         <<key.c_str())
    return cdrMemoryStream(); // eek! bad input data.
  }
  // OK
  const int len =pos->second.size()/2;
  CORBA::Octet* buf =new CORBA::Octet[len]; // Make a buffer of the right size
  char str[3];
  str[2]='\0';
  for(int i=0; i<len; ++i)                  // Fill it
  {
    str[0]=pos->second[2*i  ];
    str[1]=pos->second[2*i+1];
    long byte=::strtol(str,NULL,16);
    assert(byte>=0 && byte<256);
    buf[i]=(CORBA::Octet)byte;
  }
  cdrMemoryStream memstr;                   // don't bother to clear memory.
  memstr.put_octet_array(buf,len);          // Copy it into a cdrMemoryStream
  delete[] buf; // ?? use auto_ptr
  return memstr;
}
PersistNode* PersistNode::child(const string& key) const
{
  map<string,PersistNode*>::const_iterator pos=_child.find(key);
  if(pos==_child.end())
      return NULL;
  else
      return pos->second;
}

void PersistNode::outputCdrMemoryStream(
  ostream&         os,
  cdrMemoryStream& memstr,
  const char*      prefix
)
{
  if(prefix)
      os<<prefix;
  char buf[3];
  CORBA::ULong len(memstr.bufSize());
//CORBA::Octet* ptr((CORBA::Octet*)memstr.bufPtr());
  CORBA::Octet* ptr = (CORBA::Octet*) memstr.bufPtr();
  for(CORBA::ULong i=0; i<len; ++i)
  {
    sprintf(buf,"%02x",ptr[i]);
    os.write(buf,2);
  }
}

void PersistNode::outputIOR(
  ostream&          os,
  CORBA::Object_ptr obj,
  const char*       prefix
)
{
  if(prefix)
      os<<prefix;
  CORBA::String_var iorstr =
    Repository_impl::inst()._orb->object_to_string(obj);
  os<<iorstr.in();
}

} // end namespace Omniifr
