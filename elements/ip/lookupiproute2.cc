/*
 * lookupiproute2.{cc,hh} -- element looks up next-hop address in pokeable
 * routing table.
 * Thomer M. Gil
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology.
 *
 * This software is being provided by the copyright holders under the GNU
 * General Public License, either version 2 or, at your discretion, any later
 * version. For more information, see the `COPYRIGHT' file in the source
 * distribution.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "lookupiproute2.hh"
#include "ipaddress.hh"
#include "confparse.hh"
#include "error.hh"
#include "glue.hh"

LookupIPRoute2::LookupIPRoute2()
{
  add_input();
  add_output();
}

LookupIPRoute2::~LookupIPRoute2()
{
}

LookupIPRoute2 *
LookupIPRoute2::clone() const
{
  return new LookupIPRoute2;
}

void
LookupIPRoute2::push(int, Packet *p)
{
  unsigned gw = 0;
  int index = 0;

  IPAddress a = p->dst_ip_anno();

  /*
  add_route_handler("1.0.0.0 255.255.0.0 5.5.5.5", this, (void *)0, (ErrorHandler *)0);
  add_route_handler("2.1.0.0 255.255.0.0 1.1.1.1", this, (void *)0, (ErrorHandler *)0);
  add_route_handler("2.2.0.0 255.255.0.0 2.2.2.2", this, (void *)0, (ErrorHandler *)0);
  add_route_handler("2.244.0.0 255.255.0.0 3.3.3.3", this, (void *)0, (ErrorHandler *)0);
  add_route_handler("2.0.0.0 255.255.0.0 4.4.4.4", this, (void *)0, (ErrorHandler *)0);
  del_route_handler("2.1.3.0 255.255.0.0", this, (void *)0, (ErrorHandler *)0);
  click_chatter("Lookup for %x", ntohl(a.saddr()));
  */

  if(_t.lookup(a.saddr(), gw, index)) {
    p->set_dst_ip_anno(gw);
    // click_chatter("Gateway for %x is %x", ntohl(a.saddr()), ntohl(gw));
  } else {
    click_chatter("No route found.");
  }
  output(0).push(p);
}

// Adds a route if not exists yet.
int
LookupIPRoute2::add_route_handler(const String &conf, Element *e, void *, ErrorHandler *errh)
{
  Vector<String> args;
  cp_argvec(conf, args);

  LookupIPRoute2* me = (LookupIPRoute2 *) e;

  for (int i = 0; i < args.size(); i++) {
    String arg = args[i];
    unsigned int dst, mask, gw;
    if (cp_ip_address(arg, (unsigned char *)&dst, &arg) &&
	cp_eat_space(arg) &&
        cp_ip_address(arg, (unsigned char *)&mask, &arg) &&
	cp_eat_space(arg) &&
        cp_ip_address(arg, (unsigned char *)&gw, &arg) &&
	cp_eat_space(arg))
      me->_t.add(dst, mask, gw);
    else {
      errh->error("expects DST MASK GW");
      return -1;
    }
  }

  return 0;
}

// Deletes a route. Nothing happens when entry does not exist.
int
LookupIPRoute2::del_route_handler(const String &conf, Element *e, void *, ErrorHandler *errh)
{
  Vector<String> args;
  cp_argvec(conf, args);

  LookupIPRoute2* me = (LookupIPRoute2 *) e;

  for (int i = 0; i < args.size(); i++) {
    String arg = args[i];
    unsigned int dst, mask;
    if (cp_ip_address(arg, (unsigned char *)&dst, &arg) &&
        cp_eat_space(arg) &&
        cp_ip_address(arg, (unsigned char *)&mask, &arg) &&
        cp_eat_space(arg))
      me->_t.del(dst, mask);
    else {
      errh->error("expects DST MASK");
      return -1;
    }
  }

  return 0;
}

// Prints the routing table.
String
LookupIPRoute2::look_route_handler(Element *e, void *)
{
  String ret;
  unsigned dst, mask, gw;
  LookupIPRoute2 *me;

  me = (LookupIPRoute2*) e;

  int size = me->_t.size();
  ret = "Entries: " + String(size) + "\nDST\tMASK\tGW\n";
  if(size == 0)
    return ret;

  int seen = 0; // # of valid entries handled
  for(int i = 0; seen < size; i++) {
    if(me->_t.get(i, dst, mask, gw)) {  // false if not valid
      ret += IPAddress(dst).s() + "\t" + \
             IPAddress(mask).s()+ "\t" + \
             IPAddress(gw).s()  + "\n";
      seen++;
    }
  }

  return ret;
}


void
LookupIPRoute2::add_handlers()
{
  add_write_handler("add", add_route_handler, 0);
  add_write_handler("del", del_route_handler, 0);
  add_read_handler("look", look_route_handler, 0);
}

EXPORT_ELEMENT(LookupIPRoute2)
