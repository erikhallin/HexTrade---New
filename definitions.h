#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <vector>
#include <stdlib.h>//rand
#include <math.h>
#include <string>

#define _version 1.27

#define _world_size_x 200
#define _world_size_y 200

#define _mission_expansion_time 0.20
#define _mission_trade_time 0.20
#define _anim_trade_time 0.10 //time per tile
#define _max_route_length 100
#define _city_growth_time 5.0
#define _city_starve_time 10.0

#define _pop_adjust_time_check 5.0

#define _network_send_interval 0.01
#define _network_resend_delay 0.1
#define _network_packet_store_time 10.0
#define _network_timeout_limit 20.0
#define _network_city_waiting_for_response_limit 2.0

#define _zoom_start 50.0

#define _gameover_anim_dark_time 5.0
#define _fps_interval_delay 2.0

using namespace std;

struct st_point_int
{
    st_point_int()
    {
        x=y=0;
    }
    st_point_int(int x_in,int y_in)
    {
        x=x_in;
        y=y_in;
    }

    int x,y;
};

struct st_point_float
{
    st_point_float()
    {
        x=y=0;
    }
    st_point_float(float x_in,float y_in)
    {
        x=x_in;
        y=y_in;
    }

    float x,y;
};

struct st_pop
{
    st_pop()
    {
        x_pos=y_pos=0;
    }
    st_pop(float x_in,float y_in)
    {
        x_pos=x_in;
        y_pos=y_in;
        brightness=0.0;
        flag_new=true;
        flag_remove=false;
        lifetime=0;
    }

    float x_pos,y_pos;
    float brightness;
    bool flag_new,flag_remove;
    float lifetime;
};

struct st_coord
{
    st_coord()
    {
        x_hex=y_hex=0;
    }
    st_coord(int x,int y)
    {
        x_hex=x; y_hex=y; z_hex=-(x+y);
        x_box_shift=y_box_shift=0;
    }
    st_coord(int x,int y,int box_shift_x,int box_shift_y)
    {
        x_hex=x; y_hex=y; z_hex=-(x+y);
        x_box_shift=box_shift_x; y_box_shift=box_shift_y;
    }

    int x_hex,y_hex,z_hex;
    int x_box_shift,y_box_shift;

    int distance(st_coord pos2)
    {
        int x_dif=x_hex-pos2.x_hex;
        int y_dif=y_hex-pos2.y_hex;
        int z_dif=z_hex-pos2.z_hex;
        //make abs
        if(x_dif<0) x_dif*=-1;
        if(y_dif<0) y_dif*=-1;
        if(z_dif<0) z_dif*=-1;
        //get distance
        int dist=x_dif;
        if(y_dif>dist) dist=y_dif;
        if(z_dif>dist) dist=z_dif;
        return dist;
    }
};

struct st_coord_route
{
    st_coord_route()
    {
        x_hex=y_hex=0;
        x_box_shift=y_box_shift=0;
    }
    st_coord_route(int x,int y)
    {
        x_hex=x; y_hex=y; z_hex=-(x+y);
        x_box_shift=0; y_box_shift=0;
        road[0]=-1; road[1]=-1;
        for(int i=0;i<6;i++) road_allowed_exits[i]=true;
    }
    st_coord_route(int x,int y,int box_shift_x,int box_shift_y)
    {
        x_hex=x; y_hex=y; z_hex=-(x+y);
        x_box_shift=box_shift_x; y_box_shift=box_shift_y;
        road[0]=-1; road[1]=-1;
        for(int i=0;i<6;i++) road_allowed_exits[i]=true;
    }

    int  x_hex,y_hex,z_hex;
    int  x_box_shift,y_box_shift;
    int  road[2];//road direction, 0 - 5, 0 is top left going ccw
    bool road_allowed_exits[6];

    int distance(st_coord_route pos2)
    {
        int x_dif=(x_hex+x_box_shift*_world_size_x)-(pos2.x_hex+pos2.x_box_shift*_world_size_x);
        int y_dif=(y_hex+y_box_shift*_world_size_y)-(pos2.y_hex+pos2.y_box_shift*_world_size_y);
        int this_z=-(x_hex+x_box_shift*_world_size_x+y_hex+y_box_shift*_world_size_y);
        int other_z=-(pos2.x_hex+pos2.x_box_shift*_world_size_x+pos2.y_hex+pos2.y_box_shift*_world_size_y);
        int z_dif=this_z-other_z;
        //make abs
        if(x_dif<0) x_dif*=-1;
        if(y_dif<0) y_dif*=-1;
        if(z_dif<0) z_dif*=-1;
        //get distance
        int dist=x_dif;
        if(y_dif>dist) dist=y_dif;
        if(z_dif>dist) dist=z_dif;
        return dist;
    }
    int distance(int target_x,int target_y )
    {
        int x_dif=x_hex-target_x;
        int y_dif=y_hex-target_y;
        int z_dif=z_hex-(-(target_x+target_y));
        //make abs
        if(x_dif<0) x_dif*=-1;
        if(y_dif<0) y_dif*=-1;
        if(z_dif<0) z_dif*=-1;
        //get distance
        int dist=x_dif;
        if(y_dif>dist) dist=y_dif;
        if(z_dif>dist) dist=z_dif;
        return dist;
    }
};

struct st_coord_chararr
{
    st_coord_chararr()
    {
        x_hex=y_hex=0;
        for(int i=0;i<6;i++) border[i]='n';
        border_counter=0;
    }
    st_coord_chararr(int x,int y)
    {
        x_hex=x; y_hex=y; z_hex=-(x+y);
        for(int i=0;i<6;i++) border[i]='n';
        x_shift_box=y_shift_box=0;
        border_counter=0;
    }
    st_coord_chararr(int x,int y,int x_shift,int y_shift)
    {
        x_hex=x; y_hex=y; z_hex=-(x+y);
        x_shift_box=x_shift; y_shift_box=y_shift;
        for(int i=0;i<6;i++) border[i]='n';
        border_counter=0;
    }

    int  x_hex,y_hex,z_hex;
    char border[6];// 'n' is non border, 'b' is border (starting with top-left, going ccw)
    int  border_counter,neighbour_border_counter;
    int  x_shift_box,y_shift_box;
    int  pop_max;
    vector<st_pop> pop_pos;

    int distance(st_coord_chararr pos2)
    {
        int x_dif=x_hex-pos2.x_hex;
        int y_dif=y_hex-pos2.y_hex;
        int z_dif=z_hex-pos2.z_hex;
        //make abs
        if(x_dif<0) x_dif*=-1;
        if(y_dif<0) y_dif*=-1;
        if(z_dif<0) z_dif*=-1;
        //get distance
        int dist=x_dif;
        if(y_dif>dist) dist=y_dif;
        if(z_dif>dist) dist=z_dif;
        return dist;
    }
};

struct st_coord_w_val
{
    st_coord_w_val()
    {
        x_hex=y_hex=val=0;
    }
    st_coord_w_val(int x,int y,int v)
    {
        x_hex=x;
        y_hex=y;
        val=v;
    }

    int x_hex,y_hex;
    int val;
};

enum tile_types
{
    tt_error=0,
    tt_land,
    tt_land_w_road,
    tt_land_w_city,
    tt_water,
    tt_water_w_road,
    tt_rock,
};

enum city_modes
{
    cm_growth=0,
    cm_starve,
    cm_idle,
    //cm_forced_idle,
    cm_listen_expansion,
    cm_listen_trade,
    cm_expansion_mission,
    cm_trade_mission,
    cm_waiting_for_response
};

enum game_states
{
    gs_error=0,
    gs_init,
    gs_menu_main,
    //gs_menu_host,
    //gs_menu_join,
    gs_running,
    gs_game_over
};

enum menu_states
{
    ms_error=0,
    ms_main,
    ms_host,
    ms_join,
    ms_lobby,
    ms_edit,
    ms_game_over
};

struct st_player
{
    st_player()
    {
        id=-1;
        send_delay=0.0;
        sent_pac_this_cycle=false;
    }
    string name;
    int   id;//same as socket
    float send_delay;//time until packet can be sent to that player
    bool  sent_pac_this_cycle;//for server, to track if a packet have been sent to this player this cycle
};

struct st_id_and_time
{
    st_id_and_time(int socket,int id,float time)
    {
        sock_id=socket;
        pac_id=id;
        time_left=time;
    }
    int pac_id;
    int sock_id;
    float time_left;
};

struct st_string_and_int
{
    string name;
    int value;
};

/*Error flags

1. Client received a bad expansion route from server.
2. Server received a bad expansion route from client.
3. Client received a bad trade route from server.
4. Server received a bad trade route from client. Not fatal.
5. Server have a packet to be sent to a client that is not in the players vector.
6. Packet without receiver among players.

*/


#endif
