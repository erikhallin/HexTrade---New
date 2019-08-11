#ifndef TILE_H
#define TILE_H

#include <iostream>
#include <gl/gl.h>
#include <stdlib.h>//rand
#include <math.h>

#include "definitions.h"

using namespace std;

class tile
{
    public:
        tile();
        tile(float pos_x_hex,float pos_y_hex,int type,int texture);

        bool  update(float time_current);
        bool  draw(float zoom_level,float shift_box_x,float shift_box_y);
        int   selection_test(float zoom_level,float real_mouse_pos_x,float real_mouse_pos_y);
        bool  selection_test_spe_box(float zoom_level,float real_mouse_pos_x,float real_mouse_pos_y);
        float get_distance(float zoom_level,float real_mouse_pos_x,float real_mouse_pos_y);
        bool  set_color(float r,float g,float b);
        bool  is_buildable(void);//true only if unbuilt land
        bool  set_owner(int owner_id,int city_id);//and city id, called when city is built
        bool  set_tile_type(int type,int id);
        int   get_tile_type(void);
        int   get_city_id(void);
        int   get_city_owner_id(void);
        int   get_trade_id(int selector);

    private:
        //pos
        float m_pos_x_pix,m_pos_y_pix;
        int   m_pos_x_hex,m_pos_y_hex;
        float m_shift_box_size_x,m_shift_box_size_y;

        int   m_tile_type;
        int   m_owned_by_player,m_city_id,m_trade_id[3];
        float m_brightness,m_brightness_rate;

        bool  m_temp_color;//temporary color instead of type color
        int   m_texture;
        int   m_decal_type;
        float m_color[3];
};

#endif // TILE_H
