#include "button.h"

button::button()
{
    m_ready=false;
}

button::button(int x_pos,int y_pos,int width,int height,int texture,
               float tex_offset_x_min,float tex_offset_x_max,float tex_offset_y_min,float tex_offset_y_max)
{
    float texture_size=1024;//number of pixels on sides
    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;
    m_texture=texture;

    m_x_offset_min=tex_offset_x_min/texture_size;
    m_x_offset_max=tex_offset_x_max/texture_size;
    m_y_offset_min=tex_offset_y_min/texture_size;
    m_y_offset_max=tex_offset_y_max/texture_size;

    m_state=0;
    m_toggle=false;

    m_ready=true;
}

bool button::set_toggle(bool toggle)
{
    m_toggle=toggle;
    return true;
}

bool button::draw_button(void)
{
    if(!m_ready) return false;

    glEnable(GL_BLEND);

    //draw semitransparent square (optional)
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0,0,0,0.8);
    glBegin(GL_QUADS);
    glVertex2f(m_x_pos+15,m_y_pos+12);
    glVertex2f(m_x_pos+15,m_y_pos+m_height-12);
    glVertex2f(m_x_pos+m_width-10,m_y_pos+m_height-12);
    glVertex2f(m_x_pos+m_width-10,m_y_pos+12);
    glEnd();

    glBlendFunc(GL_ONE,GL_ONE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_texture);
    glColor3f(1,1,1);//show true color of texture



    float x_offset_min=m_x_offset_min;
    float y_offset_min=1-m_y_offset_min;
    float x_offset_max=m_x_offset_max;
    float y_offset_max=1-m_y_offset_max;

    float texture_size=1024;//number of pixels on sides
    //shift texture offset down if state!=0
    switch(m_state)
    {
        case 0://Idle
        {
            ;
        }
        break;

        case 1://Hoover
        {
            float d_tex_height=m_y_offset_min-m_y_offset_max-1/texture_size;
            y_offset_min+=1*d_tex_height;
            y_offset_max+=1*d_tex_height;
        }
        break;

        case 2://Pressed
        {
            float d_tex_height=m_y_offset_min-m_y_offset_max-1/texture_size;
            y_offset_min+=2*d_tex_height;
            y_offset_max+=2*d_tex_height;
        }
        break;
    }

    glBegin(GL_QUADS);
    glTexCoord2f(x_offset_min,y_offset_min);
    glVertex2f(m_x_pos,m_y_pos);
    glTexCoord2f(x_offset_min,y_offset_max);
    glVertex2f(m_x_pos,m_y_pos+m_height);
    glTexCoord2f(x_offset_max,y_offset_max);
    glVertex2f(m_x_pos+m_width,m_y_pos+m_height);
    glTexCoord2f(x_offset_max,y_offset_min);
    glVertex2f(m_x_pos+m_width,m_y_pos);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    //if(!m_toggle) m_state=0;//reset state, tested every frame

    return true;
}

bool button::overlap_test(int mouse_x,int mouse_y)
{
    if(mouse_x>m_x_pos&&mouse_x<m_x_pos+m_width&&
       mouse_y>m_y_pos&&mouse_y<m_y_pos+m_height)
    {
        return true;
    }
    return false;
}
