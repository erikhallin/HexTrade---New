#ifndef DISPLAY_H
#define DISPLAY_H

#include <iostream> //temp XXXXXXXXXXXXXXX

#include <sstream> //for float to strign conversion
#include <stdio.h> //for char test
#include <ctype.h> //for char test
#include <string>
#include <gl\gl.h>

using namespace std;

enum alignments
{
    alig_right=0,
    alig_left,
    alig_center
};

class display
{
    public:
        //Constructors
        display();
        display(int x_pos,int y_pos,int width,int height,int max_char,int font_texture[3],float value);
        display(int x_pos,int y_pos,int width,int height,int max_char,int font_texture[3],string text);
        //Variables
        bool m_ready;
        //Functions
        bool   set_value(float value);
        float  get_value(void);
        bool   add_value(float value);
        bool   set_text(string text);
        string get_text(void);
        bool   draw_display(void);
        bool   setting_flags(bool border);
        bool   setting_flags(bool border,float colorRGB[3]);
        bool   set_text_color(float colorRGB[3]);
        bool   set_text_alignment(int value);//0 left, 1 right, 2 center
        bool   set_back_texture(int texture,int tex_max_x,int tex_min_x,int tex_max_y,int tex_min_y);

    private:
        //Variables
        int    m_x_pos,m_y_pos,m_width,m_height;//screen pos
        int    m_max_char;
        bool   m_show_text,m_show_value,m_bright_font,m_show_border,m_show_background;
        int    m_text_alignment;
        string m_text;
        float  m_value;
        float  m_char_plate[12];
        float  m_char_width;
        float  m_border_color[3];
        float  m_text_color[3];
        int    m_font_texture[3],m_back_texture;
        int    m_tex_max_x,m_tex_min_x,m_tex_max_y,m_tex_min_y;
        //Functions
        bool   calc_text_scale(void);
        bool   draw_letters_left(string text);
        bool   draw_letters_right(string text);
        bool   draw_letters_center(string text);
};

#endif // DISPLAY_H
