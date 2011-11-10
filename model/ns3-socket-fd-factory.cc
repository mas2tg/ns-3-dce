#include "ns3-socket-fd-factory.h"
#include "unix-fd.h"
#include "unix-socket-fd.h"
#include "unix-datagram-socket-fd.h"
#include "unix-stream-socket-fd.h"
#include "ns3/netlink-socket-factory.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "ns3/packet-socket-address.h"
#include <netpacket/packet.h>
#include <net/ethernet.h>

NS_LOG_COMPONENT_DEFINE("Ns3SocketFdFactory");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Ns3SocketFdFactory);

TypeId 
Ns3SocketFdFactory::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ns3SocketFdFactory")
    .SetParent<SocketFdFactory> ()
    .AddConstructor<Ns3SocketFdFactory> ()
  ;
  return tid;
}

Ns3SocketFdFactory::Ns3SocketFdFactory () : m_netlink (0)
{
}

//this is necessary to assign appropriate Node to NetlinkSocketFactory
void
Ns3SocketFdFactory::NotifyNewAggregate (void)
{
  Ptr<Node> node = this->GetObject<Node> ();
  if (m_netlink == 0)
    {
      m_netlink = CreateObject<NetlinkSocketFactory> ();
      node->AggregateObject (m_netlink);
    }
}

UnixFd *
Ns3SocketFdFactory::CreateSocket (int domain, int type, int protocol)
{
  UnixSocketFd *socket = 0;
  Ptr<Socket> sock;

  if (domain == PF_INET)
    {
      switch (type) {
        case SOCK_DGRAM: {
            TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
            Ptr<SocketFactory> factory = GetObject<SocketFactory> (tid);
            sock = factory->CreateSocket ();
            socket = new UnixDatagramSocketFd (sock);
          } break;
        case SOCK_RAW: {
            TypeId tid = TypeId::LookupByName ("ns3::Ipv4RawSocketFactory");
            Ptr<SocketFactory> factory = GetObject<SocketFactory> (tid);
            sock = factory->CreateSocket ();
            sock->SetAttribute ("Protocol", UintegerValue (protocol));
            socket = new UnixDatagramSocketFd (sock);
          } break;
        case SOCK_STREAM: {
            TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
            Ptr<SocketFactory> factory = GetObject<SocketFactory> (tid);
            sock = factory->CreateSocket ();
            socket = new UnixStreamSocketFd (sock);
          } break;
        default:
          NS_FATAL_ERROR ("missing socket type");
          break;
        }
    }
  else if (domain == PF_NETLINK)
    {
      switch (type) {
        case SOCK_RAW: 
        case SOCK_DGRAM: {
	    NS_LOG_INFO ("Requesting for PF_NETLINK");
            sock = m_netlink->CreateSocket ();
            socket = new UnixDatagramSocketFd (sock);
          } break;
        default:
          NS_FATAL_ERROR ("missing socket type");
          break;
        }
    }
  else if (domain == AF_PACKET)
    {
      switch (type) {
        case SOCK_RAW: {
            TypeId tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
            Ptr<SocketFactory> factory = GetObject<SocketFactory> (tid);
            sock = factory->CreateSocket ();

            PacketSocketAddress a;
            a.SetAllDevices ();
            if ( protocol == htons ( ETH_P_ALL) )
              {
                a.SetProtocol ( 0 );
              }
            else
              {
                a.SetProtocol ( ntohs ( protocol ) );
              }

            sock->Bind (a);

            socket = new UnixDatagramSocketFd (sock);
          } break;
        default:
          NS_FATAL_ERROR ("missing socket type");
          break;
        }
    }
  else
    {
      NS_FATAL_ERROR ("unsupported domain");
    }

  return socket;
}

} // namespace ns3
