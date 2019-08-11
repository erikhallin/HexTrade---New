#include "networkCom.h"

networkCom::networkCom()
{
    m_ready=false;
    m_net_status=net_error;
}

int networkCom::get_status(void)
{
    return m_net_status;
}

bool networkCom::block_trap(void)//to trigger windows network blocking
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

    SOCKET soc = socket (AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;                           // The address structure for a TCP socket
    addr.sin_family = AF_INET;  	            // Address family Internet
    addr.sin_port = htons (666);               // Assign port to this socket
    addr.sin_addr.s_addr = htonl (INADDR_ANY);  // No destination

    //Bind socket
    if (bind(soc, (LPSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR)
    { // error
        WSACleanup ();  // unload WinSock
        m_ready=false;
        return false;
    }

    if (listen(soc,10)==SOCKET_ERROR)
    { // error!  unable to listen
        WSACleanup ();
        return false;
    }

    closesocket(soc);

    WSACleanup ();

    return true;
}

bool networkCom::init(string status)
{
    //clear old stuff
    m_ServerCom.clean_up();
    m_ClientCom.clean_up();

    //Init
    m_ready=false;
    m_connected_to_server=false;
    m_accept_new_clients=true;
    m_clients_counter=0;

    if(status=="server")
    {
        cout<<"Server mode enabled\n";
        m_net_status=net_server;
        m_ServerCom=serverCom();
        m_ServerCom.init();
        m_ready=true;
    }

    if(status=="client")
    {
        cout<<"Client mode enabled\n";
        m_net_status=net_client;
        m_ClientCom=clientCom();
        m_ClientCom.init();
        m_ready=true;
    }

    return m_ready;
}

bool networkCom::connect_to_server(string sIP,int port)
{
    if(m_net_status==net_client)
    {
        if(m_ClientCom.set_IP_and_connect(sIP,port)) return true;
    }

    return false;
}

bool networkCom::set_port_and_bind(int port)
{
    if(m_net_status==net_server)
    {
        if(m_ServerCom.set_port_and_bind(port)) return true;
    }

    return false;
}

bool networkCom::start_to_listen(int backLog)
{
    if(m_net_status==net_server)
    {
        if(m_ServerCom.start_to_listen(backLog)) return true;
    }

    return false;
}

bool networkCom::set_broadcast_port(int port_sending,int port_replying)
{
    if(m_net_status==net_server)
    {
        if(m_ServerCom.set_broadcast_port(port_sending, port_replying)) return true;
    }

    return false;
}

bool networkCom::set_broadcast_port(string net, int port_sending,int port_replying)
{
    if(m_net_status==net_client)
    {
        if(m_ClientCom.set_broadcast_net_and_port(net, port_sending, port_replying)) return true;
    }

    return false;
}

bool networkCom::test_connection(void)
{
    if(m_net_status==net_client)
    {
        if(m_ClientCom.test_connection())
        {
            m_connected_to_server=true;
            return true;
        }
        else return false;
    }

    return false;
}

bool networkCom::known_socket(SOCKET new_socket)
{
    if(m_net_status==net_server)
    {
        if(m_ServerCom.known_socket(new_socket)) return true;
    }

    return false;
}

bool networkCom::broadcast_my_ip(void)
{
    if(m_net_status==net_client)
    {
        return m_ClientCom.broadcast_my_ip();
    }

    return false;
}

bool networkCom::check_for_broadcast(void)
{
    if(m_net_status==net_server)
    {
        if(m_ServerCom.check_for_broadcast()) return true;
    }

    return false;
}

bool networkCom::check_for_broadcast_reply(void)
{
    if(m_net_status==net_client)
    {
        if(m_ClientCom.check_for_broadcast_reply()) return true;
    }

    return false;
}

bool networkCom::get_server_IP_and_port(string& IP_and_port)
{
    if(m_net_status==net_client)
    {
        if(m_ClientCom.get_server_IP_and_port(IP_and_port)) return true;
    }

    return false;
}

bool networkCom::send_data(string data_string)
{
    if(m_net_status==net_server)//to all Clients
    {
        m_ServerCom.send_data(data_string);
        return true;
    }

    if(m_net_status==net_client)
    {
        m_ClientCom.send_data(data_string);
        return true;
    }

    return false;
}

bool networkCom::send_data(char* data_array)
{
    if(m_net_status==net_server)//to all Clients
    {
        return m_ServerCom.send_data(data_array);
    }

    if(m_net_status==net_client)
    {
        return m_ClientCom.send_data(data_array);
    }

    return false;
}

bool networkCom::send_data(string data_string,SOCKET SocReceiver)
{
    if(m_net_status==net_server)
    {
        return m_ServerCom.send_data(data_string,SocReceiver);
    }
    else if(m_net_status==net_client)
    {
        return m_ClientCom.send_data(data_string);
    }

    return false;
}

bool networkCom::send_data(char* data_array,SOCKET SocReceiver)
{
    if(m_net_status==net_server)
    {
        return m_ServerCom.send_data(data_array,SocReceiver);
    }
    else if(m_net_status==net_client)
    {
        return m_ClientCom.send_data(data_array);
    }

    return false;
}

bool networkCom::recv_data(string& data_string,SOCKET SocSender)
{
    if(m_net_status==net_server)
    {
        if( m_ServerCom.recv_data(data_string,SocSender) ) return true;
    }
    else if(m_net_status==net_client)
    {
        if( m_ClientCom.recv_data(data_string) ) return true;
    }

    return false;
}

bool networkCom::recv_data(char* data_array,SOCKET SocSender)
{
    if(m_net_status==net_server)
    {
        if( m_ServerCom.recv_data(data_array,SocSender) ) return true;
    }
    else if(m_net_status==net_client)
    {
        if( m_ClientCom.recv_data(data_array) ) return true;
    }

    return false;
}

bool networkCom::add_client(SOCKET& new_client)
{
    if(m_net_status==net_server)
    {
        if(m_ServerCom.add_client(new_client))
        {
            m_clients_counter++;
            return true;
        }
    }

    return false;
}

bool networkCom::remove_client(SOCKET client_to_remove)
{
    if(m_net_status==net_server)
    {
        if(m_ServerCom.remove_client(client_to_remove))
        {
            m_clients_counter--;
            return true;
        }
    }

    return false;
}

SOCKET networkCom::get_server_socket(void)
{
    if(m_net_status==net_server)
    {
        return m_ServerCom.get_server_socket();
    }

    if(m_net_status==net_client)
    {
        return m_ClientCom.get_server_socket();
    }

    return SOCKET();
}

void networkCom::lost_connection(void)
{
    m_connected_to_server=false;
}

bool networkCom::clean_up(void)
{
    m_net_status=net_error;

    m_ServerCom.clean_up();
    m_ClientCom.clean_up();

    return true;
}

string networkCom::get_server_IP(void)
{
    if(m_net_status==net_client)
    {
        return m_ClientCom.get_server_IP();
    }

    return "error";
}

bool networkCom::is_connected_to_server(void)
{
    return m_connected_to_server;
}

bool networkCom::set_accept_new_clients_flag(bool flag)
{
    m_accept_new_clients=flag;
    return true;
}

bool networkCom::get_accept_new_clients_flag(void)
{
    return m_accept_new_clients;
}

