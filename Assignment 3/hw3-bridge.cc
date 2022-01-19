#include <iostream>
#include <fstream>
#include "ns3/flow-monitor-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace ns3;

void experiment(bool b1_2_b3_link, u_int32_t DATA_RATE){
  NodeContainer pcNodes;
  pcNodes.Create(6);

  NodeContainer bridges;
  bridges.Create(3);


  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DATA_RATE));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(0.001)));

  NetDeviceContainer LanDevices;
  NetDeviceContainer bridge1Devices;


  for (int i = 0; i < 3; i++)
    {
      NetDeviceContainer link = csma.Install (NodeContainer (pcNodes.Get (i), bridges.Get(0)));
      LanDevices.Add (link.Get (0));
      bridge1Devices.Add (link.Get (1));
    }

  NetDeviceContainer bridge3Devices;
  for (int i = 3; i < 6; i++)
    {
      NetDeviceContainer link = csma.Install (NodeContainer (pcNodes.Get (i), bridges.Get(2)));
      LanDevices.Add (link.Get (0));
      bridge3Devices.Add (link.Get (1));
    }

  
  NetDeviceContainer bridge2Devices;
  NetDeviceContainer link = csma.Install (NodeContainer (bridges.Get(0), bridges.Get(1)));
  bridge1Devices.Add (link.Get (0));
  bridge2Devices.Add (link.Get (1));
  
  NetDeviceContainer link1 = csma.Install (NodeContainer (bridges.Get(2), bridges.Get(1)));
  bridge3Devices.Add (link1.Get (0));
  bridge2Devices.Add (link1.Get (1));

  if (b1_2_b3_link==true) {
      NetDeviceContainer link2 = csma.Install (NodeContainer (bridges.Get(0), bridges.Get(2)));
      bridge1Devices.Add (link2.Get (0));
      bridge3Devices.Add (link2.Get (1));
  }



  Ptr<Node> bridge1 = bridges.Get(0);
  Ptr<Node> bridge2 = bridges.Get(1);
  Ptr<Node> bridge3 = bridges.Get(2);
  
  BridgeHelper bridge;
  bridge.Install (bridge1, bridge1Devices);
  bridge.Install (bridge2, bridge2Devices);
  bridge.Install (bridge3, bridge3Devices);


  InternetStackHelper internet;
  internet.Install(pcNodes);
  
 
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.62.0", "255.255.255.0");
  ipv4.Assign (LanDevices);

	ApplicationContainer cbrApps;
	uint16_t cbrPort = 12345;
	OnOffHelper onOffHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address ("192.168.62.6"), cbrPort)));
	onOffHelper.SetAttribute ("PacketSize", UintegerValue (15000));
	onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
	onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	// Setting the Attributes of the Flow
	
	onOffHelper.SetAttribute ("DataRate", StringValue ("1000000"));
	onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.000000)));
	cbrApps.Add (onOffHelper.Install (pcNodes.Get (0)));


  //
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events.
  // Trace output will be sent to the file "csma-bridge-one-hop.tr"
  //
  AsciiTraceHelper ascii;
  if (b1_2_b3_link==false && DATA_RATE==1000000){
    csma.EnableAsciiAll (ascii.CreateFileStream ("csma-bridge_1Mbps_without_link.tr"));
  } else if (b1_2_b3_link==false && DATA_RATE==100000000)
  {
    csma.EnableAsciiAll (ascii.CreateFileStream ("csma-bridge_100Mbps_without_link.tr"));
  } else if (b1_2_b3_link==true && DATA_RATE==1000000)
  {
    csma.EnableAsciiAll (ascii.CreateFileStream ("csma-bridge_1Mbps_with_link.tr"));
  } else{
    csma.EnableAsciiAll (ascii.CreateFileStream ("csma-bridge_100Mbps_with_link.tr"));
  }
  
  
  
  //
  // Also configure some tcpdump traces; each interface will be traced.
  // The output files will be named:
  //     csma-bridge-one-hop-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -r" command (use "-tt" option to
  // display timestamps correctly)
  //
  std::string rate = std::to_string(DATA_RATE/1000000);
  if (b1_2_b3_link==true){
    std::string filename = "csma-bridge"+rate+"Mbps_"+"with_link";
    csma.EnablePcapAll (filename, false);
    // 
  } else{
    std::string filename = "csma-bridge"+rate+"Mbps_"+"without_link";
    csma.EnablePcapAll (filename, false);
  }


  //
  // Now, do the actual simulation.
  //
  
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
  
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      // first 2 FlowIds are for ECHO apps, we don't want to display them
      //
      // Duration for throughput measurement is 9.0 seconds, since
      //   StartTime of the OnOffApplication is at about "second 1"
      // and
      //   Simulator::Stops at "second 10".
     // if (i->first > 2)
       // {
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
          std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
       // }
    }

  Simulator::Destroy ();


}

int 
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  std::cout << "Bridge topology without the link from bridge 1 to bridge 3 with 1 Mbps links \n";
  experiment(false, 1000000);
  std::cout << "------------------------------------------------\n";
  std::cout << "Bridge topology with the link from bridge 1 to bridge 3 with 1 Mbps links  \n";
  experiment(true, 1000000);
  std::cout << "------------------------------------------------\n";
  std::cout << "Bridge topology without the link from bridge 1 to bridge 3 with 100 Mbps links \n";
  experiment(false, 100000000);
  std::cout << "------------------------------------------------\n";
  std::cout << "Bridge topology with the link from bridge 1 to bridge 3 with 100 Mbps links  \n";
  experiment(true, 100000000);

  
}
