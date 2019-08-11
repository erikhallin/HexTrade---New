#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <fcntl.h> //for console
#include <stdio.h>
#include <string>
#include <winsock2.h>
#include "game.h"
#include "networkCom.h"
#include "resource.h"

#define WM_WSAASYNC (WM_USER +5) //for Async windows messages

using namespace std;

int g_window_width=1024;//800;
int g_window_height=768;//600;

game*       pGame;
networkCom* pNetCom;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    string command_line=lpCmdLine;
    bool debug_mode=false;
    bool multisend=true;
    //get commands
    vector<string> vec_commands;
    string word;
    stringstream ss(command_line);
    while(ss>>word)
    {
        vec_commands.push_back(word);
    }
    //interpret commands
    for(int com=0;com<(int)vec_commands.size();com++)
    {
        if(vec_commands[com]=="debug")
        {
            debug_mode=true;
        }
        else if(vec_commands[com]=="no_multisend")
        {
            multisend=false;
        }
    }

    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    //register window class
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_HEX));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "CityLinker";
    wcex.hIconSm = LoadIcon(NULL, NULL);


    if (!RegisterClassEx(&wcex))
        return 0;

    //create main window
    hwnd = CreateWindowEx(0,
                          "CityLinker",
                          "CityLinker",
                          //WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,//for locked pos
                          //WS_OVERLAPPEDWINDOW,// for window
                          (WS_OVERLAPPEDWINDOW | WS_SYSMENU) - (WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME),
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          g_window_width,
                          g_window_height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    //if debug mode start console
    if(debug_mode)
    {
        //Open a console window
        AllocConsole();
        //Connect console output
        HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
        int hCrt          = _open_osfhandle((long) handle_out, _O_TEXT);
        FILE* hf_out      = _fdopen(hCrt, "w");
        setvbuf(hf_out, NULL, _IONBF, 1);
        *stdout = *hf_out;
        //Connect console input
        HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
        hCrt             = _open_osfhandle((long) handle_in, _O_TEXT);
        FILE* hf_in      = _fdopen(hCrt, "r");
        setvbuf(hf_in, NULL, _IONBF, 128);
        *stdin = *hf_in;
        //Set console title
        SetConsoleTitle("Debug Console");
    }

    //enable OpenGL for the window
    EnableOpenGL(hwnd, &hDC, &hRC);

    pNetCom=new networkCom();
    pNetCom->block_trap();
    pGame=new game(g_window_width,g_window_height,pNetCom);

    //tell game about commands
    pGame->set_debug_mode(debug_mode);
    pGame->set_multisend(multisend);

    //program main loop
    while (!bQuit)
    {
        //check for messages
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            //handle or dispatch messages
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            int status=pGame->cycle();
            switch(status)
            {
                case 4://join button pressed
                {
                    cout<<"Network: Now Client\n";
                    pNetCom->init("client");
                    WSAAsyncSelect( pNetCom->get_server_socket() , hwnd, WM_WSAASYNC, FD_WRITE | FD_CONNECT | FD_READ | FD_CLOSE);
                    //broadcast to get server ip
                    if( !pNetCom->broadcast_my_ip() ) cout<<"ERROR: Network: Broadcast problem\n";
                    pGame->set_check_for_broadcast_reply_flag(true);
                }break;

                case 6://exit
                {
                    bQuit=true;
                }break;

                case 7://start hosting
                {
                    cout<<"Network: Now Server\n";
                    pNetCom->init("server");
                    pNetCom->set_port_and_bind(5001);
                    WSAAsyncSelect( pNetCom->get_server_socket() , hwnd, WM_WSAASYNC, FD_READ | FD_WRITE | FD_ACCEPT | FD_CLOSE);
                    pNetCom->start_to_listen(10);
                    pGame->set_check_for_broadcast_flag(true);
                    pGame->add_server_player();
                }break;

                case 8://try to join server
                {
                    if( !pNetCom->is_connected_to_server() )
                    {
                        cout<<"Network: Connecting to server...\n";
                        pNetCom->connect_to_server( pGame->get_server_ip() ,5001);
                    }
                    else cout<<"Network: Already connected to a server\n";

                }break;

                case 9://server starts the game
                {

                }break;

                case 10://backed out from lobby or join menu, reset network
                {
                    cout<<"Network: Reset\n";
                    pGame->set_check_for_broadcast_flag(false);
                    pGame->set_check_for_broadcast_reply_flag(false);
                    pNetCom->clean_up();
                }break;

            }

            //check for broadcast
            switch( pNetCom->get_status() )
            {
                case net_server://check for broadcast from client
                {
                    if( pGame->get_check_for_broadcast_flag() )
                    {
                        pNetCom->check_for_broadcast();
                    }
                }break;

                case net_client://check for broadcast reply from server
                {
                    if( pGame->get_check_for_broadcast_reply_flag() )
                    {
                        if( pNetCom->check_for_broadcast_reply() )
                        {//server responded
                            //cout<<"\nRESPONSE fom serverN\n\n";
                            pGame->set_check_for_broadcast_reply_flag(false);
                            pGame->set_server_ip( pNetCom->get_server_IP() );
                        }
                    }
                }break;
            }

            SwapBuffers(hDC);
        }
    }

    pNetCom->clean_up();
    delete pNetCom;
    delete pGame;

    //shutdown OpenGL
    DisableOpenGL(hwnd, hDC, hRC);

    //destroy the window explicitly
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_MOUSEMOVE:
        {
             pGame->set_mouse_pos( int( LOWORD(lParam) ),int( HIWORD(lParam) ) );
        }
        break;

        case WM_LBUTTONDOWN:
        {
             pGame->set_mouse_button_left(true);
        }
        break;

        case WM_LBUTTONUP:
        {
             pGame->set_mouse_button_left(false);
        }
        break;

        case WM_RBUTTONDOWN:
        {
             pGame->set_mouse_button_right(true);
        }
        break;

        case WM_RBUTTONUP:
        {
             pGame->set_mouse_button_right(false);
        }
        break;

        case WM_MOUSEWHEEL:
        {
            if(HIWORD(wParam)>5000)
                pGame->set_mouse_scroll_down(true);
            if(HIWORD(wParam)>100&&HIWORD(wParam)<5000)
                pGame->set_mouse_scroll_up(true);
        }
        break;

        case WM_KEYDOWN:
		{
			pGame->set_keyboard_key( int(wParam),true );
		}
		break;

		case WM_KEYUP:
		{
			pGame->set_keyboard_key( int(wParam),false );
		}
		break;

		case WM_WSAASYNC://network message
        {
            // what word?
            switch(WSAGETSELECTEVENT(lParam))
            {
                case FD_READ: //incomming data from SOCKET wParam
                {
                    cout<<"FD_READ\n";
                    pGame->recv_data(wParam);
                }break;

                case FD_WRITE: //only used if sending large files
                {
                    cout<<"FD_WRITE\n";
                }break;

                case FD_ACCEPT: //client wants to join
                {
                    cout<<"FD_ACCEPT\n";
                    if(pNetCom->add_client(wParam))
                    {
                        cout<<"Network: New Client connected, Socket: "<<int(wParam)<<endl;
                    }
                    else cout<<"Network: Bad Client tried to connect\n";
                    //test if server allows new players to join
                    if(!pNetCom->get_accept_new_clients_flag())
                    {
                        cout<<"Network: Server does not accept new clients in this state\n";
                        pGame->send_client_denied_package(wParam);
                    }

                    return(0);
                }break;

                case FD_CONNECT: //Client is now connected to server
                {
                    cout<<"FD_CONNECT\n";

                    //Test Connection
                    if(pNetCom->test_connection())
                    {
                        cout<<"You are now connected to the server, Socket: "<<int(pNetCom->get_server_socket())<<endl;
                        //send start package to server with name
                        pGame->send_start_package_to_server();
                    }
                    else//not connected
                    {
                        cout<<"Could not connect to server\n";
                        break;
                    }
                } break;

                case FD_CLOSE: // lost client
                {
                    cout<<"FD_CLOSE\n";
                    if(pNetCom->get_status()==net_server)
                    {
                        if(pNetCom->remove_client(wParam))
                        {
                            cout<<"Client Removed\n";
                            pGame->lost_player(wParam);
                        }
                    }
                    else//client
                    {
                        cout<<"Lost connection to Server\n";
                        pNetCom->lost_connection();
                        pGame->lost_server();
                    }
                }break;

            }
        }

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    //set 2D mode
    glClearColor(0.0,0.0,0.0,0.0);  //Set the cleared screen colour to black
    glViewport(0,0,g_window_width,g_window_height);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window

    //Set up the orthographic projection so that coordinates (0, 0) are in the top left
    //and the minimum and maximum depth is -10 and 10. To enable depth just put in
    //glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,g_window_width,g_window_height,0,-1,1);

    //Back to the modelview so we can draw stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);

    //3D
    /*glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	gluPerspective(45.0f,(GLfloat)g_window_width/(GLfloat)g_window_height, 0.1f, 10000.0f);*/
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

