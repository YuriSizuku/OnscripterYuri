/* -*- C++ -*-
 *
 *  ONScripter_rmenu.cpp - Right click menu handler of ONScripter
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
#include "Utils.h"
#include "coding2utf16.h"

#define DIALOG_W 241
#define DIALOG_H 167
#define DIALOG_HEADER 32
#define DIALOG_FOOTER 64
#define DIALOG_BUTTON_W 96
#define DIALOG_BUTTON_H 42

#if defined(ENABLE_1BYTE_CHAR) && defined(FORCE_1BYTE_CHAR)
#define MESSAGE_SAVE_EXIST "`%s%s    Date %s/%s    Time %s:%s"
#define MESSAGE_SAVE_EMPTY "`%s%s    ------------------------"
#define MESSAGE_SAVE_CONFIRM "`Save in slot %s%s?"
#define MESSAGE_LOAD_CONFIRM "`Load from slot %s%s?"
#define MESSAGE_RESET_CONFIRM "`Return to Title Menu?"
#define MESSAGE_END_CONFIRM "`Quit?"
#define MESSAGE_YES "Yes"
#define MESSAGE_NO "No"
#define MESSAGE_OK "OK"
#define MESSAGE_CANCEL "Cancel"
#else
extern Coding2UTF16 *coding2utf16;
#define MESSAGE_SAVE_EXIST coding2utf16->MESSAGE_SAVE_EXIST
#define MESSAGE_SAVE_EMPTY coding2utf16->MESSAGE_SAVE_EMPTY
#define MESSAGE_SAVE_CONFIRM coding2utf16->MESSAGE_SAVE_CONFIRM
#define MESSAGE_LOAD_CONFIRM coding2utf16->MESSAGE_LOAD_CONFIRM
#define MESSAGE_RESET_CONFIRM coding2utf16->MESSAGE_RESET_CONFIRM
#define MESSAGE_END_CONFIRM coding2utf16->MESSAGE_END_CONFIRM
#define MESSAGE_YES coding2utf16->MESSAGE_YES
#define MESSAGE_NO coding2utf16->MESSAGE_NO
#define MESSAGE_OK coding2utf16->MESSAGE_OK
#define MESSAGE_CANCEL coding2utf16->MESSAGE_CANCEL
#endif

#ifdef ANDROID
#include <stdarg.h>
static int osprintf(char *str, const char *format, ...)
{
    str[0] = 0;
    va_list list;
    va_start( list, format );
    while(*format){
        if (IS_TWO_BYTE(*format)){
            strncat(str, format, 2);
            format += 2;
        }
        else if (format[0] == '%' && format[1] == 's'){
            strcat(str, va_arg(list, char*));
            format += 2;
        }
        else{
            strncat(str, format++, 1);
        }
    }
    va_end( list );
    return strlen(str);
}
#define sprintf osprintf
#endif

void ONScripter::enterSystemCall()
{
    shelter_button_link = root_button_link.next;
    root_button_link.next = NULL;
    shelter_select_link = root_select_link.next;
    root_select_link.next = NULL;
    shelter_event_mode = event_mode;
    shelter_mouse_state.x = last_mouse_state.x;
    shelter_mouse_state.y = last_mouse_state.y;
    event_mode = IDLE_EVENT_MODE;
    shelter_display_mode = display_mode;
    display_mode = DISPLAY_MODE_TEXT;
    shelter_draw_cursor_flag = draw_cursor_flag;
    draw_cursor_flag = false;
}

void ONScripter::leaveSystemCall( bool restore_flag )
{
    current_font = &sentence_font;
    display_mode = shelter_display_mode;

    if ( restore_flag ){
        
        current_page = cached_page;
        SDL_BlitSurface( backup_surface, NULL, text_info.image_surface, NULL );
        root_button_link.next = shelter_button_link;
        root_select_link.next = shelter_select_link;

        event_mode = shelter_event_mode;
        draw_cursor_flag = shelter_draw_cursor_flag;
        if ( event_mode & WAIT_BUTTON_MODE ){
            int x = shelter_mouse_state.x * screen_device_width / screen_width;
            int y = shelter_mouse_state.y * screen_device_width / screen_width;
            warpMouse(x, y);
        }
    }
    dirty_rect.fill( screen_width, screen_height );
    flush( refreshMode() );

    //utils::printInfo("leaveSystemCall %d %d\n",event_mode, clickstr_state);

    refreshMouseOverButton();

    system_menu_mode = SYSTEM_NULL;
}

int ONScripter::executeSystemCall()
{
    SDL_BlitSurface( text_info.image_surface, NULL, backup_surface, NULL );
    
    enterSystemCall();

    while(system_menu_mode != SYSTEM_NULL){
        dirty_rect.fill( screen_width, screen_height );
        switch(system_menu_mode){
          case SYSTEM_SKIP:
            executeSystemSkip();
            return 2; // continue parsing text
            break;
          case SYSTEM_RESET:
            if (executeSystemReset()) return 1; // stop parsing text
            break;
          case SYSTEM_SAVE:
            executeSystemSave();
            break;
          case SYSTEM_LOAD:
            if (executeSystemLoad()) return 1; // stop parsing text
            break;
          case SYSTEM_LOOKBACK:
            executeSystemLookback();
            break;
          case SYSTEM_WINDOWERASE:
            executeWindowErase();
            break;
          case SYSTEM_MENU:
            executeSystemMenu();
            break;
          case SYSTEM_AUTOMODE:
            executeSystemAutomode();
            return 2; // continue parsing text
            break;
          case SYSTEM_END:
            executeSystemEnd();
            break;
          default:
            leaveSystemCall();
        }
    }

    return 0;
}

void ONScripter::executeSystemMenu()
{
    current_font = &menu_font;

    if ( menuselectvoice_file_name[MENUSELECTVOICE_OPEN] )
        playSound(menuselectvoice_file_name[MENUSELECTVOICE_OPEN],
                  SOUND_CHUNK, false, MIX_WAVE_CHANNEL);

    text_info.fill( 0, 0, 0, 0 );
    flush( refreshMode() );

    menu_font.num_xy[0] = rmenu_link_width;
    menu_font.num_xy[1] = rmenu_link_num;
    menu_font.top_xy[0] = (screen_width * screen_ratio2 / screen_ratio1 - menu_font.num_xy[0] * menu_font.pitch_xy[0]) / 2;
    menu_font.top_xy[1] = (screen_height * screen_ratio2 / screen_ratio1  - menu_font.num_xy[1] * menu_font.pitch_xy[1]) / 2;
    menu_font.setXY( (menu_font.num_xy[0] - rmenu_link_width) / 2,
                     (menu_font.num_xy[1] - rmenu_link_num) / 2 );

    RMenuLink *link = root_rmenu_link.next;
    int counter = 1;
    while( link ){
        ButtonLink *button = getSelectableSentence( link->label, &menu_font, false );
        root_button_link.insert( button );
        button->no = counter++;

        link = link->next;
        flush( refreshMode() );
    }

    flushEvent();
    refreshMouseOverButton();

    event_mode = WAIT_BUTTON_MODE;
    do waitEventSub(-1);
    while (current_button_state.button == 0);

    deleteButtonLink();

    if ( current_button_state.button == -1 ){
        if ( menuselectvoice_file_name[MENUSELECTVOICE_CANCEL] )
            playSound(menuselectvoice_file_name[MENUSELECTVOICE_CANCEL], 
                      SOUND_CHUNK, false, MIX_WAVE_CHANNEL);
        leaveSystemCall();
        return;
    }
    
    if ( menuselectvoice_file_name[MENUSELECTVOICE_CLICK] )
        playSound(menuselectvoice_file_name[MENUSELECTVOICE_CLICK], 
                  SOUND_CHUNK, false, MIX_WAVE_CHANNEL);

    link = root_rmenu_link.next;
    counter = 1;
    while ( link ){
        if ( current_button_state.button == counter++ ){
            system_menu_mode = link->system_call_no;
            break;
        }
        link = link->next;
    }
}

void ONScripter::executeSystemSkip()
{
    skip_mode |= SKIP_NORMAL;
    leaveSystemCall();
}

void ONScripter::executeSystemAutomode()
{
    automode_flag = true;
    skip_mode &= ~SKIP_NORMAL;
    utils::printInfo("systemcall_automode: change to automode\n");
    leaveSystemCall();
}

bool ONScripter::executeSystemReset()
{
    if ( executeSystemYesNo( SYSTEM_RESET ) ){
        resetCommand();
        leaveSystemCall( false );
        
        return true;
    }

    leaveSystemCall();

    return false;
}

void ONScripter::executeSystemEnd()
{
    if ( executeSystemYesNo( SYSTEM_END ) )
        endCommand();
    else
        leaveSystemCall();
}

void ONScripter::executeWindowErase()
{
    if (windowchip_sprite_no >= 0)
        sprite_info[windowchip_sprite_no].visible = false;

    display_mode = DISPLAY_MODE_NORMAL;
    flush(mode_saya_flag ? REFRESH_SAYA_MODE : REFRESH_NORMAL_MODE);

    event_mode = WAIT_TIMER_MODE | WAIT_BUTTON_MODE;
    waitEventSub(-1);
        
    if (windowchip_sprite_no >= 0)
        sprite_info[windowchip_sprite_no].visible = true;

    leaveSystemCall();
}

bool ONScripter::executeSystemLoad()
{
    current_font = &menu_font;

    text_info.fill( 0, 0, 0, 0 );
        
    menu_font.num_xy[0] = (strlen(save_item_name)+1)/2+2+13;
    menu_font.num_xy[1] = num_save_file+2;
    menu_font.top_xy[0] = (screen_width * screen_ratio2 / screen_ratio1 - menu_font.num_xy[0] * menu_font.pitch_xy[0]) / 2;
    menu_font.top_xy[1] = (screen_height * screen_ratio2 / screen_ratio1  - menu_font.num_xy[1] * menu_font.pitch_xy[1]) / 2;
    menu_font.setXY( (menu_font.num_xy[0] - strlen( load_menu_name ) / 2) / 2, 0 );
    uchar3 color = {0xff, 0xff, 0xff};
    drawString( load_menu_name, color, &menu_font, true, accumulation_surface, NULL, &text_info );
    menu_font.newLine();
    menu_font.newLine();
        
    flush( refreshMode() );
        
    bool nofile_flag;
    char *buffer = new char[ strlen( save_item_name ) + 31 + 1 ];

    SaveFileInfo save_file_info;
    for ( unsigned int i=1 ; i<=num_save_file ; i++ ){
        searchSaveFile( save_file_info, i );
        menu_font.setXY( (menu_font.num_xy[0] - (strlen( save_item_name ) / 2 + 15) ) / 2 );

        if ( save_file_info.valid ){
            sprintf( buffer, MESSAGE_SAVE_EXIST,
                     save_item_name,
                     save_file_info.sjis_no,
                     save_file_info.sjis_month,
                     save_file_info.sjis_day,
                     save_file_info.sjis_hour,
                     save_file_info.sjis_minute );
            nofile_flag = false;
        }
        else{
            sprintf( buffer, MESSAGE_SAVE_EMPTY,
                     save_item_name,
                     save_file_info.sjis_no );
            nofile_flag = true;
        }
        ButtonLink *button = getSelectableSentence( buffer, &menu_font, false, nofile_flag );
        root_button_link.insert( button );
        button->no = i;
        flush( refreshMode() );
    }
    delete[] buffer;

    refreshMouseOverButton();

    event_mode = WAIT_BUTTON_MODE;
    int file_no = 0;
    while(1){
        waitEventSub(-1);

        if ( current_button_state.button > 0 ){
            file_no = current_button_state.button;
            searchSaveFile( save_file_info, file_no );
            if ( !save_file_info.valid ) continue;
        }

        if (current_button_state.button != 0) break;
    }

    if ( current_button_state.button > 0 ){
        deleteButtonLink();

        if (executeSystemYesNo( SYSTEM_LOAD, file_no )){
            current_font = &sentence_font;
            system_menu_mode = 0; // for fadeout in mp3stopCommand()
            if ( loadSaveFile( file_no ) )
                return false;

            leaveSystemCall( false );
            saveon_flag = true;
            internal_saveon_flag = true;
            text_on_flag = false;
            indent_offset = 0;
            line_enter_status = 0;
            page_enter_status = 0;
            string_buffer_offset = 0;
            break_flag = false;

            flushEvent();

#ifdef USE_LUA
            if (lua_handler.isCallbackEnabled(LUAHandler::LUA_LOAD)){
                if (lua_handler.callFunction(true, "load", &file_no))
                    errorAndExit( lua_handler.error_str );
            }
#endif

            if (loadgosub_label)
                gosubReal( loadgosub_label, script_h.getCurrent() );

            return true;
        }

        return false;
    }

    deleteButtonLink();
    leaveSystemCall();
    
    return false;
}

void ONScripter::executeSystemSave()
{
    current_font = &menu_font;

    text_info.fill( 0, 0, 0, 0 );

    menu_font.num_xy[0] = (strlen(save_item_name)+1)/2+2+13;
    menu_font.num_xy[1] = num_save_file+2;
    menu_font.top_xy[0] = (screen_width * screen_ratio2 / screen_ratio1 - menu_font.num_xy[0] * menu_font.pitch_xy[0]) / 2;
    menu_font.top_xy[1] = (screen_height * screen_ratio2 / screen_ratio1  - menu_font.num_xy[1] * menu_font.pitch_xy[1]) / 2;
    menu_font.setXY((menu_font.num_xy[0] - strlen( save_menu_name ) / 2 ) / 2, 0);
    uchar3 color = {0xff, 0xff, 0xff};
    drawString( save_menu_name, color, &menu_font, true, accumulation_surface, NULL, &text_info );
    menu_font.newLine();
    menu_font.newLine();
        
    flush( refreshMode() );
        
    bool nofile_flag;
    char *buffer = new char[ strlen( save_item_name ) + 31 + 1 ];
    
    for ( unsigned int i=1 ; i<=num_save_file ; i++ ){
        SaveFileInfo save_file_info;
        searchSaveFile( save_file_info, i );
        menu_font.setXY( (menu_font.num_xy[0] - (strlen( save_item_name ) / 2 + 15) ) / 2 );

        if ( save_file_info.valid ){
            sprintf( buffer, MESSAGE_SAVE_EXIST,
                     save_item_name,
                     save_file_info.sjis_no,
                     save_file_info.sjis_month,
                     save_file_info.sjis_day,
                     save_file_info.sjis_hour,
                     save_file_info.sjis_minute );
            nofile_flag = false;
        }
        else{
            sprintf( buffer, MESSAGE_SAVE_EMPTY,
                     save_item_name,
                     save_file_info.sjis_no );
            nofile_flag = true;
        }
        ButtonLink *button = getSelectableSentence( buffer, &menu_font, false, nofile_flag );
        root_button_link.insert( button );
        button->no = i;
        flush( refreshMode() );
    }
    delete[] buffer;

    refreshMouseOverButton();

    event_mode = WAIT_BUTTON_MODE;
    do waitEventSub(-1);
    while (current_button_state.button == 0);

    deleteButtonLink();

    if ( current_button_state.button > 0 ){
        int file_no = current_button_state.button;
        if (executeSystemYesNo( SYSTEM_SAVE, file_no )){
            if (saveon_flag && internal_saveon_flag) storeSaveFile();
            writeSaveFile( file_no );
            leaveSystemCall();
        }
        return;
    }

    leaveSystemCall();
}

bool ONScripter::executeSystemYesNo( int caller, int file_no )
{
    current_font = &menu_font;

    text_info.fill( 0, 0, 0, 0 );
    dirty_rect.fill( screen_width, screen_height );

    char name[64] = {'\0'};
    if ( caller == SYSTEM_SAVE ){
        SaveFileInfo save_file_info;
        searchSaveFile( save_file_info, file_no );
        sprintf( name, MESSAGE_SAVE_CONFIRM,
                 save_item_name,
                 save_file_info.sjis_no );
    }
    else if ( caller == SYSTEM_LOAD ){
        SaveFileInfo save_file_info;
        searchSaveFile( save_file_info, file_no );
        sprintf( name, MESSAGE_LOAD_CONFIRM,
                 save_item_name,
                 save_file_info.sjis_no );
    }
    else if ( caller ==  SYSTEM_RESET )
        strcpy( name, MESSAGE_RESET_CONFIRM );
    else if ( caller ==  SYSTEM_END )
        strcpy( name, MESSAGE_END_CONFIRM );
        
        
    menu_font.num_xy[0] = strlen(name)/2;
    menu_font.num_xy[1] = 3;
    menu_font.top_xy[0] = (screen_width * screen_ratio2 / screen_ratio1 - menu_font.num_xy[0] * menu_font.pitch_xy[0]) / 2;
    menu_font.top_xy[1] = (screen_height * screen_ratio2 / screen_ratio1  - menu_font.num_xy[1] * menu_font.pitch_xy[1]) / 2;
    menu_font.setXY(0, 0);
    uchar3 color = {0xff, 0xff, 0xff};
    drawString( name, color, &menu_font, true, accumulation_surface, NULL, &text_info );

    flush( refreshMode() );
        
    int offset1 = strlen(name)/5;
    int offset2 = strlen(name)/2 - offset1;
    strcpy( name, MESSAGE_YES );
    menu_font.setXY(offset1-2, 2);
    ButtonLink *button = getSelectableSentence( name, &menu_font, false );
    root_button_link.insert( button );
    button->no = 1;

    strcpy( name, MESSAGE_NO );
    menu_font.setXY(offset2, 2);
    button = getSelectableSentence( name, &menu_font, false );
    root_button_link.insert( button );
    button->no = 2;
        
    flush( refreshMode() );
        
    refreshMouseOverButton();

    event_mode = WAIT_BUTTON_MODE;
    do waitEventSub(-1);
    while (current_button_state.button == 0);
        
    deleteButtonLink();

    if ( current_button_state.button == 1 ){ // yes is selected
        if ( menuselectvoice_file_name[MENUSELECTVOICE_YES] )
            playSound(menuselectvoice_file_name[MENUSELECTVOICE_YES],
                      SOUND_CHUNK, false, MIX_WAVE_CHANNEL);
        return true;
    }
    else{
        if ( menuselectvoice_file_name[MENUSELECTVOICE_NO] )
            playSound(menuselectvoice_file_name[MENUSELECTVOICE_NO],
                      SOUND_CHUNK, false, MIX_WAVE_CHANNEL);
        return false;
    }
}

void ONScripter::setupLookbackButton()
{
    deleteButtonLink();
    
    /* ---------------------------------------- */
    /* Previous button check */
    if ( (current_page->previous->text_count > 0 ) &&
         current_page != start_page ){
        ButtonLink *button = new ButtonLink();
        root_button_link.insert( button );
    
        button->no = 1;

        if ( lookback_sp[0] >= 0 ){
            button->button_type = ButtonLink::SPRITE_BUTTON;
            button->sprite_no = lookback_sp[0];
            AnimationInfo &si = sprite_info[ button->sprite_no ];
            si.visible = true;
            button->select_rect = si.pos;
            button->image_rect  = si.pos;
        }
        else{
            button->button_type = ButtonLink::LOOKBACK_BUTTON;
            button->select_rect = sentence_font_info.pos;
            button->select_rect.h /= 3;
            button->show_flag = 2;
            button->anim[0] = &lookback_info[0];
            button->anim[1] = &lookback_info[1];
            button->image_rect.x = sentence_font_info.pos.x + sentence_font_info.pos.w - button->anim[0]->pos.w;
            button->image_rect.y = sentence_font_info.pos.y;
            button->image_rect.w = button->anim[0]->pos.w;
            button->image_rect.h = button->anim[0]->pos.h;
            button->anim[0]->pos.x = button->anim[1]->pos.x = button->image_rect.x;
            button->anim[0]->pos.y = button->anim[1]->pos.y = button->image_rect.y;
        }
    }
    else if (lookback_sp[0] >= 0){
        sprite_info[ lookback_sp[0] ].visible = false;
    }

    /* ---------------------------------------- */
    /* Next button check */
    if ( current_page->next != cached_page ){
        ButtonLink *button = new ButtonLink();
        root_button_link.insert( button );
    
        button->no = 2;

        if ( lookback_sp[1] >= 0 ){
            button->button_type = ButtonLink::SPRITE_BUTTON;
            button->sprite_no = lookback_sp[1];
            AnimationInfo &si = sprite_info[ button->sprite_no ];
            si.visible = true;
            button->select_rect = si.pos;
            button->image_rect  = si.pos;
        }
        else{
            button->button_type = ButtonLink::LOOKBACK_BUTTON;
            button->select_rect = sentence_font_info.pos;
            button->select_rect.y += sentence_font_info.pos.h*2/3;
            button->select_rect.h /= 3;
            button->show_flag = 2;
            button->anim[0] = &lookback_info[2];
            button->anim[1] = &lookback_info[3];
            button->image_rect.x = sentence_font_info.pos.x + sentence_font_info.pos.w - button->anim[0]->pos.w;
            button->image_rect.y = sentence_font_info.pos.y + sentence_font_info.pos.h - button->anim[0]->pos.h;
            button->image_rect.w = button->anim[0]->pos.w;
            button->image_rect.h = button->anim[0]->pos.h;
            button->anim[0]->pos.x = button->anim[1]->pos.x = button->image_rect.x;
            button->anim[0]->pos.y = button->anim[1]->pos.y = button->image_rect.y;
        }
    }
    else if (lookback_sp[1] >= 0){
        sprite_info[ lookback_sp[1] ].visible = false;
    }
}

void ONScripter::executeSystemLookback()
{
    int i;
    uchar3 color;
    
    current_font = &sentence_font;

    current_page = current_page->previous;
    if ( current_page->text_count == 0 ){
        if ( lookback_sp[0] >= 0 )
            sprite_info[ lookback_sp[0] ].visible = false;
        if ( lookback_sp[1] >= 0 )
            sprite_info[ lookback_sp[1] ].visible = false;
        leaveSystemCall();
        return;
    }

    while(1){
        setupLookbackButton();
        refreshMouseOverButton();

        dirty_rect.fill( screen_width, screen_height );
        flush( refreshMode() & ~REFRESH_TEXT_MODE);

        for ( i=0 ; i<3 ; i++ ){
            color[i] = sentence_font.color[i];
            sentence_font.color[i] = lookback_color[i];
        }
        restoreTextBuffer(accumulation_surface);
        for ( i=0 ; i<3 ; i++ ) sentence_font.color[i] = color[i];
        flush( REFRESH_NONE_MODE );

        event_mode = WAIT_BUTTON_MODE;
        waitEventSub(-1);
    
        if ( current_button_state.button == 0 ||
             ( current_page == start_page &&
               current_button_state.button == -2 ) ){
            continue;
        }
        if ( current_button_state.button == -1 ||
             ( current_button_state.button == -3 &&
               current_page->next == cached_page ) ||
             current_button_state.button <= -4 )
        {
            event_mode = IDLE_EVENT_MODE;
            deleteButtonLink();
            if ( lookback_sp[0] >= 0 )
                sprite_info[ lookback_sp[0] ].visible = false;
            if ( lookback_sp[1] >= 0 )
                sprite_info[ lookback_sp[1] ].visible = false;
            leaveSystemCall();
            return;
        }
        
        if ( current_button_state.button == 1 ||
             current_button_state.button == -2 ){
            current_page = current_page->previous;
        }
        else
            current_page = current_page->next;
    }
}

void ONScripter::buildDialog(bool yesno_flag, const char *mes1, const char *mes2)
{
    SDL_PixelFormat *fmt = image_surface->format;
    SDL_Surface *s = SDL_CreateRGBSurface( SDL_SWSURFACE, DIALOG_W, DIALOG_H,
                                           fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask );

    SDL_Rect rect;
    unsigned char col = 255;
    SDL_FillRect(s, NULL, SDL_MapRGBA(s->format, col, col, col, 0xff));

    rect.x = 2; rect.y = DIALOG_HEADER;
    rect.w = DIALOG_W-4; rect.h = DIALOG_H-rect.y-2;
    col = 105;
    SDL_FillRect(s, &rect, SDL_MapRGBA(s->format, col, col, col, 0xff));
    
    rect.x++; rect.y++; rect.w-=2; rect.h-=2;
    col = 255;
    SDL_FillRect(s, &rect, SDL_MapRGBA(s->format, col, col, col, 0xff));

    rect.h = DIALOG_FOOTER;
    rect.y = DIALOG_H-3-rect.h;
    col = 240;
    SDL_FillRect(s, &rect, SDL_MapRGBA(s->format, col, col, col, 0xff));

    SDL_Surface *s2 = s;
    if (screen_ratio2 != screen_ratio1){
        s2 = SDL_CreateRGBSurface( SDL_SWSURFACE, DIALOG_W*screen_ratio1/screen_ratio2, DIALOG_H*screen_ratio1/screen_ratio2,
                                   fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask );
        resizeSurface(s, s2);
        SDL_FreeSurface(s);
    }

    uchar3 col3={0, 0, 0};
    dialog_font.top_xy[0] = 7;
    dialog_font.top_xy[1] = DIALOG_HEADER+5;
    dialog_font.num_xy[0] = (DIALOG_W-7*2)/dialog_font.pitch_xy[0];
    dialog_font.num_xy[1] = 3;
    dialog_font.clear();
    drawString( mes1, col3, &dialog_font, false, s2, NULL, NULL );

    dialog_font.top_xy[0] = 5;
    dialog_font.top_xy[1] = (DIALOG_HEADER-dialog_font.font_size_xy[1])/2;
    dialog_font.setLineArea( strlen(mes2)/2+1 );
    dialog_font.clear();
    drawString( mes2, col3, &dialog_font, false, s2, NULL, NULL );

    dialog_info.deleteSurface();
    dialog_info.num_of_cells = 1;
    dialog_info.setImage(s2, texture_format);
    
    dialog_info.pos.x = (screen_width  - dialog_info.pos.w)/2;
    dialog_info.pos.y = (screen_height - dialog_info.pos.h)/2;

    // buttons
    const char* mes[2];
    if (yesno_flag){
        mes[0] = MESSAGE_YES;
        mes[1] = MESSAGE_NO;
    }
    else{
        mes[0] = MESSAGE_OK;
        mes[1] = MESSAGE_CANCEL;
    }

    for (int i=0 ; i<2 ; i++){
        SDL_Surface *bs = SDL_CreateRGBSurface( SDL_SWSURFACE, DIALOG_BUTTON_W*2, DIALOG_BUTTON_H,
                                                fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask );

        for (int j=0 ; j<2 ; j++){
            rect.x = DIALOG_BUTTON_W*j;
            rect.y = 0;
            rect.w = DIALOG_BUTTON_W; rect.h = DIALOG_BUTTON_H;

            col = 105;
            SDL_FillRect(bs, &rect, SDL_MapRGBA(bs->format, col, col, col, 0xff));

            rect.w--; rect.h--;
            col = 255;
            SDL_FillRect(bs, &rect, SDL_MapRGBA(bs->format, col, col, col, 0xff));

            rect.x++; rect.y++; rect.w--; rect.h--;
            col = 227;
            SDL_FillRect(bs, &rect, SDL_MapRGBA(bs->format, col, col, col, 0xff));

            rect.x++; rect.y++; rect.w-=2; rect.h-=2;
            col = 240;
            if (j==1) col = 214;
            SDL_FillRect(bs, &rect, SDL_MapRGBA(bs->format, col, col, col, 0xff));
        }

        SDL_Surface *bs2 = bs;
        if (screen_ratio2 != screen_ratio1){
            bs2 = SDL_CreateRGBSurface( SDL_SWSURFACE, DIALOG_BUTTON_W*2*screen_ratio1/screen_ratio2, DIALOG_BUTTON_H*screen_ratio1/screen_ratio2,
                                        fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask );
            resizeSurface(bs, bs2);
            SDL_FreeSurface(bs);
        }

        for (int j=0 ; j<2 ; j++){
            rect.x = DIALOG_BUTTON_W*j+2;
            rect.y = 2;
            rect.w = DIALOG_BUTTON_W-4; rect.h = DIALOG_BUTTON_H-4;

            dialog_font.top_xy[0] = rect.x+(rect.w-dialog_font.pitch_xy[0]*strlen(mes[i])/2)/2;
            dialog_font.top_xy[1] = rect.y+(rect.h-dialog_font.font_size_xy[1])/2;
            dialog_font.setLineArea( strlen(mes[i])/2+1 );
            dialog_font.clear();
            drawString( mes[i], col3, &dialog_font, false, bs2, NULL, NULL );
        }

        ButtonLink *btn = new ButtonLink();
        btn->no = i+1;
        btn->button_type = ButtonLink::TMP_SPRITE_BUTTON;
        btn->anim[0] = new AnimationInfo();
        btn->anim[0]->num_of_cells = 2;
        btn->anim[0]->setImage(bs2, texture_format);
        btn->show_flag = 1;

        btn->anim[0]->pos.x = dialog_info.pos.x + (DIALOG_W-3-(DIALOG_BUTTON_W+8)*(2-i))*screen_ratio1/screen_ratio2;
        btn->anim[0]->pos.y = dialog_info.pos.y + (DIALOG_H-3-(DIALOG_FOOTER+DIALOG_BUTTON_H)/2)*screen_ratio1/screen_ratio2;

        btn->anim[0]->visible = true;
        btn->select_rect = btn->image_rect = btn->anim[0]->pos;

        root_button_link.insert( btn );
    }
}
