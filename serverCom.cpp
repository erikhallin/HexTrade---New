#include "serverCom.h"

serverCom::serverCom()
{
    m_ready=false;
}

bool serverCom::init(void)
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
    m_broadcast_port=0;

    return true;
}

bool serverCom::set_port_and_bind(int port)
{
    m_host_port=port;

    sockaddr_in addr;                           // The address structure for a TCP socket
    addr.sin_family = AF_INET;  	            // Address family Internet
    addr.sin_port = htons (port);               // Assign port to this socket
    addr.sin_addr.s_addr = htonl (INADDR_ANY);  // No destination

    //Show IP
    char hostName[80];
    if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR)
    {
        cerr << "ERROR: " << WSAGetLastError() << " when getting local host name." << endl;
        m_ready=false;
        return false;
    }
    m_host_name=string(hostName);

    struct hostent* hostInfo = gethostbyname(hostName);
    if (hostInfo == 0)
    {
        cerr << "ERROR: No host name" << endl;
        m_ready=false;
        return false;
    }

    for (int i = 0; hostInfo->h_addr_list[i] != 0; ++i)
    {
        struct in_addr addr;
        memcpy(&addr, hostInfo->h_addr_list[i], sizeof(struct in_addr));
        m_host_IP=string( inet_ntoa(addr) );
        break;
    }

    //Bind socket
    if (bind(m_SocServer, (LPSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR)
    { // error
        WSACleanup ();  // unload WinSock
        m_ready=false;
        return false;
    }
    //cout<<"bind()\n";

    return true;
}

bool serverCom::start_to_listen(int backLog)
{
    if(backLog<1 || backLog>10) return false;

    if (listen(m_SocServer,10)==SOCKET_ERROR)
    { // error!  unable to listen
        WSACleanup ();
        return false;
    }
    //cout<<"listen()\n";

    return true;
}

bool serverCom::known_socket(SOCKET new_socket)
{
    for(int i=0;i<(int)m_vSocClients.size();i++)
    {
        if(m_vSocClients[i]==new_socket) return true;
    }

    return false;
}

bool serverCom::check_for_broadcast(void)
{
    if(!m_broadcast_ready)
    {
        if(!init_broadcast_socket()) return false;
        else m_broadcast_ready=true;
    }

    //cout<<"Listening for client broadcast...\n";

    //listen for broadcast
    char buff[256]={0};
    int retVal=recvfrom(m_SocUDP_for_broadcast_recv, buff, sizeof(buff), NULL, NULL, NULL);
    if(retVal!=-1)//incoming data
    {
        cout<<"Received message: "<<buff<<endl;
        //test if buffer contains a valid IP
        if( ip_is_valid(buff) )
        {
            cout<<"Received valid IP: "<<buff<<endl;
            string sIP(buff);
            reply_to_broadcaster(sIP);
        }
        else //ip not valid
        {
            cout<<"Received a bad IP from broadcast\n";
        }

    }

    return true;
}

bool serverCom::set_broadcast_port(int port_sending,int port_replying)
{
    m_broadcast_port=port_sending;
    m_broadcast_port_reply=port_replying;

    close_broadcast_socket();

    m_broadcast_ready=false;

    return true;
}

bool serverCom::send_data(string data_string)//send to all clients
{
    bool error_flag=false;
    for(int i=0;i<(int)m_vSocClients.size();i++)
    {
        int val=send(m_vSocClients[i],(char*)data_string.c_str(),sizeof(char)*(data_string.length()+1),0);

        if(val!=(int)data_string.length()+1) error_flag=true;//sent wrong amount
    }

    if(error_flag) return false;//error

    return true;
}

bool serverCom::send_data(char* data_array)//send to all clients
{
    int packet_size=0;
    memcpy(&packet_size,data_array,4);

    bool error_flag=false;
    for(int i=0;i<(int)m_vSocClients.size();i++)
    {
        int val=send(m_vSocClients[i],data_array,packet_size,0);

        //cout<<"Sent data to client nr: "<<i<<", id: "<<m_vSocClients[i]<<endl;//TEMP


        if(val!=packet_size) error_flag=true;//sent wrong amount
    }

    if(error_flag) return false;//error

    return true;
}

bool serverCom::send_data(string data_string,SOCKET SocReceiver)//send to specific socket
{
    //cout<<"sending to socket: "<<SocReceiver<<endl; //xXXXXXXXXXXXXXXXXXXXXXXX

    int val=send(SocReceiver,(char*)data_string.c_str(),sizeof(char)*(data_string.length()+1),0);

    if(val!=(int)data_string.length()+1) return false;//sent wrong amount

    return true;
}

bool serverCom::send_data(char* data_array,SOCKET SocReceiver)//send to specific socket
{
    int packet_size=0;
    memcpy(&packet_size,data_array,4);

    int val=send(SocReceiver,data_array,packet_size,0);

    if(val!=packet_size) return false;//sent wrong amount

    return true;
}

bool serverCom::recv_data(string& data_string,SOCKET SocSender)
{
    char buffer[1024];
    int val=recv(SocSender,buffer,1024,0);

    data_string=string(buffer);

    //if(val!=(int)data_string.length()+1) return false;//received wrong amount

    return true;
}

bool serverCom::recv_data(char* data_array,SOCKET SocSender)
{
    int val=recv(SocSender,data_array,1024,0);

    //if(val!=int(data_array[0]*sizeof(float))) return false;//received wrong amount

    return true;
}

bool serverCom::add_client(SOCKET& new_client)
{
    SOCKET accepted_client=accept(new_client, NULL, NULL);
    if(accepted_client==INVALID_SOCKET) return false; //bad socket

    new_client=accepted_client;//update socket value

    m_vSocClients.push_back(accepted_client); //good socket

    //cout<<"stored socket: "<<accepted_client<<endl; //xxxxxxXXXXXXXXXXXXXXXX

    return true;
}

bool serverCom::remove_client(SOCKET client_to_remove)
{
    for(int i=0;i<(int)m_vSocClients.size();i++)
    {
        if(m_vSocClients[i]==client_to_remove)
        {
            closesocket(client_to_remove);
            m_vSocClients.erase( m_vSocClients.begin()+i );
            return true;
        }
    }
    return false;//client not found
}

SOCKET serverCom::get_server_socket(void)
{
    return m_SocServer;
}

bool serverCom::clean_up(void)
{
    for(int i=0;i<(int)m_vSocClients.size();i++) closesocket( m_vSocClients[i] );
    WSACleanup();
    m_ready=false;
    return true;
}

bool serverCom::init_broadcast_socket(void)
{
    m_SocUDP_for_broadcast_recv = socket(AF_INET, SOCK_DGRAM, 0);

    if(m_broadcast_port==0)
    {
        m_broadcast_port=5002; //default value
        m_broadcast_port_reply=5003; //default value
    }

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_broadcast_port);

    bind(m_SocUDP_for_broadcast_recv,(SOCKADDR*)&addr,sizeof(addr));

    u_long on = 1;
    ioctlsocket(m_SocUDP_for_broadcast_recv, FIONBIO, &on);//now non-blocking

    return true;
}

bool serverCom::reply_to_broadcaster(string sIP)
{
    //create UDP socket
    SOCKET broadcasterSocket = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addr;
    int len;

    addr.sin_addr.s_addr = inet_addr( (const char *)sIP.c_str() );
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_broadcast_port_reply);

    len = sizeof(addr);

    string IP_and_port;//server ip and port
    IP_and_port.append(m_host_IP);
    IP_and_port.append(1,':');
    char buff[10];
    itoa(m_host_port, buff, 10);
    IP_and_port.append(buff);

    cout<<"Network: Broadcast socket: "<<(int)broadcasterSocket<<endl;
    //send Server IP
    sendto(broadcasterSocket, (const char*)IP_and_port.c_str(), sizeof(char)*(IP_and_port.length()+1), 0, (struct sockaddr *) &addr, len);
    cout<<"Network: Replyed Server IP to Client IP: "<<IP_and_port<<endl;

    return true;
}

bool serverCom::close_broadcast_socket(void)
{
    if(closesocket(m_SocUDP_for_broadcast_recv)==0)
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

bool serverCom::ip_is_valid(char *str)
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


