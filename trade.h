#ifndef TRADE_H
#define TRADE_H

#include <gl/gl.h>
#include <vector>
#include <iostream>

#include "definitions.h"

using namespace std;

class trade
{
    public:
        trade();
        trade(vector<st_coord_route> vec_trade_route,int m_id,int city_id_a,int city_id_b);

        bool update(float time);
        bool draw(float zoom_level);
        bool get_trade_route_list(vector<st_coord_route>& vec_trade_route_list);
        int  get_trade_id(void);
        int  get_city_a_id(void);
        int  get_city_b_id(void);
        int  get_trade_distance(void);
        bool remove_tile_from_route(int x_hex,int y_hex);//not used
        bool test_trade(void);//not used
        bool city_expansion_update(int from_x_hex,int from_y_hex,int to_x_hex,int to_y_hex);
        bool set_light_pulse(float value);

    private:

        int m_trade_id;
        int m_city_id_a,m_city_id_b;
        float m_distance_bird,m_anim_trade_progress,m_time_since_start,m_light_pulse;
        bool m_anim_reverse;

        //route
        //st_coord m_from_hex,m_to_hex;
        vector<st_coord_route> m_vec_trade_route;
};

#endif // TRADE_H
