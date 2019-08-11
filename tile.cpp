#include "tile.h"

tile::tile()
{
    m_pos_x_pix=m_pos_y_pix=0;
    m_pos_x_hex=m_pos_y_hex=0;
}

tile::tile(float pos_x_hex,float pos_y_hex,int type,int texture)
{
    m_pos_x_pix=pos_x_hex+pos_y_hex*0.5;
    m_pos_y_pix=pos_y_hex*0.75;
    m_pos_x_hex=pos_x_hex;
    m_pos_y_hex=pos_y_hex;
    m_shift_box_size_x=_world_size_x+_world_size_y*0.5;
    m_shift_box_size_y=_world_size_y*0.75;
    m_texture=texture;
    m_tile_type=type;
    m_trade_id[0]=m_trade_id[1]=m_trade_id[2]=-1;
    m_temp_color=false;
    m_owned_by_player=-1;
    m_decal_type=rand()%10;
    m_brightness=0.5+(rand()%500)/1000.0;
    m_brightness_rate=0.1+float(rand()%3000)/1000.0;// 0.1 - 3.1
}

bool tile::update(float time_current)
{
    m_brightness=0.5+(sinf(time_current*m_brightness_rate)+1.0)/4.0;

    return true;
}

bool tile::draw(float zoom_level,float shift_box_x,float shift_box_y)
{
    glPushMatrix();

    /*//type dependent color
    switch(m_tile_type)
    {
        case tt_error:        glColor3f(1.0,0.0,1.0);break;
        case tt_land:         glColor3f(0.4,0.6,0.4);break;
        case tt_land_w_road:  glColor3f(0.2,0.6,0.2);break;
        case tt_land_w_city:  glColor3f(0.1,0.1,0.1);break;//dont draw tile with city
        case tt_water:        glColor3f(0.3,0.3,0.9);break;
        case tt_water_w_road: glColor3f(0.2,0.2,0.9);break;
        case tt_rock:         glColor3f(0.5,0.1,0.1);break;

        default: glColor3f(1,1,1);
    }*/
    /*if(m_temp_color)//overrule type color
    {
        glColor3f(m_color[0],m_color[1],m_color[2]);
        //m_temp_color=false;
    }*/

    //go to pos
    float scale=zoom_level;
    //glTranslatef( m_pos_x_pix*scale, m_pos_y_pix*scale,0);
    //glTranslatef( (m_pos_x_pix+shift_box_x*m_shift_box_size_x)*scale, (m_pos_y_pix+shift_box_y*m_shift_box_size_y)*scale,0);
    float x_shift=(m_pos_x_hex+_world_size_x*shift_box_x)+(m_pos_y_hex+_world_size_y*shift_box_y)*0.5;
    float y_shift=(m_pos_y_hex+_world_size_y*shift_box_y)*0.75;
    glTranslatef( x_shift*scale, y_shift*scale,0);

    //tile pic
    //shift tex coord to center of tile image
    glColor3f(m_brightness,m_brightness,m_brightness);
    float tex_offset_x=(float)m_decal_type*(100.0/1024.0);
    float tex_offset_y=0;
    switch(m_tile_type)
    {
        case tt_error:        tex_offset_y=3.0*(100.0/1024.0); break;
        case tt_land:         tex_offset_y=2.0*(100.0/1024.0); break;
        case tt_land_w_road:  tex_offset_y=2.0*(100.0/1024.0); break;
        case tt_land_w_city:  tex_offset_y=2.0*(100.0/1024.0); break;
        case tt_water:        tex_offset_y=0.0*(100.0/1024.0); break;
        case tt_water_w_road: tex_offset_y=0.0*(100.0/1024.0); break;
        case tt_rock:         tex_offset_y=1.0*(100.0/1024.0); break;
    }

    //glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D,m_texture);

    /*glBegin(GL_TRIANGLE_FAN);//triangle fan
    glTexCoord2f(50.0/1024.0+tex_offset_x, 1.0-50.0/1024.0-tex_offset_y);
    glVertex2f(0.0,0.0);//center
    glTexCoord2f(50.0/1024.0+tex_offset_x, 1.0-6.0/1024.0-tex_offset_y);
    glVertex2f(0*scale,0.50*scale);//top
    glTexCoord2f(7.0/1024.0+tex_offset_x, 1.0-26.0/1024.0-tex_offset_y);
    glVertex2f(-0.50*scale,0.25*scale);
    glTexCoord2f(7.0/1024.0+tex_offset_x, 1.0-71.0/1024.0-tex_offset_y);
    glVertex2f(-0.5*scale,-0.25*scale);
    glTexCoord2f(50.0/1024.0+tex_offset_x, 1.0-94.0/1024.0-tex_offset_y);
    glVertex2f(0*scale,-0.5*scale);
    glTexCoord2f(93.0/1024.0+tex_offset_x, 1.0-72.0/1024.0-tex_offset_y);
    glVertex2f(0.5*scale,-0.25*scale);
    glTexCoord2f(93.0/1024.0+tex_offset_x, 1.0-26.0/1024.0-tex_offset_y);
    glVertex2f(0.5*scale,0.25*scale);
    glTexCoord2f(50.0/1024.0+tex_offset_x, 1.0-6.0/1024.0-tex_offset_y);
    glVertex2f(0*scale,0.5*scale);
    glEnd();*/

    /*glBegin(GL_TRIANGLE_STRIP);//triangle strip w precalc, not faster
    glTexCoord2f(0.048828125+tex_offset_x, 0.994140625-tex_offset_y);
    glVertex2f(0*scale,0.50*scale);//top
    glTexCoord2f(0.0068359375+tex_offset_x, 0.974609375-tex_offset_y);
    glVertex2f(-0.50*scale,0.25*scale);//tl
    glTexCoord2f(0.0908203125+tex_offset_x, 0.974609375-tex_offset_y);
    glVertex2f(0.5*scale,0.25*scale);//tr
    glTexCoord2f(0.0068359375+tex_offset_x, 0.9306640625-tex_offset_y);
    glVertex2f(-0.5*scale,-0.25*scale);//dl
    glTexCoord2f(0.0908203125+tex_offset_x, 0.9296875-tex_offset_y);
    glVertex2f(0.5*scale,-0.25*scale);//dr
    glTexCoord2f(0.048828125+tex_offset_x, 0.908203125-tex_offset_y);
    glVertex2f(0*scale,-0.5*scale);//d
    glEnd();*/

    glBegin(GL_TRIANGLE_STRIP);//triangle strip
    glTexCoord2f(50.0/1024.0+tex_offset_x, 1.0-6.0/1024.0-tex_offset_y);
    glVertex2f(0*scale,0.50*scale);//top
    glTexCoord2f(7.0/1024.0+tex_offset_x, 1.0-26.0/1024.0-tex_offset_y);
    glVertex2f(-0.50*scale,0.25*scale);//tl
    glTexCoord2f(93.0/1024.0+tex_offset_x, 1.0-26.0/1024.0-tex_offset_y);
    glVertex2f(0.5*scale,0.25*scale);//tr
    glTexCoord2f(7.0/1024.0+tex_offset_x, 1.0-71.0/1024.0-tex_offset_y);
    glVertex2f(-0.5*scale,-0.25*scale);//dl
    glTexCoord2f(93.0/1024.0+tex_offset_x, 1.0-72.0/1024.0-tex_offset_y);
    glVertex2f(0.5*scale,-0.25*scale);//dr
    glTexCoord2f(50.0/1024.0+tex_offset_x, 1.0-94.0/1024.0-tex_offset_y);
    glVertex2f(0*scale,-0.5*scale);//d
    glEnd();

    /*
    glBegin(GL_TRIANGLE_FAN);//hexagon

    glVertex2f(0*scale,0*scale);//center
    glVertex2f(0*scale,0.45*scale);//top
    glVertex2f(-0.45*scale,0.225*scale);
    glVertex2f(-0.45*scale,-0.225*scale);
    glVertex2f(0*scale,-0.45*scale);
    glVertex2f(0.45*scale,-0.225*scale);
    glVertex2f(0.45*scale,0.225*scale);
    glVertex2f(0*scale,0.45*scale);
    */

    /*glBegin(GL_LINE_STRIP);//lines

    glVertex2f(0*scale,0.45*scale);//top
    glVertex2f(-0.45*scale,0.225*scale);
    glVertex2f(-0.45*scale,-0.225*scale);
    glVertex2f(0*scale,-0.45*scale);
    glVertex2f(0.45*scale,-0.225*scale);
    glVertex2f(0.45*scale,0.225*scale);
    glVertex2f(0*scale,0.45*scale);*/

    /*//smaller
    glVertex2f(0*scale,0.40*scale);//top
    glVertex2f(-0.40*scale,0.20*scale);
    glVertex2f(-0.40*scale,-0.20*scale);
    glVertex2f(0*scale,-0.40*scale);
    glVertex2f(0.40*scale,-0.20*scale);
    glVertex2f(0.40*scale,0.20*scale);
    glVertex2f(0*scale,0.40*scale);*/

    /*glEnd();

    //center image
    switch(m_tile_type)
    {
        case tt_error:break;
        case tt_land: switch(m_decal_type)
        {
            case 0:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.21*scale,-0.19*scale);
                glVertex2f(0.15*scale,-0.19*scale);

                glVertex2f(-0.30*scale,0.16*scale);
                glVertex2f(0.20*scale,0.16*scale);
                glEnd();
            }break;
            case 1:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.30*scale,-0.20*scale);
                glVertex2f(-0.03*scale,-0.20*scale);

                glVertex2f(0.05*scale,0.06*scale);
                glVertex2f(0.31*scale,0.06*scale);

                glVertex2f(-0.25*scale,0.22*scale);
                glVertex2f(0.07*scale,0.22*scale);
                glEnd();
            }break;
            case 2:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.25*scale,-0.18*scale);
                glVertex2f(0.30*scale,-0.18*scale);

                glVertex2f(0.10*scale,0.03*scale);
                glVertex2f(0.25*scale,0.03*scale);

                glVertex2f(-0.35*scale,0.10*scale);
                glVertex2f(-0.05*scale,0.10*scale);
                glEnd();
            }break;
            case 3:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.29*scale,-0.09*scale);
                glVertex2f(0.04*scale,-0.09*scale);

                glVertex2f(-0.02*scale,0.21*scale);
                glVertex2f(0.23*scale,0.21*scale);
                glEnd();
            }break;
            case 4:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.27*scale,0.01*scale);
                glVertex2f(0.12*scale,0.01*scale);
                glEnd();
            }break;
            case 5:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.11*scale,-0.11*scale);
                glVertex2f(0.15*scale,-0.11*scale);

                glVertex2f(-0.24*scale,0.22*scale);
                glVertex2f(0.11*scale,0.22*scale);
                glEnd();
            }break;
            case 6:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.11*scale,-0.11*scale);
                glVertex2f(0.15*scale,-0.11*scale);

                glVertex2f(-0.24*scale,0.22*scale);
                glVertex2f(0.11*scale,0.22*scale);
                glEnd();
            }break;
            case 7:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.33*scale,-0.06*scale);
                glVertex2f(-0.18*scale,-0.06*scale);

                glVertex2f(0.13*scale,-0.08*scale);
                glVertex2f(0.27*scale,-0.08*scale);

                glVertex2f(0.03*scale,0.16*scale);
                glVertex2f(0.20*scale,0.16*scale);
                glEnd();
            }break;
            case 8:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.13*scale,-0.21*scale);
                glVertex2f(0.10*scale,-0.21*scale);

                glVertex2f(-0.30*scale,0.00*scale);
                glVertex2f(-0.05*scale,0.00*scale);

                glVertex2f(0.04*scale,0.23*scale);
                glVertex2f(0.27*scale,0.23*scale);
                glEnd();
            }break;
        }
        break;
        case tt_land_w_road:break;
        case tt_land_w_city:break;
        case tt_water: switch(m_decal_type)
        {
            case 0:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                //2 tip
                glVertex2f(-0.13*scale,0.10*scale);
                glVertex2f(-0.03*scale,0.05*scale);

                glVertex2f(-0.03*scale,0.05*scale);
                glVertex2f(0.07*scale,0.10*scale);

                glVertex2f(0.07*scale,0.10*scale);
                glVertex2f(0.17*scale,0.05*scale);

                glVertex2f(0.17*scale,0.05*scale);
                glVertex2f(0.27*scale,0.10*scale);
                //end
                //3 tip
                glVertex2f(-0.26*scale,-0.10*scale);
                glVertex2f(-0.16*scale,-0.15*scale);

                glVertex2f(-0.16*scale,-0.15*scale);
                glVertex2f(-0.06*scale,-0.10*scale);

                glVertex2f(-0.06*scale,-0.10*scale);
                glVertex2f(0.04*scale,-0.15*scale);

                glVertex2f(0.04*scale,-0.15*scale);
                glVertex2f(0.14*scale,-0.10*scale);

                glVertex2f(0.14*scale,-0.10*scale);
                glVertex2f(0.24*scale,-0.15*scale);

                glVertex2f(0.24*scale,-0.15*scale);
                glVertex2f(0.34*scale,-0.10*scale);
                //end
                glEnd();
            }break;
            case 1:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                //3 tip
                glVertex2f(-0.25*scale,0.025*scale);
                glVertex2f(-0.15*scale,-0.025*scale);

                glVertex2f(-0.15*scale,-0.025*scale);
                glVertex2f(-0.05*scale,0.025*scale);

                glVertex2f(-0.05*scale,0.025*scale);
                glVertex2f(0.05*scale,-0.025*scale);

                glVertex2f(0.05*scale,-0.025*scale);
                glVertex2f(0.15*scale,0.025*scale);

                glVertex2f(0.15*scale,0.025*scale);
                glVertex2f(0.25*scale,-0.025*scale);

                glVertex2f(0.25*scale,-0.025*scale);
                glVertex2f(0.35*scale,0.025*scale);
                //end
                //2 tip
                glVertex2f(-0.35*scale,0.15*scale);
                glVertex2f(-0.25*scale,0.10*scale);

                glVertex2f(-0.25*scale,0.10*scale);
                glVertex2f(-0.15*scale,0.15*scale);

                glVertex2f(-0.15*scale,0.15*scale);
                glVertex2f(-0.05*scale,0.10*scale);

                glVertex2f(-0.05*scale,0.10*scale);
                glVertex2f(0.05*scale,0.15*scale);
                //end
                //2 tip
                glVertex2f(-0.30*scale,-0.10*scale);
                glVertex2f(-0.20*scale,-0.15*scale);

                glVertex2f(-0.20*scale,-0.15*scale);
                glVertex2f(-0.10*scale,-0.10*scale);

                glVertex2f(-0.10*scale,-0.10*scale);
                glVertex2f(-0.00*scale,-0.15*scale);

                glVertex2f(-0.00*scale,-0.15*scale);
                glVertex2f(0.10*scale,-0.10*scale);
                //end
                glEnd();
            }break;
            case 2:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                //2 tip
                glVertex2f(-0.05*scale,0.15*scale);
                glVertex2f(0.05*scale,0.10*scale);

                glVertex2f(0.05*scale,0.10*scale);
                glVertex2f(0.15*scale,0.15*scale);

                glVertex2f(0.15*scale,0.15*scale);
                glVertex2f(0.25*scale,0.10*scale);

                glVertex2f(0.25*scale,0.10*scale);
                glVertex2f(0.35*scale,0.15*scale);
                //end
                //2 tip
                glVertex2f(-0.30*scale,-0.10*scale);
                glVertex2f(-0.20*scale,-0.15*scale);

                glVertex2f(-0.20*scale,-0.15*scale);
                glVertex2f(-0.10*scale,-0.10*scale);

                glVertex2f(-0.10*scale,-0.10*scale);
                glVertex2f(-0.00*scale,-0.15*scale);

                glVertex2f(-0.00*scale,-0.15*scale);
                glVertex2f(0.10*scale,-0.10*scale);
                //end
                glEnd();
            }break;
            case 3:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                //2 tip
                glVertex2f(-0.05*scale,-0.10*scale);
                glVertex2f(0.05*scale,-0.15*scale);

                glVertex2f(0.05*scale,-0.15*scale);
                glVertex2f(0.15*scale,-0.10*scale);

                glVertex2f(0.15*scale,-0.10*scale);
                glVertex2f(0.25*scale,-0.15*scale);

                glVertex2f(0.25*scale,-0.15*scale);
                glVertex2f(0.35*scale,-0.10*scale);
                //end
                //2 tip
                glVertex2f(-0.30*scale,0.15*scale);
                glVertex2f(-0.20*scale,0.10*scale);

                glVertex2f(-0.20*scale,0.10*scale);
                glVertex2f(-0.10*scale,0.15*scale);

                glVertex2f(-0.10*scale,0.15*scale);
                glVertex2f(-0.00*scale,0.10*scale);

                glVertex2f(-0.00*scale,0.10*scale);
                glVertex2f(0.10*scale,0.15*scale);
                //end
                glEnd();
            }break;
            case 4:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                //2 tip
                glVertex2f(-0.15*scale,-0.10*scale);
                glVertex2f(-0.05*scale,-0.15*scale);

                glVertex2f(-0.05*scale,-0.15*scale);
                glVertex2f(0.05*scale,-0.10*scale);

                glVertex2f(0.05*scale,-0.10*scale);
                glVertex2f(0.15*scale,-0.15*scale);

                glVertex2f(0.15*scale,-0.15*scale);
                glVertex2f(0.25*scale,-0.10*scale);
                //end
                //3 tip
                glVertex2f(-0.25*scale,0.10*scale);
                glVertex2f(-0.15*scale,0.05*scale);

                glVertex2f(-0.15*scale,0.05*scale);
                glVertex2f(-0.05*scale,0.10*scale);

                glVertex2f(-0.05*scale,0.10*scale);
                glVertex2f(0.05*scale,0.05*scale);

                glVertex2f(0.05*scale,0.05*scale);
                glVertex2f(0.15*scale,0.10*scale);

                glVertex2f(0.15*scale,0.10*scale);
                glVertex2f(0.25*scale,0.05*scale);

                glVertex2f(0.25*scale,0.05*scale);
                glVertex2f(0.35*scale,0.10*scale);
                //end
                glEnd();
            }break;
            case 5:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                //2 tip
                glVertex2f(-0.15*scale,0.10*scale);
                glVertex2f(-0.05*scale,0.05*scale);

                glVertex2f(-0.05*scale,0.05*scale);
                glVertex2f(0.05*scale,0.10*scale);

                glVertex2f(0.05*scale,0.10*scale);
                glVertex2f(0.15*scale,0.05*scale);

                glVertex2f(0.15*scale,0.05*scale);
                glVertex2f(0.25*scale,0.10*scale);
                //end
                //3 tip
                glVertex2f(-0.25*scale,-0.10*scale);
                glVertex2f(-0.15*scale,-0.15*scale);

                glVertex2f(-0.15*scale,-0.15*scale);
                glVertex2f(-0.05*scale,-0.10*scale);

                glVertex2f(-0.05*scale,-0.10*scale);
                glVertex2f(0.05*scale,-0.15*scale);

                glVertex2f(0.05*scale,-0.15*scale);
                glVertex2f(0.15*scale,-0.10*scale);

                glVertex2f(0.15*scale,-0.10*scale);
                glVertex2f(0.25*scale,-0.15*scale);

                glVertex2f(0.25*scale,-0.15*scale);
                glVertex2f(0.35*scale,-0.10*scale);
                //end
                glEnd();
            }break;
            case 6:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                //3 tip
                glVertex2f(-0.20*scale,0.025*scale);
                glVertex2f(-0.10*scale,-0.025*scale);

                glVertex2f(-0.10*scale,-0.025*scale);
                glVertex2f(0.00*scale,0.025*scale);

                glVertex2f(0.00*scale,0.025*scale);
                glVertex2f(0.10*scale,-0.025*scale);

                glVertex2f(0.10*scale,-0.025*scale);
                glVertex2f(0.20*scale,0.025*scale);

                glVertex2f(0.20*scale,0.025*scale);
                glVertex2f(0.30*scale,-0.025*scale);

                glVertex2f(0.30*scale,-0.025*scale);
                glVertex2f(0.40*scale,0.025*scale);
                //end
                //3 tip
                glVertex2f(-0.35*scale,0.15*scale);
                glVertex2f(-0.25*scale,0.10*scale);

                glVertex2f(-0.25*scale,0.10*scale);
                glVertex2f(-0.15*scale,0.15*scale);

                glVertex2f(-0.15*scale,0.15*scale);
                glVertex2f(-0.05*scale,0.10*scale);

                glVertex2f(-0.05*scale,0.10*scale);
                glVertex2f(0.05*scale,0.15*scale);

                glVertex2f(0.05*scale,0.15*scale);
                glVertex2f(0.15*scale,0.10*scale);

                glVertex2f(0.15*scale,0.10*scale);
                glVertex2f(0.25*scale,0.15*scale);
                //end
                //2 tip
                glVertex2f(-0.20*scale,-0.15*scale);
                glVertex2f(-0.10*scale,-0.20*scale);

                glVertex2f(-0.10*scale,-0.20*scale);
                glVertex2f(0.00*scale,-0.15*scale);

                glVertex2f(0.00*scale,-0.15*scale);
                glVertex2f(0.10*scale,-0.20*scale);

                glVertex2f(0.10*scale,-0.20*scale);
                glVertex2f(0.20*scale,-0.15*scale);
                //end
                glEnd();
            }break;
            case 7:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                //3 tip
                glVertex2f(-0.22*scale,0.027*scale);
                glVertex2f(-0.12*scale,-0.025*scale);

                glVertex2f(-0.12*scale,-0.027*scale);
                glVertex2f(-0.02*scale,0.027*scale);

                glVertex2f(-0.02*scale,0.027*scale);
                glVertex2f(0.08*scale,-0.027*scale);

                glVertex2f(0.08*scale,-0.027*scale);
                glVertex2f(0.18*scale,0.027*scale);

                glVertex2f(0.18*scale,0.027*scale);
                glVertex2f(0.28*scale,-0.027*scale);

                glVertex2f(0.28*scale,-0.027*scale);
                glVertex2f(0.38*scale,0.027*scale);
                //end
                //3 tip
                glVertex2f(-0.34*scale,0.18*scale);
                glVertex2f(-0.24*scale,0.13*scale);

                glVertex2f(-0.24*scale,0.13*scale);
                glVertex2f(-0.14*scale,0.18*scale);

                glVertex2f(-0.14*scale,0.18*scale);
                glVertex2f(-0.04*scale,0.13*scale);

                glVertex2f(-0.04*scale,0.13*scale);
                glVertex2f(0.06*scale,0.18*scale);

                glVertex2f(0.06*scale,0.18*scale);
                glVertex2f(0.16*scale,0.13*scale);

                glVertex2f(0.16*scale,0.13*scale);
                glVertex2f(0.26*scale,0.18*scale);
                //end
                //2 tip
                glVertex2f(-0.20*scale,-0.15*scale);
                glVertex2f(-0.10*scale,-0.20*scale);

                glVertex2f(-0.10*scale,-0.20*scale);
                glVertex2f(0.00*scale,-0.15*scale);

                glVertex2f(0.00*scale,-0.15*scale);
                glVertex2f(0.10*scale,-0.20*scale);

                glVertex2f(0.10*scale,-0.20*scale);
                glVertex2f(0.20*scale,-0.15*scale);
                //end
                glEnd();
            }break;
            case 8:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                //2 tip
                glVertex2f(-0.01*scale,0.15*scale);
                glVertex2f(0.09*scale,0.10*scale);

                glVertex2f(0.09*scale,0.10*scale);
                glVertex2f(0.19*scale,0.15*scale);

                glVertex2f(0.19*scale,0.15*scale);
                glVertex2f(0.29*scale,0.10*scale);

                glVertex2f(0.29*scale,0.10*scale);
                glVertex2f(0.39*scale,0.15*scale);
                //end
                //2 tip
                glVertex2f(-0.30*scale,-0.11*scale);
                glVertex2f(-0.20*scale,-0.16*scale);

                glVertex2f(-0.20*scale,-0.16*scale);
                glVertex2f(-0.10*scale,-0.11*scale);

                glVertex2f(-0.10*scale,-0.11*scale);
                glVertex2f(-0.00*scale,-0.16*scale);

                glVertex2f(-0.00*scale,-0.16*scale);
                glVertex2f(0.10*scale,-0.11*scale);
                //end
                glEnd();
            }break;
        }
        break;
        case tt_water_w_road:break;
        case tt_rock: switch(m_decal_type)
        {
            case 0:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.39*scale,0.19*scale);
                glVertex2f(-0.28*scale,-0.01*scale);
                glVertex2f(-0.29*scale,-0.01*scale);
                glVertex2f(-0.14*scale,0.23*scale);

                glVertex2f(-0.18*scale,0.01*scale);
                glVertex2f(0.03*scale,-0.29*scale);
                glVertex2f(0.03*scale,-0.29*scale);
                glVertex2f(0.38*scale,0.19*scale);
                glEnd();
            }break;
            case 1:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.30*scale,0.20*scale);
                glVertex2f(-0.05*scale,-0.30*scale);
                glVertex2f(-0.05*scale,-0.30*scale);
                glVertex2f(0.10*scale,-0.05*scale);
                glVertex2f(0.0*scale,0.15*scale);
                glVertex2f(0.15*scale,-0.10*scale);
                glVertex2f(0.15*scale,-0.10*scale);
                glVertex2f(0.30*scale,0.13*scale);
                glEnd();
            }break;
            case 2:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.35*scale,0.10*scale);
                glVertex2f(-0.20*scale,-0.25*scale);
                glVertex2f(-0.20*scale,-0.25*scale);
                glVertex2f(-0.10*scale,0.00*scale);
                glVertex2f(-0.05*scale,-0.10*scale);
                glVertex2f(0.10*scale,-0.20*scale);
                glVertex2f(0.10*scale,-0.20*scale);
                glVertex2f(0.32*scale,0.10*scale);
                glVertex2f(-0.10*scale,0.18*scale);
                glVertex2f(0.05*scale,-0.02*scale);
                glVertex2f(0.05*scale,-0.02*scale);
                glVertex2f(0.15*scale,0.13*scale);
                glEnd();
            }break;
            case 3:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.35*scale,0.10*scale);
                glVertex2f(-0.22*scale,-0.22*scale);
                glVertex2f(-0.22*scale,-0.22*scale);
                glVertex2f(-0.05*scale,0.06*scale);

                glVertex2f(-0.06*scale,-0.10*scale);
                glVertex2f(0.10*scale,-0.26*scale);
                glVertex2f(0.10*scale,-0.26*scale);
                glVertex2f(0.33*scale,0.12*scale);
                glEnd();
            }break;
            case 4:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.36*scale,0.17*scale);
                glVertex2f(-0.25*scale,-0.02*scale);
                glVertex2f(-0.25*scale,-0.02*scale);
                glVertex2f(-0.12*scale,0.19*scale);

                glVertex2f(-0.16*scale,-0.03*scale);
                glVertex2f(-0.01*scale,-0.27*scale);
                glVertex2f(-0.01*scale,-0.27*scale);
                glVertex2f(0.14*scale,0.04*scale);

                glVertex2f(0.08*scale,0.24*scale);
                glVertex2f(0.24*scale,0.05*scale);
                glVertex2f(0.24*scale,0.05*scale);
                glVertex2f(0.35*scale,0.22*scale);
                glEnd();
            }break;
            case 5:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.34*scale,0.22*scale);
                glVertex2f(-0.12*scale,0.08*scale);
                glVertex2f(-0.12*scale,0.08*scale);
                glVertex2f(0.28*scale,0.23*scale);

                glVertex2f(-0.21*scale,0.03*scale);
                glVertex2f(-0.02*scale,-0.24*scale);
                glVertex2f(-0.02*scale,-0.24*scale);
                glVertex2f(0.16*scale,0.06*scale);
                glEnd();
            }break;
            case 6:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.31*scale,0.25*scale);
                glVertex2f(-0.15*scale,0.11*scale);
                glVertex2f(-0.15*scale,0.11*scale);
                glVertex2f(0.15*scale,0.26*scale);

                glVertex2f(-0.31*scale,0.06*scale);
                glVertex2f(-0.18*scale,-0.25*scale);
                glVertex2f(-0.18*scale,-0.25*scale);
                glVertex2f(-0.03*scale,0.05*scale);

                glVertex2f(0.01*scale,-0.09*scale);
                glVertex2f(0.17*scale,-0.19*scale);
                glVertex2f(0.17*scale,-0.19*scale);
                glVertex2f(0.32*scale,0.16*scale);
                glEnd();
            }break;
            case 7:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.40*scale,0.08*scale);
                glVertex2f(-0.24*scale,-0.22*scale);
                glVertex2f(-0.24*scale,-0.22*scale);
                glVertex2f(-0.15*scale,-0.07*scale);

                glVertex2f(-0.23*scale,0.22*scale);
                glVertex2f(-0.03*scale,-0.16*scale);
                glVertex2f(-0.03*scale,-0.16*scale);
                glVertex2f(0.18*scale,0.20*scale);

                glVertex2f(0.15*scale,-0.06*scale);
                glVertex2f(0.27*scale,-0.17*scale);
                glVertex2f(0.27*scale,-0.17*scale);
                glVertex2f(0.39*scale,0.06*scale);
                glEnd();
            }break;
            case 8:
            {
                glLineWidth(1);
                glBegin(GL_LINES);
                glVertex2f(-0.37*scale,0.15*scale);
                glVertex2f(-0.15*scale,-0.23*scale);
                glVertex2f(-0.15*scale,-0.23*scale);
                glVertex2f(0.03*scale,0.11*scale);

                glVertex2f(0.02*scale,-0.08*scale);
                glVertex2f(0.14*scale,-0.23*scale);
                glVertex2f(0.14*scale,-0.23*scale);
                glVertex2f(0.28*scale,0.03*scale);

                glVertex2f(0.04*scale,0.30*scale);
                glVertex2f(0.16*scale,0.02*scale);
                glVertex2f(0.16*scale,0.02*scale);
                glVertex2f(0.29*scale,0.23*scale);
                glEnd();
            }break;
        }
        break;
    }
    */

    glPopMatrix();

    //glDisable(GL_TEXTURE_2D);

    return true;
}

int tile::selection_test(float zoom_level,float real_mouse_pos_x,float real_mouse_pos_y)
{
    //selection test
    for(int x_shift=-1;x_shift<2;x_shift++)
    for(int y_shift=-1;y_shift<2;y_shift++)
    {
        float x_pix=m_pos_x_hex+_world_size_x*x_shift+(m_pos_y_hex+_world_size_y*y_shift)*0.5;
        float y_pix=(m_pos_y_hex+_world_size_y*y_shift)*0.75;
        if(x_pix*zoom_level<real_mouse_pos_x+zoom_level/2.0 && x_pix*zoom_level>real_mouse_pos_x-zoom_level/2.0 &&
           y_pix*zoom_level<real_mouse_pos_y+zoom_level/2.0 && y_pix*zoom_level>real_mouse_pos_y-zoom_level/2.0)
        {
            if(x_shift==-1 && y_shift==-1) return 0;
            if(x_shift==-1 && y_shift==0) return 1;
            if(x_shift==-1 && y_shift==1) return 2;
            if(x_shift==0 && y_shift==-1) return 3;
            if(x_shift==0 && y_shift==0) return 4;//center
            if(x_shift==0 && y_shift==1) return 5;
            if(x_shift==1 && y_shift==-1) return 6;
            if(x_shift==1 && y_shift==0) return 7;
            if(x_shift==1 && y_shift==1) return 8;
        }
    }
    /*if(m_pos_x_pix*zoom_level<real_mouse_pos_x+zoom_level/2.8 && m_pos_x_pix*zoom_level>real_mouse_pos_x-zoom_level/2.8 &&
       m_pos_y_pix*zoom_level<real_mouse_pos_y+zoom_level/2.8 && m_pos_y_pix*zoom_level>real_mouse_pos_y-zoom_level/2.8)
    {
        return true;
    }*/
    m_temp_color=false;

    return -1;
}

bool tile::selection_test_spe_box(float zoom_level,float real_mouse_pos_x,float real_mouse_pos_y)
{
    //test all 9 boxes
    for(int x_shift=-1;x_shift<2;x_shift++)
    for(int y_shift=-1;y_shift<2;y_shift++)
    {
        float x_pix=m_pos_x_hex+_world_size_x*x_shift+(m_pos_y_hex+_world_size_y*y_shift)*0.5;
        float y_pix=(m_pos_y_hex+_world_size_y*y_shift)*0.75;
        //selection test
        if(x_pix*zoom_level<=real_mouse_pos_x+zoom_level/2.0 && x_pix*zoom_level>=real_mouse_pos_x-zoom_level/2.0 &&
           y_pix*zoom_level<=real_mouse_pos_y+zoom_level/2.0 && y_pix*zoom_level>=real_mouse_pos_y-zoom_level/2.0)
        {//inside box, more testing req.
            //test top right line
            if( real_mouse_pos_y/zoom_level-y_pix >=  (real_mouse_pos_x/zoom_level-x_pix)*0.5-0.5 &&
                real_mouse_pos_y/zoom_level-y_pix <   (real_mouse_pos_x/zoom_level-x_pix)*0.5+0.5 &&
                real_mouse_pos_y/zoom_level-y_pix >= -(real_mouse_pos_x/zoom_level-x_pix)*0.5-0.5 &&
                real_mouse_pos_y/zoom_level-y_pix <  -(real_mouse_pos_x/zoom_level-x_pix)*0.5+0.5 )
            {//inside hex
                return true;
            }
        }
    }

    return false;
}

float tile::get_distance(float zoom_level,float real_mouse_pos_x,float real_mouse_pos_y)
{
    float min_dist=10000.0;
    for(int x_shift=-1;x_shift<2;x_shift++)
    for(int y_shift=-1;y_shift<2;y_shift++)
    {
        float x_pix=m_pos_x_hex+_world_size_x*x_shift+(m_pos_y_hex+_world_size_y*y_shift)/2.0;
        float y_pix=(m_pos_y_hex+_world_size_y*y_shift)*0.75;

        float dist=sqrtf( (real_mouse_pos_x-x_pix)*(real_mouse_pos_x-x_pix)+
                          (real_mouse_pos_y-y_pix)*(real_mouse_pos_y-y_pix) );

        if(dist<min_dist) min_dist=dist;
    }

    return min_dist;
}

bool tile::set_color(float r,float g,float b)
{
    m_color[0]=r; m_color[1]=g; m_color[2]=b;
    m_temp_color=true;

    return true;
}

bool tile::is_buildable(void)
{
    if(m_tile_type==tt_land || m_tile_type==tt_land_w_road ) return true;

    return false;
}

bool tile::set_owner(int owner_id,int city_id)//called when city is built
{
    m_tile_type=tt_land_w_city;
    m_owned_by_player=owner_id;
    m_city_id=city_id;

    return true;
}

bool tile::set_tile_type(int type,int id)//id sets the city id or trade id, or the trade id that should be removed
{
    int last_tt=m_tile_type;
    m_tile_type=type;

    switch(type)
    {
        case tt_error:        m_trade_id[0]=-1; m_trade_id[1]=-1; m_trade_id[2]=-1; m_city_id=-1; break;
        case tt_land:
        {
            if(last_tt==tt_land_w_road)
            {
                //find special id and remove that trade
                if(m_trade_id[0]==id)
                {//remove first and relocate others
                    m_trade_id[0]=m_trade_id[1];
                    m_trade_id[1]=m_trade_id[2];
                    m_trade_id[2]=-1;
                }
                else if(m_trade_id[1]==id)
                {//remove first and relocate others
                    m_trade_id[1]=m_trade_id[2];
                    m_trade_id[2]=-1;
                }
                else if(m_trade_id[2]==id)
                {//remove first and relocate others
                    m_trade_id[2]=-1;
                }
                else//bad id
                {
                    cout<<"ERROR: Setting tile type: Received bad bad trade ID for removal\n";
                    return false;
                }

                //test if any other road left
                if(m_trade_id[0]!=-1) m_tile_type=tt_land_w_road;//if roads left, go back to tt_land_w_road
            }
            else
            {
                m_trade_id[0]=-1; m_trade_id[1]=-1; m_trade_id[2]=-1; m_city_id=-1;
            }
        }break;
        case tt_land_w_road:
        {
            //find first empty
            if     (m_trade_id[0]==-1) m_trade_id[0]=id;
            else if(m_trade_id[1]==-1) m_trade_id[1]=id;
            else if(m_trade_id[2]==-1) m_trade_id[2]=id;
            else
            {
                cout<<"ERROR: Setting tile type, on land with road: No empty slot for trade ID\n";
                return false;
            }
        }break;
        case tt_land_w_city:  m_trade_id[0]=-1; m_trade_id[1]=-1; m_trade_id[2]=-1; m_city_id=id; break;
        case tt_water:
        {
            if(last_tt==tt_water_w_road)
            {
                //find special id and remove that trade
                if(m_trade_id[0]==id)
                {//remove first and relocate others
                    m_trade_id[0]=m_trade_id[1];
                    m_trade_id[1]=m_trade_id[2];
                    m_trade_id[2]=-1;
                }
                else if(m_trade_id[1]==id)
                {//remove first and relocate others
                    m_trade_id[1]=m_trade_id[2];
                    m_trade_id[2]=-1;
                }
                else if(m_trade_id[2]==id)
                {//remove first and relocate others
                    m_trade_id[2]=-1;
                }
                else//bad id
                {
                    cout<<"ERROR: Setting tile type, on land: Received bad bad trade ID for removal\n";
                    return false;
                }

                //test if any other road left
                if(m_trade_id[0]!=-1) m_tile_type=tt_water_w_road;//if roads left, go back to tt_water_w_road
            }
        }break;
        case tt_water_w_road:
        {
            //find first empty
            if     (m_trade_id[0]==-1) m_trade_id[0]=id;
            else if(m_trade_id[1]==-1) m_trade_id[1]=id;
            else if(m_trade_id[2]==-1) m_trade_id[2]=id;
            else
            {
                cout<<"ERROR: Setting tile type, in water: No empty slot for trade ID\n";
                return false;
            }
        }break;
        case tt_rock:         m_trade_id[0]=-1; m_trade_id[1]=-1; m_trade_id[2]=-1; m_city_id=-1; break;
    }

    return true;
}

int tile::get_tile_type(void)
{
    return m_tile_type;
}

int tile::get_city_id(void)
{
    return m_city_id;
}

int tile::get_city_owner_id(void)
{
    return m_owned_by_player;
}

int tile::get_trade_id(int selector)
{
    if(selector<0 || selector>2)
    {
        cout<<"ERROR: Requesting bad trade ID from tile\n";
        return -1;
    }

    return m_trade_id[selector];
}
