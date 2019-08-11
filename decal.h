#ifndef DECAL_H
#define DECAL_H

#include <gl\gl.h>

class decal
{
    public:
        //Constructors
        decal();
          //mask should be located directly beneath texture in same file
          //mask: black will be drawn, not white
          //pic:  black will not be drawn
        decal(int x_pos,int y_pos,int width,int height,int texture,
              float tex_offset_x_min,float tex_offset_x_max,float tex_offset_y_min,float tex_offset_y_max);//send location of texture in pixel count
        //Variables
        bool m_ready;
        //Functions
        bool set_location(int x_pos,int y_pos,int width,int height);
        bool shift_texture_offset(float x_shift,float y_shift);
        bool draw(void);
        bool masking(bool flag);
        bool show_decal(bool flag);
        bool set_brightness(float value);
        bool draw_transparent(bool flag);
    private:
        //Variables
        bool  m_show,m_have_mask,m_draw_transparent;
        float m_brightness;
        int   m_x_pos,m_y_pos,m_width,m_height;
        int   m_texture;
        float m_x_offset_min,m_y_offset_min,m_x_offset_max,m_y_offset_max;//texture offset values
        //Functions

};

#endif // DECAL_H
