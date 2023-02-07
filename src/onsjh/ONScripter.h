/* -*- C++ -*-
 * 
 *  ONScripter.h - Execution block parser of ONScripter
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

#ifndef __ONSCRIPTER_H__
#define __ONSCRIPTER_H__

#include "ScriptParser.h"
#include "DirtyRect.h"
#include "ButtonLink.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "renderer/gles_renderer.h"

#if defined(USE_SMPEG)
#include <smpeg.h>
#endif    

#define DEFAULT_VIDEO_SURFACE_FLAG (SDL_SWSURFACE)

#define DEFAULT_BLIT_FLAG (0)
//#define DEFAULT_BLIT_FLAG (SDL_RLEACCEL)

#define MAX_SPRITE_NUM 1000
#define MAX_SPRITE2_NUM 256
#define MAX_TEXTURE_NUM 16
#define MAX_PARAM_NUM 100
#define MAX_EFFECT_NUM 256

#define DEFAULT_VOLUME 100
#define ONS_MIX_CHANNELS 50
#define ONS_MIX_EXTRA_CHANNELS 4
#define MIX_WAVE_CHANNEL (ONS_MIX_CHANNELS+0)
#define MIX_BGM_CHANNEL (ONS_MIX_CHANNELS+1)
#define MIX_LOOPBGM_CHANNEL0 (ONS_MIX_CHANNELS+2)
#define MIX_LOOPBGM_CHANNEL1 (ONS_MIX_CHANNELS+3)

#define DEFAULT_WM_TITLE "ONScripter"
#define DEFAULT_WM_ICON  "ONScripter"

class ONScripter : public ScriptParser
{
public:
    typedef AnimationInfo::ONSBuf ONSBuf;
    
    struct ButtonState{
        unsigned int event_type;
        unsigned char event_button;
        int x, y, button;
        char str[16];
        bool down_flag;
    };
        
    ONScripter();
    ~ONScripter();

    // ----------------------------------------
    // start-up options
    void enableCDAudio();
    void setCDNumber(int cdrom_drive_number);
    void setFontFile(const char *filename);
    void setRegistryFile(const char *filename);
    void setDLLFile(const char *filename);
    void setArchivePath(const char *path);
    void setSaveDir(const char *path);
    void setFullscreenMode();
    void setWindowMode();
    void setVsyncOff();
    void setFontCache();
    void setDebugLevel(int debug);
    void enableButtonShortCut();
    void enableWheelDownAdvance();
    void disableRescale();
    void renderFontOutline();
    void enableEdit();
    void setKeyEXE(const char *path);
    const char* getArchivePath() { return archive_path; }
    void setWindowWidth(int width);
    void setWindowHeight(int height);
    void setSharpness(float sharpness);
    int  getWidth(){ return screen_width;};
    int  getHeight(){return screen_height;};
    ButtonState &getCurrentButtonState(){return current_button_state;};
    int  getSkip(){return automode_flag?2:((skip_mode&SKIP_NORMAL)?1:0);};
    AnimationInfo *getSMPEGInfo(){return smpeg_info;};
    
    int  openScript();
    int  init();

    // ----------------------------------------
    // Commands
    typedef int (ONScripter::*FuncList)();
    struct FuncLUT{
        char command[30];
        FuncList method;
    };
    struct FuncHash{
        FuncLUT *func;
        int num;
    } func_hash['z'-'a'+1];

    void makeFuncLUT();

    int yesnoboxCommand();
    int wavestopCommand();
    int waveCommand();
    int waittimerCommand();
    int waitCommand();
    int vspCommand();
    int voicevolCommand();
    int vCommand();
    int trapCommand();
    int transbtnCommand();
    int textspeeddefaultCommand();
    int textspeedCommand();
    int textshowCommand();
    int textonCommand();
    int textoffCommand();
    int texthideCommand();
    int textcolorCommand();
    int textclearCommand();
    int texecCommand();
    int tateyokoCommand();
    int talCommand();
    int tablegotoCommand();
    int systemcallCommand();
    int strspCommand();
    int stopCommand();
    int sp_rgb_gradationCommand();
    int spstrCommand();
    int spreloadCommand();
    int splitCommand();
    int spclclkCommand();
    int spbtnCommand();
    int skipoffCommand();
    int sevolCommand();
    int setwindow3Command();
    int setwindow2Command();
    int setwindowCommand();
    int setcursorCommand();
    int selectCommand();
    int savetimeCommand();
    int savepointCommand();
    int saveonCommand();
    int saveoffCommand();
    int savegameCommand();
    int savefileexistCommand();
    int savescreenshotCommand();
    int resettimerCommand();
    int resetCommand();
    int repaintCommand();
    int rndCommand();
    int rmodeCommand();
    int quakeCommand();
    int puttextCommand();
    int prnumclearCommand();
    int prnumCommand();
    int printCommand();
    int playstopCommand();
    int playonceCommand();
    int playCommand();
    int okcancelboxCommand();
    int ofscopyCommand();
    int negaCommand();
    int nextcselCommand();
    int mspCommand();
    int mpegplayCommand();
    int mp3volCommand();
    int mp3stopCommand();
    int mp3fadeoutCommand();
    int mp3fadeinCommand();
    int mp3Command();
    int movieCommand();
    int movemousecursorCommand();
    int monocroCommand();
    int menu_windowCommand();
    int menu_fullCommand();
    int menu_click_pageCommand();
    int menu_click_defCommand();
    int menu_automodeCommand();
    int lsp2Command();
    int lspCommand();
    int loopbgmstopCommand();
    int loopbgmCommand();
    int lookbackflushCommand();
    int lookbackbuttonCommand();
    int logspCommand();
    int locateCommand();
    int loadgameCommand();
    int ldCommand();
    int layermessageCommand();
    int kinsokuCommand();
    int jumpfCommand();
    int jumpbCommand();
    int ispageCommand();
    int isfullCommand();
    int isskipCommand();
    int isdownCommand();
    int inputCommand();
    int indentCommand();
    int humanorderCommand();
    int getzxcCommand();
    int getvoicevolCommand();
    int getversionCommand();
    int gettimerCommand();
    int gettextCommand();
    int gettaglogCommand();
    int gettagCommand();
    int gettabCommand();
    int getspsizeCommand();
    int getspposCommand();
    int getspmodeCommand();
    int getsevolCommand();
    int getscreenshotCommand();
    int getsavestrCommand();
    int getretCommand();
    int getregCommand();
    int getpageupCommand();
    int getpageCommand();
    int getmp3volCommand();
    int getmouseposCommand();
    int getmouseoverCommand();
    int getmclickCommand();
    int getlogCommand();
    int getinsertCommand();
    int getfunctionCommand();
    int getenterCommand();
    int getcursorpos2Command();
    int getcursorposCommand();
    int getcursorCommand();
    int getcselstrCommand();
    int getcselnumCommand();
    int gameCommand();
    int flushoutCommand();
    int fileexistCommand();
    int exec_dllCommand();
    int exbtnCommand();
    int erasetextwindowCommand();
    int endCommand();
    int dwavestopCommand();
    int dwaveCommand();
    int dvCommand();
    int drawtextCommand();
    int drawsp3Command();
    int drawsp2Command();
    int drawspCommand();
    int drawfillCommand();
    int drawclearCommand();
    int drawbg2Command();
    int drawbgCommand();
    int drawCommand();
    int deletescreenshotCommand();
    int delayCommand();
    int defineresetCommand();
    int cspCommand();
    int cselgotoCommand();
    int cselbtnCommand();
    int clickCommand();
    int clCommand();
    int chvolCommand();
    int checkpageCommand();
    int checkkeyCommand();
    int cellCommand();
    int captionCommand();
    int btnwait2Command();
    int btnwaitCommand();
    int btntime2Command();
    int btntimeCommand();
    int btndownCommand();
    int btndefCommand();
    int btnCommand();
    int bspCommand();
    int brCommand();
    int bltCommand();
    int bgcopyCommand();
    int bgCommand();
    int bdownCommand();
    int barclearCommand();
    int barCommand();
    int aviCommand();
    int automode_timeCommand();
    int autoclickCommand();
    int allsp2resumeCommand();
    int allspresumeCommand();
    int allsp2hideCommand();
    int allsphideCommand();
    int amspCommand();

    void NSDCallCommand(int texnum, const char *str1, int proc, const char *str2);
    void NSDDeleteCommand(int texnum);
    void NSDLoadCommand(int texnum, const char *str);
    void NSDPresentRectCommand(int x1, int y1, int x2, int y2);
    void NSDSp2Command(int texnum, int dcx, int dcy, int sx, int sy, int w, int h,
                       int xs, int ys, int rot, int alpha);
    void NSDSetSpriteCommand(int spnum, int texnum, const char *tag);

    void stopSMPEG();
    void updateEffectDst();
    
private:
    // ----------------------------------------
    // global variables and methods
    enum { IDLE_EVENT_MODE  = 0,
           WAIT_RCLICK_MODE = 1, // for lrclick
           WAIT_BUTTON_MODE = 2, // For select, btnwait and rmenu.
           WAIT_INPUT_MODE  = (4|8),  // can be skipped by a click
           WAIT_TIMER_MODE  = 32,
           WAIT_VOICE_MODE  = 128,
           WAIT_TEXT_MODE   = 256 // clickwait, newpage, select
    };
    int  event_mode;

    bool is_script_read;
    char *wm_title_string;
    char *wm_icon_string;
    char wm_edit_string[256];
    bool fullscreen_mode;
    bool window_mode;

    // start-up options
    bool cdaudio_flag;
    char *default_font;
    char *registry_file;
    char *dll_file;
    char *getret_str;
    int  getret_int;
    bool enable_wheeldown_advance_flag;
    bool disable_rescale_flag;
    bool edit_flag;
    char *key_exe_file;
    bool vsync;
    bool cacheFont;
    bool screen_dirty_flag;

    // variables relevant to button
    ButtonState current_button_state, last_mouse_state;

    ButtonLink root_button_link, *current_button_link, exbtn_d_button_link;
    bool is_exbtn_enabled;

    bool btntime2_flag;
    long btntime_value;
    long internal_button_timer;
    long btnwait_time;
    bool btndown_flag;
    bool transbtn_flag;

    int current_over_button;
    int shift_over_button;

    bool bexec_flag;
    bool getzxc_flag;
    bool gettab_flag;
    bool getpageup_flag;
    bool getpagedown_flag;
    bool getmclick_flag;
    bool getinsert_flag;
    bool getfunction_flag;
    bool getenter_flag;
    bool getcursor_flag;
    bool spclclk_flag;

    bool getmouseover_flag;
    int  getmouseover_lower;
    int  getmouseover_upper;

    // variables relevant to selection
    enum { SELECT_GOTO_MODE  = 0, 
           SELECT_GOSUB_MODE = 1, 
           SELECT_NUM_MODE   = 2, 
           SELECT_CSEL_MODE  = 3 
    };
    struct SelectLink{
        SelectLink *next;
        char *text;
        char *label;

        SelectLink(){
            next = NULL;
            text = label = NULL;
        };

        ~SelectLink(){
            if ( text )  delete[] text;
            if ( label ) delete[] label;
        };
    } root_select_link;
    NestInfo select_label_info;
    int  shortcut_mouse_line;

    void initSDL();
    void calcRenderRect();
    void openAudio(int freq=-1);
    void reset(); // called on definereset
    void resetSub(); // called on reset
    void resetSentenceFont();
    void flush( int refresh_mode, SDL_Rect *rect=NULL, bool clear_dirty_flag=true, bool direct_flag=false );
    void flushDirect( SDL_Rect &rect, int refresh_mode );
    #ifdef USE_SMPEG
    void flushDirectYUV(SDL_Overlay *overlay);
    #endif
    void mouseOverCheck( int x, int y );
    void warpMouse(int x, int y);
    void setFullScreen(bool fullscreen);
public:
    void executeLabel();
    void runScript();
    AnimationInfo *getSpriteInfo(int no){ return &sprite_info[no]; };
    AnimationInfo *getSprite2Info(int no){ return &sprite2_info[no]; };
    Uint32 getTextureFormat() { return texture_format; };
private:
    int  parseLine();
    void deleteButtonLink();
    void refreshMouseOverButton();
    void deleteSelectLink();
    void clearCurrentPage();
    void shadowTextDisplay( SDL_Surface *surface, SDL_Rect &clip );
    void newPage();
    ButtonLink *getSelectableSentence( char *buffer, FontInfo *info, bool flush_flag = true, bool nofile_flag = false );
    void decodeExbtnControl( const char *ctl_str, SDL_Rect *check_src_rect=NULL, SDL_Rect *check_dst_rect=NULL );
    void saveAll();
    void loadEnvData();
    void saveEnvData();
    int  refreshMode();
    void quit();
    void disableGetButtonFlag();
    int  getNumberFromBuffer( const char **buf );

    // ----------------------------------------
    // variables and methods relevant to animation
    enum { ALPHA_BLEND_CONST          = 1,
           ALPHA_BLEND_MULTIPLE       = 2,
           ALPHA_BLEND_FADE_MASK      = 3,
           ALPHA_BLEND_CROSSFADE_MASK = 4
    };

    AnimationInfo btndef_info, bg_info, cursor_info[2];
    AnimationInfo tachi_info[3]; // 0 ... left, 1 ... center, 2 ... right
    AnimationInfo *sprite_info, *sprite2_info;
    AnimationInfo *texture_info;
    AnimationInfo *bar_info[MAX_PARAM_NUM], *prnum_info[MAX_PARAM_NUM];
    AnimationInfo lookback_info[4];
    AnimationInfo dialog_info;

    int  human_order[3];
    bool all_sprite_hide_flag;
    bool all_sprite2_hide_flag;
    bool show_dialog_flag;
    
    int  calcDurationToNextAnimation();
    void proceedAnimation(int current_time);
    void setupAnimationInfo(AnimationInfo *anim, FontInfo *info=NULL);
    void parseTaggedString(AnimationInfo *anim );
    void drawTaggedSurface(SDL_Surface *dst_surface, AnimationInfo *anim, SDL_Rect &clip);
    void stopAnimation(int click);
    void loadCursor(int no, const char *str, int x, int y, bool abs_flag = false);

    // ----------------------------------------
    // variables and methods relevant to effect
    DirtyRect dirty_rect; // only this region is updated
    int  effect_counter, effect_duration; // counter in each effect
    int  effect_timer_resolution;
    int  effect_start_time;
    int  effect_start_time_old;
    volatile bool update_effect_dst;

    void generateEffectDst(int effect_no);
    bool setEffect( EffectLink *effect );
    bool doEffect( EffectLink *effect, bool clear_dirty_region=true );
    void drawEffect( SDL_Rect *dst_rect, SDL_Rect *src_rect, SDL_Surface *surface );
    void generateMosaic( SDL_Surface *src_surface, int level );
    
    struct BreakupCell {
        int cell_x, cell_y;
        int dir;
        int state;
        int radius;
        BreakupCell(): cell_x(0), cell_y(0),
                       dir(0), state(0), radius(0){}
    } *breakup_cells;
    bool *breakup_cellforms, *breakup_mask;
    void buildBreakupCellforms();
    void buildBreakupMask();
    void initBreakup( char *params );
    void effectBreakup( char *params, int duration );

#ifdef USE_BUILTIN_EFFECTS
    //cascade
    void effectCascade(char *params, int duration);

    //trig
    int *sin_table, *cos_table;
    void buildSinTable();
    void buildCosTable();
    void effectTrvswave(char *params, int duration);
    int *whirl_table;
    void buildWhirlTable();
    void effectWhirl(char *params, int duration);
#endif

    // ----------------------------------------
    // variables and methods relevant to event
    enum { NOT_EDIT_MODE            = 0,
           EDIT_SELECT_MODE         = 1,
           EDIT_VARIABLE_INDEX_MODE = 2,
           EDIT_VARIABLE_NUM_MODE   = 3,
           EDIT_MP3_VOLUME_MODE     = 4,
           EDIT_VOICE_VOLUME_MODE   = 5,
           EDIT_SE_VOLUME_MODE      = 6
    };
    
    int  next_time;
    int  variable_edit_mode;
    int  variable_edit_index;
    int  variable_edit_num;
    int  variable_edit_sign;
    int  shift_pressed_status;
    int  ctrl_pressed_status;
    int  num_fingers; // numbur of fingers touching on the screen
    
    void flushEventSub( SDL_Event &event );
    void flushEvent();
    void removeEvent(int type);
    void removeBGMFadeEvent();
public:
    void waitEventSub(int count);
private:
    bool waitEvent(int count);
    bool trapHandler();
    bool mouseMoveEvent( SDL_MouseMotionEvent *event );
    bool mousePressEvent( SDL_MouseButtonEvent *event );

    bool mouseWheelEvent(SDL_MouseWheelEvent *event);

    void variableEditMode( SDL_KeyboardEvent *event );
    void shiftCursorOnButton( int diff );
    bool keyDownEvent( SDL_KeyboardEvent *event );
    void keyUpEvent( SDL_KeyboardEvent *event );
    bool keyPressEvent( SDL_KeyboardEvent *event );
    void timerEvent(bool init_flag);
#if (defined(IOS) || defined(ANDROID) || defined(WINRT))
    bool convTouchKey(SDL_TouchFingerEvent &finger);
#endif
    void runEventLoop();

    // ----------------------------------------
    // variables and methods relevant to file/file2
    void searchSaveFile( SaveFileInfo &info, int no );
    char *readSaveStrFromFile( int no );
    int  loadSaveFile( int no );
    void saveMagicNumber( bool output_flag );
    void storeSaveFile();
    int  writeSaveFile( int no=0, const char *savestr=NULL );

    int  loadSaveFile2( int file_version );
    void saveSaveFile2( bool output_flag );
    
    // ----------------------------------------
    // variables and methods relevant to image
    bool monocro_flag;
    uchar3 monocro_color;
    uchar3 monocro_color_lut[256];
    int  nega_mode;

    enum { REFRESH_NONE_MODE        = 0,
           REFRESH_NORMAL_MODE      = 1,
           REFRESH_SAYA_MODE        = 2,
           REFRESH_SHADOW_MODE      = 4,
           REFRESH_TEXT_MODE        = 8,
           REFRESH_CURSOR_MODE      = 16
    };
    int  refresh_shadow_text_mode;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    void setCaption(const char *title, const char *iconstr = NULL);
    void setScreenDirty(bool screen_dirty);
    // format = SDL_PIXELFORMAT_ABGR8888 for OpenGL ES 1.x, OpenGL ES 2.x (Android, iOS)
    // format = SDL_PIXELFORMAT_ARGB8888 for OpenGL, Direct3D (Windows, Linux, MacOSX) or for any 32bit surface without SDL_Renderer
    // format = SDL_PIXELFORMAT_RGB565 for any 16bit surface without SDL_Renderer (Android, Zaurus)
    Uint32 texture_format;
    SDL_Surface *accumulation_surface; // Final image, i.e. picture_surface (+ shadow + text_surface)
    SDL_Surface *backup_surface; // Final image w/o (shadow + text_surface) used in leaveTextDisplayMode()
    SDL_Surface *screen_surface; // Text + Select_image + Tachi image + background
    SDL_Surface *effect_dst_surface; // Intermediate source buffer for effect
    SDL_Surface *effect_src_surface; // Intermediate destnation buffer for effect
    SDL_Surface *effect_tmp_surface; // Intermediate buffer for effect
    SDL_Surface *screenshot_surface; // Screenshot
    int screenshot_w, screenshot_h;
    SDL_Surface *image_surface; // Reference for loadImage()
    int max_texture_width, max_texture_height;
    SDL_Texture *blt_texture;
    SDL_Rect blt_texture_src_rect;
    GlesRenderer *gles_renderer;

    unsigned char *tmp_image_buf;
    unsigned long tmp_image_buf_length;
    unsigned long mean_size_of_loaded_images;
    unsigned long num_loaded_images;

    unsigned char *resize_buffer;
    size_t resize_buffer_size;

    SDL_Surface *loadImage(char *filename, bool *has_alpha=NULL, int *location=NULL, unsigned char *alpha=NULL);
    SDL_Surface *createRectangleSurface(char *filename, bool *has_alpha, unsigned char *alpha=NULL);
    SDL_Surface *createSurfaceFromFile(char *filename,bool *has_alpha, int *location);

    int  resizeSurface( SDL_Surface *src, SDL_Surface *dst );
    void alphaBlend(SDL_Surface *mask_surface, int trans_mode, Uint32 mask_value = 255, SDL_Rect *clip = NULL,
        SDL_Surface *src1 = NULL, SDL_Surface *src2 = NULL, SDL_Surface *dst = NULL);
    void alphaBlendText( SDL_Surface *dst_surface, SDL_Rect dst_rect,
                         SDL_Surface *src_surface, SDL_Color &color, SDL_Rect *clip, bool rotate_flag );
    void makeNegaSurface( SDL_Surface *surface, SDL_Rect &clip );
    void makeMonochromeSurface( SDL_Surface *surface, SDL_Rect &clip );
    void refreshSurface( SDL_Surface *surface, SDL_Rect *clip_src, int refresh_mode = REFRESH_NORMAL_MODE );
    void refreshSprite( int sprite_no, bool active_flag, int cell_no, SDL_Rect *check_src_rect, SDL_Rect *check_dst_rect );
    void createBackground();

    // ----------------------------------------
    // variables and methods relevant to rmenu
    bool system_menu_enter_flag;
    int  system_menu_mode;

    int  shelter_event_mode;
    int  shelter_display_mode;
    bool shelter_draw_cursor_flag;
    Page *cached_page;
    ButtonLink *shelter_button_link;
    SelectLink *shelter_select_link;
    ButtonState shelter_mouse_state;
    
    void enterSystemCall();
    void leaveSystemCall( bool restore_flag = true );
    int  executeSystemCall();
    
    void executeSystemMenu();
    void executeSystemSkip();
    void executeSystemAutomode();
    bool executeSystemReset();
    void executeSystemEnd();
    void executeWindowErase();
    bool executeSystemLoad();
    void executeSystemSave();
    bool executeSystemYesNo( int caller, int file_no=0 );
    void setupLookbackButton();
    void executeSystemLookback();
    void buildDialog(bool yesno_flag, const char *mes1, const char *mes2);

    // ----------------------------------------
    // variables and methods relevant to sound
    enum{
        SOUND_NONE    =  0,
        SOUND_PRELOAD =  1,
        SOUND_CHUNK   =  2, // WAV, Ogg Vorbis
        SOUND_MUSIC   =  4, // WAV, MP3, Ogg Vorbis (streaming)
        SOUND_MIDI    =  8,
        SOUND_OTHER   = 16
    };
    int  cdrom_drive_number;
    char *default_cdrom_drive;
    bool cdaudio_on_flag; // false if mute
    bool volume_on_flag; // false if mute
    SDL_AudioSpec audio_format;
    bool audio_open_flag;
    
    bool wave_play_loop_flag;
    char *wave_file_name;
    
    bool midi_play_loop_flag;
    char *midi_file_name;
    Mix_Music *midi_info;

#ifdef USE_CDROM    
    SDL_CD *cdrom_info;
#endif
    int current_cd_track;
    bool cd_play_loop_flag;
    bool music_play_loop_flag;
    double music_loopback_offset;
    bool mp3save_flag;
    char *music_file_name;
    unsigned char *music_buffer; // for looped music
    long music_buffer_length;
    Uint32 mp3fade_start;
    Uint32 mp3fadeout_duration;
    Uint32 mp3fadein_duration;
    Uint32 mp3fadeout_duration_internal;
    Uint32 mp3fadein_duration_internal;
    char *fadeout_music_file_name;
    Mix_Music *music_info;
    char *loop_bgm_name[2];
    
    Mix_Chunk *wave_sample[ONS_MIX_CHANNELS+ONS_MIX_EXTRA_CHANNELS];

    char *midi_cmd;

    unsigned char *layer_smpeg_buffer;
    bool layer_smpeg_loop_flag;
    AnimationInfo *smpeg_info;
#if defined(USE_SMPEG)
    SMPEG* layer_smpeg_sample;
    SMPEG_Filter layer_smpeg_filter;
#endif
    
    int playSound(const char *filename, int format, bool loop_flag, int channel=0);
    void playCDAudio();
    int playWave(Mix_Chunk *chunk, int format, bool loop_flag, int channel);
    int playMIDI(bool loop_flag);
    
    int playMPEG(const char *filename, bool click_flag, bool loop_flag=false);
    int playAVI( const char *filename, bool click_flag );
    enum { WAVE_PLAY        = 0,
           WAVE_PRELOAD     = 1,
           WAVE_PLAY_LOADED = 2
    };
    void stopBGM( bool continue_flag );
    void stopAllDWAVE();
    void playClickVoice();
    
    // ----------------------------------------
    // variables and methods relevant to text
    enum { DISPLAY_MODE_NORMAL  = 0, 
           DISPLAY_MODE_TEXT    = 1
    };
    int  display_mode;

    enum { SKIP_NONE   = 0,
           SKIP_NORMAL = 1, // skip endlessly (press 's' button)
           SKIP_TO_EOL = 2, // skip to end of line
           SKIP_TO_EOP = 4  // skip to end of page (press 'o' button)
    };
    int  skip_mode;

    int effect_tmp; //tmp variable for use by effect routines

    enum { TRAP_NONE        = 0,
           TRAP_LEFT_CLICK  = 1,
           TRAP_RIGHT_CLICK = 2,
           TRAP_NEXT_SELECT = 4,
           TRAP_STOP        = 8,
           TRAP_CLICKED     = 16
    };
    int  trap_mode;
    char *trap_dist;

    long internal_timer;
    bool automode_flag;
    long automode_time;
    long autoclick_time;

    bool saveon_flag;
    bool internal_saveon_flag; // to saveoff at the head of text

    bool new_line_skip_flag;
    int  text_speed_no;
    bool is_kinsoku;
    AnimationInfo text_info;
    AnimationInfo sentence_font_info;
    char *font_file;
    void *font_cache = NULL;
    int erase_text_window_mode;
    bool text_on_flag; // suppress the effect of erase_text_window_mode
    bool draw_cursor_flag;
    int  indent_offset;

    int force_window_width, force_window_height;
    float sharpness = NAN;

    void setwindowCore();
    
    void shiftHalfPixelX(SDL_Surface *surface);
    void shiftHalfPixelY(SDL_Surface *surface);
    void drawGlyph( SDL_Surface *dst_surface, FontInfo *info, SDL_Color &color, char *text, int xy[2], AnimationInfo *cache_info, SDL_Rect *clip, SDL_Rect &dst_rect );
    void drawChar( char* text, FontInfo *info, bool flush_flag, bool lookback_flag, SDL_Surface *surface, AnimationInfo *cache_info, SDL_Rect *clip=NULL );
    void drawString( const char *str, uchar3 color, FontInfo *info, bool flush_flag, SDL_Surface *surface, SDL_Rect *rect = NULL, AnimationInfo *cache_info=NULL, bool pack_hankaku=true );
    void restoreTextBuffer(SDL_Surface *surface = NULL);
    void enterTextDisplayMode(bool text_flag = true);
    void leaveTextDisplayMode(bool force_leave_flag = false);
    bool doClickEnd();
    bool clickWait( char *out_text );
    bool clickNewPage( char *out_text );
    void startRuby(const char *buf, FontInfo &info);
    void endRuby(bool flush_flag, bool lookback_flag, SDL_Surface *surface, AnimationInfo *cache_info);
    int  textCommand();
    bool checkLineBreak(const char *buf, FontInfo *fi);
    void processEOT();
    bool processText();
};

#endif // __ONSCRIPTER_H__
