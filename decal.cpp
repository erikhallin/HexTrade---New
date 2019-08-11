#include "decal.h"

decal::decal()
{
    m_ready=false;
}

decal::decal(int x_pos,int y_pos,int width,int height,int texture,float tex_offset_x_min,float tex_offset_x_max,float tex_offset_y_min,float tex_offset_y_max)
{
    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;
    m_texture=texture;
    float texture_size=1024;
    m_x_offset_min=tex_offset_x_min/texture_size;
    m_y_offset_min=tex_offset_y_min/texture_size;
    m_x_offset_max=tex_offset_x_max/texture_size;
    m_y_offset_max=tex_offset_y_max/texture_size;

    m_brightness=1.0;
    m_show=true;
    m_have_mask=true;
    m_draw_transparent=false;
    m_ready=true;
}

bool decal::set_location(int x_pos,int y_pos,int width,int height)
{
    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;

    return true;
}

bool decal::shift_texture_offset(float x_shift,float y_shift)
{
    float texture_size=1024;
    float d_y=m_y_offset_max-m_y_offset_min;
    float d_x=m_x_offset_max-m_x_offset_min;
    m_x_offset_min+=x_shift*(d_x+0.5/texture_size);
    m_x_offset_max+=x_shift*(d_x+0.5/texture_size);
    m_y_offset_min+=y_shift*(d_y+0.5/texture_size);
    m_y_offset_max+=y_shift*(d_y+0.5/texture_size);

    return true;
}

bool decal::draw(void)
{
    if(!m_ready||!m_show) return false;

    //Init
    glColor3f(1,1,1);//allows all colors = white text
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    if(m_draw_transparent)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    }

    float x_offset_min=m_x_offset_min;
    float y_offset_min=1-m_y_offset_min;
    float x_offset_max=m_x_offset_max;
    float y_offset_max=1-m_y_offset_max;
    float texture_size=1024;
    float d_y=m_y_offset_max-m_y_offset_min;
    float d_x=m_x_offset_max-m_x_offset_min;

    if(m_have_mask)
    {
        //Draw mask
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_COLOR,GL_ZERO);

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

        //Draw picture
        glColor3f(m_brightness,m_brightness,m_brightness);
        y_offset_min-=(d_y+1/texture_size);
        y_offset_max-=(d_y+1/texture_size);

        glDepthMask(GL_TRUE);
        glBlendFunc(GL_ONE,GL_ONE);

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

        glDisable(GL_BLEND);
    }
    else
    {
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
    }

    glDisable(GL_TEXTURE_2D);
    if(m_draw_transparent)
    {
        glDisable(GL_BLEND);
    }

    return true;
}

bool decal::masking(bool flag)
{
    m_have_mask=flag;
    return true;
}

bool decal::show_decal(bool flag)
{
    m_show=flag;
    return true;
}

bool decal::set_brightness(float value)
{
    m_brightness=value;

    return true;
}

bool decal::draw_transparent(bool flag)
{
    m_draw_transparent=flag;

    return true;
}
