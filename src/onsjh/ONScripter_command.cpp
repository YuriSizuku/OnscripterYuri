/* -*- C++ -*-
 * 
 *  ONScripter_command.cpp - Command executer of ONScripter
 *
 *  Copyright (c) 2001-2018 Ogapee. All rights reserved.
 *            (C) 2014-2019 jh10001 <jh10001@live.cn>
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ONScripter.h"
#if defined(LINUX) || defined(MACOSX) || defined(IOS)
#include <sys/types.h>
#include <sys/stat.h>
#elif defined(WIN32)
#include <direct.h>
#endif
#include "version.h"
#include "Utils.h"

#if defined(MACOSX) && (SDL_COMPILEDVERSION >= 1208)
#include <CoreFoundation/CoreFoundation.h>
#endif

extern SDL_TimerID timer_bgmfade_id;
extern "C" Uint32 SDLCALL bgmfadeCallback( Uint32 interval, void *param );
extern "C" void smpegCallback();
    
#define CONTINUOUS_PLAY

int ONScripter::yesnoboxCommand()
{
    bool yesno_flag = true;
    if ( script_h.isName( "okcancelbox" ) ) yesno_flag = false;

    script_h.readInt();
    script_h.pushVariable();

    script_h.readStr();
    const char *mes1 = script_h.saveStringBuffer();
    const char *mes2 = script_h.readStr();
    ButtonLink *tmp_button_link = root_button_link.next;
    root_button_link.next = NULL;
    buildDialog(yesno_flag, mes1, mes2);

    show_dialog_flag = true;
    dirty_rect.add(dialog_info.pos);
    flush(refreshMode());

    while(1){
        event_mode = WAIT_BUTTON_MODE;
        waitEvent(-1);

        if (current_button_state.button == -1 ||
            current_button_state.button == 2){
            script_h.setInt(&script_h.pushed_variable, 0);
            break;
        }
        else if (current_button_state.button == 1){
            script_h.setInt(&script_h.pushed_variable, 1);
            break;
        }
    }
    
    show_dialog_flag = false;
    delete root_button_link.next->next;
    delete root_button_link.next;
    root_button_link.next = tmp_button_link;
    dirty_rect.add(dialog_info.pos);
    flush(refreshMode());

    return RET_CONTINUE;
}

int ONScripter::wavestopCommand()
{
    if ( wave_sample[MIX_WAVE_CHANNEL] ){
        Mix_Pause( MIX_WAVE_CHANNEL );
        Mix_FreeChunk( wave_sample[MIX_WAVE_CHANNEL] );
        wave_sample[MIX_WAVE_CHANNEL] = NULL;
    }
    setStr( &wave_file_name, NULL );

    return RET_CONTINUE;
}

int ONScripter::waveCommand()
{
    wave_play_loop_flag = false;
    
    if (script_h.isName( "waveloop" ))
        wave_play_loop_flag = true;

    wavestopCommand();

    setStr(&wave_file_name, script_h.readStr());
    playSound(wave_file_name, SOUND_CHUNK, wave_play_loop_flag, MIX_WAVE_CHANNEL);
        
    return RET_CONTINUE;
}

int ONScripter::waittimerCommand()
{
    int count = script_h.readInt() + internal_timer - SDL_GetTicks();
    if (count < 0) count = 0;

    event_mode = WAIT_TIMER_MODE;
    waitEvent( count );
    
    return RET_CONTINUE;
}

int ONScripter::waitCommand()
{
    event_mode = WAIT_TIMER_MODE;
    waitEvent( script_h.readInt() );

    return RET_CONTINUE;
}

int ONScripter::vspCommand()
{
    leaveTextDisplayMode();

    bool vsp2_flag = false;
    if (script_h.isName("vsp2")) vsp2_flag = true;

    int no = script_h.readInt();
    int v  = script_h.readInt();

    if (vsp2_flag){
        sprite2_info[no].visible = (v==1)?true:false;
        dirty_rect.add( sprite2_info[no].bounding_rect );
    }
    else{
        sprite_info[no].visible = (v==1)?true:false;
        dirty_rect.add( sprite_info[no].pos );
    }
    
    return RET_CONTINUE;
}

int ONScripter::voicevolCommand()
{
    voice_volume = script_h.readInt();
    if ( wave_sample[0] ) Mix_Volume( 0, voice_volume * MIX_MAX_VOLUME / 100 );
    
    return RET_CONTINUE;
}

int ONScripter::vCommand()
{
    char buf[256];
    
    sprintf(buf, RELATIVEPATH "wav%c%s.wav", DELIMITER, script_h.getStringBuffer()+1);
    playSound(buf, SOUND_CHUNK, false, MIX_WAVE_CHANNEL);
    
    return RET_CONTINUE;
}

int ONScripter::trapCommand()
{
    bool is_clicked = trap_mode & TRAP_CLICKED;
    
    if      ( script_h.isName( "lr_trap" ) ){
        trap_mode = TRAP_LEFT_CLICK | TRAP_RIGHT_CLICK;
    }
    else if ( script_h.isName( "r_trap" ) ){
        trap_mode = TRAP_RIGHT_CLICK;
    }
    else if ( script_h.isName( "trap" ) ){
        trap_mode = TRAP_LEFT_CLICK;
    }

    if ( script_h.compareString("off") ){
        script_h.readLabel();
        trap_mode = TRAP_NONE;
        return RET_CONTINUE;
    }
    else if ( script_h.compareString("stop") ){
        script_h.readLabel();
        trap_mode |= TRAP_STOP;
        return RET_CONTINUE;
    }
    else if ( script_h.compareString("resume") ){
        script_h.readLabel();
        if (is_clicked) trapHandler();
        return RET_CONTINUE;
    }

    const char *buf = script_h.readStr();
    if ( buf[0] == '*' ){
        setStr(&trap_dist, buf+1);
    }
    else{
        utils::printInfo("trapCommand: [%s] is not supported\n", buf );
    }
              
    return RET_CONTINUE;
}

int ONScripter::transbtnCommand()
{
    transbtn_flag = true;

    return RET_CONTINUE;
}

int ONScripter::textspeeddefaultCommand()
{
    sentence_font.wait_time = -1;
    return RET_CONTINUE;
}

int ONScripter::textspeedCommand()
{
    sentence_font.wait_time = script_h.readInt();

    return RET_CONTINUE;
}

int ONScripter::textshowCommand()
{
    dirty_rect.fill( screen_width, screen_height );
    refresh_shadow_text_mode = REFRESH_NORMAL_MODE | REFRESH_SHADOW_MODE | REFRESH_TEXT_MODE;
    flush(refreshMode());

    return RET_CONTINUE;
}

int ONScripter::textonCommand()
{
    if (windowchip_sprite_no >= 0)
        sprite_info[windowchip_sprite_no].visible = true;

    enterTextDisplayMode();

    text_on_flag = true;

    return RET_CONTINUE;
}

int ONScripter::textoffCommand()
{
    if (windowchip_sprite_no >= 0)
        sprite_info[windowchip_sprite_no].visible = false;

    leaveTextDisplayMode(true);

    text_on_flag = false;

    return RET_CONTINUE;
}

int ONScripter::texthideCommand()
{
    dirty_rect.fill( screen_width, screen_height );
    refresh_shadow_text_mode = REFRESH_NORMAL_MODE | REFRESH_SHADOW_MODE;
    flush(refreshMode());

    return RET_CONTINUE;
}

int ONScripter::textcolorCommand()
{
    readColor( &sentence_font.color, script_h.readStr() );

    return RET_CONTINUE;
}

int ONScripter::textclearCommand()
{
    newPage();
    return RET_CONTINUE;
}

int ONScripter::texecCommand()
{
    if ( textgosub_clickstr_state == CLICK_NEWPAGE )
        newPage();
    else if ( textgosub_clickstr_state == (CLICK_WAIT|CLICK_EOL) ){
        processEOT();
        page_enter_status = 0;
    }

    saveon_flag = true;
    
    return RET_CONTINUE;
}

int ONScripter::tateyokoCommand()
{
    sentence_font.setTateyokoMode( script_h.readInt() );
    
    return RET_CONTINUE;
}

int ONScripter::talCommand()
{
    leaveTextDisplayMode();
    
    char loc = script_h.readLabel()[0];
    int no = -1, trans = 0;
    if      ( loc == 'l' ) no = 0;
    else if ( loc == 'c' ) no = 1;
    else if ( loc == 'r' ) no = 2;

    if (no >= 0)
        trans = script_h.readInt();

    if (no >= 0){
        tachi_info[ no ].trans = trans;
        dirty_rect.add( tachi_info[ no ].pos );
    }

    EffectLink *el = parseEffect(true);
    if (setEffect(el)) return RET_CONTINUE;
    while (doEffect(el));

    return RET_CONTINUE;
}

int ONScripter::tablegotoCommand()
{
    int count = 0;
    int no = script_h.readInt();

    while( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
        const char *buf = script_h.readStr();
        if ( count++ == no ){
            setCurrentLabel( buf+1 );
            break;
        }
    }

    return RET_CONTINUE;
}

int ONScripter::systemcallCommand()
{
    system_menu_mode = getSystemCallNo( script_h.readLabel() );

    executeSystemCall();
    
    return RET_CONTINUE;
}

int ONScripter::strspCommand()
{
    leaveTextDisplayMode();
    
    bool v = true;
    if ( script_h.isName( "strsph" ) ) v = false;

    int sprite_no = script_h.readInt();
    AnimationInfo *ai = &sprite_info[sprite_no];
    ai->font_size_xy[0] = -1;

    if (ai->image_surface && ai->visible)
        dirty_rect.add( ai->pos );

    ai->removeTag();
    setStr(&ai->file_name, script_h.readStr());
    ai->orig_pos.x = script_h.readInt();
    ai->orig_pos.y = script_h.readInt();
    ai->scalePosXY( screen_ratio1, screen_ratio2 );

    FontInfo fi;
    fi.is_newline_accepted = true;
    fi.num_xy[0] = script_h.readInt();
    fi.num_xy[1] = script_h.readInt();
    fi.font_size_xy[0] = script_h.readInt();
    fi.font_size_xy[1] = script_h.readInt();
    fi.pitch_xy[0] = script_h.readInt() + fi.font_size_xy[0];
    fi.pitch_xy[1] = script_h.readInt() + fi.font_size_xy[1];
    fi.is_bold = script_h.readInt()?true:false;
    fi.is_shadow = script_h.readInt()?true:false;

    char *buffer = script_h.getNext();
    while(script_h.getEndStatus() & ScriptHandler::END_COMMA){
        ai->num_of_cells++;
        script_h.readStr();
    }
    if (ai->num_of_cells == 0){
        ai->num_of_cells = 1;
        ai->color_list = new uchar3[ai->num_of_cells];
        ai->color_list[0][0] = ai->color_list[0][1] = ai->color_list[0][2] = 0xff;
    }
    else{
        ai->color_list = new uchar3[ai->num_of_cells];
        script_h.setCurrent(buffer);
        for (int i=0 ; i<ai->num_of_cells ; i++)
            readColor(&ai->color_list[i], script_h.readStr());
    }

    ai->trans_mode = AnimationInfo::TRANS_STRING;
    ai->trans = -1;
    ai->visible = v;
    ai->is_single_line = false;
    ai->is_tight_region = false;
    ai->is_ruby_drawable = sentence_font.rubyon_flag;
    setupAnimationInfo(ai, &fi);
    if ( ai->visible ) dirty_rect.add( ai->pos );

    return RET_CONTINUE;
}

int ONScripter::stopCommand()
{
    mp3stopCommand();
    wavestopCommand();
    
    return RET_CONTINUE;
}

int ONScripter::sp_rgb_gradationCommand()
{
    int no = script_h.readInt();
    int upper_r = script_h.readInt();
    int upper_g = script_h.readInt();
    int upper_b = script_h.readInt();
    int lower_r = script_h.readInt();
    int lower_g = script_h.readInt();
    int lower_b = script_h.readInt();
    ONSBuf key_r = script_h.readInt();
    ONSBuf key_g = script_h.readInt();
    ONSBuf key_b = script_h.readInt();
    Uint32 alpha = script_h.readInt();

    AnimationInfo *ai;
    if (no == -1) ai = &sentence_font_info;
    else          ai = &sprite_info[no];
    SDL_Surface *surface = ai->image_surface;
    if (surface == NULL) return RET_CONTINUE;

    SDL_PixelFormat *fmt = surface->format;
    
    ONSBuf key_mask = (((key_r >> fmt->Rloss) << fmt->Rshift) |
                       ((key_g >> fmt->Gloss) << fmt->Gshift) |
                       ((key_b >> fmt->Bloss) << fmt->Bshift));
    ONSBuf rgb_mask = fmt->Rmask | fmt->Gmask | fmt->Bmask;

    SDL_LockSurface(surface);
    // check upper and lower bound
    int i, j;
    int upper_bound=0, lower_bound=0;
    bool is_key_found = false;
    for (i=0 ; i<surface->h ; i++){
        ONSBuf *buf = (ONSBuf *)surface->pixels + surface->w * i;
        for (j=0 ; j<surface->w ; j++, buf++){
            if ((*buf & rgb_mask) == key_mask){
                if (is_key_found == false){
                    is_key_found = true;
                    upper_bound = lower_bound = i;
                }
                else{
                    lower_bound = i;
                }
                break;
            }
        }
    }
    
    // replace pixels of the key-color with the specified color in gradation
    for (i=upper_bound ; i<=lower_bound ; i++){
        ONSBuf *buf = (ONSBuf *)surface->pixels + surface->w * i;
#if defined(BPP16)    
        unsigned char *alphap = ai->alpha_buf + surface->w * i;
#else
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        unsigned char *alphap = (unsigned char *)buf + 3;
#else
        unsigned char *alphap = (unsigned char *)buf;
#endif
#endif
        Uint32 color = alpha << surface->format->Ashift;
        if (upper_bound != lower_bound){
            color |= (((lower_r - upper_r) * (i-upper_bound) / (lower_bound - upper_bound) + upper_r) >> fmt->Rloss) << fmt->Rshift;
            color |= (((lower_g - upper_g) * (i-upper_bound) / (lower_bound - upper_bound) + upper_g) >> fmt->Gloss) << fmt->Gshift;
            color |= (((lower_b - upper_b) * (i-upper_bound) / (lower_bound - upper_bound) + upper_b) >> fmt->Bloss) << fmt->Bshift;
        }
        else{
            color |= (upper_r >> fmt->Rloss) << fmt->Rshift;
            color |= (upper_g >> fmt->Gloss) << fmt->Gshift;
            color |= (upper_b >> fmt->Bloss) << fmt->Bshift;
        }
        
        for (j=0 ; j<surface->w ; j++, buf++){
            if ((*buf & rgb_mask) == key_mask){
                *buf = color;
                *alphap = alpha;
            }
#if defined(BPP16)                
            alphap++;
#else
            alphap += 4;
#endif                
        }
    }
    
    SDL_UnlockSurface(surface);
    
    if ( ai->visible )
        dirty_rect.add( ai->pos );

    return RET_CONTINUE;
}

int ONScripter::spstrCommand()
{
    decodeExbtnControl( script_h.readStr() );
    
    return RET_CONTINUE;
}

int ONScripter::spreloadCommand()
{
    int no = script_h.readInt();
    AnimationInfo *ai;
    if (no == -1) ai = &sentence_font_info;
    else          ai = &sprite_info[no];

    parseTaggedString( ai );
    setupAnimationInfo( ai );
    
    if ( ai->visible )
        dirty_rect.add( ai->pos );
    
    return RET_CONTINUE;
}

int ONScripter::splitCommand()
{
    script_h.readStr();
    const char *save_buf = script_h.saveStringBuffer();
    
    char delimiter = script_h.readStr()[0];

    char token256[256], *token=NULL;
    while( script_h.getEndStatus() & ScriptHandler::END_COMMA ){

        unsigned int c=0;
        while(save_buf[c] != delimiter && save_buf[c] != '\0'){
            if (IS_TWO_BYTE(save_buf[c]))
                c += 2;
            else
                c++;
        }
        
        if (c < 256) 
            token = token256;
        else
            token = new char[c+1];
        
        memcpy( token, save_buf, c );
        token[c] = '\0';
        
        script_h.readVariable();
        if ( script_h.current_variable.type & ScriptHandler::VAR_INT ||
             script_h.current_variable.type & ScriptHandler::VAR_ARRAY ){
            script_h.setInt( &script_h.current_variable, atoi(token) );
        }
        else if ( script_h.current_variable.type & ScriptHandler::VAR_STR ){
            setStr( &script_h.getVariableData(script_h.current_variable.var_no).str, token );
        }

        if (c >= 256) delete[] token;
        
        save_buf += c;
        if (save_buf[0] != '\0') save_buf++;
    }
    
    return RET_CONTINUE;
}

int ONScripter::spclclkCommand()
{
    if ( !force_button_shortcut_flag )
        spclclk_flag = true;
    return RET_CONTINUE;
}

int ONScripter::spbtnCommand()
{
    bool cellcheck_flag = false;

    if ( script_h.isName( "cellcheckspbtn" ) )
        cellcheck_flag = true;

    int sprite_no = script_h.readInt();
    int no        = script_h.readInt();
    if (no < 1 || 
        sprite_no < 0 ||
        sprite_no >= MAX_SPRITE_NUM || 
        sprite_info[sprite_no].image_surface == NULL)
        return RET_CONTINUE;

    if ( cellcheck_flag ){
        if ( sprite_info[ sprite_no ].num_of_cells < 2 ) return RET_CONTINUE;
    }
    else{
        if ( sprite_info[ sprite_no ].num_of_cells == 0 ) return RET_CONTINUE;
    }

    ButtonLink *button = new ButtonLink();
    root_button_link.insert( button );

    button->button_type = ButtonLink::SPRITE_BUTTON;
    button->sprite_no   = sprite_no;
    button->no          = no;

    if ( sprite_info[ sprite_no ].image_surface ||
         sprite_info[ sprite_no ].trans_mode == AnimationInfo::TRANS_STRING )
        button->image_rect = button->select_rect = sprite_info[ sprite_no ].pos;

    return RET_CONTINUE;
}

int ONScripter::skipoffCommand() 
{ 
    skip_mode &= ~SKIP_NORMAL;
 
    return RET_CONTINUE; 
} 

int ONScripter::sevolCommand()
{
    se_volume = script_h.readInt();

    for ( int i=1 ; i<ONS_MIX_CHANNELS ; i++ )
        if ( wave_sample[i] ) Mix_Volume( i, se_volume * MIX_MAX_VOLUME / 100 );

    if ( wave_sample[MIX_LOOPBGM_CHANNEL0] ) Mix_Volume( MIX_LOOPBGM_CHANNEL0, se_volume * MIX_MAX_VOLUME / 100 );
    if ( wave_sample[MIX_LOOPBGM_CHANNEL1] ) Mix_Volume( MIX_LOOPBGM_CHANNEL1, se_volume * MIX_MAX_VOLUME / 100 );
    
    return RET_CONTINUE;
}

void ONScripter::setwindowCore()
{
    sentence_font.ttf_font[0] = NULL;
    sentence_font.ttf_font[1] = NULL;
    sentence_font.top_xy[0] = script_h.readInt();
    sentence_font.top_xy[1] = script_h.readInt();
    sentence_font.num_xy[0] = script_h.readInt();
    sentence_font.num_xy[1] = script_h.readInt();
    sentence_font.font_size_xy[0] = script_h.readInt();
    sentence_font.font_size_xy[1] = script_h.readInt();
    sentence_font.pitch_xy[0] = script_h.readInt() + sentence_font.font_size_xy[0];
    sentence_font.pitch_xy[1] = script_h.readInt() + sentence_font.font_size_xy[1];
    sentence_font.wait_time = script_h.readInt();
    sentence_font.is_bold = script_h.readInt()?true:false;
    sentence_font.is_shadow = script_h.readInt()?true:false;

    const char *buf = script_h.readStr();
    dirty_rect.add( sentence_font_info.pos );

    AnimationInfo *ai = &sentence_font_info;
    if ( buf[0] == '#' ){
        sentence_font.is_transparent = true;
        readColor( &sentence_font.window_color, buf );

        ai->remove();
        ai->orig_pos.x = script_h.readInt();
        ai->orig_pos.y = script_h.readInt();
        ai->orig_pos.w = script_h.readInt() - ai->orig_pos.x + 1;
        ai->orig_pos.h = script_h.readInt() - ai->orig_pos.y + 1;
        ai->scalePosXY( screen_ratio1, screen_ratio2 );
        ai->scalePosWH( screen_ratio1, screen_ratio2 );
    }
    else{
        ai->setImageName( buf );
        parseTaggedString( ai );
        setupAnimationInfo( ai );
        ai->orig_pos.x = script_h.readInt();
        ai->orig_pos.y = script_h.readInt();
        ai->scalePosXY( screen_ratio1, screen_ratio2 );

        sentence_font.is_transparent = false;
        sentence_font.window_color[0] = sentence_font.window_color[1] = sentence_font.window_color[2] = 0xff;
    }

    sentence_font.old_xy[0] = sentence_font.x(false);
    sentence_font.old_xy[1] = sentence_font.y(false);
}

int ONScripter::setwindow3Command()
{
    setwindowCore();
    
    clearCurrentPage();
    indent_offset = 0;
    line_enter_status = 0;
    page_enter_status = 0;
    display_mode = DISPLAY_MODE_NORMAL;
    flush( refreshMode(), &sentence_font_info.pos );
    
    return RET_CONTINUE;
}

int ONScripter::setwindow2Command()
{
    const char *buf = script_h.readStr();
    if ( buf[0] == '#' ){
        sentence_font.is_transparent = true;
        readColor( &sentence_font.window_color, buf );
        sentence_font_info.remove();
    }
    else{
        sentence_font.is_transparent = false;
        sentence_font_info.setImageName( buf );
        parseTaggedString( &sentence_font_info );
        setupAnimationInfo( &sentence_font_info );
    }
    repaintCommand();

    return RET_CONTINUE;
}

int ONScripter::setwindowCommand()
{
    setwindowCore();
    
    lookbackflushCommand();
    indent_offset = 0;
    line_enter_status = 0;
    page_enter_status = 0;
    display_mode = DISPLAY_MODE_NORMAL;
    flush( refreshMode(), &sentence_font_info.pos );
    
    return RET_CONTINUE;
}

int ONScripter::setcursorCommand()
{
    bool abs_flag;

    if ( script_h.isName( "abssetcursor" ) ){
        abs_flag = true;
    }
    else{
        abs_flag = false;
    }
    
    int no = script_h.readInt();
    script_h.readStr();
    const char* buf = script_h.saveStringBuffer();
    int x = script_h.readInt();
    int y = script_h.readInt();

    loadCursor( no, buf, x, y, abs_flag );
    
    return RET_CONTINUE;
}

int ONScripter::selectCommand()
{
    enterTextDisplayMode();

    int select_mode = SELECT_GOTO_MODE;
    SelectLink *last_select_link;

    if ( script_h.isName( "selnum" ) )
        select_mode = SELECT_NUM_MODE;
    else if ( script_h.isName( "selgosub" ) )
        select_mode = SELECT_GOSUB_MODE;
    else if ( script_h.isName( "select" ) )
        select_mode = SELECT_GOTO_MODE;
    else if ( script_h.isName( "csel" ) )
        select_mode = SELECT_CSEL_MODE;

    if ( select_mode == SELECT_NUM_MODE ){
        script_h.readVariable();
        script_h.pushVariable();
    }

    bool comma_flag = true;
    if ( select_mode == SELECT_CSEL_MODE ){
        if (saveon_flag && internal_saveon_flag) storeSaveFile();
        saveon_flag = false;
    }
    shortcut_mouse_line = -1;

    int xy[2];
    xy[0] = sentence_font.xy[0];
    xy[1] = sentence_font.xy[1];

    if ( selectvoice_file_name[SELECTVOICE_OPEN] )
        playSound(selectvoice_file_name[SELECTVOICE_OPEN],
                  SOUND_CHUNK, false, MIX_WAVE_CHANNEL );

    last_select_link = &root_select_link;

    while(1){
        if ( script_h.getNext()[0] != 0x0a && comma_flag == true ){

            const char *buf = script_h.readStr();
            comma_flag = (script_h.getEndStatus() & ScriptHandler::END_COMMA);
            if ( select_mode != SELECT_NUM_MODE && !comma_flag )
                errorAndExit( "select: missing comma." );

            // Text part
            SelectLink *slink = new SelectLink();
            setStr( &slink->text, buf );
            //utils::printInfo("Select text %s\n", slink->text);

            // Label part
            if (select_mode != SELECT_NUM_MODE){
                script_h.readStr();
                setStr( &slink->label, script_h.getStringBuffer()+1 );
                //utils::printInfo("Select label %s\n", slink->label );
            }
            last_select_link->next = slink;
            last_select_link = last_select_link->next;

            comma_flag = (script_h.getEndStatus() & ScriptHandler::END_COMMA);
            //utils::printInfo("2 comma %d %c %x\n", comma_flag, script_h.getCurrent()[0], script_h.getCurrent()[0]);
        }
        else if (script_h.getNext()[0] == 0x0a){
            //utils::printInfo("comma %d\n", comma_flag);
            char *buf = script_h.getNext() + 1; // consume eol
            while ( *buf == ' ' || *buf == '\t' ) buf++;
                
            if (comma_flag && *buf == ',')
                errorAndExit( "select: double comma." );

            bool comma2_flag = false;
            if (*buf == ','){
                comma2_flag = true;
                buf++;
                while ( *buf == ' ' || *buf == '\t' ) buf++;
            }
            script_h.setCurrent(buf);
                
            if (*buf == 0x0a){
                comma_flag |= comma2_flag;
                continue;
            }
                
            if (!comma_flag && !comma2_flag){
                select_label_info.next_script = buf;
                //utils::printInfo("select: stop at the end of line\n");
                break;
            }

            //utils::printInfo("continue\n");
            comma_flag = true;
        }
        else{ // if select ends at the middle of the line
            select_label_info.next_script = script_h.getNext();
            //utils::printInfo("select: stop at the middle of the line\n");
            break;
        }
    }

    if ( select_mode != SELECT_CSEL_MODE ){
        last_select_link = root_select_link.next;
        int counter = 1;
        while( last_select_link ){
            if ( *last_select_link->text ){
                ButtonLink *button = getSelectableSentence( last_select_link->text, &sentence_font );
                root_button_link.insert( button );
                button->no = counter;
            }
            counter++;
            last_select_link = last_select_link->next;
        }
    }

    if ( select_mode == SELECT_CSEL_MODE ){
        setCurrentLabel( "customsel" );
        return RET_CONTINUE;
    }
    automode_flag = false;
    sentence_font.xy[0] = xy[0];
    sentence_font.xy[1] = xy[1];

    flush( refreshMode() );
        
    refreshMouseOverButton();

    event_mode = WAIT_TEXT_MODE | WAIT_BUTTON_MODE | WAIT_TIMER_MODE;
    do{
        skip_mode &= ~SKIP_NORMAL;
        if (waitEvent(-1)) return RET_CONTINUE;
    }
    while(current_button_state.button <= 0 || skip_mode & SKIP_NORMAL);
        
    if ( selectvoice_file_name[SELECTVOICE_SELECT] )
        playSound(selectvoice_file_name[SELECTVOICE_SELECT], 
                  SOUND_CHUNK, false, MIX_WAVE_CHANNEL );

    deleteButtonLink();

    int counter = 1;
    last_select_link = root_select_link.next;
    while ( last_select_link ){
        if ( current_button_state.button == counter++ ) break;
        last_select_link = last_select_link->next;
    }

    if ( select_mode  == SELECT_GOTO_MODE ){
        setCurrentLabel( last_select_link->label );
    }
    else if ( select_mode == SELECT_GOSUB_MODE ){
        gosubReal( last_select_link->label, select_label_info.next_script );
    }
    else{ // selnum
        script_h.setInt( &script_h.pushed_variable, current_button_state.button - 1 );
        current_label_info = script_h.getLabelByAddress( select_label_info.next_script );
        current_line = script_h.getLineByAddress( select_label_info.next_script );
        script_h.setCurrent( select_label_info.next_script );
    }
    deleteSelectLink();

    newPage();

    return RET_CONTINUE;
}

int ONScripter::savetimeCommand()
{
    int no = script_h.readInt();

    SaveFileInfo info;
    searchSaveFile( info, no );

    script_h.readVariable();
    if ( !info.valid ){
        script_h.setInt( &script_h.current_variable, 0 );
        for ( int i=0 ; i<3 ; i++ )
            script_h.readVariable();
        return RET_CONTINUE;
    }

    script_h.setInt( &script_h.current_variable, info.month );
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, info.day );
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, info.hour );
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, info.minute );

    return RET_CONTINUE;
}

int ONScripter::savescreenshotCommand()
{
    bool delete_flag = true;
    if      ( script_h.isName( "savescreenshot2" ) ){
        delete_flag = false;
    }       

    if (screenshot_surface == NULL)
        screenshot_surface = AnimationInfo::alloc32bitSurface(screen_device_width, screen_device_height, texture_format);

    SDL_Surface *surface = AnimationInfo::alloc32bitSurface( screenshot_w, screenshot_h, texture_format );
    resizeSurface( screenshot_surface, surface );

    const char *buf = script_h.readStr();
#ifdef ANDROID
    FILE *fp = fopen(buf, "wb");
    SDL_RWops *rwops = SDL_RWFromFP(fp, SDL_TRUE);
#else
    SDL_RWops *rwops = SDL_RWFromFile(buf, "wb");
#endif
    if (rwops == nullptr || SDL_SaveBMP_RW(surface, rwops, 1) != 0)
        utils::printError("Save screenshot failed: %s\n", SDL_GetError());
    SDL_FreeSurface(surface);

    return RET_CONTINUE;
}

int ONScripter::savepointCommand()
{
    storeSaveFile();

    return RET_CONTINUE;
}

int ONScripter::saveonCommand()
{
    if (!autosaveoff_flag)
        saveon_flag = true;

    return RET_CONTINUE;
}

int ONScripter::saveoffCommand()
{
    if (!autosaveoff_flag){
        if (saveon_flag && internal_saveon_flag) storeSaveFile();
    
        saveon_flag = false;
    }

    return RET_CONTINUE;
}

int ONScripter::savegameCommand()
{
    bool savegame2_flag = false;
    if ( script_h.isName( "savegame2" ) )
        savegame2_flag = true;
    
    int no = script_h.readInt();

    const char* savestr = NULL;
    if (savegame2_flag)
        savestr = script_h.readStr();

    if (saveon_flag && internal_saveon_flag) storeSaveFile();
    writeSaveFile( no, savestr ); 

    return RET_CONTINUE;
}

int ONScripter::savefileexistCommand()
{
    script_h.readInt();
    script_h.pushVariable();
    int no = script_h.readInt();

    SaveFileInfo info;
    searchSaveFile( info, no );

    script_h.setInt( &script_h.pushed_variable, (info.valid==true)?1:0 );

    return RET_CONTINUE;
}

int ONScripter::rndCommand()
{
    int upper, lower;
    
    if ( script_h.isName( "rnd2" ) ){
        script_h.readInt();
        script_h.pushVariable();
        
        lower = script_h.readInt();
        upper = script_h.readInt();
    }
    else{
        script_h.readInt();
        script_h.pushVariable();

        lower = 0;
        upper = script_h.readInt() - 1;
    }

    script_h.setInt( &script_h.pushed_variable, lower + (int)( (double)(upper-lower+1)*rand()/(RAND_MAX+1.0)) );

    return RET_CONTINUE;
}

int ONScripter::rmodeCommand()
{
    if ( script_h.readInt() == 1 ) rmode_flag = true;
    else                           rmode_flag = false;

    return RET_CONTINUE;
}

int ONScripter::resettimerCommand()
{
    internal_timer = SDL_GetTicks();

    return RET_CONTINUE;
}

int ONScripter::resetCommand()
{
    int fadeout = mp3fadeout_duration;
    mp3fadeout_duration = 0; //don't use fadeout during a reset
    resetSub();
    mp3fadeout_duration = fadeout;

    start_page = current_page = &page_list[0];
    clearCurrentPage();
    flush( refreshMode(), &sentence_font_info.pos );
    
    /* Initialize local variables */
    for (int i=0 ; i<script_h.global_variable_border ; i++)
        script_h.getVariableData(i).reset(false);

    setCurrentLabel( "start" );
    storeSaveFile();
    
    return RET_CONTINUE;
}

int ONScripter::repaintCommand()
{
    dirty_rect.fill( screen_width, screen_height );
    flush( refreshMode() );
    
    return RET_CONTINUE;
}

int ONScripter::quakeCommand()
{
    int quake_type;

    if      ( script_h.isName( "quakey" ) ){
        quake_type = 0;
    }
    else if ( script_h.isName( "quakex" ) ){
        quake_type = 1;
    }
    else{
        quake_type = 2;
    }

    tmp_effect.no       = script_h.readInt();
    tmp_effect.duration = script_h.readInt();
    if ( tmp_effect.duration < tmp_effect.no * 4 ) tmp_effect.duration = tmp_effect.no * 4;
    tmp_effect.effect   = MAX_EFFECT_NUM + quake_type;

    dirty_rect.fill( screen_width, screen_height );
    SDL_BlitSurface( accumulation_surface, NULL, effect_dst_surface, NULL );

    if (setEffect(&tmp_effect)) return RET_CONTINUE;
    while (doEffect(&tmp_effect));

    return RET_CONTINUE;
}

int ONScripter::puttextCommand()
{
    enterTextDisplayMode(false);

    script_h.readStr();

    string_buffer_offset = 0;
    if (script_h.getEndStatus() & ScriptHandler::END_1BYTE_CHAR)
        string_buffer_offset = 1; // skip the heading `

    int s = line_enter_status;
    while(processText());
    line_enter_status = s;

    return RET_CONTINUE;
}

int ONScripter::prnumclearCommand()
{
    for ( int i=0 ; i<MAX_PARAM_NUM ; i++ ) {
        if ( prnum_info[i] ) {
            dirty_rect.add( prnum_info[i]->pos );
            delete prnum_info[i];
            prnum_info[i] = NULL;
        }
    }
    return RET_CONTINUE;
}

int ONScripter::prnumCommand()
{
    leaveTextDisplayMode();
    
    int no = script_h.readInt();
    if (no < 0 || no >= MAX_PARAM_NUM){
        script_h.readInt();
        script_h.readInt();
        script_h.readInt();
        script_h.readInt();
        script_h.readInt();
        script_h.readStr();
        return RET_CONTINUE;
    }
    
    if ( prnum_info[no] ){
        dirty_rect.add( prnum_info[no]->pos );
        delete prnum_info[no];
    }
    AnimationInfo *ai = prnum_info[no] = new AnimationInfo();
    ai->trans_mode = AnimationInfo::TRANS_STRING;
    ai->num_of_cells = 1;
    ai->setCell(0);
    ai->color_list = new uchar3[ ai->num_of_cells ];
    
    ai->param = script_h.readInt();
    ai->orig_pos.x = script_h.readInt();
    ai->orig_pos.y = script_h.readInt();
    ai->scalePosXY( screen_ratio1, screen_ratio2 );
    ai->font_size_xy[0] = script_h.readInt();
    ai->font_size_xy[1] = script_h.readInt();
    ai->font_pitch[0] = ai->font_size_xy[0];
    ai->font_pitch[1] = ai->font_size_xy[1];

    const char *buf = script_h.readStr();
    readColor( &ai->color_list[0], buf );

    char num_buf[7];
    script_h.getStringFromInteger( num_buf, ai->param, 3 );
    setStr( &ai->file_name, num_buf );

    setupAnimationInfo( ai );
    dirty_rect.add( ai->pos );
    
    return RET_CONTINUE;
}

int ONScripter::printCommand()
{
    leaveTextDisplayMode();

    EffectLink *el = parseEffect(true);
    if (setEffect(el)) return RET_CONTINUE;
    while (doEffect(el));

    return RET_CONTINUE;
}

int ONScripter::playstopCommand()
{
    stopBGM( false );

    return RET_CONTINUE;
}

int ONScripter::playCommand()
{
    bool loop_flag = true;
    if ( script_h.isName( "playonce" ) )
        loop_flag = false;

    const char *buf = script_h.readStr();
    if ( buf[0] == '*' ){
        cd_play_loop_flag = loop_flag;
        int new_cd_track = atoi( buf + 1 );
#ifdef CONTINUOUS_PLAY        
        if ( current_cd_track != new_cd_track ) {
#endif        
            stopBGM( false );
            current_cd_track = new_cd_track;
            playCDAudio();
#ifdef CONTINUOUS_PLAY        
        }
#endif
    }
    else{ // play MIDI
        stopBGM( false );
        
        setStr(&midi_file_name, buf);
        midi_play_loop_flag = loop_flag;
        if (playSound(midi_file_name, SOUND_MIDI, midi_play_loop_flag) != SOUND_MIDI){
            utils::printError("can't play MIDI file %s\n", midi_file_name);
        }
    }

    return RET_CONTINUE;
}

int ONScripter::ofscopyCommand()
{
    SDL_Surface *tmp_surface = AnimationInfo::alloc32bitSurface(render_view_rect.w, render_view_rect.h, texture_format);
    SDL_LockSurface(tmp_surface);
    SDL_RenderReadPixels(renderer, &render_view_rect, tmp_surface->format->format, tmp_surface->pixels, tmp_surface->pitch);
    SDL_UnlockSurface(tmp_surface);
    resizeSurface( tmp_surface, accumulation_surface );
    SDL_FreeSurface(tmp_surface);

    return RET_CONTINUE;
}

int ONScripter::negaCommand()
{
    nega_mode = script_h.readInt();

    dirty_rect.fill( screen_width, screen_height );

    return RET_CONTINUE;
}

int ONScripter::nextcselCommand()
{
    script_h.readInt();

    if (last_nest_info != &root_nest_info &&
        last_nest_info->nest_mode == NestInfo::LABEL){
        char *buf = last_nest_info->next_script;
        while (*buf == ' ' || *buf == '\t' || *buf == 0x0a) buf++;
        if (strncmp( buf, "csel", 4) == 0)
            script_h.setInt( &script_h.current_variable, 1 );
        else
            script_h.setInt( &script_h.current_variable, 0 );
    }
    else
        script_h.setInt( &script_h.current_variable, 0 );

    return RET_CONTINUE;
}

int ONScripter::mspCommand()
{
    leaveTextDisplayMode();

    bool msp2_flag = false;
    if (script_h.isName("msp2")) msp2_flag = true;

    int no = script_h.readInt();
    AnimationInfo *ai = NULL;
    if (msp2_flag) {
        ai = &sprite2_info[no];
        dirty_rect.add( ai->bounding_rect );
    }
    else{
        ai = &sprite_info[no];
        dirty_rect.add( ai->pos );
    }

    ai->orig_pos.x += script_h.readInt();
    ai->orig_pos.y += script_h.readInt();
    ai->scalePosXY( screen_ratio1, screen_ratio2 );
    if (msp2_flag){
        ai->scale_x += script_h.readInt();
        ai->scale_y += script_h.readInt();
        ai->rot     += script_h.readInt();
        ai->calcAffineMatrix();
        dirty_rect.add( ai->bounding_rect );
    }
    else{
        dirty_rect.add( ai->pos );
    }
    
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
        if (ai->trans == -1)
            ai->trans = 255 + script_h.readInt();
        else
            ai->trans += script_h.readInt();
        if      (ai->trans <   0) ai->trans = 0;
        else if (ai->trans > 255) ai->trans = 255;
    }

    return RET_CONTINUE;
}

int ONScripter::mpegplayCommand()
{
    script_h.readStr();
    const char *save_buf = script_h.saveStringBuffer();
    
    bool click_flag = (script_h.readInt()==1)?true:false;

    stopBGM( false );
    if (playMPEG( save_buf, click_flag )) endCommand();

    repaintCommand();
    
    return RET_CONTINUE;
}

int ONScripter::mp3volCommand()
{
    music_volume = script_h.readInt();
    Mix_VolumeMusic( music_volume * MIX_MAX_VOLUME / 100 );

    return RET_CONTINUE;
}

int ONScripter::mp3stopCommand()
{
    if (Mix_PlayingMusic() == 1 && timer_bgmfade_id && mp3fadeout_duration_internal > 0) // already in fadeout
        return RET_CONTINUE;
    
    if (Mix_PlayingMusic() == 1 && mp3fadeout_duration > 0){
        // do a bgm fadeout
        Mix_HookMusicFinished( NULL );
        mp3fadeout_duration_internal = mp3fadeout_duration;
        mp3fade_start = SDL_GetTicks();
        timer_bgmfade_id = SDL_AddTimer(20, bgmfadeCallback, 0);
        setStr(&fadeout_music_file_name, music_file_name);

        char *ext = NULL;
        if (music_file_name) ext = strrchr(music_file_name, '.');
        if (ext && (!strcmp(ext+1, "OGG") || !strcmp(ext+1, "ogg"))){
            // do not wait until fadout is finished when playing ogg
            event_mode = IDLE_EVENT_MODE;
            waitEvent(0);
            setStr( &music_file_name, NULL ); // to ensure not to play music during fadeout

            return RET_CONTINUE;
        }
        else{
            // wait until fadout is finished when playing music other than ogg
            event_mode = WAIT_TIMER_MODE;
            waitEvent(-1);
        }
    }

    stopBGM( false );

    return RET_CONTINUE;
}

int ONScripter::mp3fadeoutCommand()
{
    mp3fadeout_duration = script_h.readInt();

    return RET_CONTINUE;
}

int ONScripter::mp3fadeinCommand()
{
    mp3fadein_duration = script_h.readInt();

    return RET_CONTINUE;
}

int ONScripter::mp3Command()
{
    bool loop_flag = false;
    if      ( script_h.isName( "mp3save" ) ){
        mp3save_flag = true;
    }
    else if ( script_h.isName( "bgmonce" ) ){
        mp3save_flag = false;
    }
    else if ( script_h.isName( "mp3loop" ) ||
              script_h.isName( "bgm" ) ){
        mp3save_flag = true;
        loop_flag = true;
    }
    else{
        mp3save_flag = false;
    }

    mp3stopCommand();
    stopBGM( false );

    music_play_loop_flag = loop_flag;
    music_loopback_offset = 0.0;

    const char *buf = script_h.readStr();
    if (buf[0] != '\0'){
        if (buf[0]=='('){
            buf++;
            bool integer_flag = true;
            double decimal = 0.1;
            while (*buf != ')' && *buf != '\0'){
                if (*buf >= '0' && *buf <= '9'){
                    if (integer_flag)
                        music_loopback_offset = music_loopback_offset*10.0 + *buf - '0';
                    else{
                        music_loopback_offset += decimal*(*buf - '0');
                        decimal *= 0.1;
                    }
                }
                else if (*buf == '.')
                    integer_flag = false;
                buf++;
            }
            if (*buf == ')') buf++;
        }

        int tmp = music_volume;
        setStr(&music_file_name, buf);

        if (mp3fadein_duration > 0)
            music_volume = 0;

        playSound(music_file_name, 
                  SOUND_MUSIC | SOUND_MIDI | SOUND_CHUNK,
                  music_play_loop_flag, MIX_BGM_CHANNEL);

        music_volume = tmp;

        if (mp3fadein_duration > 0) {
            // do a bgm fadein
            mp3fadein_duration_internal = mp3fadein_duration;
            mp3fade_start = SDL_GetTicks();
            timer_bgmfade_id = SDL_AddTimer(20, bgmfadeCallback,
                                            (void*)&timer_bgmfade_id);

            char *ext = NULL;
            if (music_file_name) ext = strrchr(music_file_name, '.');
            if (ext && (!strcmp(ext+1, "OGG") || !strcmp(ext+1, "ogg"))){
                // do not wait until fadin is finished when playing ogg
                event_mode = IDLE_EVENT_MODE;
                waitEvent(0);
            }
            else{
                // wait until fadin is finished when playing music other than ogg
                event_mode = WAIT_TIMER_MODE;
                waitEvent(-1);
            }
        }
    }
        
    return RET_CONTINUE;
}

int ONScripter::movieCommand()
{
    if (script_h.compareString("stop")){
        script_h.readLabel();
        utils::printError(" [movie stop] is not supported yet!!\n");
        return RET_CONTINUE;
    }

    script_h.readStr();
    const char *filename = script_h.saveStringBuffer();
    
    stopBGM(false);

    bool click_flag = false;
    bool loop_flag = false;

    while (script_h.getEndStatus() & ScriptHandler::END_COMMA){
        if (script_h.compareString("pos")){ // not supported yet
            script_h.readLabel();
            script_h.readInt();
            script_h.readInt();
            script_h.readInt();
            script_h.readInt();
            utils::printError(" [movie pos] is not supported yet!!\n");
        }
        else if (script_h.compareString("click")){
            script_h.readLabel();
            click_flag = true;
        }
        else if (script_h.compareString("loop")){
            script_h.readLabel();
            loop_flag = true;
        }
        else if (script_h.compareString("async")){ // not supported yet
            script_h.readLabel();
            utils::printError(" [movie async] is not supported yet!!\n");
        }
        else{
            script_h.readLabel();
        }
    }
    
    if (playMPEG(filename, click_flag, loop_flag)) endCommand();

    return RET_CONTINUE;
}

int ONScripter::movemousecursorCommand()
{
    int x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    int y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    x = x * screen_device_width / screen_width;
    y = y * screen_device_width / screen_width;

    warpMouse(x, y);
    
    return RET_CONTINUE;
}

int ONScripter::monocroCommand()
{
    if ( script_h.compareString( "off" ) ){
        script_h.readLabel();
        monocro_flag = false;
    }
    else{
        monocro_flag = true;
        readColor( &monocro_color, script_h.readStr() );
        
        for (int i=0 ; i<256 ; i++){
            monocro_color_lut[i][0] = (monocro_color[0] * i) >> 8;
            monocro_color_lut[i][1] = (monocro_color[1] * i) >> 8;
            monocro_color_lut[i][2] = (monocro_color[2] * i) >> 8;
        }
    }
    
    dirty_rect.fill( screen_width, screen_height );

    return RET_CONTINUE;
}

int ONScripter::menu_windowCommand()
{
    setFullScreen(false);
    return RET_CONTINUE;
}

int ONScripter::menu_fullCommand()
{
    setFullScreen(true);
    return RET_CONTINUE;
}

int ONScripter::menu_click_pageCommand()
{
    skip_mode |= SKIP_TO_EOP;
    return RET_CONTINUE;
}

int ONScripter::menu_click_defCommand()
{
    skip_mode &= ~SKIP_TO_EOP;
    return RET_CONTINUE;
}

int ONScripter::menu_automodeCommand()
{
    automode_flag = true;
    skip_mode &= ~SKIP_NORMAL;
    utils::printInfo("menu_automode: change to automode\n");
    
    return RET_CONTINUE;
}

int ONScripter::lsp2Command()
{
    leaveTextDisplayMode();

    bool v=true;
    if ( script_h.isName( "lsph2" ) ||
         script_h.isName( "lsph2add" ) ||
         script_h.isName( "lsph2sub" ))
        v = false;

    int blend_mode = AnimationInfo::BLEND_NORMAL;
    if ( script_h.isName( "lsp2add" ) || script_h.isName( "lsph2add" ))
        blend_mode = AnimationInfo::BLEND_ADD;
    else if ( script_h.isName( "lsp2sub" ) || script_h.isName( "lsph2sub" ))
        blend_mode = AnimationInfo::BLEND_SUB;

    int no = script_h.readInt();
    AnimationInfo *ai = &sprite2_info[no];
    
    if (ai->image_surface && ai->visible)
        dirty_rect.add( ai->bounding_rect );
    ai->visible = v;
    ai->blending_mode = blend_mode;
    
    const char *buf = script_h.readStr();
    ai->setImageName( buf );

    ai->orig_pos.x = script_h.readInt();
    ai->orig_pos.y = script_h.readInt();
    ai->scalePosXY( screen_ratio1, screen_ratio2 );
    ai->scale_x = script_h.readInt();
    ai->scale_y = script_h.readInt();
    ai->rot     = script_h.readInt();

    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA )
        ai->trans = script_h.readInt();
    else
        ai->trans = -1;

    parseTaggedString( ai );
    setupAnimationInfo( ai );
    ai->calcAffineMatrix();

    if ( ai->visible )
        dirty_rect.add( ai->bounding_rect );

    return RET_CONTINUE;
}

int ONScripter::lspCommand()
{
    leaveTextDisplayMode();

    bool v=true;
    if ( script_h.isName( "lsph" ) ) v = false;

    int no = script_h.readInt();
    AnimationInfo *ai = &sprite_info[no];

    if (ai->image_surface && ai->visible)
        dirty_rect.add( ai->pos );
    ai->visible = v;
    
    const char *buf = script_h.readStr();
    ai->setImageName(buf);
    ai->orig_pos.x = script_h.readInt();
    ai->orig_pos.y = script_h.readInt();
    ai->scalePosXY(screen_ratio1, screen_ratio2);

    if (script_h.getEndStatus() & ScriptHandler::END_COMMA)
        ai->trans = script_h.readInt();
    else
        ai->trans = -1;
    
    parseTaggedString( ai );
    setupAnimationInfo( ai );

    if ( ai->visible ) dirty_rect.add( ai->pos );

    return RET_CONTINUE;
}

int ONScripter::loopbgmstopCommand()
{
    if ( wave_sample[MIX_LOOPBGM_CHANNEL0] ){
        Mix_Pause(MIX_LOOPBGM_CHANNEL0);
        Mix_FreeChunk( wave_sample[MIX_LOOPBGM_CHANNEL0] );
        wave_sample[MIX_LOOPBGM_CHANNEL0] = NULL;
    }
    if ( wave_sample[MIX_LOOPBGM_CHANNEL1] ){
        Mix_Pause(MIX_LOOPBGM_CHANNEL1);
        Mix_FreeChunk( wave_sample[MIX_LOOPBGM_CHANNEL1] );
        wave_sample[MIX_LOOPBGM_CHANNEL1] = NULL;
    }
    setStr(&loop_bgm_name[0], NULL);
    
    return RET_CONTINUE;
}

int ONScripter::loopbgmCommand()
{
    const char *buf = script_h.readStr();
    setStr( &loop_bgm_name[0], buf );
    buf = script_h.readStr();
    setStr( &loop_bgm_name[1], buf );

    playSound(loop_bgm_name[1],
              SOUND_PRELOAD|SOUND_CHUNK, false, MIX_LOOPBGM_CHANNEL1);
    playSound(loop_bgm_name[0],
              SOUND_CHUNK, false, MIX_LOOPBGM_CHANNEL0);
    
    return RET_CONTINUE;
}

int ONScripter::lookbackflushCommand()
{
    current_page = current_page->next;
    for ( int i=0 ; i<max_page_list-1 ; i++ ){
        current_page->text_count = 0;
        current_page = current_page->next;
    }
    clearCurrentPage();
    start_page = current_page;
    
    return RET_CONTINUE;
}

int ONScripter::lookbackbuttonCommand()
{
    for ( int i=0 ; i<4 ; i++ ){
        const char *buf = script_h.readStr();
        setStr( &lookback_info[i].image_name, buf );
        parseTaggedString( &lookback_info[i] );
        setupAnimationInfo( &lookback_info[i] );
    }
    return RET_CONTINUE;
}

int ONScripter::logspCommand()
{
    leaveTextDisplayMode();

    bool logsp2_flag = false;

    if ( script_h.isName( "logsp2" ) )
        logsp2_flag = true;

    int no = script_h.readInt();
    AnimationInfo *ai = &sprite_info[no];

    if (ai->image_surface && ai->visible)
        dirty_rect.add( ai->pos );
    ai->remove();
    setStr( &ai->file_name, script_h.readStr() );

    ai->orig_pos.x = script_h.readInt();
    ai->orig_pos.y = script_h.readInt();
    ai->scalePosXY( screen_ratio1, screen_ratio2 );
    
    ai->trans_mode = AnimationInfo::TRANS_STRING;
    if (logsp2_flag){
        ai->font_size_xy[0] = script_h.readInt();
        ai->font_size_xy[1] = script_h.readInt();
        ai->font_pitch[0] = script_h.readInt() + ai->font_size_xy[0];
        ai->font_pitch[1] = script_h.readInt() + ai->font_size_xy[1];
    }
    else{
        ai->font_size_xy[0] = sentence_font.font_size_xy[0];
        ai->font_size_xy[1] = sentence_font.font_size_xy[1];
        ai->font_pitch[0] = sentence_font.pitch_xy[0];
        ai->font_pitch[1] = sentence_font.pitch_xy[1];
    }
    
    char *current = script_h.getNext();
    int num = 0;
    while(script_h.getEndStatus() & ScriptHandler::END_COMMA){
        script_h.readStr();
        num++;
    }

    script_h.setCurrent(current);
    if (num == 0){
        ai->num_of_cells = 1;
        ai->color_list = new uchar3[ ai->num_of_cells ];
        readColor( &ai->color_list[0], "#ffffff" );
    }
    else{
        ai->num_of_cells = num;
        ai->color_list = new uchar3[ ai->num_of_cells ];
        for (int i=0 ; i<num ; i++){
            readColor( &ai->color_list[i], script_h.readStr() );
        }
    }

    ai->is_single_line = false;
    ai->is_tight_region = false;
    ai->is_ruby_drawable = sentence_font.rubyon_flag;
    sentence_font.is_newline_accepted = true;
    setupAnimationInfo( ai );
    sentence_font.is_newline_accepted = false;
    ai->visible = true;
    dirty_rect.add( ai->pos );
    
    return RET_CONTINUE;
}

int ONScripter::locateCommand()
{
    int x = script_h.readInt();
    int y = script_h.readInt();
    sentence_font.setXY( x, y );

    return RET_CONTINUE;
}

int ONScripter::loadgameCommand()
{
    int no = script_h.readInt();

    int fadeout = mp3fadeout_duration;
    mp3fadeout_duration = 0; //don't use fadeout during a load
    if ( !loadSaveFile( no ) ){
        dirty_rect.fill( screen_width, screen_height );
        flush( refreshMode() );

        saveon_flag = true;
        internal_saveon_flag = true;
        skip_mode &= ~SKIP_NORMAL;
        automode_flag = false;
        deleteButtonLink();
        deleteSelectLink();
        text_on_flag = false;
        indent_offset = 0;
        line_enter_status = 0;
        page_enter_status = 0;
        string_buffer_offset = 0;
        break_flag = false;

        flushEvent();

#ifdef USE_LUA
        if (lua_handler.isCallbackEnabled(LUAHandler::LUA_LOAD)){
            if (lua_handler.callFunction(true, "load", &no))
                errorAndExit( lua_handler.error_str );
        }
#endif

        if (loadgosub_label)
            gosubReal( loadgosub_label, script_h.getCurrent() );
    }

    mp3fadeout_duration = fadeout;

    return RET_CONTINUE;
}

int ONScripter::ldCommand()
{
    leaveTextDisplayMode();

    char loc = script_h.readLabel()[0];
    int no = -1;
    if      (loc == 'l') no = 0;
    else if (loc == 'c') no = 1;
    else if (loc == 'r') no = 2;

    const char *buf = NULL;
    if (no >= 0) buf = script_h.readStr();
    
    if (no >= 0){
        AnimationInfo *ai = &tachi_info[no];

        if (ai->image_surface) dirty_rect.add( ai->pos );
        ai->setImageName( buf );
        parseTaggedString( ai );
        setupAnimationInfo( ai );

        if ( ai->image_surface ){
            ai->visible = true;
            ai->orig_pos.x = screen_width * (no+1) * screen_ratio2 / (4 * screen_ratio1) - ai->orig_pos.w / 2;
            ai->orig_pos.y = underline_value - ai->image_surface->h * screen_ratio2 / screen_ratio1;
            ai->scalePosXY( screen_ratio1, screen_ratio2 );
            dirty_rect.add( ai->pos );
        }
    }

    EffectLink *el = parseEffect(true);
    if (setEffect(el)) return RET_CONTINUE;
    while (doEffect(el));

    return RET_CONTINUE;
}
#if defined(USE_SMPEG)
static void smpeg_filter_callback( SDL_Overlay * dst, SDL_Overlay * src, SDL_Rect * region, SMPEG_FilterInfo * filter_info, void * data )
{
    if (dst){
        dst->w = 0;
        dst->h = 0;
    }

    ONScripter *ons = (ONScripter*)data;
    AnimationInfo *ai = ons->getSMPEGInfo();
    if (!ai) return;

    ai->convertFromYUV(src);
    ons->updateEffectDst();
}

static void smpeg_filter_destroy( struct SMPEG_Filter * filter )
{
}
#endif

#ifdef USE_BUILTIN_LAYER_EFFECTS
#include "builtin_layer.h"
#endif

int ONScripter::layermessageCommand()
{
  int no = script_h.readInt();
  const char *message = script_h.readStr();

#ifndef USE_BUILTIN_LAYER_EFFECTS
  utils::printInfo("layermessage: layer effect support not available (%d,'%s')\n", no, message);
  return RET_CONTINUE;
#else
  LayerInfo *tmp = &layer_info[no];
  if (tmp->handler) {
    getret_str = tmp->handler->message(message, getret_int);
    //utils::printInfo("layermessage returned: '%s', %d\n", getret_str, getret_int);
  }
#endif // ndef NO_LAYER_EFFECTS

  return RET_CONTINUE;
}


int ONScripter::kinsokuCommand()
{
    if (script_h.compareString("on")){
        is_kinsoku = true;
        script_h.readLabel();
    }
    else if (script_h.compareString("off")){
        is_kinsoku = false;
        script_h.readLabel();
    }

    return RET_CONTINUE;
}

int ONScripter::jumpfCommand()
{
    char *buf = script_h.getNext();
    while(*buf != '\0' && *buf != '~') buf++;
    if (*buf == '~') buf++;
    
    script_h.setCurrent(buf);
    current_label_info = script_h.getLabelByAddress(buf);
    current_line = script_h.getLineByAddress(buf);
    
    return RET_CONTINUE;
}

int ONScripter::jumpbCommand()
{
    script_h.setCurrent( last_tilde.next_script );
    current_label_info = script_h.getLabelByAddress( last_tilde.next_script );
    current_line = script_h.getLineByAddress( last_tilde.next_script );

    return RET_CONTINUE;
}

int ONScripter::ispageCommand()
{
    script_h.readInt();

    if ( textgosub_clickstr_state == CLICK_NEWPAGE )
        script_h.setInt( &script_h.current_variable, 1 );
    else
        script_h.setInt( &script_h.current_variable, 0 );
    
    return RET_CONTINUE;
}

int ONScripter::isfullCommand()
{
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, fullscreen_mode?1:0 );
    
    return RET_CONTINUE;
}

int ONScripter::isskipCommand()
{
    script_h.readInt();

    if ( automode_flag )
        script_h.setInt( &script_h.current_variable, 2 );
    else if ( skip_mode & SKIP_NORMAL )
        script_h.setInt( &script_h.current_variable, 1 );
    else
        script_h.setInt( &script_h.current_variable, 0 );
    
    return RET_CONTINUE;
}

int ONScripter::isdownCommand()
{
    script_h.readInt();

#if defined(IOS) || defined(ANDROID) || defined(WINRT)
    if (num_fingers > 1)
        current_button_state.down_flag = false;
#endif

    if ( current_button_state.down_flag )
        script_h.setInt( &script_h.current_variable, 1 );
    else
        script_h.setInt( &script_h.current_variable, 0 );
    
    return RET_CONTINUE;
}

int ONScripter::inputCommand()
{
    script_h.readStr();
    
    if ( script_h.current_variable.type != ScriptHandler::VAR_STR ) 
        errorAndExit( "input: no string variable." );
    int no = script_h.current_variable.var_no;

    script_h.readStr(); // description
    const char *buf = script_h.readStr(); // default value
    setStr( &script_h.getVariableData(no).str, buf );

    utils::printInfo( "*** inputCommand(): $%d is set to the default value: %s\n",
            no, buf );
    script_h.readInt(); // maxlen
    script_h.readInt(); // widechar flag
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
        script_h.readInt(); // window width
        script_h.readInt(); // window height
        script_h.readInt(); // text box width
        script_h.readInt(); // text box height
    }

    return RET_CONTINUE;
}

int ONScripter::indentCommand()
{
    indent_offset = script_h.readInt();
    
    return RET_CONTINUE;
}

int ONScripter::humanorderCommand()
{
    leaveTextDisplayMode();
    
    const char *buf = script_h.readStr();
    int i;
    for (i=0 ; i<3 ; i++){
        if      (buf[i] == 'l') human_order[i] = 0;
        else if (buf[i] == 'c') human_order[i] = 1;
        else if (buf[i] == 'r') human_order[i] = 2;
        else                    human_order[i] = -1;
    }

    for ( i=0 ; i<3 ; i++ )
        if (tachi_info[i].image_surface)
            dirty_rect.add( tachi_info[i].pos );

    EffectLink *el = parseEffect(true);
    if (setEffect(el)) return RET_CONTINUE;
    while (doEffect(el));

    return RET_CONTINUE;
}

int ONScripter::getzxcCommand()
{
    getzxc_flag = true;

    return RET_CONTINUE;
}

int ONScripter::getvoicevolCommand()
{
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, voice_volume );
    return RET_CONTINUE;
}

int ONScripter::getversionCommand()
{
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, NSC_VERSION );

    return RET_CONTINUE;
}

int ONScripter::gettimerCommand()
{
    bool gettimer_flag=false;
    
    if      ( script_h.isName( "gettimer" ) ){
        gettimer_flag = true;
    }
    else if ( script_h.isName( "getbtntimer" ) ){
    }

    script_h.readInt();

    if ( gettimer_flag ){
        script_h.setInt( &script_h.current_variable, SDL_GetTicks() - internal_timer );
    }
    else{
        script_h.setInt( &script_h.current_variable, btnwait_time );
    }
        
    return RET_CONTINUE; 
}

int ONScripter::gettextCommand()
{
    script_h.readStr();
    int no = script_h.current_variable.var_no;

    char *buf = new char[ current_page->text_count + 1 ];
    int i, j;
    for ( i=0, j=0 ; i<current_page->text_count ; i++ ){
        if ( current_page->text[i] != 0x0a )
            buf[j++] = current_page->text[i];
    }
    buf[j] = '\0';

    setStr( &script_h.getVariableData(no).str, buf );
    delete[] buf;
    
    return RET_CONTINUE;
}

int ONScripter::gettaglogCommand()
{
    script_h.readVariable();
    script_h.pushVariable();

    int page_no = script_h.readInt();

    Page *page = current_page;
    while(page != start_page && page_no > 0){
        page_no--;
        page = page->previous;
    }

    if (page->tag)
        setStr(&script_h.getVariableData(script_h.pushed_variable.var_no).str, page->tag);
    else
        setStr(&script_h.getVariableData(script_h.pushed_variable.var_no).str, NULL);

    return RET_CONTINUE;
}

int ONScripter::gettagCommand()
{
    if ( !last_nest_info->previous || last_nest_info->nest_mode != NestInfo::LABEL )
        errorAndExit( "gettag: not in a subroutine, i.e. pretextgosub" );

    char *buf = pretext_buf;

    if (buf[0] == '[')
        buf++;
    else if (zenkakko_flag && buf[0] == "��"[0] && buf[1] == "��"[1])
        buf += 2;
    else
        buf = NULL;
    
    int end_status;
    do{
        script_h.readVariable();
        end_status = script_h.getEndStatus();
        script_h.pushVariable();

        if ( script_h.pushed_variable.type & ScriptHandler::VAR_INT ||
             script_h.pushed_variable.type & ScriptHandler::VAR_ARRAY ){
            if (buf)
                script_h.setInt( &script_h.pushed_variable, script_h.parseInt(&buf));
            else
                script_h.setInt( &script_h.pushed_variable, 0);
        }
        else if ( script_h.pushed_variable.type & ScriptHandler::VAR_STR ){
            if (buf){
                const char *buf_start = buf;
                while(*buf != '/' && *buf != 0 && *buf != ']' && 
                      (!zenkakko_flag || buf[0] != "��"[0] || buf[1] != "��"[1])){
                    if (IS_TWO_BYTE(*buf))
                        buf += 2;
                    else
                        buf++;
                }
                setStr( &script_h.getVariableData(script_h.pushed_variable.var_no).str, buf_start, buf-buf_start );
            }
            else{
                setStr( &script_h.getVariableData(script_h.pushed_variable.var_no).str, NULL);
            }
        }

        if (buf) pretext_buf = buf;
        if (buf && *buf == '/')
            buf++;
        else
            buf = NULL;
    }
    while(end_status & ScriptHandler::END_COMMA);

    if (pretext_buf[0] == ']')
        pretext_buf++;
    else if (zenkakko_flag && 
             pretext_buf[0] == "��"[0] && pretext_buf[1] == "��"[1])
        pretext_buf += 2;

    return RET_CONTINUE;
}

int ONScripter::gettabCommand()
{
    gettab_flag = true;
    
    return RET_CONTINUE;
}

int ONScripter::getspsizeCommand()
{
    int no = script_h.readInt();

    script_h.readVariable();
    script_h.setInt( &script_h.current_variable, sprite_info[no].orig_pos.w );
    script_h.readVariable();
    script_h.setInt( &script_h.current_variable, sprite_info[no].orig_pos.h );
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
        script_h.readVariable();
        script_h.setInt( &script_h.current_variable, sprite_info[no].num_of_cells );
    }

    return RET_CONTINUE;
}

int ONScripter::getspposCommand()
{
    int no = script_h.readInt();
    
    script_h.readVariable();
    script_h.setInt( &script_h.current_variable, sprite_info[no].orig_pos.x );

    script_h.readVariable();
    script_h.setInt( &script_h.current_variable, sprite_info[no].orig_pos.y );

    return RET_CONTINUE;
}

int ONScripter::getspmodeCommand()
{
    script_h.readVariable();
    script_h.pushVariable();

    int no = script_h.readInt();
    script_h.setInt( &script_h.pushed_variable, sprite_info[no].visible?1:0 );

    return RET_CONTINUE;
}

int ONScripter::getsevolCommand()
{
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, se_volume );
    return RET_CONTINUE;
}

int ONScripter::getscreenshotCommand()
{
    int w = script_h.readInt();
    if (disable_rescale_flag) w = w * screen_ratio1 / screen_ratio2;
    int h = script_h.readInt();
    if (disable_rescale_flag) h = h * screen_ratio1 / screen_ratio2;
    if ( w == 0 ) w = 1;
    if ( h == 0 ) h = 1;

    screenshot_w = w;
    screenshot_h = h;

    if (screenshot_surface != nullptr && (screenshot_surface->w != render_view_rect.w || screenshot_surface->h != render_view_rect.h)){
        SDL_FreeSurface(screenshot_surface);
        screenshot_surface = nullptr;
    }
    if (screenshot_surface == NULL) screenshot_surface = AnimationInfo::alloc32bitSurface(render_view_rect.w, render_view_rect.h, texture_format);
    if (screen_dirty_flag) {
        SDL_LockSurface(screenshot_surface);
        SDL_RenderReadPixels(renderer, &render_view_rect, screenshot_surface->format->format, screenshot_surface->pixels, screenshot_surface->pitch);
        SDL_UnlockSurface(screenshot_surface);
    } else {
        SDL_BlitSurface(accumulation_surface, nullptr, screenshot_surface, nullptr);
    }

    return RET_CONTINUE;
}

int ONScripter::getsavestrCommand()
{
    script_h.readVariable();
    if ( script_h.current_variable.type != ScriptHandler::VAR_STR )
        errorAndExit( "getsavestr: no string variable." );
        
    script_h.pushVariable();

    int no = script_h.readInt();
    char *buf = readSaveStrFromFile( no );

    setStr( &script_h.getVariableData(script_h.pushed_variable.var_no).str, buf );
    if (buf) delete[] buf;

    return RET_CONTINUE;
}

int ONScripter::getpageupCommand()
{
    getpageup_flag = true;
    
    return RET_CONTINUE;
}

int ONScripter::getpageCommand()
{
    getpageup_flag = true;
    getpagedown_flag = true;
    
    return RET_CONTINUE;
}

int ONScripter::getretCommand()
{
    script_h.readVariable();

    if ( script_h.current_variable.type == ScriptHandler::VAR_INT ||
         script_h.current_variable.type == ScriptHandler::VAR_ARRAY ){
        script_h.setInt( &script_h.current_variable, getret_int );
    }
    else if ( script_h.current_variable.type == ScriptHandler::VAR_STR ){
        int no = script_h.current_variable.var_no;
        setStr( &script_h.getVariableData(no).str, getret_str );
    }
    else errorAndExit( "getret: no variable." );
    
    return RET_CONTINUE;
}

int ONScripter::getregCommand()
{
    script_h.readVariable();
    
    if ( script_h.current_variable.type != ScriptHandler::VAR_STR ) 
        errorAndExit( "getreg: no string variable." );
    int no = script_h.current_variable.var_no;

    const char *buf = script_h.readStr();
    char path[256], key[256];
    strcpy( path, buf );
    buf = script_h.readStr();
    strcpy( key, buf );

    utils::printInfo("  reading Registry file for [%s] %s\n", path, key );
        
    FILE *fp;
    if ( ( fp = fopen( registry_file, "r" ) ) == NULL ){
        utils::printError("Cannot open file [%s]\n", registry_file );
        return RET_CONTINUE;
    }

    char reg_buf[256], reg_buf2[256];
    bool found_flag = false;
    while( fgets( reg_buf, 256, fp) && !found_flag ){
        if ( reg_buf[0] == '[' ){
            unsigned int c=0;
            while ( reg_buf[c] != ']' && reg_buf[c] != '\0' ) c++;
            if ( !strncmp( reg_buf + 1, path, (c-1>strlen(path))?(c-1):strlen(path) ) ){
                while( fgets( reg_buf2, 256, fp) ){

                    script_h.pushCurrent( reg_buf2 );
                    buf = script_h.readStr();
                    if ( strncmp( buf,
                                  key,
                                  (strlen(buf)>strlen(key))?strlen(buf):strlen(key) ) ){
                        script_h.popCurrent();
                        continue;
                    }
                    
                    if ( !script_h.compareString("=") ){
                        script_h.popCurrent();
                        continue;
                    }
                    script_h.setCurrent(script_h.getNext()+1);

                    buf = script_h.readStr();
                    setStr( &script_h.getVariableData(no).str, buf );
                    script_h.popCurrent();
                    utils::printInfo("  $%d = %s\n", no, script_h.getVariableData(no).str );
                    found_flag = true;
                    break;
                }
            }
        }
    }

    if ( !found_flag ) utils::printError("  The key is not found.\n" );
    fclose(fp);

    return RET_CONTINUE;
}

int ONScripter::getmclickCommand()
{
    getmclick_flag = true;
    
    return RET_CONTINUE;
}

int ONScripter::getmp3volCommand()
{
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, music_volume );
    return RET_CONTINUE;
}

int ONScripter::getmouseposCommand()
{
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, current_button_state.x * screen_ratio2 / screen_ratio1 );
    
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, current_button_state.y * screen_ratio2 / screen_ratio1 );
    
    return RET_CONTINUE;
}

int ONScripter::getmouseoverCommand()
{
    getmouseover_flag = true;

    getmouseover_lower = script_h.readInt();
    getmouseover_upper = script_h.readInt();
    
    return RET_CONTINUE;
}

int ONScripter::getlogCommand()
{
    bool getlogtext_flag=false;
    
    if ( script_h.isName( "getlogtext" ) )
        getlogtext_flag = true;

    script_h.readVariable();
    script_h.pushVariable();

    int page_no = script_h.readInt();

    Page *page = current_page;
    while(page != start_page && page_no > 0){
        page_no--;
        page = page->previous;
    }

    if (page_no > 0)
        setStr( &script_h.getVariableData(script_h.pushed_variable.var_no).str, NULL );
    else{
        char *buf = page->text;
        int count = page->text_count;
        if (getlogtext_flag){
            char *p = page->text;
            char *p2 = buf = new char[page->text_count];
            count = 0;
            for (int i=0 ; i<page->text_count ; i++){
                if (IS_TWO_BYTE(*p)){
                    p2[count++] = *p++;
                    p2[count++] = *p++;
                    i++;
                }
                else if (*p != 0x0a)
                    p2[count++] = *p++;
                else
                    p++;
            }
        }
    
        setStr( &script_h.getVariableData(script_h.pushed_variable.var_no).str, buf, count );

        if (getlogtext_flag) delete[] buf;
    }

    return RET_CONTINUE;
}

int ONScripter::getinsertCommand()
{
    getinsert_flag = true;

    return RET_CONTINUE;
}

int ONScripter::getfunctionCommand()
{
    getfunction_flag = true;
    
    return RET_CONTINUE;
}

int ONScripter::getenterCommand()
{
    if ( !force_button_shortcut_flag )
        getenter_flag = true;
    
    return RET_CONTINUE;
}

int ONScripter::getcursorpos2Command()
{
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, sentence_font.old_xy[0] );
    
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, sentence_font.old_xy[1] );
    
    return RET_CONTINUE;
}

int ONScripter::getcursorposCommand()
{
    FontInfo fi = sentence_font;
    
    if ( fi.isEndOfLine() ){
        fi.newLine();
        for (int i=0 ; i<indent_offset ; i++)
            fi.advanceCharInHankaku(2);
    }

    script_h.readInt();
    script_h.setInt( &script_h.current_variable, fi.x(false) );
    
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, fi.y(false) );
    
    return RET_CONTINUE;
}

int ONScripter::getcursorCommand()
{
    if ( !force_button_shortcut_flag )
        getcursor_flag = true;
    
    return RET_CONTINUE;
}

int ONScripter::getcselstrCommand()
{
    script_h.readVariable();
    script_h.pushVariable();

    int csel_no = script_h.readInt();

    int counter = 0;
    SelectLink *link = root_select_link.next;
    while (link){
        if (csel_no == counter++) break;
        link = link->next;
    }

    setStr(&script_h.getVariableData(script_h.pushed_variable.var_no).str, link?(link->text):NULL);

    return RET_CONTINUE;
}

int ONScripter::getcselnumCommand()
{
    int count = 0;

    SelectLink *link = root_select_link.next;
    while ( link ) {
        count++;
        link = link->next;
    }
    script_h.readInt();
    script_h.setInt( &script_h.current_variable, count );

    return RET_CONTINUE;
}

int ONScripter::gameCommand()
{
    if ( current_mode != DEFINE_MODE )
        errorAndExit( "game: not in the define section" );

    int i;
    current_mode = NORMAL_MODE;

    /* ---------------------------------------- */
    if ( !lookback_info[0].image_surface ){
        setStr( &lookback_info[0].image_name, DEFAULT_LOOKBACK_NAME0 );
        parseTaggedString( &lookback_info[0] );
        setupAnimationInfo( &lookback_info[0] );
    }
    if ( !lookback_info[1].image_surface ){
        setStr( &lookback_info[1].image_name, DEFAULT_LOOKBACK_NAME1 );
        parseTaggedString( &lookback_info[1] );
        setupAnimationInfo( &lookback_info[1] );
    }
    if ( !lookback_info[2].image_surface ){
        setStr( &lookback_info[2].image_name, DEFAULT_LOOKBACK_NAME2 );
        parseTaggedString( &lookback_info[2] );
        setupAnimationInfo( &lookback_info[2] );
    }
    if ( !lookback_info[3].image_surface ){
        setStr( &lookback_info[3].image_name, DEFAULT_LOOKBACK_NAME3 );
        parseTaggedString( &lookback_info[3] );
        setupAnimationInfo( &lookback_info[3] );
    }
    
    /* ---------------------------------------- */
    /* Initialize text buffer */
    page_list = new Page[max_page_list];
    for ( i=0 ; i<max_page_list-1 ; i++ ){
        page_list[i].next = &page_list[i+1];
        page_list[i+1].previous = &page_list[i];
    }
    page_list[0].previous = &page_list[max_page_list-1];
    page_list[max_page_list-1].next = &page_list[0];

    resetCommand();

    loadCursor( 0, NULL, 0, 0 );
    loadCursor( 1, NULL, 0, 0 );

#ifdef USE_LUA
    lua_handler.loadInitScript();
    if (lua_handler.isCallbackEnabled(LUAHandler::LUA_RESET)){
        if (lua_handler.callFunction(true, "reset"))
            errorAndExit( lua_handler.error_str );
    }
#endif

    return RET_CONTINUE;
}

int ONScripter::flushoutCommand()
{
    //Mion: flushout special effect
    // not quite the same as NScr's, but looks good
    // does a "flushout" in 30 stages while fading to white
    tmp_effect.duration = script_h.readInt();
    tmp_effect.effect = MAX_EFFECT_NUM + 3;

    dirty_rect.fill(screen_width, screen_height);

    if (setEffect(&tmp_effect)) return RET_CONTINUE;

    setStr(&bg_info.file_name, "white");
    createBackground();
    SDL_BlitSurface(bg_info.image_surface, NULL, effect_dst_surface, NULL);
    SDL_BlitSurface(accumulation_surface, NULL, effect_tmp_surface, NULL);
    while (doEffect(&tmp_effect));

    return RET_CONTINUE;
}

int ONScripter::fileexistCommand()
{
    script_h.readInt();
    script_h.pushVariable();
    const char *buf = script_h.readStr();

    script_h.setInt( &script_h.pushed_variable, (script_h.cBR->getFileLength(buf)>0)?1:0 );

    return RET_CONTINUE;
}

int ONScripter::exec_dllCommand()
{
    const char *buf = script_h.readStr();
    char dll_name[256];
    unsigned int c=0;
    while(buf[c] != '/'){
        dll_name[c] = buf[c];
        c++;
    }
    dll_name[c] = '\0';

    if (strcmp(dll_name, "fileutil.dll") == 0){
        if (strncmp(buf+c, "/mkdir", 6) == 0){
            c += 7;
            char *dir = new char[strlen(archive_path) + strlen(buf+c) + 1];
            sprintf(dir, "%s%s", archive_path, buf+c);
#if defined(LINUX) || defined(MACOSX) || defined(IOS)
            mkdir(dir, 0755);
#elif defined(WIN32)
            _mkdir(dir);
#endif
            delete[] dir;
        }
        return RET_CONTINUE;
    }

    FILE *fp;
    if ( ( fp = fopen( dll_file, "r" ) ) == NULL ){
        utils::printError( "Cannot open file [%s] while reading %s\n", dll_file, dll_name );
        return RET_CONTINUE;
    }

    char dll_buf[256], dll_buf2[256];
    bool found_flag = false;
    while( fgets( dll_buf, 256, fp) && !found_flag ){
        if ( dll_buf[0] == '[' ){
            c=0;
            while ( dll_buf[c] != ']' && dll_buf[c] != '\0' ) c++;
            if ( !strncmp( dll_buf + 1, dll_name, (c-1>strlen(dll_name))?(c-1):strlen(dll_name) ) ){
                found_flag = true;
                while( fgets( dll_buf2, 256, fp) ){
                    c=0;
                    while ( dll_buf2[c] == ' ' || dll_buf2[c] == '\t' ) c++;
                    if ( !strncmp( &dll_buf2[c], "str", 3 ) ){
                        c+=3;
                        while ( dll_buf2[c] == ' ' || dll_buf2[c] == '\t' ) c++;
                        if ( dll_buf2[c] != '=' ) continue;
                        c++;
                        while ( dll_buf2[c] != '"' ) c++;
                        unsigned int c2 = ++c;
                        while ( dll_buf2[c2] != '"' && dll_buf2[c2] != '\0' ) c2++;
                        dll_buf2[c2] = '\0';
                        setStr( &getret_str, &dll_buf2[c] );
                        utils::printInfo("  getret_str = %s\n", getret_str );
                    }
                    else if ( !strncmp( &dll_buf2[c], "ret", 3 ) ){
                        c+=3;
                        while ( dll_buf2[c] == ' ' || dll_buf2[c] == '\t' ) c++;
                        if ( dll_buf2[c] != '=' ) continue;
                        c++;
                        while ( dll_buf2[c] == ' ' || dll_buf2[c] == '\t' ) c++;
                        getret_int = atoi( &dll_buf2[c] );
                        utils::printInfo("  getret_int = %d\n", getret_int );
                    }
                    else if ( dll_buf2[c] == '[' )
                        break;
                }
            }
        }
    }

    if ( !found_flag ) utils::printError("  The DLL is not found in %s.\n", dll_file );
    fclose( fp );
    
    return RET_CONTINUE;
}

int ONScripter::exbtnCommand()
{
    int sprite_no=-1, no=0;
    ButtonLink *bl;
    
    if ( script_h.isName( "exbtn_d" ) ||
         script_h.isName( "bdef" )){
        bl = &exbtn_d_button_link;
        for (int i=0 ; i<3 ; i++){
            if ( bl->exbtn_ctl[i] ){
                delete[] bl->exbtn_ctl[i];
                bl->exbtn_ctl[i] = NULL;
            }
        }
    }
    else{
        bool cellcheck_flag = false;

        if ( script_h.isName( "cellcheckexbtn" ) )
            cellcheck_flag = true;

        sprite_no = script_h.readInt();
        no = script_h.readInt();

        if (no < 1 || 
            sprite_no < 0 ||
            sprite_no >= MAX_SPRITE_NUM || 
            sprite_info[sprite_no].image_surface == NULL ||
            ( cellcheck_flag && sprite_info[ sprite_no ].num_of_cells < 2) ||
            (!cellcheck_flag && sprite_info[ sprite_no ].num_of_cells == 0)){
            script_h.readStr();
            return RET_CONTINUE;
        }
        
        bl = new ButtonLink();
        root_button_link.insert( bl );
        is_exbtn_enabled = true;
    }

    const char *buf = script_h.readStr();
    
    bl->button_type = ButtonLink::SPRITE_BUTTON;
    bl->sprite_no   = sprite_no;
    bl->no          = no;
    setStr( &bl->exbtn_ctl[1], buf );
    
    if ( sprite_no >= 0 &&
         ( sprite_info[ sprite_no ].image_surface ||
           sprite_info[ sprite_no ].trans_mode == AnimationInfo::TRANS_STRING ) )
        bl->image_rect = bl->select_rect = sprite_info[ sprite_no ].pos;

    return RET_CONTINUE;
}

int ONScripter::erasetextwindowCommand()
{
    erase_text_window_mode = script_h.readInt();

    return RET_CONTINUE;
}

int ONScripter::endCommand()
{
    quit();
    stopSMPEG();
    exit(0);
    return RET_CONTINUE; // dummy
}

int ONScripter::dwavestopCommand()
{
    int ch = script_h.readInt();
    if      (ch < 0) ch = 0;
    else if (ch >= ONS_MIX_CHANNELS) ch = ONS_MIX_CHANNELS-1;

    if ( wave_sample[ch] ){
        Mix_Pause( ch );
        Mix_FreeChunk( wave_sample[ch] );
        wave_sample[ch] = NULL;
    }

    return RET_CONTINUE;
}

int ONScripter::dwaveCommand()
{
    int play_mode = WAVE_PLAY;
    bool loop_flag = false;
    
    if ( script_h.isName( "dwaveloop" ) ){
        loop_flag = true;
    }
    else if ( script_h.isName( "dwaveload" ) ){
        play_mode = WAVE_PRELOAD;
    }
    else if ( script_h.isName( "dwaveplayloop" ) ){
        play_mode = WAVE_PLAY_LOADED;
        loop_flag = true;
    }
    else if ( script_h.isName( "dwaveplay" ) ){
        play_mode = WAVE_PLAY_LOADED;
        loop_flag = false;
    }

    int ch = script_h.readInt();
    if      (ch < 0) ch = 0;
    else if (ch >= ONS_MIX_CHANNELS) ch = ONS_MIX_CHANNELS-1;

    if (play_mode == WAVE_PLAY_LOADED){
        Mix_PlayChannel(ch, wave_sample[ch], loop_flag?-1:0);
    }
    else{
        const char *buf = script_h.readStr();
        int fmt = SOUND_CHUNK;
        if (play_mode == WAVE_PRELOAD) fmt |= SOUND_PRELOAD;
        playSound(buf, fmt, loop_flag, ch);
    }
        
    return RET_CONTINUE;
}

int ONScripter::dvCommand()
{
    char buf[256];
    
    sprintf(buf, RELATIVEPATH "voice%c%s.wav", DELIMITER, script_h.getStringBuffer()+2);
    playSound(buf, SOUND_CHUNK, false, 0);
    
    return RET_CONTINUE;
}

int ONScripter::drawtextCommand()
{
    SDL_Rect clip;
    clip.x = clip.y = 0;
    clip.w = accumulation_surface->w;
    clip.h = accumulation_surface->h;
    text_info.blendOnSurface( accumulation_surface, 0, 0, clip );
    
    return RET_CONTINUE;
}

int ONScripter::drawsp3Command()
{
    int sprite_no = script_h.readInt();
    int cell_no = script_h.readInt();
    int alpha = script_h.readInt();
    int x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    int y = script_h.readInt() * screen_ratio1 / screen_ratio2;

    AnimationInfo *ai = &sprite_info[sprite_no];
    int old_cell_no = ai->current_cell;
    ai->setCell(cell_no);

    ai->mat[0][0] = script_h.readInt();
    ai->mat[0][1] = script_h.readInt();
    ai->mat[1][0] = script_h.readInt();
    ai->mat[1][1] = script_h.readInt();

    int denom = (ai->mat[0][0]*ai->mat[1][1]-ai->mat[0][1]*ai->mat[1][0])/1000;
    if (denom != 0){
        ai->inv_mat[0][0] =  ai->mat[1][1] * 1000 / denom;
        ai->inv_mat[0][1] = -ai->mat[0][1] * 1000 / denom;
        ai->inv_mat[1][0] = -ai->mat[1][0] * 1000 / denom;
        ai->inv_mat[1][1] =  ai->mat[0][0] * 1000 / denom;
    }

    ai->blendOnSurface2( accumulation_surface, x, y, screen_rect, alpha );
    ai->setCell(old_cell_no);

    return RET_CONTINUE;
}

int ONScripter::drawsp2Command()
{
    int sprite_no = script_h.readInt();
    int cell_no = script_h.readInt();
    int alpha = script_h.readInt();

    AnimationInfo *ai = &sprite_info[sprite_no];
    ai->orig_pos.x = script_h.readInt();
    ai->orig_pos.y = script_h.readInt();
    ai->scalePosXY( screen_ratio1, screen_ratio2 );
    ai->scale_x = script_h.readInt();
    ai->scale_y = script_h.readInt();
    ai->rot     = script_h.readInt();
    ai->calcAffineMatrix();
    ai->setCell(cell_no);

    ai->blendOnSurface2( accumulation_surface, ai->pos.x, ai->pos.y, screen_rect, alpha );

    return RET_CONTINUE;
}

int ONScripter::drawspCommand()
{
    int sprite_no = script_h.readInt();
    int cell_no = script_h.readInt();
    int alpha = script_h.readInt();
    int x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    int y = script_h.readInt() * screen_ratio1 / screen_ratio2;

    AnimationInfo *ai = &sprite_info[sprite_no];
    int old_cell_no = ai->current_cell;
    ai->setCell(cell_no);
    SDL_Rect clip;
    clip.x = clip.y = 0;
    clip.w = accumulation_surface->w;
    clip.h = accumulation_surface->h;
    ai->blendOnSurface( accumulation_surface, x, y, clip, alpha );
    ai->setCell(old_cell_no);

    return RET_CONTINUE;
}

int ONScripter::drawfillCommand()
{
    int r = script_h.readInt();
    int g = script_h.readInt();
    int b = script_h.readInt();

    SDL_FillRect( accumulation_surface, NULL, SDL_MapRGBA( accumulation_surface->format, r, g, b, 0xff) );
    
    return RET_CONTINUE;
}

int ONScripter::drawclearCommand()
{
    SDL_FillRect( accumulation_surface, NULL, SDL_MapRGBA( accumulation_surface->format, 0, 0, 0, 0xff) );
    
    return RET_CONTINUE;
}

int ONScripter::drawbgCommand()
{
    SDL_Rect clip;
    clip.x = clip.y = 0;
    clip.w = accumulation_surface->w;
    clip.h = accumulation_surface->h;
    bg_info.blendOnSurface( accumulation_surface, bg_info.pos.x, bg_info.pos.y, clip );
    
    return RET_CONTINUE;
}

int ONScripter::drawbg2Command()
{
    AnimationInfo bi = bg_info;
    bi.orig_pos.x = script_h.readInt();
    bi.orig_pos.y = script_h.readInt();
    bi.scalePosXY( screen_ratio1, screen_ratio2 );
    bi.scale_x = script_h.readInt();
    bi.scale_y = script_h.readInt();
    bi.rot     = script_h.readInt();
    bi.calcAffineMatrix();

    bi.blendOnSurface2( accumulation_surface, bi.pos.x, bi.pos.y, screen_rect, 255 );

    return RET_CONTINUE;
}

int ONScripter::drawCommand()
{
    flushDirect( screen_rect, REFRESH_NONE_MODE );
    dirty_rect.clear();
    
    return RET_CONTINUE;
}

int ONScripter::deletescreenshotCommand()
{
  if (screenshot_surface) {
    SDL_FreeSurface(screenshot_surface);
    screenshot_surface = NULL;
  }
  return RET_CONTINUE;
}

int ONScripter::delayCommand()
{
    int val = script_h.readInt();
    
    if (skip_mode & SKIP_NORMAL || ctrl_pressed_status)
        return RET_CONTINUE;

    event_mode = WAIT_TIMER_MODE | WAIT_INPUT_MODE;
    waitEvent( val );

    return RET_CONTINUE;
}

int ONScripter::defineresetCommand()
{
    saveGlovalData();

    script_h.reset();
    ScriptParser::reset();
    reset();

    setCurrentLabel( "define" );

    if ( loadFileIOBuf( "gloval.sav" ) > 0 )
        readVariables( script_h.global_variable_border, script_h.variable_range );

#ifdef USE_LUA
    lua_handler.init(this, &script_h, screen_ratio1, screen_ratio2);
#endif    

    current_mode = DEFINE_MODE;

    return RET_CONTINUE;
}

int ONScripter::cspCommand()
{
    leaveTextDisplayMode();
    
    bool csp2_flag = false;
    if (script_h.isName("csp2")) csp2_flag = true;

    int no = script_h.readInt();
    AnimationInfo *si = NULL;
    int num = 0;
    if (csp2_flag) {
        num = MAX_SPRITE2_NUM;
        si = sprite2_info;
    }
    else{
        num = MAX_SPRITE_NUM;
        si = sprite_info;
    }

    if ( no == -1 )
        for ( int i=0 ; i<num ; i++ ){
            if ( si[i].visible ){
                if (csp2_flag)
                    dirty_rect.add( si[i].bounding_rect );
                else
                    dirty_rect.add( si[i].pos );
            }
            if ( si[i].image_name ){
                si[i].orig_pos.x = -1000;
                si[i].orig_pos.y = -1000;
                si[i].scalePosXY( screen_ratio1, screen_ratio2 );
            }
            if (!csp2_flag) root_button_link.removeSprite(i);
            si[i].remove();
        }
    else if (no >= 0 && no < MAX_SPRITE_NUM){
        if ( si[no].visible ){
            if (csp2_flag)
                dirty_rect.add( si[no].bounding_rect );
            else
                dirty_rect.add( si[no].pos );
        }
        if (!csp2_flag) root_button_link.removeSprite(no);
        si[no].remove();
    }

    return RET_CONTINUE;
}

int ONScripter::cselgotoCommand()
{
    int csel_no = script_h.readInt();

    int counter = 0;
    SelectLink *link = root_select_link.next;
    while( link ){
        if ( csel_no == counter++ ) break;
        link = link->next;
    }
    if ( !link ) errorAndExit( "cselgoto: no select link" );

    setCurrentLabel( link->label );
    
    deleteSelectLink();
    newPage();
    
    return RET_CONTINUE;
}

int ONScripter::cselbtnCommand()
{
    int csel_no   = script_h.readInt();
    int button_no = script_h.readInt();

    FontInfo csel_info = sentence_font;
    csel_info.rubyon_flag = false;
    csel_info.top_xy[0] = script_h.readInt();
    csel_info.top_xy[1] = script_h.readInt();

    int counter = 0;
    SelectLink *link = root_select_link.next;
    while ( link ){
        if ( csel_no == counter++ ) break;
        link = link->next;
    }
    if ( link == NULL || link->text == NULL || *link->text == '\0' )
        return RET_CONTINUE;

    csel_info.setLineArea( strlen(link->text)/2+1 );
    csel_info.clear();
    ButtonLink *button = getSelectableSentence( link->text, &csel_info );
    root_button_link.insert( button );
    button->no          = button_no;
    button->sprite_no   = csel_no;

    sentence_font.ttf_font[0] = csel_info.ttf_font[0];
    sentence_font.ttf_font[1] = csel_info.ttf_font[1];

    return RET_CONTINUE;
}

int ONScripter::clickCommand()
{
    bool lrclick_flag = false;
    if ( script_h.isName( "lrclick" ) ) lrclick_flag = true;

    skip_mode &= ~SKIP_NORMAL;

    event_mode = WAIT_TIMER_MODE | WAIT_INPUT_MODE;
    if (lrclick_flag) event_mode |= WAIT_RCLICK_MODE;
    waitEvent(-1);

    if (lrclick_flag)
        getret_int = (current_button_state.button == -1)?0:1;
        
    return RET_CONTINUE;
}

int ONScripter::clCommand()
{
    leaveTextDisplayMode();
    
    char loc = script_h.readLabel()[0];
    
    if ( loc == 'l' || loc == 'a' ){
        dirty_rect.add( tachi_info[0].pos );
        tachi_info[0].remove();
    }
    if ( loc == 'c' || loc == 'a' ){
        dirty_rect.add( tachi_info[1].pos );
        tachi_info[1].remove();
    }
    if ( loc == 'r' || loc == 'a' ){
        dirty_rect.add( tachi_info[2].pos );
        tachi_info[2].remove();
    }

    EffectLink *el = parseEffect(true);
    if (setEffect(el)) return RET_CONTINUE;
    while (doEffect(el));

    return RET_CONTINUE;
}

int ONScripter::chvolCommand()
{
    int ch  = script_h.readInt();
    if      (ch < 0) ch = 0;
    else if (ch >= ONS_MIX_CHANNELS) ch = ONS_MIX_CHANNELS-1;

    int vol = script_h.readInt();
    if ( wave_sample[ch] ) Mix_Volume( ch, vol * MIX_MAX_VOLUME / 100 );
    
    return RET_CONTINUE;
}

int ONScripter::checkpageCommand()
{
    script_h.readVariable();
    script_h.pushVariable();

    if ( script_h.pushed_variable.type != ScriptHandler::VAR_INT &&
         script_h.pushed_variable.type != ScriptHandler::VAR_ARRAY )
        errorAndExit( "checkpage: no integer variable." );

    int page_no = script_h.readInt();
    
    Page *page = current_page;
    while(page != start_page && page_no > 0){
        page_no--;
        page = page->previous;
    }

    if (page_no > 0)
        script_h.setInt( &script_h.pushed_variable, 0 );
    else
        script_h.setInt( &script_h.pushed_variable, 1 );

    return RET_CONTINUE;
}

int ONScripter::checkkeyCommand()
{
    script_h.readVariable();
    script_h.pushVariable();
    const char *str = script_h.readStr();

    if (strcmp(current_button_state.str, str) == 0)
        script_h.setInt( &script_h.pushed_variable, 1 );
    else
        script_h.setInt( &script_h.pushed_variable, 0 );
    
    return RET_CONTINUE;
}

int ONScripter::cellCommand()
{
    int sprite_no = script_h.readInt();
    int no        = script_h.readInt();

    sprite_info[sprite_no].setCell(no);
    dirty_rect.add( sprite_info[sprite_no].pos );
        
    return RET_CONTINUE;
}

int ONScripter::captionCommand()
{
    const char* buf = script_h.readStr();
    size_t len = strlen(buf);

    char *buf2 = new char[len*3+1];

    DirectReader::convertCodingToUTF8(buf2, buf);
    
    setStr( &wm_title_string, buf2 );
    setStr( &wm_icon_string,  buf2 );
    delete[] buf2;
    
    setCaption( wm_title_string, wm_icon_string );

    return RET_CONTINUE;
}

int ONScripter::btnwaitCommand()
{
    bool del_flag=false, textbtn_flag=false;
    bool bexec_int_flag=false;
    bexec_flag = false;

    if ( script_h.isName( "btnwait2" ) ){
        leaveTextDisplayMode();
    }
    else if ( script_h.isName( "btnwait" ) ){
        del_flag = true;
        leaveTextDisplayMode();
    }
    else if ( script_h.isName( "textbtnwait" ) ){
        textbtn_flag = true;
    }
    else if ( script_h.isName( "bexec" ) ){
        bexec_flag = true;
    }

    if (bexec_flag){
        script_h.readStr();
        script_h.pushVariable();
        if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
            bexec_int_flag = true;
            script_h.readInt();
        }
        getpageup_flag = true;
        getpagedown_flag = true;
        getmclick_flag = true;
        getfunction_flag = true;
    }
    else{
        script_h.readInt();
    }

    ButtonLink *bl = root_button_link.next;
    while( bl ){
        bl->show_flag = 0;
        if ( bl->button_type == ButtonLink::SPRITE_BUTTON ){
            if ( bl->exbtn_ctl[0] ){
                SDL_Rect check_src_rect = bl->image_rect;
                SDL_Rect check_dst_rect = {0, 0, 0, 0};
                decodeExbtnControl( bl->exbtn_ctl[0], &check_src_rect, &check_dst_rect );
            }
            else{
                sprite_info[ bl->sprite_no ].visible = true;
                sprite_info[ bl->sprite_no ].setCell(0);
            }
        }
        else if ( bl->button_type == ButtonLink::TMP_SPRITE_BUTTON ){
            bl->show_flag = 1;
            sprite_info[ bl->sprite_no ].visible = true;
            sprite_info[ bl->sprite_no ].setCell(0);
        }
        else if ( bl->anim[1] != NULL ){
            bl->show_flag = 2;
        }
        dirty_rect.add( bl->image_rect );
        bl = bl->next;
    }

    if (is_exbtn_enabled && exbtn_d_button_link.exbtn_ctl[1]){
        SDL_Rect check_src_rect = screen_rect;
        if (is_exbtn_enabled) decodeExbtnControl( exbtn_d_button_link.exbtn_ctl[1], &check_src_rect );
    }

    if ((textbtn_flag || bexec_flag) && 
        (skip_mode & SKIP_NORMAL || 
         (skip_mode & SKIP_TO_EOP && (textgosub_clickstr_state & 0x03) == CLICK_WAIT) || 
         ctrl_pressed_status) ){
        waitEventSub(0); // for checking keyup event
        current_button_state.button = 0;
        if (bexec_flag) current_button_state.button = -1;
        if (skip_mode & SKIP_NORMAL || 
            (skip_mode & SKIP_TO_EOP && (textgosub_clickstr_state & 0x03) == CLICK_WAIT))
            sprintf(current_button_state.str, "SKIP");
        else
            sprintf(current_button_state.str, "CTRL");
    }
    else{
        shortcut_mouse_line = 0;
        skip_mode &= ~SKIP_NORMAL;

        flush( refreshMode() );

        event_mode = WAIT_BUTTON_MODE;
        refreshMouseOverButton();

        int t = -1;
        if ( btntime_value >= 0 ){
            if ( btntime2_flag )
                event_mode |= WAIT_VOICE_MODE;
            t = btntime_value;
            //if ( usewheel_flag ) current_button_state.button = -5;
            //else                 current_button_state.button = -2;
        }
        internal_button_timer = SDL_GetTicks();

        if ( textbtn_flag ){
            event_mode |= WAIT_INPUT_MODE;
            if ( btntime_value == -1 ){
                if ( automode_flag ){
                    event_mode |= WAIT_VOICE_MODE;
                    if ( automode_time < 0 ){
                        if (t == -1 || t > -automode_time * num_chars_in_sentence)
                            t = -automode_time * num_chars_in_sentence;
                    }
                    else{
                        if (t == -1 || t > automode_time)
                            t = automode_time;
                    }
                    //current_button_state.button = 0;
                }
                else if (autoclick_time > 0 &&
                         (t == -1 || t > autoclick_time))
                    t = autoclick_time;
            }
        }

        event_mode |= WAIT_TIMER_MODE;
        waitEvent(t);
        skip_mode &= ~SKIP_TO_EOL;
    }

    btnwait_time = SDL_GetTicks() - internal_button_timer;
    num_chars_in_sentence = 0;

    if (bexec_flag){
        setStr( &script_h.getVariableData(script_h.pushed_variable.var_no).str, current_button_state.str );
        if (bexec_int_flag){
            if (current_button_state.button >= 0)
                script_h.setInt( &script_h.current_variable, current_button_state.button );
            else
                script_h.setInt( &script_h.current_variable, -1);
        }
    }
    else{
        script_h.setInt( &script_h.current_variable, current_button_state.button );
    }

    if ( current_button_state.button >= 1 && del_flag ){
        deleteButtonLink();
    }

    event_mode = IDLE_EVENT_MODE;
    disableGetButtonFlag();
        
    bl = root_button_link.next;
    while( bl ){
        bl->show_flag = 0;
        bl = bl->next;
    }
            
    return RET_CONTINUE;
}

int ONScripter::btntimeCommand()
{
    bool btime_flag = false;
    if ( script_h.isName( "btime" )){
        btime_flag = true;
        btntime2_flag = false;
    }
    else if ( script_h.isName( "btntime2" ) )
        btntime2_flag = true;
    else
        btntime2_flag = false;
    
    btntime_value = script_h.readInt();

    if ( btime_flag && script_h.getEndStatus() & ScriptHandler::END_COMMA )
        if (script_h.readInt() == 1) btntime2_flag = true;
    
    return RET_CONTINUE;
}

int ONScripter::btndownCommand()
{
    btndown_flag = (script_h.readInt()==1)?true:false;

    return RET_CONTINUE;
}

int ONScripter::btndefCommand()
{
    if (script_h.isName( "bclear" )){
    }
    else if (script_h.compareString("clear")){
        script_h.readLabel();
    }
    else{
        const char *buf = script_h.readStr();

        btndef_info.remove();
        if (blt_texture != NULL) SDL_DestroyTexture(blt_texture);
        blt_texture = NULL;

        if ( buf[0] != '\0' ){
            btndef_info.setImageName( buf );
            parseTaggedString( &btndef_info );
            btndef_info.trans_mode = AnimationInfo::TRANS_COPY;
            setupAnimationInfo( &btndef_info );

            SDL_SetSurfaceBlendMode(btndef_info.image_surface, SDL_BLENDMODE_NONE);
        }
    }
    
    btntime_value = -1;
    transbtn_flag = false;
    deleteButtonLink();

    disableGetButtonFlag();
    
    return RET_CONTINUE;
}

int ONScripter::btnCommand()
{
    SDL_Rect src_rect;
    
    ButtonLink *button = new ButtonLink();
    
    button->no           = script_h.readInt();
    button->image_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    button->image_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    button->image_rect.w = script_h.readInt() * screen_ratio1 / screen_ratio2;
    button->image_rect.h = script_h.readInt() * screen_ratio1 / screen_ratio2;
    button->select_rect = button->image_rect;

    src_rect.x = script_h.readInt() * screen_ratio1 / screen_ratio2;
    src_rect.y = script_h.readInt() * screen_ratio1 / screen_ratio2;
    if (btndef_info.image_surface &&
        src_rect.x + button->image_rect.w > btndef_info.image_surface->w){
        button->image_rect.w = btndef_info.image_surface->w - src_rect.x;
    }
    if (btndef_info.image_surface &&
        src_rect.y + button->image_rect.h > btndef_info.image_surface->h){
        button->image_rect.h = btndef_info.image_surface->h - src_rect.y;
    }
    src_rect.w = button->image_rect.w;
    src_rect.h = button->image_rect.h;

    AnimationInfo *ai = button->anim[0] = new AnimationInfo();
    ai->num_of_cells = 1;
    ai->trans_mode = AnimationInfo::TRANS_COPY;
    ai->pos.x = button->image_rect.x;
    ai->pos.y = button->image_rect.y;
    ai->allocImage( button->image_rect.w, button->image_rect.h, texture_format );
    ai->fill( 0, 0, 0, 0 );
    ai->copySurface( btndef_info.image_surface, &src_rect );
    
    root_button_link.insert( button );

    return RET_CONTINUE;
}

int ONScripter::bspCommand()
{
    int no = script_h.readInt();
    if (no < 0 || no >= MAX_SPRITE_NUM || 
        sprite_info[no].image_surface == NULL){
        for (int i=0 ; i<3 ; i++)
            if ( script_h.getEndStatus() & ScriptHandler::END_COMMA )
                script_h.readStr();
        return RET_CONTINUE;
    }

    ButtonLink *bl = new ButtonLink();
    root_button_link.insert( bl );

    bl->button_type = ButtonLink::SPRITE_BUTTON;
    bl->sprite_no   = no;
    bl->no          = no;

    if ( sprite_info[no].image_surface ||
         sprite_info[no].trans_mode == AnimationInfo::TRANS_STRING )
        bl->image_rect = bl->select_rect = sprite_info[no].pos;

    for (int i=0 ; i<3 ; i++)
        if ( script_h.getEndStatus() & ScriptHandler::END_COMMA )
            setStr( &bl->exbtn_ctl[i], script_h.readStr() );

    return RET_CONTINUE;
}

int ONScripter::brCommand()
{
    enterTextDisplayMode();

    sentence_font.newLine();
    current_page->add( 0x0a );

    return RET_CONTINUE;
}

static SDL_Texture* createMaximumTexture(SDL_Renderer *renderer, SDL_Rect &blt_rect, const SDL_Rect &src_rect, SDL_Surface *blt_surface,
    Uint32 texture_format, int max_texture_width, int max_texture_height) {
    if (src_rect.w > max_texture_width || src_rect.h > max_texture_height) utils::printInfo("Texture too large");
    blt_rect.w = blt_surface->w - src_rect.x > max_texture_width ? max_texture_width : blt_surface->w - src_rect.x;
    blt_rect.h = blt_surface->h - src_rect.y > max_texture_height ? max_texture_height : blt_surface->h - src_rect.y;
    blt_rect.x = src_rect.x;
    blt_rect.y = src_rect.y;
    SDL_Surface *surface = AnimationInfo::alloc32bitSurface(blt_rect.w, blt_rect.h, texture_format);
    SDL_BlitSurface(blt_surface, &blt_rect, surface, NULL);
    SDL_Texture *blt_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return blt_texture;
}

int ONScripter::bltCommand()
{
    Sint16 dx,dy,sx,sy;
    Sint16 dw,dh,sw,sh;

    dx = script_h.readInt() * screen_ratio1 / screen_ratio2;
    dy = script_h.readInt() * screen_ratio1 / screen_ratio2;
    dw = script_h.readInt() * screen_ratio1 / screen_ratio2;
    dh = script_h.readInt() * screen_ratio1 / screen_ratio2;
    sx = script_h.readInt() * screen_ratio1 / screen_ratio2;
    sy = script_h.readInt() * screen_ratio1 / screen_ratio2;
    sw = script_h.readInt() * screen_ratio1 / screen_ratio2;
    sh = script_h.readInt() * screen_ratio1 / screen_ratio2;

    if (btndef_info.image_surface == NULL) return RET_CONTINUE;
    if (dw == 0 || dh == 0 || sw == 0 || sh == 0) return RET_CONTINUE;
    
    if (sx >= 0 && sy >= 0 && sw > 0 && sh > 0) {
        if (sx + sw > btndef_info.image_surface->w) sw = btndef_info.image_surface->w - sx;
        if (sy + sh > btndef_info.image_surface->h) sh = btndef_info.image_surface->h - sy;
        if (dx + dw > screen_width) dw = screen_width - dx;
        else if (dx + dw < 0) dx = -dx;
        if (dy + dh > screen_height) dh = screen_height - dy;
        else if (dy + dh < 0) dh = -dy;
        SDL_Rect src_rect = {sx,sy,sw,sh};
        SDL_Rect dst_rect = {dx,dy,dw,dh};

        if (blt_texture == NULL) {
          if (btndef_info.image_surface->w > max_texture_width || btndef_info.image_surface->h > max_texture_height) {
            blt_texture = createMaximumTexture(renderer, blt_texture_src_rect, src_rect, btndef_info.image_surface,
              texture_format, max_texture_width, max_texture_height);
          } else {
            blt_texture_src_rect.x = 0;
            blt_texture_src_rect.y = 0;
            blt_texture_src_rect.w = btndef_info.image_surface->w;
            blt_texture_src_rect.h = btndef_info.image_surface->h;
            blt_texture = SDL_CreateTextureFromSurface(renderer, btndef_info.image_surface);
          }
        } else {
          if (sx < blt_texture_src_rect.x || sy < blt_texture_src_rect.y
            || sx + sw > blt_texture_src_rect.x + blt_texture_src_rect.w || sy + sh > blt_texture_src_rect.y + blt_texture_src_rect.h) {
            SDL_DestroyTexture(blt_texture);
            blt_texture = createMaximumTexture(renderer, blt_texture_src_rect, src_rect, btndef_info.image_surface,
              texture_format, max_texture_width, max_texture_height);
          }       
        }
        src_rect.x -= blt_texture_src_rect.x;
        src_rect.y -= blt_texture_src_rect.y;
        screen_dirty_flag = true;
        SDL_RenderCopy(renderer, blt_texture, &src_rect, &dst_rect);
        SDL_RenderPresent(renderer);
        dirty_rect.clear();
    } else {
      utils::printError("blt:Wrong arguments.");
    }

    return RET_CONTINUE;
}

int ONScripter::bgcopyCommand()
{
    ofscopyCommand();
    
    setStr( &bg_info.file_name, "*bgcpy" );
    bg_info.num_of_cells = 1;
    bg_info.trans_mode = AnimationInfo::TRANS_COPY;
    bg_info.pos.x = 0;
    bg_info.pos.y = 0;
    bg_info.copySurface( accumulation_surface, NULL );

    return RET_CONTINUE;
}

int ONScripter::bgCommand()
{
    leaveTextDisplayMode();

    const char *buf;
    if (script_h.compareString("white")){
        buf = "white";
        script_h.readLabel();
    }
    else if (script_h.compareString("black")){
        buf = "black";
        script_h.readLabel();
    }
    else{
        buf = script_h.readStr();
    }

    for ( int i=0 ; i<3 ; i++ )
        tachi_info[i].remove();

    bg_info.remove();
    setStr( &bg_info.file_name, buf );

    createBackground();
    dirty_rect.fill( screen_width, screen_height );

    EffectLink *el = parseEffect(true);
    if (setEffect(el)) return RET_CONTINUE;
    while (doEffect(el));

    return RET_CONTINUE;
}

int ONScripter::bdownCommand()
{
    btndown_flag = true;

    return RET_CONTINUE;
}

int ONScripter::barclearCommand()
{
    for ( int i=0 ; i<MAX_PARAM_NUM ; i++ ) {
        if ( bar_info[i] ) {
            dirty_rect.add( bar_info[i]->pos );
            delete bar_info[i];
            bar_info[i] = NULL;
        }
    }
    return RET_CONTINUE;
}

int ONScripter::barCommand()
{
    int no = script_h.readInt();
    AnimationInfo *ai = bar_info[no];
    
    if ( ai ){
        dirty_rect.add( ai->pos );
        ai->remove();
    }
    else{
        ai = bar_info[no] = new AnimationInfo();
    }

    ai->trans_mode = AnimationInfo::TRANS_COPY;
    ai->num_of_cells = 1;
    ai->setCell(0);

    ai->param      = script_h.readInt();
    ai->orig_pos.x = script_h.readInt();
    ai->orig_pos.y = script_h.readInt();
    ai->max_width  = script_h.readInt();
    ai->orig_pos.w = 0;
    ai->orig_pos.h = script_h.readInt();
    ai->max_param  = script_h.readInt();

    ai->scalePosXY( screen_ratio1, screen_ratio2 );

    const char *buf = script_h.readStr();
    readColor( &ai->color, buf );

    int w = 0;
    if (ai->max_param != 0) w = ai->max_width * ai->param / ai->max_param;
    if (ai->max_width > 0 && w > 0) ai->orig_pos.w = w;

    ai->scalePosWH( screen_ratio1, screen_ratio2 );
    ai->allocImage( ai->pos.w, ai->pos.h, texture_format );
    ai->fill( ai->color[0], ai->color[1], ai->color[2], 0xff );
    dirty_rect.add( ai->pos );

    return RET_CONTINUE;
}

int ONScripter::aviCommand()
{
    script_h.readStr();
    const char *save_buf = script_h.saveStringBuffer();
    
    bool click_flag = (script_h.readInt()==1)?true:false;

    stopBGM( false );
    if (playAVI( save_buf, click_flag )) endCommand();

    // should be commented out
    //repaintCommand();

    return RET_CONTINUE;
}

int ONScripter::automode_timeCommand()
{
    automode_time = script_h.readInt();
    
    return RET_CONTINUE;
}

int ONScripter::autoclickCommand()
{
    autoclick_time = script_h.readInt();

    return RET_CONTINUE;
}

int ONScripter::amspCommand()
{
    leaveTextDisplayMode();

    bool amsp2_flag = false;
    if (script_h.isName("amsp2")) amsp2_flag = true;

    int no = script_h.readInt();
    AnimationInfo *ai = NULL;
    if (amsp2_flag){
        ai = &sprite2_info[no];
        dirty_rect.add( ai->bounding_rect );
    }
    else{
        ai = &sprite_info[no];
        dirty_rect.add( ai->pos );
    }

    ai->orig_pos.x = script_h.readInt();
    ai->orig_pos.y = script_h.readInt();
    ai->scalePosXY( screen_ratio1, screen_ratio2 );
    if (amsp2_flag){
        ai->scale_x = script_h.readInt();
        ai->scale_y = script_h.readInt();
        ai->rot     = script_h.readInt();
        ai->calcAffineMatrix();
        dirty_rect.add( ai->bounding_rect );
    }
    else{
        dirty_rect.add( ai->pos );
    }
    
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
        ai->trans = script_h.readInt();
        if      (ai->trans <   0) ai->trans = 0;
        else if (ai->trans > 255) ai->trans = 255;
    }

    return RET_CONTINUE;
}

int ONScripter::allsp2resumeCommand()
{
    all_sprite2_hide_flag = false;

    for ( int i=0 ; i<MAX_SPRITE2_NUM ; i++ ){
        AnimationInfo &ai = sprite2_info[i];
        if (ai.image_surface && ai.visible)
            dirty_rect.add( ai.bounding_rect );
    }
    return RET_CONTINUE;
}

int ONScripter::allspresumeCommand()
{
    all_sprite_hide_flag = false;

    for ( int i=0 ; i<3 ; i++ ){
        AnimationInfo &ai = tachi_info[i];
        if (ai.image_surface && ai.visible)
            dirty_rect.add( ai.pos );
    }

    for ( int i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        AnimationInfo &ai = sprite_info[i];
        if (ai.image_surface && ai.visible)
            dirty_rect.add( ai.pos );
    }

    return RET_CONTINUE;
}

int ONScripter::allsp2hideCommand()
{
    all_sprite2_hide_flag = true;

    for ( int i=0 ; i<MAX_SPRITE2_NUM ; i++ ){
        AnimationInfo *ai = &sprite2_info[i];
        if (ai->image_surface && ai->visible)
            dirty_rect.add( ai->bounding_rect );
    }
    return RET_CONTINUE;
}

int ONScripter::allsphideCommand()
{
    all_sprite_hide_flag = true;

    for ( int i=0 ; i<3 ; i++ ){
        AnimationInfo &ai = tachi_info[i];
        if (ai.image_surface && ai.visible)
            dirty_rect.add( ai.pos );
    }

    for ( int i=0 ; i<MAX_SPRITE_NUM ; i++ ){
        AnimationInfo &ai = sprite_info[i];
        if (ai.image_surface && ai.visible)
            dirty_rect.add( ai.pos );
    }

    return RET_CONTINUE;
}

void ONScripter::NSDCallCommand(int texnum, const char *str1, int proc, const char *str2)
{
    if (texnum < 0 || texnum >= MAX_TEXTURE_NUM) return;

    NSDLoadCommand(texnum, str1);

    if (proc == 1){ // deffontd.dll, Font
        FontInfo f_info = sentence_font;
        f_info.rubyon_flag = false;
        f_info.setTateyokoMode(0);
        f_info.top_xy[0] = f_info.top_xy[1] = 0;
        f_info.clear();
            
        f_info.ttf_font[0] = NULL;
        f_info.ttf_font[1] = NULL;
        
        RubyStruct rs_old = ruby_struct;
        ruby_struct.font_name = NULL;

        const char *start[8];
        start[0] = str2;
        int i=0, num_param=1;
        while(str2[i] && num_param<8) if (str2[i++]==',') start[num_param++] = str2+i;
        switch(num_param){
          case 8: case 7:
            for (i=0 ; i<2 ; i++){
                int j=0;
                ruby_struct.font_size_xy[i] = 0;
                while(start[4+i][j]>='0' && start[4+i][j]<='9')
                    ruby_struct.font_size_xy[i] = ruby_struct.font_size_xy[i]*10 + start[4+i][j++] - '0';
            }
          case 5:
            i=0;
            while(start[3][i] != ',' && start[3][i] != 0){
                if (start[3][i++] == 'r'){
                    f_info.rubyon_flag = true;
                    break;
                }
            }
          case 4: case 3:
            for (i=0 ; i<2 ; i++){
                int j=0;
                f_info.font_size_xy[i] = 0;
                while(start[i][j]>='0' && start[i][j]<='9')
                    f_info.font_size_xy[i] = f_info.font_size_xy[i]*10 + start[i][j++] - '0';
            }
            f_info.font_size_xy[0] *= 2;
            f_info.pitch_xy[0] = f_info.font_size_xy[0];
            f_info.pitch_xy[1] = f_info.font_size_xy[1];
        }
        uchar3 color = {0xff, 0xff, 0xff};
        char *p = (char*)start[num_param-1], *p2 = (char*)start[num_param-1];
        while(*p){
            if (IS_TWO_BYTE(*p)){
                *p2++ = *p++;
                *p2++ = *p++;
            }
            else if (*p == '%'){
                p++;
                if (*p == '%' || *p == '(' || *p == ')') // fix me later
                    *p2++ = *p++;
                else if (*p == '#'){
                    readColor( &color, p );
                    p += 7;
                }
            }
            else{
                *p2++ = *p++;
            }
        }
        *p2 = 0;

        drawString(start[num_param-1], color, &f_info, false, NULL, NULL, &texture_info[texnum], false);

        ruby_struct = rs_old;
    }
}

void ONScripter::NSDDeleteCommand(int texnum)
{
    if (texnum < 0 || texnum >= MAX_TEXTURE_NUM) return;

    texture_info[texnum].remove();
}

void ONScripter::NSDLoadCommand(int texnum, const char *str)
{
    if (texnum < 0 || texnum >= MAX_TEXTURE_NUM) return;

    AnimationInfo *ai = &texture_info[texnum];
    if (str[0] != '*'){
        ai->setImageName( str );
        ai->trans = -1;
    }
    else{
        int c=1, n=0, val[6]={0}; // val[6] = {width, height, R, G, B, alpha}

        while(str[c] != 0 && n<6){
            if (str[c] >= '0' && str[c] <= '9')
                val[n] = val[n]*10 + str[c] - '0';
            if (str[c] == ',') n++;
            c++;
        }

        char buf[32];
        sprintf(buf, ">%d,%d,#%02x%02x%02x", val[0], val[1], val[2], val[3], val[4]);
        ai->setImageName( buf );
        ai->default_alpha = val[5];
    }

    ai->visible = true;
    parseTaggedString( ai );
    ai->trans_mode = AnimationInfo::TRANS_ALPHA;
    setupAnimationInfo( ai );
}

void ONScripter::NSDPresentRectCommand(int x1, int y1, int x2, int y2)
{
    SDL_Rect clip_src;
    clip_src.x = x1;
    clip_src.y = y1;
    clip_src.w = x2-x1+1;
    clip_src.h = y2-y1+1;
    
    SDL_Rect clip;
    clip.x = clip.y = 0;
    clip.w = accumulation_surface->w;
    clip.h = accumulation_surface->h;
    if ( AnimationInfo::doClipping( &clip, &clip_src ) ) return;

    for (int i=MAX_TEXTURE_NUM-1 ; i>0 ; i--)
        if (texture_info[i].image_surface && texture_info[i].visible)
            drawTaggedSurface( accumulation_surface, &texture_info[i], clip );

    flushDirect(clip, REFRESH_NONE_MODE);
}

void ONScripter::NSDSp2Command(int texnum, int dcx, int dcy, int sx, int sy, int w, int h,
                               int xs, int ys, int rot, int alpha)
{
    if (texnum < 0 || texnum >= MAX_TEXTURE_NUM) return;

    AnimationInfo *ai = &texture_info[texnum];
    ai->orig_pos.x = dcx;
    ai->orig_pos.y = dcy;
    ai->scalePosXY( screen_ratio1, screen_ratio2 );
    ai->scale_x = xs;
    ai->scale_y = ys;
    ai->rot     = rot;
    ai->trans = alpha;

    ai->affine_pos.x = sx*screen_ratio1/screen_ratio2;
    ai->affine_pos.y = sy*screen_ratio1/screen_ratio2;
    ai->affine_pos.w =  w*screen_ratio1/screen_ratio2;
    ai->affine_pos.h =  h*screen_ratio1/screen_ratio2;
    ai->calcAffineMatrix();
    ai->affine_flag = true;
}

void ONScripter::NSDSetSpriteCommand(int spnum, int texnum, const char *tag)
{
    if (spnum < 0 || spnum >= MAX_SPRITE_NUM) return;
    if (texnum < 0 || texnum >= MAX_TEXTURE_NUM) return;

    AnimationInfo *ais = &sprite_info[spnum];
    AnimationInfo *ait = &texture_info[texnum];
    *ais = *ait;
    ais->visible = true;

    char buf[256];
    if (tag)
        sprintf(buf, "%s%s", tag, ait->file_name);
    else
        sprintf(buf, ":a;%s", ait->file_name);
    ais->setImageName(buf);
    parseTaggedString(ais);

    if (ais->affine_flag){
        ais->orig_pos.x = ait->orig_pos.x;
        if (ait->num_of_cells > 0)
            ais->orig_pos.x -= ait->orig_pos.w/ait->num_of_cells/2;
        else
            ais->orig_pos.x -= ait->orig_pos.w/2;
        ais->orig_pos.y = ait->orig_pos.y - ait->orig_pos.h/2;
        ais->scalePosXY( screen_ratio1, screen_ratio2 );
        ais->affine_flag = false;
    }
}

void ONScripter::stopSMPEG()
{
#if defined(USE_SMPEG)
    if (layer_smpeg_sample){
        SMPEG_stop( layer_smpeg_sample );
        SMPEG_delete( layer_smpeg_sample );
        layer_smpeg_sample = NULL;
    }
    if (layer_smpeg_buffer){
        delete[] layer_smpeg_buffer;
        layer_smpeg_buffer = NULL;
    }
#endif        
}
