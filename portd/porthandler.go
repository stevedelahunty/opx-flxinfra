package main
import ("git.apache.org/thrift.git/lib/go/thrift"
        "asicdServices"
	    "encoding/json"
	    "io/ioutil"
		"strconv"
        _"errors"
		"portdServices"
		"ribd"
        "l3/rib/ribdCommonDefs"
        "net")

type PortServiceHandler struct {
}


type PortClientBase struct {
	Address            string
	Transport          thrift.TTransport
	PtrProtocolFactory *thrift.TBinaryProtocolFactory
	IsConnected        bool
}

type AsicdClient struct {
	PortClientBase
	ClientHdl          *asicdServices.AsicdServiceClient
}

type RibClient struct {
	PortClientBase
	ClientHdl *ribd.RouteServiceClient
}

type ClientJson struct {
	Name string `json:Name`
	Port int    `json:Port`
}

var asicdclnt AsicdClient
var ribdclnt RibClient

func (m PortServiceHandler) CreateV4Intf(   ipAddr          string, 
                                            intf            int32) (rc portdServices.Int, err error) {
    logger.Println("Received create intf request")
    var ipMask net.IP 
	if(asicdclnt.IsConnected == true) {
		asicdclnt.ClientHdl.CreateIPv4Intf(ipAddr, intf)
	}
	if(ribdclnt.IsConnected == true) {
       ip, ipNet, err := net.ParseCIDR(ipAddr)
       if err != nil {
          return -1, err
       }
       ipMask = make(net.IP, 4)
       copy(ipMask, ipNet.Mask)
	   ipAddrStr := ip.String()
	   ipMaskStr := net.IP(ipMask).String()
	   ribdclnt.ClientHdl.CreateV4Route(ipAddrStr, ipMaskStr, 0, "0", ribd.Int(intf), ribdCommonDefs.CONNECTED)
	}
    return 0, err
}

func (m PortServiceHandler) DeleteV4Intf( ipAddr         string,
                                          intf           int32) (rc portdServices.Int, err error) {
    logger.Println("Received Intf Delete request")
    var ipMask net.IP 
	if(asicdclnt.IsConnected == true) {
		asicdclnt.ClientHdl.DeleteIPv4Intf(ipAddr, intf)
   }
	if(ribdclnt.IsConnected == true) {
       ip, ipNet, err := net.ParseCIDR(ipAddr)
       if err != nil {
          return -1, err
       }
       ipMask = make(net.IP, 4)
       copy(ipMask, ipNet.Mask)
	   ipAddrStr := ip.String()
	   ipMaskStr := net.IP(ipMask).String()
	   ribdclnt.ClientHdl.DeleteV4Route(ipAddrStr, ipMaskStr, ribdCommonDefs.CONNECTED)
	}
    return 0, err
}

func (m PortServiceHandler) CreateV4Neighbor( 
										    ipAddr string, 
                                              macAddr string, 
                                              vlanId int32, 
											routerIntf int32) (rc portdServices.Int, err error) {
    logger.Println("Received create neighbor intf request")
	if(asicdclnt.IsConnected == true) {
		asicdclnt.ClientHdl.CreateIPv4Neighbor(ipAddr, macAddr, vlanId, routerIntf)
	}
    return 0, err
}

func (m PortServiceHandler) DeleteV4Neighbor(ipAddr string, macAddr string, vlanId int32, routerIntf int32) (rc portdServices.Int, err error) {
    logger.Println("Received delete neighbor intf request")
	if(asicdclnt.IsConnected == true) {
		asicdclnt.ClientHdl.DeleteIPv4Neighbor(ipAddr, macAddr, vlanId, routerIntf)
	}
    return 0, err
}

func CreateIPCHandles(address string) (thrift.TTransport, *thrift.TBinaryProtocolFactory) {
	var transportFactory thrift.TTransportFactory
	var transport thrift.TTransport
	var protocolFactory *thrift.TBinaryProtocolFactory
	var err error

	protocolFactory = thrift.NewTBinaryProtocolFactoryDefault()
	transportFactory = thrift.NewTTransportFactory()
	transport, err = thrift.NewTSocket(address)
	transport = transportFactory.GetTransport(transport)
	if err = transport.Open(); err != nil {
		logger.Println("Failed to Open Transport", transport, protocolFactory)
		return nil, nil
	}
	return transport, protocolFactory
}

func ConnectToClients(paramsFile string){
	var clientsList []ClientJson

	bytes, err := ioutil.ReadFile(paramsFile)
	if err != nil {
		logger.Println("Error in reading configuration file")
		return
	}

	err = json.Unmarshal(bytes, &clientsList)
	if err != nil {
		logger.Println("Error in Unmarshalling Json")
		return
	}

	for _, client := range clientsList {
		logger.Println("#### Client name is ", client.Name)
        if(client.Name == "asicd") {
			logger.Printf("found asicd at port %d", client.Port)
	        asicdclnt.Address = "localhost:"+strconv.Itoa(client.Port)
	        asicdclnt.Transport, asicdclnt.PtrProtocolFactory = CreateIPCHandles(asicdclnt.Address)
	        if asicdclnt.Transport != nil && asicdclnt.PtrProtocolFactory != nil {
		       logger.Println("connecting to asicd")
		       asicdclnt.ClientHdl = asicdServices.NewAsicdServiceClientFactory(asicdclnt.Transport, asicdclnt.PtrProtocolFactory)
               asicdclnt.IsConnected = true
	        }
		}
        if(client.Name == "ribd") {
			logger.Printf("found ribd at port %d", client.Port)
	        ribdclnt.Address = "localhost:"+strconv.Itoa(client.Port)
	        ribdclnt.Transport, ribdclnt.PtrProtocolFactory = CreateIPCHandles(ribdclnt.Address)
	        if ribdclnt.Transport != nil && ribdclnt.PtrProtocolFactory != nil {
		       logger.Println("connecting to ribd")
		       ribdclnt.ClientHdl = ribd.NewRouteServiceClientFactory(ribdclnt.Transport, ribdclnt.PtrProtocolFactory)
               ribdclnt.IsConnected = true
	        }
		}
   }	
}


func NewPortServiceHandler () *PortServiceHandler {
	configFile := "../../config/params/clients.json"
	ConnectToClients(configFile)
    return &PortServiceHandler{}
}
