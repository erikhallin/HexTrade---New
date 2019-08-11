#include "clientCom.h"

clientCom::clientCom()
{
    m_ready=false;
}

bool clientCom::init(void)
{
    WSADATA w;
    int error = WSAStartup (0x0202, &w);
    if (error)
    { // there was an error
        cout<<"ERROR wrong version of WinSock\n";
        return false;
    }
    if (w.wVersion != 0x0202)
    { // wrong WinSock version!
        cout<<"ERROR wrong version of WinSock\n";
        WSACleanup (); // unload ws2_32.dll
        return false;
    }

    m_SocServer = socket (AF_INET, SOCK_STREAM, 0); // Create socket (TCP: SOCK_STREAM  UDP:SOCK_DGRAM)

    m_ready=true;
    m_broadcast_ready=false;
    m_found_server=false;

    return true;
}

bool clientCom::set_IP_and_connect(string sIP,int port)
{
    //test if IP is valid
    char cIP[30]={0};
    strcpy( cIP,sIP.c_str() );//non constant char* needed
    if( !ip_is_valid(cIP) )
    {
        //invalid IP
        cout<<"Invalid IP: "<<cIP<<endl;;
        return false;
    }

    sockaddr_in target;
    target.sin_family = AF_INET;       	                             // address family Internet
    target.sin_port = htons (port);    	                             // set server’s port number
    target.sin_addr.s_addr = inet_addr ((const char *)sIP.c_str());  // set server’s IP

    connect(m_SocServer, (SOCKADDR*) &target, sizeof(target));
    //cout<<"connect()\n";

    return true;
}

bool clientCom::test_connection(void)
{
    fd_set sockets;
    sockets.fd_count=1;
    sockets.fd_array[0]=m_SocServer;
    //set waiting time
    timeval waiting_time;
    waiting_time.tv_sec=2;
    waiting_time.tv_usec=2000;

    //int val=select( 0, &sockets, NULL, NULL, &waiting_time );
    int val=select( 0, NULL, &sockets, NULL, &waiting_time );

    if(val==1) return true;//connected to one SOCKET, the server

    return false;//not connected
}

bool clientCom::broadcast_my_ip(void)
{
    //init socket
    if(!m_broadcast_ready)
    {
        if(!init_broadcast_socket()) return false;
        else m_broadcast_ready=true;
        cout<<"Network: Broadcast: Init\n";
    }

    sockaddr_in addr;
    int len;

    addr.sin_addr.s_addr = inet_addr( (const char *)m_broadcast_net.c_str() );//where to send, for broadcasting
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_broadcast_port);

    len = sizeof(addr);

    sendto(m_SocUDP_for_broadcasting, (const char*)m_my_IP.c_str(), sizeof(char)*(m_my_IP.length()+1), 0, (struct sockaddr *) &addr, len);

    cout<<"Broadcasted my IP: "<<m_my_IP<<endl;

    return true;
}

bool clientCom::check_for_broadcast_reply(void)
{
    if(!m_broadcast_ready)
    {
        cout<<"ERROR: Can not listen to broadcast reply if sockets are not initiated\n";
        return false; //sockets not ready
    }

    cout<<"Listening for broadcast response...\n";

    //listen for broadcast reply
    char buff[256]={0};
    int retVal=recvfrom(m_SocUDP_for_broadcast_reply, buff, sizeof(buff), NULL, NULL, NULL);
    if(retVal!=-1)//incoming data
    {
        cout<<"Received message: "<<buff<<endl;
        //separate IP and port
        string sIP;
        int port=0;
        for(int i=0;i<256;i++) //256 is size of buff
        {
            if(buff[i]!=':')
            {
                sIP.append(1,buff[i]);
            }
            else //now comes the port value
            {
                string sPort;
                for(int j=0;j<10;j++) //10 is maximum port value length
                {
                    if(buff[i+1+j]!='\0')
                    {
                        sPort.append( 1, buff[i+1+j] );
                    }
                    else //have port value
                    {
                        port=atoi(sPort.c_str());//convert string to int
                        break;
                    }
                }
                break;
            }
        }
        //test if buffer contains a valid IP
        char cIP[30]={0};
        strcpy( cIP,sIP.c_str() );//non constant char* needed
        if( ip_is_valid(cIP) )
        {
            cout<<"Got Server IP: "<<sIP<<", and port: "<<port<<endl;
            //store values
            m_found_server=true;
            m_server_IP=sIP;
            m_server_port=port;
            return true;
        }
        else //ip not valid
        {
            cout<<"Received a bad IP from broadcast reply\n";
        }

    }

    return false; //no server found
}

bool clientCom::set_broadcast_net_and_port(string net,int port_sending,int port_replying)
{
    m_broadcast_net=net;            //net for broadcasting
    m_broadcast_port=port_sending;  //port for broadcasting
    m_broadcast_port=port_replying; //port for broadcast reply listening

    close_broadcast_socket();

    m_broadcast_ready=false;

    return true;
}

string clientCom::get_server_IP(void)
{
    return m_server_IP;
}

bool clientCom::get_server_IP_and_port(string& IP_and_port)
{
    if(m_found_server)
    {
        IP_and_port=m_server_IP;
        IP_and_port.append(1,':');
        char buff[10];
        itoa(m_server_port, buff, 10);//convert int to string
        IP_and_port.append(buff);

        return true;
    }

    return false;
}

bool clientCom::send_data(string data_string)
{
    int val=send(m_SocServer,(const char*)data_string.c_str(),sizeof(char)*(data_string.length()+1),0);

    if(val!=(int)data_string.length()+1) return false;//sent wrong amount

    return true;
}

bool clientCom::send_data(char* data_array)
{
    int packet_size=0;
    memcpy(&packet_size,data_array,4);

    int val=send(m_SocServer,data_array,packet_size,0);

    if(val!=packet_size) return false;//sent wrong amount

    return true;
}

bool clientCom::recv_data(string& data_string)
{
    char buffer[1024];
    int val=recv(m_SocServer,buffer,1024,0);

    data_string=string(buffer);

    //if(val!=(int)data_string.length()+1) return false;//received wrong amount

    return true;
}

bool clientCom::recv_data(char* data_array)
{
    int val=recv(m_SocServer,data_array,1024,0);

    //if(val!=int(data_array[0]*sizeof(float))) return false;//received wrong amount

    return true;
}

SOCKET clientCom::get_server_socket(void)
{
    return m_SocServer;
}

bool clientCom::clean_up(void)
{
    closesocket(m_SocServer);
    WSACleanup();
    m_ready=false;
    return true;
}



bool clientCom::init_broadcast_socket(void)
{
    // Create a datagram socket
    m_SocUDP_for_broadcasting = socket(AF_INET, SOCK_DGRAM, 0);

    if(m_broadcast_net.length()<1)//use default net and port
    {
        m_broadcast_net="255.255.255.255";  //net for broadcasting
        m_broadcast_port=5002;              //port for broadcasting
        m_broadcast_port_reply=5003;        //port for broadcast listening
    }

    //Turn on broadcasting
    const int broadcast_on = 1;
    setsockopt(m_SocUDP_for_broadcasting, SOL_SOCKET, SO_BROADCAST,(const char *) &broadcast_on, sizeof(broadcast_on));

    //get clients computer name and IP
    char hostName[80];
    if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR)
    {
        cout << "ERROR: " << WSAGetLastError() << " when getting local host name." << endl;
        m_ready=false;
        return false;
    }
    cout<<hostName<<endl;
    m_my_name=string(hostName);

    struct hostent* hostInfo = gethostbyname(hostName);
    if (hostInfo == 0)
    {
        cout << "ERROR: No host name" << endl;
        m_ready=false;
        return false;
    }
    for (int i = 0; hostInfo->h_addr_list[i] != 0; ++i)
    {
        struct in_addr addr;
        memcpy(&addr, hostInfo->h_addr_list[i], sizeof(struct in_addr));
        m_my_IP=string( inet_ntoa(addr) );
        break;
    }

    //Create socket for listening for broadcast reply
    m_SocUDP_for_broadcast_reply = socket(AF_INET, SOCK_DGRAM, 0);

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_broadcast_port_reply);

    bind(m_SocUDP_for_broadcast_reply,(SOCKADDR*)&addr,sizeof(addr));

    u_long nonblocking_on = 1;
    ioctlsocket(m_SocUDP_for_broadcast_reply, FIONBIO, &nonblocking_on);//now non-blocking

    return true;
}

bool clientCom::close_broadcast_socket(void)
{
    if(closesocket(m_SocUDP_for_broadcasting)==0 || closesocket(m_SocUDP_for_broadcast_reply==0))
    {
        cerr << "ERROR: Could not close socket" << endl;
        m_broadcast_ready=false;
        return false;
    }
    else
    {
        m_broadcast_ready=false;
    }

    return true;
}

bool clientCom::ip_is_valid(char *str)
{
    int segs = 0;   //Segment count
    int chcnt = 0;  // Character count within segment
    int accum = 0;  // Accumulator for segment

    //Catch NULL pointer
    if (str == NULL)
        return 0;

    //Process every character in string

    while(*str != '\0')
    {
        //Segment changeover
        if (*str == '.')
        {
            //Must have some digits in segment
            if (chcnt == 0) return false;
            //Limit number of segments
            if (++segs == 4) return false;
            //Reset segment values and restart loop
            chcnt = accum = 0;
            str++;
            continue;
        }

        //Check numeric
        if ((*str < '0') || (*str > '9')) return false;

        //Accumulate and check segment
        if ((accum = accum * 10 + *str - '0') > 255) return false;

        //Advance other segment specific stuff and continue loop
        chcnt++;
        str++;
    }

    //Check enough segments and enough characters in last segment
    if (segs != 3) return false;

    if (chcnt == 0) return false;

    //Address okay
    return true;
}
