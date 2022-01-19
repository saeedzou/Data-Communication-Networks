// Saeedreza Zouashkiani 400206262
#include "ns3/core-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/applications-module.h"
#include "ns3/animation-interface.h"
#include <fstream>
#include "ns3/gnuplot.h"

using namespace ns3;

double experiment(uint32_t RATE, uint32_t nStation, uint16_t cbrPort, uint32_t packetSize)
{

    // Creating Contention Window Parameters
    /** \todo **/
    Config::SetDefault ("ns3::Txop::MinCw", UintegerValue (31));
    Config::SetDefault ("ns3::Txop::MaxCw", UintegerValue (1023));


    // Creating AP and STA Nodes
    NodeContainer apNodes, staNodes;
    /** \todo **/
    apNodes.Create(1);
    staNodes.Create(nStation);


    // Create YansWifiChannelHelper & YansWifiPhyHelper
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());

    // Create WifiHelper (Use WIFI_PHY_STANDARD_80211g and IdealWifiManager)
    /** \todo **/
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::IdealWifiManager");


    // Create WifiMacHelper
    WifiMacHelper wifiMac;

    // Install wifi On Stations
    /** \todo **/
    wifiMac.SetType("ns3::StaWifiMac");
    NetDeviceContainer staDev = wifi.Install(wifiPhy, wifiMac, staNodes);


    // Install wifi On Access Points
    /** \todo **/
    wifiMac.SetType("ns3::ApWifiMac");
    NetDeviceContainer apDev = wifi.Install(wifiPhy, wifiMac, apNodes);


    // Create MobilityHelper and set parameters
    /** \todo **/
    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");


    // Install Mobility On APs And Stations
    /** \todo **/
    mobility.Install(staNodes);
    mobility.Install(apNodes);



    // Install TCP/IP Stack On All Nodes
    InternetStackHelper internet;
    internet.InstallAll ();

    // Assign IP Addresses
    /** \todo **/
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("192.168.62.0", "255.255.255.0");
    Ipv4InterfaceContainer apNodeInterface;
    Ipv4InterfaceContainer staNodeInterface;
    
    apNodeInterface = ipv4.Assign (apDev);
    staNodeInterface = ipv4.Assign (staDev);


    // Create OnOffHelper and Set Attributes
    /** \todo **/
    std::string DATA_RATE_kbps = std::to_string(RATE) + "kbps";
    cbrPort = 12345;
    OnOffHelper onOffHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (apNodeInterface.GetAddress(0), cbrPort)));
    onOffHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
    onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    // Setting the Attributes of the Flow
    onOffHelper.SetAttribute ("DataRate", StringValue (DATA_RATE_kbps));
    onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.000000)));
  
    // Start OnOff Applications
    /** \todo **/
    ApplicationContainer cbrApps = onOffHelper.Install(staNodes.Get(0));
    cbrApps.Start(Seconds(1.0));
    cbrApps.Stop(Seconds(10.0));

    // Installing Sink Application On The Destination
    PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), cbrPort));
    ApplicationContainer sinkApps = sink.Install (apNodes.Get(0));

    // Start Sink Applications
    sinkApps.Start (Seconds (1.0));
    sinkApps.Stop (Seconds (10.0));


    // Enable Pcap Tracing + Create NetAnim File
    /** \todo **/


    // Creating Install FlowMonitor On all nodes
    /** \todo **/
    double throughput;
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    // Starting Simulation + Destory when over
    Simulator::Stop (Seconds (10.0));
    Simulator::Run ();

    // Print Out The Summary Of Flows
    /** \todo **/
      // 10. Print per flow statistics
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
      {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        if ((t.sourceAddress=="192.168.62.2" && t.destinationAddress == "192.168.62.1"))
        {
            // std::cout << "Flow " << i->first  << " (" << t.sourceAddress<<":"<<t.sourcePort << " -> " << t.destinationAddress <<":"<<t.destinationPort<< ")\n";
            // std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
            // std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
            // std::cout << "  Throughput: " << i->second.rxBytes * 8.0 /9/1000  << " kbps\n";
            throughput = i->second.rxBytes * 8.0 /9/1000;
        }
       }

    Simulator::Destroy ();
    return throughput;
}


int main(int argc, char *argv[])
{
    uint32_t nStation;
    uint16_t cbrPort;
    uint32_t packetSize;

    // Creating Command Line Options
    CommandLine cmd;
    cmd.AddValue ("nStation", "Number of stations in the access point vicinity", nStation);
    cmd.AddValue ("Port", "The port used by OnOff application", cbrPort);
    cmd.AddValue ("PacketSize", "Size of each UDP packet", packetSize);
    cmd.Parse (argc, argv);
    std::string fileNameWithNoExtension = "plot-2d";
    std::string graphicsFileName        = fileNameWithNoExtension + ".png";
    std::string plotFileName            = fileNameWithNoExtension + ".plt";
    std::string plotTitle               = "2-D Plot";
    std::string dataTitle               = "2-D Data";

    // Instantiate the plot and set its title.
    Gnuplot plot (graphicsFileName);
    plot.SetTitle (plotTitle);

    // Make the graphics file, which the plot file will create when it
    // is used with Gnuplot, be a PNG file.
    plot.SetTerminal ("png");

    // Set the labels for each axis.
    plot.SetLegend ("X Values", "Y Values");

    // Set the range for the x axis.
    plot.AppendExtra ("set xrange [50:5000]");

    // Instantiate the dataset, set its title, and make the points be
    // plotted along with connecting lines.
    Gnuplot2dDataset dataset;
    dataset.SetTitle (dataTitle);
    dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    uint32_t x;
    double y;

    // Create the 2-D dataset.
    for (x = 50; x <= 5000; x += 50)
        {
        y = experiment(x, nStation, cbrPort, packetSize);;

        // Add this point.
        dataset.Add (x, y);
        }

    // Add the dataset to the plot.
    plot.AddDataset (dataset);

    // Open the plot file.
    std::ofstream plotFile (plotFileName.c_str());

    // Write the plot file.
    plot.GenerateOutput (plotFile);

    // Close the plot file.
    plotFile.close ();
    // std::cout << experiment(300, nStation, cbrPort, packetSize);
 
    return 0;
}
