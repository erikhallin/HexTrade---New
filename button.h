#ifndef BUTTON_H
#define BUTTON_H

#include <gl\gl.h>
#include <iostream> //temp xxxxxxx

class button
{
    public:
        button();
        button(int x_pos,int y_pos,int width,int height,int texture,
               float tex_offset_x_min,float tex_offset_x_max,float tex_offset_y_min,float tex_offset_y_max);//send location of texture in pixel count

        bool  m_ready;
        int   m_state; // 0 = Idle, 1 = Hoovering, 2 = Pressed


        bool set_toggle(bool toggle);
        bool draw_button(void);
        bool overlap_test(int x,int y);

    private:
        bool  m_toggle;
        int   m_x_pos,m_y_pos,m_width,m_height;
        int   m_texture;
        float m_x_offset_min,m_y_offset_min,m_x_offset_max,m_y_offset_max;//texture offset values


};

#endif // BUTTON_H
