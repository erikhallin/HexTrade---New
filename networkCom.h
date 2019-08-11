
/* INSTRUCTIONS:

#include <winsock2.h> //must be included and libws2_32.a must be linked
#define WM_WSAASYNC (WM_USER +5) //for Async windows messages

FOR SERVER:

1. Construct an networkCom object.
networkCom g_NetCom;

2. Initialize by calling init(string server_or_client).
g_NetCom.init("server");

3. Set which network port to use with set_port_and_bind(int port).
g_NetCom.set_port_and_bind(5001);

4. Make sockets Async by calling WSAAsyncSelect(SOCKET mySocket,HWND window_handle,windows_messages...).
WSAAsyncSelect( g_NetCom.get_server_socket() , hwnd, WM_WSAASYNC, FD_READ | FD_WRITE | FD_ACCEPT | FD_CLOSE);

5. Allow clients to join by calling start_to_listen(int backLog).
g_NetCom.start_to_listen(10);


Inside message handler:
case WM_WSAASYNC:
{
    // what word?
    switch(WSAGETSELECTEVENT(lParam))
    {
        case FD_READ: //incomming data from SOCKET wParam
        {
            cout<<"FD_READ\n";
        } break;

        case FD_WRITE: //only used if sending large files
        {
            cout<<"FD_WRITE\n";
        } break;

        case FD_ACCEPT: // client wants to join
        {
            cout<<"FD_ACCEPT\n";
            if(g_NetCom.add_client(wParam)) cout<<"New Client Joined\n";
            else cout<<"Bad Client tried to join\n";
            return(0);
        } break;

        case FD_CLOSE: // lost client
        {
            cout<<"FD_CLOSE\n";
            if(g_NetCom.remove_client(wParam))
            {
                cout<<"Client Removed\n";
            }

        } break;

    }
}


FOR CLIENT:

1. Construct an networkCom object.
networkCom g_NetCom;

2. Initialize by calling init(string server_or_client).
g_NetCom.init("client");

3. Make sockets Async by calling WSAAsyncSelect(SOCKET mySocket,HWND window_handle,windows_messages...).
WSAAsyncSelect( g_NetCom.get_server_socket() , hwnd, WM_WSAASYNC, FD_WRITE | FD_CONNECT | FD_READ | FD_CLOSE);

4. Try to connect to server by calling connect_to_server(string IP_number,int port).
g_NetCom.connect_to_server("192.168.0.1",5001);


Inside message handler:
case WM_WSAASYNC:
{
    // what word?
    switch(WSAGETSELECTEVENT(lParam))
    {
        case FD_READ: //incomming data from SOCKET wParam
        {
            cout<<"FD_READ\n";
        } break;

        case FD_WRITE: //only used if sending large files
        {
            cout<<"FD_WRITE\n";
        } break;

        case FD_CONNECT: //Client is now connected to server
        {
            cout<<"FD_CONNECT\n";

            //Test Connection
            if(g_NetCom.test_connection())
            {
                cout<<"You are now connected to the server\n";
            }
            else//not connected
            {
                cout<<"Could not connect to server\n";
                break;
            }
        } break;

        case FD_CLOSE: //lost connection to server
        {
            cout<<"FD_CLOSE\n";
            g_NetCom.lost_connection();
        } break;

    }
}

FOR BOTH:

SENDING data is done by calling send_data()
Data format is either a string or a float*

string data_string("test");
g_NetCom.send_data(data_string);
or
float* data_array = new float[size];
data_array[0] = size; //First element must indicate size of array (number of elements)
g_NetCom.send_data(data_array);
delete[] data_array;

The Client always sends to the Server
The Server sends to all Clients or to specific client by specifying a SOCKET send_data(data,client_socket)


RECEIVING data is handled inside the message handler (FD_READ)
Either a string or a float* is received

string data_string;
g_NetCom.recv_data(data_string);
or
float data_array[256]; //maximum size is 256 elements
g_NetCom.recv_data(data_array);

For the Server the senders SOCKET must be provided recv_data(data,client_socket), (in message handler the socket is wParam)
For the Client the sender is assumed to be the servers socket


Both the sending and receiving functions will return false if an error occured


BROADCASTING:

The Client will broadcast it's IP adress to all IPs within the local "net" (default 255.255.255.255).
The Server will periodically check if any broadcasted IP was intercepted.
If the Server receives a broadcasted, valid IP, the Server will reply to that IP and send the IP and port to the Server.
The Client will periodically check if any reply to the broadcasted message has been received.
If the Client receives a reply to the broadcasted message, that contains a valid IP, this IP and port will be stored.



Usage:
The Client broadcasts it's IP by calling broadcast_my_ip().
g_NetCom.broadcast_my_ip();

The Server listens for broadcasts by calling check_for_broadcast(), reply will be sent automatically.
g_NetCom.check_for_broadcast();

The Client listens for broadcast reply by calling check_for_broadcast_reply(), returns true if received reply.
g_NetCom.check_for_broadcast_reply();

The Servers IP and port will be stored in the clientCom object and is obtained by calling get_server_IP_and_port(string IP_and_port_container).
g_NetCom.get_server_IP_and_port(IP_and_port_container);

To change the broadcast listen and reply port for the Server use set_broadcast_port(int port_sending, int port_replying).
To change the broadcast net, listen and reply port for the Client use set_broadcast_port(string net, int port_sending, int port_replying).

*/


#ifndef NETWORKCOM_H
#define NETWORKCOM_H

#include <iostream> //For Debug

#include <string>
#include <vector>
#include <winsock2.h>
#include "serverCom.h"
#include "clientCom.h"

using namespace std;

enum net_statuses{net_error,net_server,net_client};

struct net_packet
{
    net_packet()
    {
        soc_client_recv=0;
        send_only_once=false;
        delay_sec=0.0;
        pac_id=0;
    }
    net_packet(int socket,char* data,int id)
    {
        soc_client_recv=socket;
        data_array=data;
        send_only_once=false;
        resend_sec=0.0;
        pac_id=id;
        delay_sec=0.0;
    }
    net_packet(int socket,char* data,int id,float delay,bool resend_flag)
    {
        soc_client_recv=socket;
        data_array=data;
        send_only_once=resend_flag;
        resend_sec=delay;
        pac_id=id;
        delay_sec=0.0;
    }

    int soc_client_recv;//SOCKET
    char* data_array;
    bool send_only_once;
    float delay_sec,resend_sec;
    int pac_id;
};

class networkCom
{
    public:
        networkCom();
        //Variables
        bool m_ready;
        //Functions
        int  get_status(void);
        bool block_trap(void);
        bool init(string status);
        bool connect_to_server(string sIP,int port);                                //only for CLIENT
        bool set_port_and_bind(int port);                                           //only for SERVER
        bool start_to_listen(int backLog);                                          //only for SERVER
        bool test_connection(void);                                                 //only for CLIENT
        bool set_broadcast_port(int port_sending, int port_replying);               //only for SERVER
        bool set_broadcast_port(string net, int port_sending, int port_replying);   //only for CLIENT
        bool known_socket(SOCKET new_socket);                                       //only for SERVER
        bool broadcast_my_ip(void);                                                 //only for CLIENT
        bool check_for_broadcast(void);                                             //only for SERVER
        bool check_for_broadcast_reply(void);                                       //only for CLIENT
        bool get_server_IP_and_port(string& IP_and_port);                           //only for CLIENT
        bool send_data(string data_string);
        bool send_data(char* data_array);
        bool send_data(string data_string,SOCKET SocReceiver);                      //only for SERVER
        bool send_data(char* data_array,SOCKET SocReceiver);                        //only for SERVER
        bool recv_data(string& data_string,SOCKET SocSender);                       //only for SERVER
        bool recv_data(char* data_array,SOCKET SocSender);                          //only for SERVER
        bool add_client(SOCKET& new_client);                                        //only for SERVER
        bool remove_client(SOCKET client_to_remove);                                //only for SERVER
        SOCKET get_server_socket(void);
        void lost_connection(void);
        bool clean_up(void);
        string get_server_IP(void);                                                 //only for CLIENT
        bool is_connected_to_server(void);
        bool set_accept_new_clients_flag(bool flag);                                //only for SERVER
        bool get_accept_new_clients_flag(void);                                     //only for SERVER

    private:
        //Variables
        bool      m_accept_new_clients;                                             //only for SERVER
        bool      m_connected_to_server;                                            //only for CLIENT
        int       m_net_status;
        int       m_clients_counter;                                                //only for SERVER
        serverCom m_ServerCom;                                                      //only for SERVER
        clientCom m_ClientCom;                                                      //only for CLIENT
        //Functions
};

#endif // NETWORKCOM_H
