#ifndef TEXT_INPUT_BOX_H
#define TEXT_INPUT_BOX_H

#include "sound.h"
#include "display.h"
#include <string>
#include <gl\gl.h>

using namespace std;

class text_input_box
{
    public:
        //Constructors
        text_input_box();
        text_input_box(int x_pos,int y_pos,int width,int height,int max_char,int font_texture[3],string text);
        text_input_box(int x_pos,int y_pos,int width,int height,int max_char,int font_texture[3],string text,sound* pSound);
        //Variables
        bool m_ready;
        //Functions
        bool   input_text(string text_string);
        bool   set_text(string sText);
        string get_text(void);
        bool   set_value(float value);
        float  get_value(void);
        bool   mark_test(int mouse_x_pos,int mouse_y_pos);
        bool   is_marked(void);
        bool   unmark(void);
        bool   change_border_color(float color[3]);
        bool   set_only_number(bool flag);
        bool   update(bool keys[],float time);
        bool   draw(void);
        bool   set_text_alignment(int value);//0 left, 1 right, 2 center
        bool   set_back_texture(int texture,int tex_max_x,int tex_min_x,int tex_max_y,int tex_min_y,int shift_y);

    private:
        //Variables
        bool    m_sound_enabled,m_show_texture;
        bool    m_only_number_input;
        int     m_x_pos,m_y_pos,m_width,m_height,m_max_char;
        int     m_back_texture,m_tex_max_x,m_tex_min_x,m_tex_max_y,m_tex_min_y,m_texture_shift;
        int     m_text_alignment;
        bool    m_caps_lock,m_marked;//if marked, input will be put in text display
        float   m_key_delay[256],m_time_last_cycle;
        string  m_text;
        display m_diTextBox;
        sound*  m_pSound;
        //Functions
        bool add_char(char letter);
        bool erase_char(void);
        char int_to_char(int key_id);
};



#endif // TEXT_INPUT_BOX_H
