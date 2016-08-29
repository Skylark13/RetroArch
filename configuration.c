/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2016 - Daniel De Matteis
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>

#include <file/config_file.h>
#include <file/file_path.h>
#include <compat/strl.h>
#include <compat/posix_string.h>
#include <retro_stat.h>
#include <retro_assert.h>
#include <string/stdstring.h>

#include "file_path_special.h"
#include "audio/audio_driver.h"
#include "configuration.h"
#include "content.h"
#include "config.def.h"
#include "input/input_config.h"
#include "input/input_keymaps.h"
#include "input/input_remapping.h"
#include "defaults.h"
#include "general.h"
#include "core.h"
#include "retroarch.h"
#include "system.h"
#include "verbosity.h"
#include "lakka.h"

#include "tasks/tasks_internal.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define SETTING_PATH(key, defval, configval) \
{ \
   if (count == 0) \
      tmp = (struct config_path_setting_ptr*)malloc(sizeof(struct config_path_setting_ptr) * (count + 1)); \
   else \
      tmp = (struct config_path_setting_ptr*)realloc(tmp, sizeof(struct config_path_setting_ptr) * (count + 1)); \
   tmp[count].ident    = key; \
   tmp[count].defaults = defval; \
   tmp[count].value    = configval; \
   count++; \
} \

#define SETTING_BOOL(key, configval) \
{ \
   if (count == 0) \
      tmp = (struct config_bool_setting_ptr*)malloc(sizeof(struct config_bool_setting_ptr) * (count + 1)); \
   else \
      tmp = (struct config_bool_setting_ptr*)realloc(tmp, sizeof(struct config_bool_setting_ptr) * (count + 1)); \
   tmp[count].ident    = key; \
   tmp[count].ptr      = configval; \
   count++; \
} 

#define SETTING_FLOAT(key, configval) \
{ \
   if (count == 0) \
      tmp = (struct config_float_setting_ptr*)malloc(sizeof(struct config_float_setting_ptr) * (count + 1)); \
   else \
      tmp = (struct config_float_setting_ptr*)realloc(tmp, sizeof(struct config_float_setting_ptr) * (count + 1)); \
   tmp[count].ident    = key; \
   tmp[count].ptr      = configval; \
   count++; \
}

#define SETTING_INT(key, configval) \
{ \
   if (count == 0) \
      tmp = (struct config_int_setting_ptr*)malloc(sizeof(struct config_int_setting_ptr) * (count + 1)); \
   else \
      tmp = (struct config_int_setting_ptr*)realloc(tmp, sizeof(struct config_int_setting_ptr) * (count + 1)); \
   tmp[count].ident    = key; \
   tmp[count].ptr      = configval; \
   count++; \
}

#define SETTING_STRING(key, configval) \
{ \
   if (count == 0) \
      tmp = (struct config_string_setting_ptr*)malloc(sizeof(struct config_string_setting_ptr) * (count + 1)); \
   else \
      tmp = (struct config_string_setting_ptr*)realloc(tmp, sizeof(struct config_string_setting_ptr) * (count + 1)); \
   tmp[count].ident    = key; \
   tmp[count].value    = configval; \
   count++; \
} \

struct defaults g_defaults;
static settings_t *configuration_settings = NULL;

settings_t *config_get_ptr(void)
{
   return configuration_settings;
}

void config_free(void)
{
   free(configuration_settings);
   configuration_settings = NULL;
}

bool config_init(void)
{
   configuration_settings = (settings_t*)calloc(1, sizeof(settings_t));

   if (!configuration_settings)
      return false;
   return true;
}

/**
 * config_get_default_audio:
 *
 * Gets default audio driver.
 *
 * Returns: Default audio driver.
 **/
const char *config_get_default_audio(void)
{
   switch (AUDIO_DEFAULT_DRIVER)
   {
      case AUDIO_RSOUND:
         return "rsound";
      case AUDIO_OSS:
         return "oss";
      case AUDIO_ALSA:
         return "alsa";
      case AUDIO_ALSATHREAD:
         return "alsathread";
      case AUDIO_ROAR:
         return "roar";
      case AUDIO_COREAUDIO:
         return "coreaudio";
      case AUDIO_AL:
         return "openal";
      case AUDIO_SL:
         return "opensl";
      case AUDIO_SDL:
         return "sdl";
      case AUDIO_SDL2:
         return "sdl2";
      case AUDIO_DSOUND:
         return "dsound";
      case AUDIO_XAUDIO:
         return "xaudio";
      case AUDIO_PULSE:
         return "pulse";
      case AUDIO_EXT:
         return "ext";
      case AUDIO_XENON360:
         return "xenon360";
      case AUDIO_PS3:
         return "ps3";
      case AUDIO_WII:
         return "gx";
      case AUDIO_PSP:
#ifdef VITA
         return "vita";
#else
         return "psp";
#endif
      case AUDIO_CTR:
         return "csnd";
      case AUDIO_RWEBAUDIO:
         return "rwebaudio";
      default:
         break;
   }

   return "null";
}

const char *config_get_default_record(void)
{
   switch (RECORD_DEFAULT_DRIVER)
   {
      case RECORD_FFMPEG:
         return "ffmpeg";
      default:
         break;
   }

   return "null";
}

/**
 * config_get_default_audio_resampler:
 *
 * Gets default audio resampler driver.
 *
 * Returns: Default audio resampler driver.
 **/
const char *config_get_default_audio_resampler(void)
{
   switch (AUDIO_DEFAULT_RESAMPLER_DRIVER)
   {
      case AUDIO_RESAMPLER_CC:
         return "cc";
      case AUDIO_RESAMPLER_SINC:
         return "sinc";
      case AUDIO_RESAMPLER_NEAREST:
         return "nearest";
      default:
         break;
   }

   return "null";
}

/**
 * config_get_default_video:
 *
 * Gets default video driver.
 *
 * Returns: Default video driver.
 **/
const char *config_get_default_video(void)
{
   switch (VIDEO_DEFAULT_DRIVER)
   {
      case VIDEO_GL:
         return "gl";
      case VIDEO_VULKAN:
         return "vulkan";
      case VIDEO_DRM:
         return "drm";
      case VIDEO_WII:
         return "gx";
      case VIDEO_XENON360:
         return "xenon360";
      case VIDEO_XDK_D3D:
      case VIDEO_D3D9:
         return "d3d";
      case VIDEO_PSP1:
         return "psp1";
      case VIDEO_VITA2D:
         return "vita2d";
      case VIDEO_CTR:
         return "ctr";
      case VIDEO_XVIDEO:
         return "xvideo";
      case VIDEO_SDL:
         return "sdl";
      case VIDEO_SDL2:
         return "sdl2";
      case VIDEO_EXT:
         return "ext";
      case VIDEO_VG:
         return "vg";
      case VIDEO_OMAP:
         return "omap";
      case VIDEO_EXYNOS:
         return "exynos";
      case VIDEO_DISPMANX:
         return "dispmanx";
      case VIDEO_SUNXI:
         return "sunxi";
      default:
         break;
   }

   return "null";
}

/**
 * config_get_default_input:
 *
 * Gets default input driver.
 *
 * Returns: Default input driver.
 **/
const char *config_get_default_input(void)
{
   switch (INPUT_DEFAULT_DRIVER)
   {
      case INPUT_ANDROID:
         return "android";
      case INPUT_PS3:
         return "ps3";
      case INPUT_PSP:
#ifdef VITA
         return "vita";
#else
         return "psp";
#endif
      case INPUT_CTR:
         return "ctr";
      case INPUT_SDL:
         return "sdl";
      case INPUT_SDL2:
         return "sdl2";
      case INPUT_DINPUT:
         return "dinput";
      case INPUT_X:
         return "x";
      case INPUT_WAYLAND:
         return "wayland";
      case INPUT_XENON360:
         return "xenon360";
      case INPUT_XINPUT:
         return "xinput";
      case INPUT_WII:
         return "gx";
      case INPUT_LINUXRAW:
         return "linuxraw";
      case INPUT_UDEV:
         return "udev";
      case INPUT_COCOA:
         return "cocoa";
      case INPUT_QNX:
      	 return "qnx_input";
      case INPUT_RWEBINPUT:
      	 return "rwebinput";
      default:
         break;
   }

   return "null";
}

/**
 * config_get_default_joypad:
 *
 * Gets default input joypad driver.
 *
 * Returns: Default input joypad driver.
 **/
const char *config_get_default_joypad(void)
{
   switch (JOYPAD_DEFAULT_DRIVER)
   {
      case JOYPAD_PS3:
         return "ps3";
      case JOYPAD_XINPUT:
         return "xinput";
      case JOYPAD_GX:
         return "gx";
      case JOYPAD_XDK:
         return "xdk";
      case JOYPAD_PSP:
#ifdef VITA
         return "vita";
#else
         return "psp";
#endif
      case JOYPAD_CTR:
         return "ctr";
      case JOYPAD_DINPUT:
         return "dinput";
      case JOYPAD_UDEV:
         return "udev";
      case JOYPAD_LINUXRAW:
         return "linuxraw";
      case JOYPAD_ANDROID:
         return "android";
      case JOYPAD_SDL:
#ifdef HAVE_SDL2
         return "sdl2";
#else
         return "sdl";
#endif
      case JOYPAD_HID:
         return "hid";
      case JOYPAD_QNX:
         return "qnx";
      default:
         break;
   }

   return "null";
}

#ifdef HAVE_MENU
/**
 * config_get_default_menu:
 *
 * Gets default menu driver.
 *
 * Returns: Default menu driver.
 **/
const char *config_get_default_menu(void)
{
   if (!string_is_empty(g_defaults.settings.menu))
      return g_defaults.settings.menu;

   switch (MENU_DEFAULT_DRIVER)
   {
      case MENU_RGUI:
         return "rgui";
      case MENU_XUI:
         return "xui";
      case MENU_MATERIALUI:
         return "glui";
      case MENU_XMB:
         return "xmb";
      case MENU_NUKLEAR:
         return "nuklear";
      default:
         break;
   }

   return "null";
}
#endif

/**
 * config_get_default_camera:
 *
 * Gets default camera driver.
 *
 * Returns: Default camera driver.
 **/
const char *config_get_default_camera(void)
{
   switch (CAMERA_DEFAULT_DRIVER)
   {
      case CAMERA_V4L2:
         return "video4linux2";
      case CAMERA_RWEBCAM:
         return "rwebcam";
      case CAMERA_ANDROID:
         return "android";
      case CAMERA_AVFOUNDATION:
         return "avfoundation";
      default:
         break;
   }

   return "null";
}

/**
 * config_get_default_location:
 *
 * Gets default location driver.
 *
 * Returns: Default location driver.
 **/
const char *config_get_default_location(void)
{
   switch (LOCATION_DEFAULT_DRIVER)
   {
      case LOCATION_ANDROID:
         return "android";
      case LOCATION_CORELOCATION:
         return "corelocation";
      default:
         break;
   }

   return "null";
}

bool config_overlay_enable_default(void)
{
   if (g_defaults.overlay.set)
      return g_defaults.overlay.enable;
   return true;
}

#ifdef HAVE_MENU
static unsigned config_menu_btn_ok_default(void)
{
   if (g_defaults.menu.controls.set)
      return g_defaults.menu.controls.menu_btn_ok;
   return default_menu_btn_ok;
}

static unsigned config_menu_btn_cancel_default(void)
{
   if (g_defaults.menu.controls.set)
      return g_defaults.menu.controls.menu_btn_cancel;
   return default_menu_btn_cancel;
}
#endif

static int populate_settings_string(settings_t *settings, struct config_string_setting_ptr **out)
{
   unsigned count                        = 0;
   struct config_string_setting_ptr *tmp = NULL;
#ifdef HAVE_NETPLAY
   global_t   *global                    = global_get_ptr();
#endif
   SETTING_STRING("bundle_assets_dst_path_subdir", settings->path.bundle_assets_dst_subdir);
   SETTING_STRING("video_filter",             settings->path.softfilter_plugin);
   SETTING_STRING("audio_dsp_plugin",         settings->path.audio_dsp_plugin);
   SETTING_STRING("playlist_names",           settings->playlist_names);
   SETTING_STRING("playlist_cores",           settings->playlist_cores);
   SETTING_STRING("video_driver",             settings->video.driver);
   SETTING_STRING("record_driver",            settings->record.driver);
   SETTING_STRING("camera_driver",            settings->camera.driver);
   SETTING_STRING("location_driver",          settings->location.driver);
#ifdef HAVE_MENU
   SETTING_STRING("menu_driver",              settings->menu.driver);
#endif
   SETTING_STRING("audio_device",             settings->audio.device);
   SETTING_STRING("core_updater_buildbot_url",settings->network.buildbot_url);
   SETTING_STRING("core_updater_buildbot_assets_url",settings->network.buildbot_assets_url);
   SETTING_STRING("camera_device",            settings->camera.device);
#ifdef HAVE_CHEEVOS
   SETTING_STRING("cheevos_username",         settings->cheevos.username);
   SETTING_STRING("cheevos_password",         settings->cheevos.password);
#endif
   SETTING_STRING("video_context_driver",     settings->video.context_driver);
   SETTING_STRING("audio_driver",             settings->audio.driver);
   SETTING_STRING("audio_resampler",          settings->audio.resampler);
#ifdef HAVE_NETPLAY
   SETTING_STRING("netplay_ip_address",       global->netplay.server);
#endif
   SETTING_STRING("netplay_nickname",         settings->username);
   SETTING_STRING("input_driver",             settings->input.driver);
   SETTING_STRING("input_joypad_driver",      settings->input.joypad_driver);
   SETTING_STRING("input_keyboard_layout",    settings->input.keyboard_layout);
   SETTING_STRING("bundle_assets_src_path",   settings->path.bundle_assets_src);
   SETTING_STRING("bundle_assets_dst_path",   settings->path.bundle_assets_dst);

   *out = 
      (struct config_string_setting_ptr*) malloc(count * sizeof(struct config_string_setting_ptr));
   memcpy(*out, tmp, sizeof(struct config_string_setting_ptr) * count);
   free(tmp);
   return count;
}

static int populate_settings_path(settings_t *settings, struct config_path_setting_ptr **out)
{
   unsigned count = 0;
   struct config_path_setting_ptr *tmp = NULL;
   global_t   *global                  = global_get_ptr();

   SETTING_PATH("recording_output_directory", false,
         global->record.output_dir);
   SETTING_PATH("recording_config_directory", false,
         global->record.config_dir);
   SETTING_PATH("libretro_directory", false,
         settings->directory.libretro);
   SETTING_PATH("core_options_path", false,
         settings->path.core_options);
   SETTING_PATH("libretro_info_path", false,
         settings->path.libretro_info);
   SETTING_PATH("video_shader", false,
         settings->path.shader);
   SETTING_PATH("content_database_path", false,
         settings->path.content_database);
   SETTING_PATH("cheat_database_path", false,
         settings->path.cheat_database);
#ifdef HAVE_MENU
   SETTING_PATH("menu_wallpaper", false,
         settings->path.menu_wallpaper);
#endif
   SETTING_PATH("content_history_path", false,
         settings->path.content_history);
   SETTING_PATH("content_music_history_path", false,
         settings->path.content_music_history);
   SETTING_PATH("content_video_history_path", false,
         settings->path.content_video_history);
   SETTING_PATH("content_image_history_path", false,
         settings->path.content_image_history);
#ifdef HAVE_OVERLAY
   SETTING_PATH("input_overlay", false,
         settings->path.overlay);
   SETTING_PATH("input_osk_overlay", false,
         settings->path.osk_overlay);
#endif
   SETTING_PATH("video_font_path", false,
         settings->path.font);
   SETTING_PATH("cursor_directory", false,
         settings->directory.cursor);
   SETTING_PATH("content_history_dir", false,
         settings->directory.content_history);
   SETTING_PATH("screenshot_directory", true,
         settings->directory.screenshot);
   SETTING_PATH("system_directory", true,
         settings->directory.system);
   SETTING_PATH("cache_directory", false,
         settings->directory.cache);
   SETTING_PATH("input_remapping_directory", false,
         settings->directory.input_remapping);
   SETTING_PATH("resampler_directory", false,
         settings->directory.resampler);
   SETTING_PATH("video_shader_dir", true,
         settings->directory.video_shader);
   SETTING_PATH("video_filter_dir", true,
         settings->directory.video_filter);
   SETTING_PATH("core_assets_directory", true,
         settings->directory.core_assets);
   SETTING_PATH("assets_directory", true,
         settings->directory.assets);
   SETTING_PATH("dynamic_wallpapers_directory", true,
         settings->directory.dynamic_wallpapers);
   SETTING_PATH("thumbnails_directory", true,
         settings->directory.thumbnails);
   SETTING_PATH("playlist_directory", true,
         settings->directory.playlist);
   SETTING_PATH("joypad_autoconfig_dir", false,
         settings->directory.autoconfig);
   SETTING_PATH("audio_filter_dir", true,
         settings->directory.audio_filter);
   SETTING_PATH("savefile_directory", true,
         global->dir.savefile);
   SETTING_PATH("savestate_directory", true,
         global->dir.savestate);
#ifdef HAVE_MENU
   SETTING_PATH("rgui_browser_directory", true,
         settings->directory.menu_content);
   SETTING_PATH("rgui_config_directory", true,
         settings->directory.menu_config);
#endif
#ifdef HAVE_OVERLAY
   SETTING_PATH("overlay_directory", true,
         settings->directory.overlay);
#endif
#ifdef HAVE_OVERLAY
   SETTING_PATH("osk_overlay_directory", true,
         global->dir.osk_overlay);
#endif
#ifndef HAVE_DYNAMIC
   SETTING_PATH("libretro_path", false,
         config_get_active_core_path());
#endif
   SETTING_PATH(
         "screenshot_directory", true,
         settings->directory.screenshot);

   *out = 
      (struct config_path_setting_ptr*) malloc(count * sizeof(struct config_path_setting_ptr));
   memcpy(*out, tmp, sizeof(struct config_path_setting_ptr) * count);
   free(tmp);
   return count;
}

static int populate_settings_bool(settings_t *settings, struct config_bool_setting_ptr **out)
{
   unsigned count                      = 0;
   global_t   *global                  = global_get_ptr();
   struct config_bool_setting_ptr *tmp = NULL;

   SETTING_BOOL("ui_companion_start_on_boot",    &settings->ui.companion_start_on_boot);
   SETTING_BOOL("ui_companion_enable",           &settings->ui.companion_enable);
   SETTING_BOOL("video_gpu_record",              &settings->video.gpu_record);
   SETTING_BOOL("input_remap_binds_enable",      &settings->input.remap_binds_enable);
   SETTING_BOOL("back_as_menu_toggle_enable",    &settings->input.back_as_menu_toggle_enable);
   SETTING_BOOL("netplay_client_swap_input",     &settings->input.netplay_client_swap_input);
   SETTING_BOOL("input_descriptor_label_show",   &settings->input.input_descriptor_label_show);
   SETTING_BOOL("input_descriptor_hide_unbound", &settings->input.input_descriptor_hide_unbound);
   SETTING_BOOL("load_dummy_on_core_shutdown",   &settings->load_dummy_on_core_shutdown);
   SETTING_BOOL("builtin_mediaplayer_enable",    &settings->multimedia.builtin_mediaplayer_enable);
   SETTING_BOOL("builtin_imageviewer_enable",    &settings->multimedia.builtin_imageviewer_enable);
   SETTING_BOOL("fps_show",                      &settings->fps_show);
   SETTING_BOOL("ui_menubar_enable",             &settings->ui.menubar_enable);
   SETTING_BOOL("suspend_screensaver_enable",    &settings->ui.suspend_screensaver_enable);
   SETTING_BOOL("rewind_enable",                 &settings->rewind_enable);
   SETTING_BOOL("audio_sync",                    &settings->audio.sync);
   SETTING_BOOL("video_shader_enable",           &settings->video.shader_enable);
   SETTING_BOOL("video_aspect_ratio_auto",       &settings->video.aspect_ratio_auto);
   SETTING_BOOL("video_allow_rotate",            &settings->video.allow_rotate);
   SETTING_BOOL("video_windowed_fullscreen",     &settings->video.windowed_fullscreen);
   SETTING_BOOL("video_crop_overscan",           &settings->video.crop_overscan);
   SETTING_BOOL( "video_scale_integer",          &settings->video.scale_integer);
   SETTING_BOOL("video_smooth",                  &settings->video.smooth);
   SETTING_BOOL("video_force_aspect",            &settings->video.force_aspect);
   SETTING_BOOL("video_threaded",                &settings->video.threaded);
   SETTING_BOOL("video_shared_context",          &settings->video.shared_context);
   SETTING_BOOL("custom_bgm_enable",             &global->console.sound.system_bgm_enable);
   SETTING_BOOL("auto_screenshot_filename",      &settings->auto_screenshot_filename);
   SETTING_BOOL("video_force_srgb_disable",      &settings->video.force_srgb_disable);
   SETTING_BOOL("video_fullscreen",              &settings->video.fullscreen);
   SETTING_BOOL("bundle_assets_extract_enable",  &settings->bundle_assets_extract_enable);
   SETTING_BOOL("video_vsync",                   &settings->video.vsync);
   SETTING_BOOL("video_hard_sync",               &settings->video.hard_sync);
   SETTING_BOOL("video_black_frame_insertion",   &settings->video.black_frame_insertion);
   SETTING_BOOL("video_disable_composition",     &settings->video.disable_composition);
   SETTING_BOOL("pause_nonactive",               &settings->pause_nonactive);
   SETTING_BOOL("debug_panel_enable",            &settings->debug_panel_enable);
   SETTING_BOOL("video_gpu_screenshot",          &settings->video.gpu_screenshot);
   SETTING_BOOL("video_post_filter_record",      &settings->video.post_filter_record);
   SETTING_BOOL("keyboard_gamepad_enable",       &settings->input.keyboard_gamepad_enable);
   SETTING_BOOL("core_set_supports_no_game_enable", &settings->set_supports_no_game_enable);
   SETTING_BOOL("audio_enable",                  &settings->audio.enable);
   SETTING_BOOL("audio_mute_enable",             &settings->audio.mute_enable);
   SETTING_BOOL("location_allow",                &settings->location.allow);
   SETTING_BOOL("video_font_enable",             &settings->video.font_enable);
   SETTING_BOOL("core_updater_auto_extract_archive", &settings->network.buildbot_auto_extract_archive);
   SETTING_BOOL("camera_allow",                  &settings->camera.allow);
#if TARGET_OS_IPHONE
   SETTING_BOOL("small_keyboard_enable",         &settings->input.small_keyboard_enable);
#endif
#ifdef GEKKO
   SETTING_BOOL("video_vfilter",                 &settings->video.vfilter);
#endif
#ifdef HAVE_MENU
#ifdef HAVE_THREADS
   SETTING_BOOL("threaded_data_runloop_enable",  &settings->threaded_data_runloop_enable);
#endif
   SETTING_BOOL("menu_throttle_framerate",       &settings->menu.throttle_framerate);
   SETTING_BOOL("menu_linear_filter",            &settings->menu.linear_filter);
   SETTING_BOOL("dpi_override_enable",           &settings->menu.dpi.override_enable);
   SETTING_BOOL("menu_pause_libretro",           &settings->menu.pause_libretro);
   SETTING_BOOL("menu_mouse_enable",             &settings->menu.mouse.enable);
   SETTING_BOOL("menu_pointer_enable",           &settings->menu.pointer.enable);
   SETTING_BOOL("menu_timedate_enable",          &settings->menu.timedate_enable);
   SETTING_BOOL("menu_core_enable",              &settings->menu.core_enable);
   SETTING_BOOL("menu_dynamic_wallpaper_enable", &settings->menu.dynamic_wallpaper_enable);
#ifdef HAVE_XMB
   SETTING_BOOL("xmb_shadows_enable",            &settings->menu.xmb.shadows_enable);
   SETTING_BOOL("xmb_show_settings",             &settings->menu.xmb.show_settings);
#ifdef HAVE_IMAGEVIEWER
   SETTING_BOOL("xmb_show_images",               &settings->menu.xmb.show_images);
#endif
#ifdef HAVE_FFMPEG
   SETTING_BOOL("xmb_show_music",                &settings->menu.xmb.show_music);
   SETTING_BOOL("xmb_show_video",                &settings->menu.xmb.show_video);
#endif
   SETTING_BOOL("xmb_show_history",              &settings->menu.xmb.show_history);
#endif
   SETTING_BOOL("rgui_show_start_screen",        &settings->menu_show_start_screen);
   SETTING_BOOL("menu_navigation_wraparound_enable", &settings->menu.navigation.wraparound.enable);
   SETTING_BOOL("menu_navigation_browser_filter_supported_extensions_enable", 
         &settings->menu.navigation.browser.filter.supported_extensions_enable);
   SETTING_BOOL("menu_show_advanced_settings",  &settings->menu.show_advanced_settings);
#endif
#ifdef HAVE_CHEEVOS
   SETTING_BOOL("cheevos_enable",               &settings->cheevos.enable);
   SETTING_BOOL("cheevos_test_unofficial",      &settings->cheevos.test_unofficial);
   SETTING_BOOL("cheevos_hardcore_mode_enable", &settings->cheevos.hardcore_mode_enable);
#endif
#ifdef HAVE_OVERLAY
   SETTING_BOOL("input_overlay_enable",         &settings->input.overlay_enable);
   SETTING_BOOL("input_overlay_enable_autopreferred", &settings->input.overlay_enable_autopreferred);
   SETTING_BOOL("input_overlay_hide_in_menu",   &settings->input.overlay_hide_in_menu);
   SETTING_BOOL("input_osk_overlay_enable",     &settings->osk.enable);
#endif
#ifdef HAVE_COMMAND
   SETTING_BOOL("network_cmd_enable",           &settings->network_cmd_enable);
   SETTING_BOOL("stdin_cmd_enable",             &settings->stdin_cmd_enable);
#endif
#ifdef HAVE_NETWORKGAMEPAD
   SETTING_BOOL("network_remote_enable",        &settings->network_remote_enable);
#endif
#ifdef HAVE_NETPLAY
   SETTING_BOOL("netplay_spectator_mode_enable",&global->netplay.is_spectate);
   SETTING_BOOL("netplay_mode",                 &global->netplay.is_client);
#endif
   SETTING_BOOL("block_sram_overwrite",         &settings->block_sram_overwrite);
   SETTING_BOOL("savestate_auto_index",         &settings->savestate_auto_index);
   SETTING_BOOL("savestate_auto_save",          &settings->savestate_auto_save);
   SETTING_BOOL("savestate_auto_load",          &settings->savestate_auto_load);
   SETTING_BOOL("history_list_enable",          &settings->history_list_enable);
   SETTING_BOOL("game_specific_options",        &settings->game_specific_options);
   SETTING_BOOL("auto_overrides_enable",        &settings->auto_overrides_enable);
   SETTING_BOOL("auto_remaps_enable",           &settings->auto_remaps_enable);
   SETTING_BOOL("auto_shaders_enable",          &settings->auto_shaders_enable);
   SETTING_BOOL("sort_savefiles_enable",        &settings->sort_savefiles_enable);
   SETTING_BOOL("sort_savestates_enable",       &settings->sort_savestates_enable);
   SETTING_BOOL("config_save_on_exit",          &settings->config_save_on_exit);
   SETTING_BOOL("show_hidden_files",            &settings->show_hidden_files);
   SETTING_BOOL("input_autodetect_enable",      &settings->input.autodetect_enable);
   SETTING_BOOL("audio_rate_control",           &settings->audio.rate_control);

   *out = 
      (struct config_bool_setting_ptr*) malloc(count *sizeof(struct config_bool_setting_ptr));
   memcpy(*out, tmp, sizeof(struct config_bool_setting_ptr) * count);
   free(tmp);
   return count;
}

static int populate_settings_float(settings_t *settings, struct config_float_setting_ptr **out)
{
   unsigned count = 0;
   struct config_float_setting_ptr *tmp = NULL;

   SETTING_FLOAT("video_aspect_ratio",       &settings->video.aspect_ratio);
   SETTING_FLOAT("video_scale",              &settings->video.scale);
   SETTING_FLOAT("video_refresh_rate",       &settings->video.refresh_rate);
   SETTING_FLOAT("audio_rate_control_delta", &settings->audio.rate_control_delta);
   SETTING_FLOAT("audio_max_timing_skew",    &settings->audio.max_timing_skew);
   SETTING_FLOAT("audio_volume",             &settings->audio.volume);
#ifdef HAVE_OVERLAY
   SETTING_FLOAT("input_overlay_opacity",    &settings->input.overlay_opacity);
   SETTING_FLOAT("input_overlay_scale",      &settings->input.overlay_scale);
#endif
#ifdef HAVE_MENU
   SETTING_FLOAT("menu_wallpaper_opacity",   &settings->menu.wallpaper.opacity);
   SETTING_FLOAT("menu_footer_opacity",      &settings->menu.footer.opacity);
   SETTING_FLOAT("menu_header_opacity",      &settings->menu.header.opacity);
#endif
   SETTING_FLOAT("video_message_pos_x",      &settings->video.msg_pos_x);
   SETTING_FLOAT("video_message_pos_y",      &settings->video.msg_pos_y);
   SETTING_FLOAT("video_font_size",          &settings->video.font_size);
   SETTING_FLOAT("fastforward_ratio",        &settings->fastforward_ratio);
   SETTING_FLOAT("slowmotion_ratio",         &settings->slowmotion_ratio);
   SETTING_FLOAT("input_axis_threshold",     &settings->input.axis_threshold);

   *out = 
      (struct config_float_setting_ptr*) malloc(count * sizeof(struct config_float_setting_ptr));
   memcpy(*out, tmp, sizeof(struct config_float_setting_ptr) * count);
   free(tmp);
   return count;
}

static int populate_settings_int(settings_t *settings, struct config_int_setting_ptr **out)
{
   unsigned count                     = 0;
   struct config_int_setting_ptr *tmp = NULL;
#ifdef HAVE_NETPLAY
   global_t   *global                 = global_get_ptr();
#endif

   SETTING_INT("input_bind_timeout",           &settings->input.bind_timeout);
   SETTING_INT("input_turbo_period",           &settings->input.turbo_period);
   SETTING_INT("input_duty_cycle",             &settings->input.turbo_duty_cycle);
   SETTING_INT("input_max_users",              &settings->input.max_users);
   SETTING_INT("input_menu_toggle_gamepad_combo", &settings->input.menu_toggle_gamepad_combo);
   SETTING_INT("audio_latency",                &settings->audio.latency);
   SETTING_INT("audio_block_frames",           &settings->audio.block_frames);
   SETTING_INT("rewind_granularity",           &settings->rewind_granularity);
   SETTING_INT("autosave_interval",            &settings->autosave_interval);
   SETTING_INT("libretro_log_level",           &settings->libretro_log_level);
   SETTING_INT("keyboard_gamepad_mapping_type",&settings->input.keyboard_gamepad_mapping_type);
   SETTING_INT("input_poll_type_behavior",     &settings->input.poll_type_behavior);
#ifdef HAVE_MENU
   SETTING_INT("menu_ok_btn",                  &settings->menu_ok_btn);
   SETTING_INT("menu_cancel_btn",              &settings->menu_cancel_btn);
   SETTING_INT("menu_search_btn",              &settings->menu_search_btn);
   SETTING_INT("menu_info_btn",                &settings->menu_info_btn);
   SETTING_INT("menu_default_btn",             &settings->menu_default_btn);
   SETTING_INT("menu_scroll_down_btn",         &settings->menu_scroll_down_btn);
#endif
   SETTING_INT("video_monitor_index",          &settings->video.monitor_index);
   SETTING_INT("video_fullscreen_x",           &settings->video.fullscreen_x);
   SETTING_INT("video_fullscreen_y",           &settings->video.fullscreen_y);
#ifdef HAVE_COMMAND
   SETTING_INT("network_cmd_port",             &settings->network_cmd_port);
#endif
#ifdef HAVE_NETWORKGAMEPAD
   SETTING_INT("network_remote_base_port",     &settings->network_remote_base_port);
#endif
   SETTING_INT("menu_scroll_up_btn",           &settings->menu_scroll_up_btn);
#ifdef HAVE_GEKKO
   SETTING_INT("video_viwidth",                &settings->video.viwidth);
#endif
#ifdef HAVE_MENU
   SETTING_INT("dpi_override_value",           &settings->menu.dpi.override_value);
   SETTING_INT("menu_thumbnails",              &settings->menu.thumbnails);
   SETTING_INT("xmb_scale_factor",             &settings->menu.xmb.scale_factor);
   SETTING_INT("xmb_alpha_factor",             &settings->menu.xmb.alpha_factor);
#ifdef HAVE_XMB
   SETTING_INT("xmb_theme",                    &settings->menu.xmb.theme);
   SETTING_INT("xmb_menu_color_theme",         &settings->menu.xmb.menu_color_theme);
#endif
   SETTING_INT("materialui_menu_color_theme",  &settings->menu.materialui.menu_color_theme);
#ifdef HAVE_SHADERPIPELINE
   SETTING_INT("menu_shader_pipeline",         &settings->menu.xmb.shader_pipeline);
#endif
#endif
   SETTING_INT("audio_out_rate",               &settings->audio.out_rate);
   SETTING_INT("custom_viewport_width",        &settings->video_viewport_custom.width);
   SETTING_INT("custom_viewport_height",       &settings->video_viewport_custom.height);
   SETTING_INT("custom_viewport_x",            (unsigned*)&settings->video_viewport_custom.x);
   SETTING_INT("custom_viewport_y",            (unsigned*)&settings->video_viewport_custom.y);
   SETTING_INT("content_history_size",         &settings->content_history_size);
   SETTING_INT("video_hard_sync_frames",       &settings->video.hard_sync_frames);
   SETTING_INT("video_frame_delay",            &settings->video.frame_delay);
   SETTING_INT("video_max_swapchain_images",   &settings->video.max_swapchain_images);
   SETTING_INT("video_swap_interval",          &settings->video.swap_interval);
   SETTING_INT("video_rotation",               &settings->video.rotation);
   SETTING_INT("aspect_ratio_index",           &settings->video.aspect_ratio_idx);
   SETTING_INT("state_slot",                   (unsigned*)&settings->state_slot);
#ifdef HAVE_NETPLAY
   SETTING_INT("netplay_ip_port",              &global->netplay.port);
   SETTING_INT("netplay_delay_frames",         &global->netplay.sync_frames);
#endif
#ifdef HAVE_LANGEXTRA
   SETTING_INT("user_language",                &settings->user_language);
#endif
   SETTING_INT("bundle_assets_extract_version_current", &settings->bundle_assets_extract_version_current);
   SETTING_INT("bundle_assets_extract_last_version",    &settings->bundle_assets_extract_last_version);

   *out = (struct config_int_setting_ptr*)malloc(count * sizeof(struct config_int_setting_ptr));
   memcpy(*out, tmp, sizeof(struct config_int_setting_ptr) * count);
   free(tmp);
   return count;
}

/**
 * config_set_defaults:
 *
 * Set 'default' configuration values.
 **/
static void config_set_defaults(void)
{
   unsigned i, j;
   settings_t *settings            = config_get_ptr();
   global_t   *global              = global_get_ptr();
   const char *def_video           = config_get_default_video();
   const char *def_audio           = config_get_default_audio();
   const char *def_audio_resampler = config_get_default_audio_resampler();
   const char *def_input           = config_get_default_input();
   const char *def_joypad          = config_get_default_joypad();
#ifdef HAVE_MENU
   const char *def_menu            = config_get_default_menu();
#endif
   const char *def_camera          = config_get_default_camera();
   const char *def_location        = config_get_default_location();
   const char *def_record          = config_get_default_record();
#ifdef HAVE_MENU
   static bool first_initialized   = true;
#endif

   if (def_camera)
      strlcpy(settings->camera.driver,
            def_camera, sizeof(settings->camera.driver));
   if (def_location)
      strlcpy(settings->location.driver,
            def_location, sizeof(settings->location.driver));
   if (def_video)
      strlcpy(settings->video.driver,
            def_video, sizeof(settings->video.driver));
   if (def_audio)
      strlcpy(settings->audio.driver,
            def_audio, sizeof(settings->audio.driver));
   if (def_audio_resampler)
      strlcpy(settings->audio.resampler,
            def_audio_resampler, sizeof(settings->audio.resampler));
   if (def_input)
      strlcpy(settings->input.driver,
            def_input, sizeof(settings->input.driver));
   if (def_joypad)
      strlcpy(settings->input.joypad_driver,
            def_joypad, sizeof(settings->input.joypad_driver));
   if (def_record)
      strlcpy(settings->record.driver,
            def_record, sizeof(settings->record.driver));
#ifdef HAVE_MENU
   if (def_menu)
      strlcpy(settings->menu.driver,
            def_menu,  sizeof(settings->menu.driver));
#ifdef HAVE_XMB
   settings->menu.xmb.scale_factor     = xmb_scale_factor;
   settings->menu.xmb.alpha_factor     = xmb_alpha_factor;
   settings->menu.xmb.theme            = xmb_icon_theme;
   settings->menu.xmb.menu_color_theme = menu_background_gradient;
   settings->menu.xmb.shadows_enable   = xmb_shadows_enable;
#ifdef HAVE_SHADERPIPELINE
   settings->menu.xmb.shader_pipeline  = menu_shader_pipeline;
#endif
   settings->menu.xmb.show_settings    = xmb_show_settings;
#ifdef HAVE_IMAGEVIEWER
   settings->menu.xmb.show_images      = xmb_show_images;
#endif
#ifdef HAVE_FFMPEG
   settings->menu.xmb.show_music       = xmb_show_music;
   settings->menu.xmb.show_video       = xmb_show_video;
#endif
   settings->menu.xmb.show_history     = xmb_show_history;
   settings->menu.xmb.font[0]          = '\0';
#endif
#ifdef HAVE_MATERIALUI
   settings->menu.materialui.menu_color_theme = MATERIALUI_THEME_BLUE;

   if (g_defaults.menu.materialui.menu_color_theme_enable)
      settings->menu.materialui.menu_color_theme = g_defaults.menu.materialui.menu_color_theme;
#endif

   settings->menu.throttle_framerate   = true;
   settings->menu.linear_filter        = true;
#endif

   settings->history_list_enable         = def_history_list_enable;
   settings->load_dummy_on_core_shutdown = load_dummy_on_core_shutdown;

#if TARGET_OS_IPHONE
   settings->input.small_keyboard_enable   = false;
#endif
   settings->input.keyboard_gamepad_enable          = true;
   settings->input.keyboard_gamepad_mapping_type    = 1;
   settings->input.poll_type_behavior               = 2;
#ifdef HAVE_FFMPEG
   settings->multimedia.builtin_mediaplayer_enable  = true;
#else
   settings->multimedia.builtin_mediaplayer_enable  = false;
#endif
   settings->multimedia.builtin_imageviewer_enable = true;
   settings->video.scale                 = scale;
   settings->video.fullscreen            = rarch_ctl(RARCH_CTL_IS_FORCE_FULLSCREEN, NULL)  ? true : fullscreen;
   settings->video.windowed_fullscreen   = windowed_fullscreen;
   settings->video.monitor_index         = monitor_index;
   settings->video.fullscreen_x          = fullscreen_x;
   settings->video.fullscreen_y          = fullscreen_y;
   settings->video.disable_composition   = disable_composition;
   settings->video.vsync                 = vsync;
   settings->video.max_swapchain_images  = max_swapchain_images;
   settings->video.hard_sync             = hard_sync;
   settings->video.hard_sync_frames      = hard_sync_frames;
   settings->video.frame_delay           = frame_delay;
   settings->video.black_frame_insertion = black_frame_insertion;
   settings->video.swap_interval         = swap_interval;
   settings->video.threaded              = video_threaded;
   settings->bundle_assets_extract_enable = bundle_assets_extract_enable;

   if (g_defaults.settings.video_threaded_enable != video_threaded)
      settings->video.threaded           = g_defaults.settings.video_threaded_enable;

#ifdef HAVE_THREADS
   settings->threaded_data_runloop_enable = threaded_data_runloop_enable;
#endif
   settings->video.shared_context              = video_shared_context;
   settings->video.force_srgb_disable          = false;
#ifdef GEKKO
   settings->video.viwidth                     = video_viwidth;
   settings->video.vfilter                     = video_vfilter;
#endif
   settings->video.smooth                      = video_smooth;
   settings->video.force_aspect                = force_aspect;
   settings->video.scale_integer               = scale_integer;
   settings->video.crop_overscan               = crop_overscan;
   settings->video.aspect_ratio                = aspect_ratio;
   settings->video.aspect_ratio_auto           = aspect_ratio_auto; /* Let implementation decide if automatic, or 1:1 PAR. */
   settings->video.aspect_ratio_idx            = aspect_ratio_idx;
   settings->video.shader_enable               = shader_enable;
   settings->video.allow_rotate                = allow_rotate;

   settings->video.font_enable                 = font_enable;
   settings->video.font_size                   = font_size;
   settings->video.msg_pos_x                   = message_pos_offset_x;
   settings->video.msg_pos_y                   = message_pos_offset_y;

   settings->video.msg_color_r                 = ((message_color >> 16) & 0xff) / 255.0f;
   settings->video.msg_color_g                 = ((message_color >>  8) & 0xff) / 255.0f;
   settings->video.msg_color_b                 = ((message_color >>  0) & 0xff) / 255.0f;

   settings->video.refresh_rate                = refresh_rate;

   if (g_defaults.settings.video_refresh_rate > 0.0 &&
         g_defaults.settings.video_refresh_rate != refresh_rate)
      settings->video.refresh_rate             = g_defaults.settings.video_refresh_rate;

   settings->video.post_filter_record          = post_filter_record;
   settings->video.gpu_record                  = gpu_record;
   settings->video.gpu_screenshot              = gpu_screenshot;
   settings->auto_screenshot_filename          = auto_screenshot_filename;
   settings->video.rotation                    = ORIENTATION_NORMAL;

   settings->audio.enable                      = audio_enable;
   settings->audio.mute_enable                 = false;
   settings->audio.out_rate                    = out_rate;
   settings->audio.block_frames                = 0;
   if (audio_device)
      strlcpy(settings->audio.device,
            audio_device, sizeof(settings->audio.device));

   if (!g_defaults.settings.out_latency)
      g_defaults.settings.out_latency          = out_latency;

   settings->audio.latency                     = g_defaults.settings.out_latency;
   settings->audio.sync                        = audio_sync;
   settings->audio.rate_control                = rate_control;
   settings->audio.rate_control_delta          = rate_control_delta;
   settings->audio.max_timing_skew             = max_timing_skew;
   settings->audio.volume                      = audio_volume;

   audio_driver_set_volume_gain(db_to_gain(settings->audio.volume));

   settings->rewind_enable                     = rewind_enable;
   settings->rewind_buffer_size                = rewind_buffer_size;
   settings->rewind_granularity                = rewind_granularity;
   settings->slowmotion_ratio                  = slowmotion_ratio;
   settings->fastforward_ratio                 = fastforward_ratio;
   settings->pause_nonactive                   = pause_nonactive;
   settings->autosave_interval                 = autosave_interval;

   settings->block_sram_overwrite              = block_sram_overwrite;
   settings->savestate_auto_index              = savestate_auto_index;
   settings->savestate_auto_save               = savestate_auto_save;
   settings->savestate_auto_load               = savestate_auto_load;
   settings->network_cmd_enable                = network_cmd_enable;
   settings->network_cmd_port                  = network_cmd_port;
   settings->network_remote_base_port           = network_remote_base_port;
   settings->stdin_cmd_enable                  = stdin_cmd_enable;
   settings->content_history_size              = default_content_history_size;
   settings->libretro_log_level                = libretro_log_level;

#ifdef HAVE_LAKKA
   settings->ssh_enable = path_file_exists(LAKKA_SSH_PATH);
   settings->samba_enable = path_file_exists(LAKKA_SAMBA_PATH);
   settings->bluetooth_enable = path_file_exists(LAKKA_BLUETOOTH_PATH);
#endif

#ifdef HAVE_MENU
   if (first_initialized)
      settings->menu_show_start_screen         = default_menu_show_start_screen;
   settings->menu.pause_libretro               = true;
   settings->menu.mouse.enable                 = def_mouse_enable;
   settings->menu.pointer.enable               = pointer_enable;
   settings->menu.timedate_enable              = true;
   settings->menu.core_enable                  = true;
   settings->menu.dynamic_wallpaper_enable     = false;
   settings->menu.wallpaper.opacity            = menu_wallpaper_opacity;
   settings->menu.footer.opacity               = menu_footer_opacity;
   settings->menu.header.opacity               = menu_header_opacity;
   settings->menu.thumbnails                   = menu_thumbnails_default;
   settings->menu.show_advanced_settings       = show_advanced_settings;
   settings->menu.entry_normal_color           = menu_entry_normal_color;
   settings->menu.entry_hover_color            = menu_entry_hover_color;
   settings->menu.title_color                  = menu_title_color;

   settings->menu.dpi.override_enable          = menu_dpi_override_enable;
   settings->menu.dpi.override_value           = menu_dpi_override_value;

   settings->menu.navigation.wraparound.setting_enable                  = true;
   settings->menu.navigation.wraparound.enable                          = true;
   settings->menu.navigation.browser.filter.supported_extensions_enable = true;
#endif

   settings->ui.companion_start_on_boot             = ui_companion_start_on_boot;
   settings->ui.companion_enable                    = ui_companion_enable;
   settings->ui.menubar_enable                      = true;
   settings->ui.suspend_screensaver_enable          = true;

   settings->location.allow                         = false;
   settings->camera.allow                           = false;

#ifdef HAVE_CHEEVOS
   settings->cheevos.enable                         = cheevos_enable;
   settings->cheevos.test_unofficial                = false;
   settings->cheevos.hardcore_mode_enable           = false;
   *settings->cheevos.username                      = '\0';
   *settings->cheevos.password                      = '\0';
#endif

   settings->input.back_as_menu_toggle_enable       = true;
   settings->input.bind_timeout                     = input_bind_timeout;
   settings->input.input_descriptor_label_show      = input_descriptor_label_show;
   settings->input.input_descriptor_hide_unbound    = input_descriptor_hide_unbound;
   settings->input.remap_binds_enable               = true;
   settings->input.max_users                        = input_max_users;
   settings->input.menu_toggle_gamepad_combo        = menu_toggle_gamepad_combo;

   retro_assert(sizeof(settings->input.binds[0]) >= sizeof(retro_keybinds_1));
   retro_assert(sizeof(settings->input.binds[1]) >= sizeof(retro_keybinds_rest));

   memcpy(settings->input.binds[0], retro_keybinds_1, sizeof(retro_keybinds_1));

   for (i = 1; i < MAX_USERS; i++)
      memcpy(settings->input.binds[i], retro_keybinds_rest,
            sizeof(retro_keybinds_rest));

   input_remapping_set_defaults();

   for (i = 0; i < MAX_USERS; i++)
   {
      for (j = 0; j < RARCH_BIND_LIST_END; j++)
      {
         settings->input.autoconf_binds[i][j].joykey  = NO_BTN;
         settings->input.autoconf_binds[i][j].joyaxis = AXIS_NONE;
      }
   }
   memset(settings->input.autoconfigured, 0,
         sizeof(settings->input.autoconfigured));

   /* Verify that binds are in proper order. */
   for (i = 0; i < MAX_USERS; i++)
      for (j = 0; j < RARCH_BIND_LIST_END; j++)
      {
         if (settings->input.binds[i][j].valid)
            retro_assert(j == settings->input.binds[i][j].id);
      }

   settings->input.axis_threshold                  = axis_threshold;
   settings->input.netplay_client_swap_input       = netplay_client_swap_input;
   settings->input.turbo_period                    = turbo_period;
   settings->input.turbo_duty_cycle                = turbo_duty_cycle;

   strlcpy(settings->network.buildbot_url, buildbot_server_url,
         sizeof(settings->network.buildbot_url));
   strlcpy(settings->network.buildbot_assets_url, buildbot_assets_server_url,
         sizeof(settings->network.buildbot_assets_url));
   settings->network.buildbot_auto_extract_archive = true;

   settings->input.overlay_enable                  = config_overlay_enable_default();

   settings->input.overlay_enable_autopreferred    = true;
   settings->input.overlay_hide_in_menu            = overlay_hide_in_menu;
   settings->input.overlay_opacity                 = 0.7f;
   settings->input.overlay_scale                   = 1.0f;
   settings->input.autodetect_enable               = input_autodetect_enable;
   *settings->input.keyboard_layout                = '\0';

   settings->osk.enable                            = true;

   for (i = 0; i < MAX_USERS; i++)
   {
      settings->input.joypad_map[i] = i;
      settings->input.analog_dpad_mode[i] = ANALOG_DPAD_NONE;
      if (!global->has_set.libretro_device[i])
         settings->input.libretro_device[i] = RETRO_DEVICE_JOYPAD;
   }

   settings->set_supports_no_game_enable        = true;

   video_driver_reset_custom_viewport();

   /* Make sure settings from other configs carry over into defaults
    * for another config. */
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_SAVE_PATH))
      *global->dir.savefile = '\0';
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_STATE_PATH))
      *global->dir.savestate = '\0';

   *settings->path.libretro_info = '\0';
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_LIBRETRO_DIRECTORY))
      *settings->directory.libretro = '\0';
   *settings->directory.cursor = '\0';
   *settings->directory.resampler = '\0';
   *settings->directory.screenshot = '\0';
   *settings->directory.system = '\0';
   *settings->directory.cache = '\0';
   *settings->directory.input_remapping = '\0';
   *settings->directory.core_assets = '\0';
   *settings->directory.assets = '\0';
   *settings->directory.dynamic_wallpapers = '\0';
   *settings->directory.thumbnails = '\0';
   *settings->directory.playlist = '\0';
   *settings->directory.autoconfig = '\0';
#ifdef HAVE_MENU
   *settings->directory.menu_content = '\0';
   *settings->directory.menu_config = '\0';
#endif
   *settings->directory.video_shader = '\0';
   *settings->directory.video_filter = '\0';
   *settings->directory.audio_filter = '\0';

   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_UPS_PREF))
      global->patch.ups_pref = false;
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_BPS_PREF))
      global->patch.bps_pref = false;
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_IPS_PREF))
      global->patch.ips_pref = false;

   *global->record.output_dir = '\0';
   *global->record.config_dir = '\0';

   *settings->path.core_options      = '\0';
   *settings->path.content_history   = '\0';
   *settings->path.content_music_history   = '\0';
   *settings->path.content_image_history   = '\0';
   *settings->path.content_video_history   = '\0';
   *settings->path.cheat_settings    = '\0';
   *settings->path.shader            = '\0';
#ifndef IOS
   *settings->path.bundle_assets_src = '\0';
   *settings->path.bundle_assets_dst = '\0';
   *settings->path.bundle_assets_dst_subdir = '\0';
#endif
   *settings->path.cheat_database    = '\0';
   *settings->path.menu_wallpaper    = '\0';
   *settings->path.content_database  = '\0';
   *settings->path.overlay           = '\0';
   *settings->path.softfilter_plugin = '\0';

   settings->bundle_assets_extract_version_current = 0;
   settings->bundle_assets_extract_last_version    = 0;
   *settings->playlist_names = '\0';
   *settings->playlist_cores = '\0';
   *settings->directory.content_history = '\0';
   *settings->path.audio_dsp_plugin = '\0';
   settings->game_specific_options = default_game_specific_options;
   settings->auto_overrides_enable = default_auto_overrides_enable;
   settings->auto_remaps_enable = default_auto_remaps_enable;
   settings->auto_shaders_enable = default_auto_shaders_enable;

   settings->sort_savefiles_enable = default_sort_savefiles_enable;
   settings->sort_savestates_enable = default_sort_savestates_enable;

#ifdef HAVE_MENU
   settings->menu_ok_btn          = config_menu_btn_ok_default();
   settings->menu_cancel_btn      = config_menu_btn_cancel_default();
   settings->menu_search_btn      = default_menu_btn_search;
   settings->menu_default_btn     = default_menu_btn_default;
   settings->menu_info_btn        = default_menu_btn_info;
   settings->menu_scroll_down_btn = default_menu_btn_scroll_down;
   settings->menu_scroll_up_btn   = default_menu_btn_scroll_up;
#endif

#ifdef HAVE_LANGEXTRA
   settings->user_language = 0;
#endif

   global->console.sound.system_bgm_enable = false;

   video_driver_default_settings();

   if (!string_is_empty(g_defaults.dir.wallpapers))
      strlcpy(settings->directory.dynamic_wallpapers,
            g_defaults.dir.wallpapers, sizeof(settings->directory.dynamic_wallpapers));
   if (!string_is_empty(g_defaults.dir.thumbnails))
      strlcpy(settings->directory.thumbnails,
            g_defaults.dir.thumbnails, sizeof(settings->directory.thumbnails));
   if (!string_is_empty(g_defaults.dir.remap))
      strlcpy(settings->directory.input_remapping,
            g_defaults.dir.remap, sizeof(settings->directory.input_remapping));
   if (!string_is_empty(g_defaults.dir.cache))
      strlcpy(settings->directory.cache,
            g_defaults.dir.cache, sizeof(settings->directory.cache));
   if (!string_is_empty(g_defaults.dir.assets))
      strlcpy(settings->directory.assets,
            g_defaults.dir.assets, sizeof(settings->directory.assets));
   if (!string_is_empty(g_defaults.dir.core_assets))
      strlcpy(settings->directory.core_assets,
            g_defaults.dir.core_assets, sizeof(settings->directory.core_assets));
   if (!string_is_empty(g_defaults.dir.playlist))
      strlcpy(settings->directory.playlist,
            g_defaults.dir.playlist, sizeof(settings->directory.playlist));
   if (!string_is_empty(g_defaults.dir.core))
      fill_pathname_expand_special(settings->directory.libretro,
            g_defaults.dir.core, sizeof(settings->directory.libretro));
   if (!string_is_empty(g_defaults.dir.audio_filter))
      strlcpy(settings->directory.audio_filter,
            g_defaults.dir.audio_filter, sizeof(settings->directory.audio_filter));
   if (!string_is_empty(g_defaults.dir.video_filter))
      strlcpy(settings->directory.video_filter,
            g_defaults.dir.video_filter, sizeof(settings->directory.video_filter));
   if (!string_is_empty(g_defaults.dir.shader))
      fill_pathname_expand_special(settings->directory.video_shader,
            g_defaults.dir.shader, sizeof(settings->directory.video_shader));

   if (!string_is_empty(g_defaults.path.buildbot_server_url))
      strlcpy(settings->network.buildbot_url,
            g_defaults.path.buildbot_server_url, sizeof(settings->network.buildbot_url));
   if (!string_is_empty(g_defaults.path.core))
      runloop_ctl(RUNLOOP_CTL_SET_LIBRETRO_PATH, g_defaults.path.core);
   if (!string_is_empty(g_defaults.dir.database))
      strlcpy(settings->path.content_database, g_defaults.dir.database,
            sizeof(settings->path.content_database));
   if (!string_is_empty(g_defaults.dir.cursor))
      strlcpy(settings->directory.cursor, g_defaults.dir.cursor,
            sizeof(settings->directory.cursor));
   if (!string_is_empty(g_defaults.dir.cheats))
      strlcpy(settings->path.cheat_database, g_defaults.dir.cheats,
            sizeof(settings->path.cheat_database));
   if (!string_is_empty(g_defaults.dir.core_info))
      fill_pathname_expand_special(settings->path.libretro_info,
            g_defaults.dir.core_info, sizeof(settings->path.libretro_info));
#ifdef HAVE_OVERLAY
   if (!string_is_empty(g_defaults.dir.overlay))
   {
      fill_pathname_expand_special(settings->directory.overlay,
            g_defaults.dir.overlay, sizeof(settings->directory.overlay));
#ifdef RARCH_MOBILE
      if (string_is_empty(settings->path.overlay))
            fill_pathname_join(settings->path.overlay,
                  settings->directory.overlay,
                  "gamepads/retropad/retropad.cfg",
                  sizeof(settings->path.overlay));
#endif
   }

   if (!string_is_empty(g_defaults.dir.osk_overlay))
   {
      fill_pathname_expand_special(global->dir.osk_overlay,
            g_defaults.dir.osk_overlay, sizeof(global->dir.osk_overlay));
#ifdef RARCH_MOBILE
      if (string_is_empty(settings->path.osk_overlay))
            fill_pathname_join(settings->path.osk_overlay,
                  global->dir.osk_overlay,
                  "keyboards/modular-keyboard/opaque/big.cfg",
                  sizeof(settings->path.osk_overlay));
#endif
   }
   else
      strlcpy(global->dir.osk_overlay,
            settings->directory.overlay,
            sizeof(global->dir.osk_overlay));
#endif
#ifdef HAVE_MENU
   if (!string_is_empty(g_defaults.dir.menu_config))
      strlcpy(settings->directory.menu_config,
            g_defaults.dir.menu_config,
            sizeof(settings->directory.menu_config));
   if (!string_is_empty(g_defaults.dir.menu_content))
      strlcpy(settings->directory.menu_content,
            g_defaults.dir.menu_content,
            sizeof(settings->directory.menu_content));
#endif
   if (!string_is_empty(g_defaults.dir.autoconfig))
      strlcpy(settings->directory.autoconfig,
            g_defaults.dir.autoconfig,
            sizeof(settings->directory.autoconfig));

   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_STATE_PATH) &&
         !string_is_empty(g_defaults.dir.savestate))
      strlcpy(global->dir.savestate,
            g_defaults.dir.savestate, sizeof(global->dir.savestate));
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_SAVE_PATH) &&
         !string_is_empty(g_defaults.dir.sram))
      strlcpy(global->dir.savefile,
            g_defaults.dir.sram, sizeof(global->dir.savefile));
   if (!string_is_empty(g_defaults.dir.system))
      strlcpy(settings->directory.system,
            g_defaults.dir.system, sizeof(settings->directory.system));
   if (!string_is_empty(g_defaults.dir.screenshot))
      strlcpy(settings->directory.screenshot,
            g_defaults.dir.screenshot,
            sizeof(settings->directory.screenshot));
   if (!string_is_empty(g_defaults.dir.resampler))
      strlcpy(settings->directory.resampler,
            g_defaults.dir.resampler,
            sizeof(settings->directory.resampler));
   if (!string_is_empty(g_defaults.dir.content_history))
      strlcpy(settings->directory.content_history,
            g_defaults.dir.content_history,
            sizeof(settings->directory.content_history));

   if (!string_is_empty(g_defaults.path.config))
      fill_pathname_expand_special(global->path.config,
            g_defaults.path.config, sizeof(global->path.config));

   settings->config_save_on_exit = config_save_on_exit;
   settings->show_hidden_files = show_hidden_files;

   /* Avoid reloading config on every content load */
   if (default_block_config_read)
      rarch_ctl(RARCH_CTL_SET_BLOCK_CONFIG_READ, NULL);
   else
      rarch_ctl(RARCH_CTL_UNSET_BLOCK_CONFIG_READ, NULL);

#ifdef HAVE_MENU
   first_initialized = false;
#endif
}

/**
 * open_default_config_file
 *
 * Open a default config file. Platform-specific.
 *
 * Returns: handle to config file if found, otherwise NULL.
 **/
static config_file_t *open_default_config_file(void)
{
   char application_data[PATH_MAX_LENGTH] = {0};
   char conf_path[PATH_MAX_LENGTH]        = {0};
   char app_path[PATH_MAX_LENGTH]         = {0};
   config_file_t *conf                    = NULL;
   global_t *global                       = global_get_ptr();

#if defined(_WIN32) && !defined(_XBOX)
   fill_pathname_application_path(app_path, sizeof(app_path));
   fill_pathname_resolve_relative(conf_path, app_path,
         file_path_str(FILE_PATH_MAIN_CONFIG), sizeof(conf_path));

   conf = config_file_new(conf_path);

   if (!conf)
   {
      if (fill_pathname_application_data(application_data,
            sizeof(application_data)))
      {
         fill_pathname_join(conf_path, application_data,
               file_path_str(FILE_PATH_MAIN_CONFIG), sizeof(conf_path));
         conf = config_file_new(conf_path);
      }
   }

   if (!conf)
   {
      bool saved = false;

      /* Try to create a new config file. */
      conf = config_file_new(NULL);


      if (conf)
      {
         /* Since this is a clean config file, we can
          * safely use config_save_on_exit. */
         fill_pathname_resolve_relative(conf_path, app_path,
               file_path_str(FILE_PATH_MAIN_CONFIG), sizeof(conf_path));
         config_set_bool(conf, "config_save_on_exit", true);
         saved = config_file_write(conf, conf_path);
      }

      if (!saved)
      {
         /* WARN here to make sure user has a good chance of seeing it. */
         RARCH_ERR("Failed to create new config file in: \"%s\".\n",
               conf_path);
         config_file_free(conf);
         return NULL;
      }

      RARCH_WARN("Created new config file in: \"%s\".\n", conf_path);
   }
#elif defined(OSX)
   if (!fill_pathname_application_data(application_data,
            sizeof(application_data)))
      return NULL;

   path_mkdir(application_data);

   fill_pathname_join(conf_path, application_data,
         file_path_str(FILE_PATH_MAIN_CONFIG), sizeof(conf_path));
   conf = config_file_new(conf_path);

   if (!conf)
   {
      bool saved = false;

      conf = config_file_new(NULL);

      if (conf)
      {
         config_set_bool(conf, "config_save_on_exit", true);
         saved = config_file_write(conf, conf_path);
      }

      if (!saved)
      {
         /* WARN here to make sure user has a good chance of seeing it. */
         RARCH_ERR("Failed to create new config file in: \"%s\".\n",
               conf_path);
         config_file_free(conf);

         return NULL;
      }

      RARCH_WARN("Created new config file in: \"%s\".\n", conf_path);
   }
#elif !defined(RARCH_CONSOLE)
   bool has_application_data = fill_pathname_application_data(application_data,
            sizeof(application_data));

   if (has_application_data)
   {
      fill_pathname_join(conf_path, application_data,
            file_path_str(FILE_PATH_MAIN_CONFIG), sizeof(conf_path));
      RARCH_LOG("Looking for config in: \"%s\".\n", conf_path);
      conf = config_file_new(conf_path);
   }

   /* Fallback to $HOME/.retroarch.cfg. */
   if (!conf && getenv("HOME"))
   {
      fill_pathname_join(conf_path, getenv("HOME"),
            ".retroarch.cfg", sizeof(conf_path));
      RARCH_LOG("Looking for config in: \"%s\".\n", conf_path);
      conf = config_file_new(conf_path);
   }

   if (!conf && has_application_data)
   {
      char basedir[PATH_MAX_LENGTH] = {0};

      /* Try to create a new config file. */

      strlcpy(conf_path, application_data, sizeof(conf_path));

      fill_pathname_basedir(basedir, conf_path, sizeof(basedir));

      fill_pathname_join(conf_path, conf_path, file_path_str(FILE_PATH_MAIN_CONFIG), sizeof(conf_path));

      if (path_mkdir(basedir))
      {
         bool saved                          = false;
         char skeleton_conf[PATH_MAX_LENGTH] = {0};

#if defined(__HAIKU__)
         fill_pathname_join(skeleton_conf, "/system/settings",
               file_path_str(FILE_PATH_MAIN_CONFIG), sizeof(skeleton_conf));
#else
         fill_pathname_join(skeleton_conf, "/etc",
               file_path_str(FILE_PATH_MAIN_CONFIG), sizeof(skeleton_conf));
#endif
         conf = config_file_new(skeleton_conf);
         if (conf)
            RARCH_WARN("Config: using skeleton config \"%s\" as base for a new config file.\n", skeleton_conf);
         else
            conf = config_file_new(NULL);

         if (conf)
         {
            /* Since this is a clean config file, we can safely use config_save_on_exit. */
            config_set_bool(conf, "config_save_on_exit", true);
            saved = config_file_write(conf, conf_path);
         }

         if (!saved)
         {
            /* WARN here to make sure user has a good chance of seeing it. */
            RARCH_ERR("Failed to create new config file in: \"%s\".\n", conf_path);
            config_file_free(conf);

            return NULL;
         }

         RARCH_WARN("Config: Created new config file in: \"%s\".\n", conf_path);
      }
   }
#endif

   (void)application_data;
   (void)conf_path;
   (void)app_path;

   if (!conf)
      return NULL;

   if (global)
      strlcpy(global->path.config, conf_path, sizeof(global->path.config));
   return conf;
}

static void read_keybinds_keyboard(config_file_t *conf, unsigned user,
      unsigned idx, struct retro_keybind *bind)
{
   const char *prefix = NULL;

   if (!input_config_bind_map_get_valid(idx))
      return;

   if (!input_config_bind_map_get_base(idx))
      return;

   prefix = input_config_get_prefix(user, input_config_bind_map_get_meta(idx));

   if (prefix)
      input_config_parse_key(conf, prefix,
            input_config_bind_map_get_base(idx), bind);
}

static void read_keybinds_button(config_file_t *conf, unsigned user,
      unsigned idx, struct retro_keybind *bind)
{
   const char *prefix = NULL;

   if (!input_config_bind_map_get_valid(idx))
      return;
   if (!input_config_bind_map_get_base(idx))
      return;

   prefix = input_config_get_prefix(user,
         input_config_bind_map_get_meta(idx));

   if (prefix)
      input_config_parse_joy_button(conf, prefix,
            input_config_bind_map_get_base(idx), bind);
}

static void read_keybinds_axis(config_file_t *conf, unsigned user,
      unsigned idx, struct retro_keybind *bind)
{
   const char *prefix = NULL;

   if (!input_config_bind_map_get_valid(idx))
      return;
   if (!input_config_bind_map_get_base(idx))
      return;

   prefix = input_config_get_prefix(user,
         input_config_bind_map_get_meta(idx));

   if (prefix)
      input_config_parse_joy_axis(conf, prefix,
            input_config_bind_map_get_base(idx), bind);
}

static void read_keybinds_user(config_file_t *conf, unsigned user)
{
   unsigned i;
   settings_t *settings = config_get_ptr();

   for (i = 0; input_config_bind_map_get_valid(i); i++)
   {
      struct retro_keybind *bind = (struct retro_keybind*)
         &settings->input.binds[user][i];

      if (!bind->valid)
         continue;

      read_keybinds_keyboard(conf, user, i, bind);
      read_keybinds_button(conf, user, i, bind);
      read_keybinds_axis(conf, user, i, bind);
   }
}

static void config_read_keybinds_conf(config_file_t *conf)
{
   unsigned i;

   for (i = 0; i < MAX_USERS; i++)
      read_keybinds_user(conf, i);
}

/* Also dumps inherited values, useful for logging. */
#if 0
static void config_file_dump_all(config_file_t *conf)
{
   struct config_entry_list *list = NULL;
   struct config_include_list *includes = conf->includes;

   while (includes)
   {
      RARCH_LOG("#include \"%s\"\n", includes->path);
      includes = includes->next;
   }

   list = conf->entries;

   while (list)
   {
      RARCH_LOG("%s = \"%s\"%s\n", list->key,
            list->value, list->readonly ? " (included)" : "");
      list = list->next;
   }
}
#endif

#ifdef HAVE_MENU
static void config_get_hex_base(config_file_t *conf, const char *key, unsigned *base)
{
   unsigned tmp = 0;
   if (!base)
      return;
   if (config_get_hex(conf, key, &tmp))
      *base = tmp;
}
#endif


/**
 * config_load:
 * @path                : path to be read from.
 * @set_defaults        : set default values first before
 *                        reading the values from the config file
 *
 * Loads a config file and reads all the values into memory.
 *
 */
static bool config_load_file(const char *path, bool set_defaults, 
   settings_t *settings)
{
   unsigned i;
   int bool_settings_size  = 0, int_settings_size    = 0,
       float_settings_size = 0, string_settings_size = 0,
       path_settings_size  = 0;
   bool tmp_bool                                   = false;
   char *save                                      = NULL;
   const char *extra_path                          = NULL;
   char tmp_str[PATH_MAX_LENGTH]                   = {0};
   char tmp_append_path[PATH_MAX_LENGTH]           = {0}; /* Don't destroy append_config_path. */
   unsigned msg_color                              = 0;
   config_file_t *conf                             = NULL;
   struct config_int_setting_ptr   *int_settings   = NULL;
   struct config_float_setting_ptr *float_settings = NULL;
   struct config_bool_setting_ptr *bool_settings   = NULL;
   global_t   *global                              = global_get_ptr();

   if (!settings)
      settings = config_get_ptr();

   bool_settings_size   = populate_settings_bool  (settings, &bool_settings);
   float_settings_size  = populate_settings_float (settings, &float_settings);
   int_settings_size    = populate_settings_int   (settings, &int_settings);
    
   (void)path_settings_size;
   (void)string_settings_size;

   if (path)
   {
      conf = config_file_new(path);
      if (!conf)
         return false;
   }
   else
      conf = open_default_config_file();

   if (!conf)
      return true;

   if (set_defaults)
      config_set_defaults();

   strlcpy(tmp_append_path, global->path.append_config,
         sizeof(tmp_append_path));
   extra_path = strtok_r(tmp_append_path, "|", &save);

   while (extra_path)
   {
      bool ret = config_append_file(conf, extra_path);

      RARCH_LOG("Config: appending config \"%s\"\n", extra_path);

      if (!ret)
         RARCH_ERR("Config: failed to append config \"%s\"\n", extra_path);
      extra_path = strtok_r(NULL, "|", &save);
   }
#if 0
   if (verbosity_is_enabled())
   {
      RARCH_LOG_OUTPUT("=== Config ===\n");
      config_file_dump_all(conf);
      RARCH_LOG_OUTPUT("=== Config end ===\n");
   }
#endif

   /* Boolean settings */

   for (i = 0; i < bool_settings_size; i++)
   {
      bool tmp = false;
      if (config_get_bool(conf, bool_settings[i].ident, &tmp))
         *bool_settings[i].ptr = tmp;
   }
   if (!rarch_ctl(RARCH_CTL_IS_FORCE_FULLSCREEN, NULL))
      CONFIG_GET_BOOL_BASE(conf, settings, video.fullscreen, "video_fullscreen");

   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_UPS_PREF))
   {
      CONFIG_GET_BOOL_BASE(conf, global, patch.ups_pref, "ups_pref");
   }

   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_BPS_PREF))
   {
      CONFIG_GET_BOOL_BASE(conf, global, patch.bps_pref, "bps_pref");
   }

   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_IPS_PREF))
   {
      CONFIG_GET_BOOL_BASE(conf, global, patch.ips_pref, "ips_pref");
   }

#ifdef HAVE_NETPLAY
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_NETPLAY_MODE))
      CONFIG_GET_BOOL_BASE(conf, global, netplay.is_spectate,
            "netplay_spectator_mode_enable");
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_NETPLAY_MODE))
      CONFIG_GET_BOOL_BASE(conf, global, netplay.is_client, "netplay_mode");
#endif
#ifdef HAVE_NETWORKGAMEPAD
   for (i = 0; i < MAX_USERS; i++)
   {
      char tmp[64]  = {0};

      snprintf(tmp, sizeof(tmp), "network_remote_enable_user_p%u", i + 1);

      if (config_get_bool(conf, tmp, &tmp_bool))
         settings->network_remote_enable_user[i] = tmp_bool;
   }
#endif
#ifdef RARCH_CONSOLE
   /* TODO - will be refactored later to make it more clean - it's more
    * important that it works for consoles right now */
   if (config_get_bool(conf, "custom_bgm_enable", &tmp_bool))
      global->console.sound.system_bgm_enable = tmp_bool;
#endif
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_VERBOSITY))
   {
      if (config_get_bool(conf, "log_verbosity", &tmp_bool))
      {
         if (tmp_bool)
            verbosity_enable();
         else
            verbosity_disable();
      }
   }
   {
      char tmp[64]  = {0};

      strlcpy(tmp, "perfcnt_enable", sizeof(tmp));
      if (config_get_bool(conf, tmp, &tmp_bool))
      {
         if (tmp_bool)
            runloop_ctl(RUNLOOP_CTL_SET_PERFCNT_ENABLE, NULL);
         else
            runloop_ctl(RUNLOOP_CTL_UNSET_PERFCNT_ENABLE, NULL);
      }
   }

   /* Integer settings */

   for (i = 0; i < int_settings_size; i++)
   {
      int tmp = 0;
      if (config_get_int(conf, int_settings[i].ident, &tmp))
         *int_settings[i].ptr = tmp;
   }

#ifdef HAVE_NETPLAY
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_NETPLAY_DELAY_FRAMES))
      CONFIG_GET_INT_BASE(conf, global, netplay.sync_frames, "netplay_delay_frames");
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_NETPLAY_IP_PORT))
      CONFIG_GET_INT_BASE(conf, global, netplay.port, "netplay_ip_port");
#endif
   for (i = 0; i < MAX_USERS; i++)
   {
      char buf[64] = {0};
      snprintf(buf, sizeof(buf), "input_player%u_joypad_index", i + 1);
      CONFIG_GET_INT_BASE(conf, settings, input.joypad_map[i], buf);

      snprintf(buf, sizeof(buf), "input_player%u_analog_dpad_mode", i + 1);
      CONFIG_GET_INT_BASE(conf, settings, input.analog_dpad_mode[i], buf);

      if (!global->has_set.libretro_device[i])
      {
         snprintf(buf, sizeof(buf), "input_libretro_device_p%u", i + 1);
         CONFIG_GET_INT_BASE(conf, settings, input.libretro_device[i], buf);
      }
   }
   {
      /* ugly hack around C89 not allowing mixing declarations and code */
      int buffer_size = 0;
      if (config_get_int(conf, "rewind_buffer_size", &buffer_size))
         settings->rewind_buffer_size = buffer_size * UINT64_C(1000000);
   }


   /* Hexadecimal settings  */

   if (config_get_hex(conf, "video_message_color", &msg_color))
   {
      settings->video.msg_color_r = ((msg_color >> 16) & 0xff) / 255.0f;
      settings->video.msg_color_g = ((msg_color >>  8) & 0xff) / 255.0f;
      settings->video.msg_color_b = ((msg_color >>  0) & 0xff) / 255.0f;
   }
#ifdef HAVE_MENU
   config_get_hex_base(conf, "menu_entry_normal_color",
         &settings->menu.entry_normal_color);
   config_get_hex_base(conf, "menu_entry_hover_color",
         &settings->menu.entry_hover_color);
   config_get_hex_base(conf, "menu_title_color",
         &settings->menu.title_color);
#endif

   /* Float settings */

   for (i = 0; i < float_settings_size; i++)
   {
      float tmp = 0.0f;
      if (config_get_float(conf, float_settings[i].ident, &tmp))
         *float_settings[i].ptr = tmp;
   }

   /* Array settings  */
   config_get_array(conf, "playlist_names", settings->playlist_names, sizeof(settings->playlist_names));
   config_get_array(conf, "playlist_cores", settings->playlist_cores, sizeof(settings->playlist_cores));
   config_get_array(conf, "audio_device", settings->audio.device, sizeof(settings->audio.device));
   config_get_array(conf, "audio_resampler", settings->audio.resampler, sizeof(settings->audio.resampler));
   config_get_array(conf, "camera_device", settings->camera.device, sizeof(settings->camera.device));
#ifdef HAVE_CHEEVOS
   config_get_array(conf, "cheevos_username", settings->cheevos.username, sizeof(settings->cheevos.username));
   config_get_array(conf, "cheevos_password", settings->cheevos.password, sizeof(settings->cheevos.password));
#endif

   config_get_array(conf, "video_driver",    settings->video.driver, sizeof(settings->video.driver));
   config_get_array(conf, "record_driver",   settings->record.driver, sizeof(settings->video.driver));
   config_get_array(conf, "camera_driver",   settings->camera.driver, sizeof(settings->camera.driver));
   config_get_array(conf, "location_driver", settings->location.driver, sizeof(settings->location.driver));
#ifdef HAVE_MENU
   config_get_array(conf, "menu_driver",     settings->menu.driver, sizeof(settings->menu.driver));
#endif
   config_get_array(conf, "video_context_driver",
         settings->video.context_driver,
         sizeof(settings->video.context_driver));
   config_get_array(conf, "audio_driver",
         settings->audio.driver,
         sizeof(settings->audio.driver));
   config_get_array(conf, "input_driver",
         settings->input.driver,
         sizeof(settings->input.driver));
   config_get_array(conf, "input_joypad_driver",
         settings->input.joypad_driver,
         sizeof(settings->input.joypad_driver));
   config_get_array(conf, "input_keyboard_layout",
         settings->input.keyboard_layout,
         sizeof(settings->input.keyboard_layout));
   config_get_array(conf, "bundle_assets_src_path",
         settings->path.bundle_assets_src,
         sizeof(settings->path.bundle_assets_src));
   config_get_array(conf, "bundle_assets_dst_path",
         settings->path.bundle_assets_dst,
         sizeof(settings->path.bundle_assets_dst));
   config_get_array(conf, "bundle_assets_dst_path_subdir",
         settings->path.bundle_assets_dst_subdir,
         sizeof(settings->path.bundle_assets_dst_subdir));

   /* Path settings  */
#ifdef HAVE_MENU
   if (config_get_path(conf, "xmb_font", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->menu.xmb.font, tmp_str, sizeof(settings->menu.xmb.font));
#endif
   if (config_get_path(conf, "menu_wallpaper", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.menu_wallpaper, tmp_str,
            sizeof(settings->path.menu_wallpaper));
   if (config_get_path(conf, "video_shader", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.shader, tmp_str, sizeof(settings->path.shader));
   if (config_get_path(conf, "video_font_path", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.font, tmp_str, sizeof(settings->path.font));
   if (config_get_path(conf, "video_filter_dir", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.video_filter, tmp_str, sizeof(settings->directory.video_filter));
   if (config_get_path(conf, "audio_filter_dir", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.audio_filter, tmp_str, sizeof(settings->directory.audio_filter));
   if (config_get_path(conf, "core_updater_buildbot_url", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->network.buildbot_url, tmp_str, sizeof(settings->network.buildbot_url));
   if (config_get_path(conf, "core_updater_buildbot_assets_url", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->network.buildbot_assets_url, tmp_str, sizeof(settings->network.buildbot_assets_url));
#ifdef HAVE_OVERLAY
   if (config_get_path(conf, "input_overlay", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.overlay, tmp_str, sizeof(settings->path.overlay));
   if (config_get_path(conf, "input_osk_overlay", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.osk_overlay, tmp_str, sizeof(settings->path.osk_overlay));
#endif
   if (config_get_path(conf, "video_filter", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.softfilter_plugin, tmp_str, sizeof(settings->path.softfilter_plugin));
   if (config_get_path(conf, "audio_dsp_plugin", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.audio_dsp_plugin, tmp_str, sizeof(settings->path.audio_dsp_plugin));

   if (config_get_path(conf, "libretro_info_path", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.libretro_info, tmp_str, sizeof(settings->path.libretro_info));

   if (config_get_path(conf, "core_options_path", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.core_options, tmp_str, sizeof(settings->path.core_options));

   if (config_get_path(conf, "system_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.system, tmp_str,
            sizeof(settings->directory.system));

   if (config_get_path(conf, "content_database_path", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.content_database, tmp_str, sizeof(settings->path.content_database));

   if (config_get_path(conf, "cheat_database_path", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.cheat_database, tmp_str, sizeof(settings->path.cheat_database));

   if (config_get_path(conf, "cursor_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.cursor, tmp_str, sizeof(settings->directory.cursor));

   if (config_get_path(conf, "cheat_settings_path", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.cheat_settings, tmp_str, sizeof(settings->path.cheat_settings));

   if (config_get_path(conf, "content_history_path", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.content_history, tmp_str, sizeof(settings->path.content_history));

   if (config_get_path(conf, "content_music_history_path", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.content_music_history, tmp_str, sizeof(settings->path.content_music_history));

   if (config_get_path(conf, "content_image_history_path", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.content_image_history, tmp_str, sizeof(settings->path.content_image_history));

   if (config_get_path(conf, "content_video_history_path", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->path.content_video_history, tmp_str, sizeof(settings->path.content_video_history));

   if (config_get_path(conf, "resampler_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.resampler, tmp_str, sizeof(settings->directory.resampler));

   if (config_get_path(conf, "cache_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.cache, tmp_str, sizeof(settings->directory.cache));

   if (config_get_path(conf, "input_remapping_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.input_remapping, tmp_str, sizeof(settings->directory.input_remapping));

   if (config_get_path(conf, "core_assets_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.core_assets, tmp_str, sizeof(settings->directory.core_assets));

   if (config_get_path(conf, "assets_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.assets, tmp_str, sizeof(settings->directory.assets));

   if (config_get_path(conf, "dynamic_wallpapers_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.dynamic_wallpapers, tmp_str, sizeof(settings->directory.dynamic_wallpapers));

   if (config_get_path(conf, "thumbnails_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.thumbnails, tmp_str, sizeof(settings->directory.thumbnails));

   if (config_get_path(conf, "playlist_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.playlist, tmp_str, sizeof(settings->directory.playlist));

   if (config_get_path(conf, "recording_output_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(global->record.output_dir, tmp_str, sizeof(global->record.output_dir));
   if (config_get_path(conf, "recording_config_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(global->record.config_dir, tmp_str, sizeof(global->record.config_dir));
#ifdef HAVE_OVERLAY
   if (config_get_path(conf, "overlay_directory", tmp_str, sizeof(tmp_str)))
         strlcpy(settings->directory.overlay, tmp_str, sizeof(settings->directory.overlay));
   if (config_get_path(conf, "osk_overlay_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(global->dir.osk_overlay, tmp_str, sizeof(global->dir.osk_overlay));
#endif
   if (config_get_path(conf, "content_history_dir", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.content_history, tmp_str, sizeof(settings->directory.content_history));
   if (config_get_path(conf, "joypad_autoconfig_dir", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.autoconfig, tmp_str, sizeof(settings->directory.autoconfig));
   if (config_get_path(conf, "screenshot_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.screenshot, tmp_str, sizeof(settings->directory.screenshot));
   if (config_get_path(conf, "video_shader_dir", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.video_shader, tmp_str, sizeof(settings->directory.video_shader));

   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_LIBRETRO_DIRECTORY))
   {
      if (config_get_path(conf, "libretro_directory", tmp_str, sizeof(tmp_str)))
            strlcpy(settings->directory.libretro, tmp_str, sizeof(settings->directory.libretro));
   }

#ifndef HAVE_DYNAMIC
   if (config_get_path(conf, "libretro_path", tmp_str, sizeof(tmp_str)))
      config_set_active_core_path(tmp_str);
#endif
#ifdef HAVE_MENU
   if (config_get_path(conf, "rgui_browser_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.menu_content, tmp_str, sizeof(settings->directory.menu_content));
   if (config_get_path(conf, "rgui_config_directory", tmp_str, sizeof(tmp_str)))
      strlcpy(settings->directory.menu_config, tmp_str, sizeof(settings->directory.menu_config));
#endif
   if (!rarch_ctl(RARCH_CTL_HAS_SET_USERNAME, NULL))
   {
      if (config_get_path(conf, "netplay_nickname",  tmp_str, sizeof(tmp_str)))
            strlcpy(settings->username, tmp_str, sizeof(settings->username));
   }
#ifdef HAVE_NETPLAY
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_NETPLAY_IP_ADDRESS))
   {
      if (config_get_path(conf, "netplay_ip_address", tmp_str, sizeof(tmp_str)))
            strlcpy(global->netplay.server, tmp_str, sizeof(global->netplay.server));
   }
#endif

#ifdef RARCH_CONSOLE
   video_driver_load_settings(conf);
#endif

   /* Post-settings load */

   if (settings->video.hard_sync_frames > 3)
      settings->video.hard_sync_frames = 3;

   if (settings->video.frame_delay > 15)
      settings->video.frame_delay = 15;

   settings->video.swap_interval = MAX(settings->video.swap_interval, 1);
   settings->video.swap_interval = MIN(settings->video.swap_interval, 4);


   audio_driver_set_volume_gain(db_to_gain(settings->audio.volume));

   if (string_is_empty(settings->path.content_history))
   {
      if (string_is_empty(settings->directory.content_history))
      {
         fill_pathname_resolve_relative(
               settings->path.content_history,
               global->path.config,
               file_path_str(FILE_PATH_CONTENT_HISTORY),
               sizeof(settings->path.content_history));
      }
      else
      {
         fill_pathname_join(settings->path.content_history,
               settings->directory.content_history,
               file_path_str(FILE_PATH_CONTENT_HISTORY),
               sizeof(settings->path.content_history));
      }
   }

   if (string_is_empty(settings->path.content_music_history))
   {
      if (string_is_empty(settings->directory.content_history))
      {
         fill_pathname_resolve_relative(
               settings->path.content_music_history,
               global->path.config,
               file_path_str(FILE_PATH_CONTENT_MUSIC_HISTORY),
               sizeof(settings->path.content_music_history));
      }
      else
      {
         fill_pathname_join(settings->path.content_music_history,
               settings->directory.content_history,
               file_path_str(FILE_PATH_CONTENT_MUSIC_HISTORY),
               sizeof(settings->path.content_music_history));
      }
   }

   if (string_is_empty(settings->path.content_video_history))
   {
      if (string_is_empty(settings->directory.content_history))
      {
         fill_pathname_resolve_relative(
               settings->path.content_video_history,
               global->path.config,
               file_path_str(FILE_PATH_CONTENT_VIDEO_HISTORY),
               sizeof(settings->path.content_video_history));
      }
      else
      {
         fill_pathname_join(settings->path.content_video_history,
               settings->directory.content_history,
               file_path_str(FILE_PATH_CONTENT_VIDEO_HISTORY),
               sizeof(settings->path.content_video_history));
      }
   }

   if (string_is_empty(settings->path.content_image_history))
   {
      if (string_is_empty(settings->directory.content_history))
      {
         fill_pathname_resolve_relative(
               settings->path.content_image_history,
               global->path.config,
               file_path_str(FILE_PATH_CONTENT_IMAGE_HISTORY),
               sizeof(settings->path.content_image_history));
      }
      else
      {
         fill_pathname_join(settings->path.content_image_history,
               settings->directory.content_history,
               file_path_str(FILE_PATH_CONTENT_IMAGE_HISTORY),
               sizeof(settings->path.content_image_history));
      }
   }


   if (!string_is_empty(settings->directory.screenshot))
   {
      if (string_is_equal(settings->directory.screenshot, "default"))
         *settings->directory.screenshot = '\0';
      else if (!path_is_directory(settings->directory.screenshot))
      {
         RARCH_WARN("screenshot_directory is not an existing directory, ignoring ...\n");
         *settings->directory.screenshot = '\0';
      }
   }

   /* Safe-guard against older behavior. */
   if (path_is_directory(config_get_active_core_path()))
   {
      RARCH_WARN("\"libretro_path\" is a directory, using this for \"libretro_directory\" instead.\n");
      strlcpy(settings->directory.libretro, config_get_active_core_path(),
            sizeof(settings->directory.libretro));
      config_clear_active_core_path();
   }

   if (string_is_equal(settings->path.menu_wallpaper, "default"))
      *settings->path.menu_wallpaper = '\0';
   if (string_is_equal(settings->directory.video_shader, "default"))
      *settings->directory.video_shader = '\0';
   if (string_is_equal(settings->directory.video_filter, "default"))
      *settings->directory.video_filter = '\0';
   if (string_is_equal(settings->directory.audio_filter, "default"))
      *settings->directory.audio_filter = '\0';
   if (string_is_equal(settings->directory.core_assets, "default"))
      *settings->directory.core_assets = '\0';
   if (string_is_equal(settings->directory.assets, "default"))
      *settings->directory.assets = '\0';
   if (string_is_equal(settings->directory.dynamic_wallpapers, "default"))
      *settings->directory.dynamic_wallpapers = '\0';
   if (string_is_equal(settings->directory.thumbnails, "default"))
      *settings->directory.thumbnails = '\0';
   if (string_is_equal(settings->directory.playlist, "default"))
      *settings->directory.playlist = '\0';
#ifdef HAVE_MENU

   if (string_is_equal(settings->directory.menu_content, "default"))
      *settings->directory.menu_content = '\0';
   if (string_is_equal(settings->directory.menu_config, "default"))
      *settings->directory.menu_config = '\0';
#endif
#ifdef HAVE_OVERLAY
   if (string_is_equal(settings->directory.overlay, "default"))
      *settings->directory.overlay = '\0';
   if (string_is_equal(global->dir.osk_overlay, "default"))
      *global->dir.osk_overlay = '\0';
#endif
   if (string_is_equal(settings->directory.system, "default"))
      *settings->directory.system = '\0';

   if (settings->slowmotion_ratio < 1.0f)
      settings->slowmotion_ratio = 1.0f;

   /* Sanitize fastforward_ratio value - previously range was -1
    * and up (with 0 being skipped) */
   if (settings->fastforward_ratio < 0.0f)
      settings->fastforward_ratio = 0.0f;

#ifdef HAVE_LAKKA
   settings->ssh_enable       = path_file_exists(LAKKA_SSH_PATH);
   settings->samba_enable     = path_file_exists(LAKKA_SAMBA_PATH);
   settings->bluetooth_enable = path_file_exists(LAKKA_BLUETOOTH_PATH);
#endif

   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_SAVE_PATH) &&
         config_get_path(conf, "savefile_directory", tmp_str, sizeof(tmp_str)))
   {
      if (string_is_equal(tmp_str, "default"))
         strlcpy(global->dir.savefile, g_defaults.dir.sram,
               sizeof(global->dir.savefile));
      else if (path_is_directory(tmp_str))
      {
         strlcpy(global->dir.savefile, tmp_str,
               sizeof(global->dir.savefile));
         strlcpy(global->name.savefile, tmp_str,
               sizeof(global->name.savefile));
         fill_pathname_dir(global->name.savefile,
               global->name.base,
               file_path_str(FILE_PATH_SRM_EXTENSION),
               sizeof(global->name.savefile));
      }
      else
         RARCH_WARN("savefile_directory is not a directory, ignoring ...\n");
   }

   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_STATE_PATH) &&
         config_get_path(conf, "savestate_directory", tmp_str, sizeof(tmp_str)))
   {
      if (string_is_equal(tmp_str, "default"))
         strlcpy(global->dir.savestate, g_defaults.dir.savestate,
               sizeof(global->dir.savestate));
      else if (path_is_directory(tmp_str))
      {
         strlcpy(global->dir.savestate, tmp_str,
               sizeof(global->dir.savestate));
         strlcpy(global->name.savestate, tmp_str,
               sizeof(global->name.savestate));
         fill_pathname_dir(global->name.savestate,
               global->name.base,
               file_path_str(FILE_PATH_STATE_EXTENSION),
               sizeof(global->name.savestate));
      }
      else
         RARCH_WARN("savestate_directory is not a directory, ignoring ...\n");
   }

   config_read_keybinds_conf(conf);


   config_file_free(conf);
   return true;
}

/**
 * config_load_override:
 *
 * Tries to append game-specific and core-specific configuration.
 * These settings will always have precedence, thus this feature
 * can be used to enforce overrides.
 *
 * This function only has an effect if a game-specific or core-specific
 * configuration file exists at respective locations.
 *
 * core-specific: $CONFIG_DIR/$CORE_NAME/$CORE_NAME.cfg fallback: $CURRENT_CFG_LOCATION/$CORE_NAME/$CORE_NAME.cfg
 * game-specific: $CONFIG_DIR/$CORE_NAME/$ROM_NAME.cfg fallback: $CURRENT_CFG_LOCATION/$CORE_NAME/$GAME_NAME.cfg
 *
 * Returns: false if there was an error or no action was performed.
 *
 */
bool config_load_override(void)
{
   char buf[PATH_MAX_LENGTH]              = {0};
   char config_directory[PATH_MAX_LENGTH] = {0};
   char core_path[PATH_MAX_LENGTH]        = {0};
   char game_path[PATH_MAX_LENGTH]        = {0};
   config_file_t *new_conf                = NULL;
   const char *core_name                  = NULL;
   const char *game_name                  = NULL;
   bool should_append                     = false;
   global_t *global                       = global_get_ptr();
   rarch_system_info_t *system            = NULL;

   runloop_ctl(RUNLOOP_CTL_SYSTEM_INFO_GET, &system);

   if (system)
      core_name = system->info.library_name;
   if (global)
      game_name = path_basename(global->name.base);

   if (string_is_empty(core_name) || string_is_empty(game_name))
      return false;

   fill_pathname_application_special(config_directory, sizeof(config_directory),
         APPLICATION_SPECIAL_DIRECTORY_CONFIG);

   /* Concatenate strings into full paths for core_path, game_path */
   fill_pathname_join_special_ext(game_path,
         config_directory, core_name,
         game_name,
         file_path_str(FILE_PATH_CONFIG_EXTENSION),
         sizeof(game_path));

   fill_pathname_join_special_ext(core_path,
         config_directory, core_name,
         core_name,
         file_path_str(FILE_PATH_CONFIG_EXTENSION),
         sizeof(core_path));

   /* Create a new config file from core_path */
   new_conf = config_file_new(core_path);

   /* If a core override exists, add its location to append_config_path */
   if (new_conf)
   {
      config_file_free(new_conf);

      RARCH_LOG("[overrides] core-specific overrides found at %s.\n", core_path);
      strlcpy(global->path.append_config, core_path, sizeof(global->path.append_config));

      should_append = true;
   }
   else
      RARCH_LOG("[overrides] no core-specific overrides found at %s.\n", core_path);

   /* Create a new config file from game_path */
   new_conf = config_file_new(game_path);

   /* If a game override exists, add it's location to append_config_path */
   if (new_conf)
   {
      config_file_free(new_conf);

      RARCH_LOG("[overrides] game-specific overrides found at %s.\n", game_path);
      if (should_append)
      {
         strlcat(global->path.append_config, "|", sizeof(global->path.append_config));
         strlcat(global->path.append_config, game_path, sizeof(global->path.append_config));
      }
      else
         strlcpy(global->path.append_config, game_path, sizeof(global->path.append_config));

      should_append = true;
   }
   else
      RARCH_LOG("[overrides] no game-specific overrides found at %s.\n", game_path);

   if (!should_append)
      return false;

   /* Re-load the configuration with any overrides that might have been found */
#ifdef HAVE_NETPLAY
   if (global->netplay.enable)
   {
      RARCH_WARN("[overrides] can't use overrides in conjunction with netplay, disabling overrides.\n");
      return false;
   }
#endif

   /* Store the libretro_path we're using since it will be 
    * overwritten by the override when reloading. */
   strlcpy(buf, config_get_active_core_path(), sizeof(buf));

   /* Toggle has_save_path to false so it resets */
   retroarch_override_setting_unset(RARCH_OVERRIDE_SETTING_STATE_PATH);
   retroarch_override_setting_unset(RARCH_OVERRIDE_SETTING_SAVE_PATH);

   if (!config_load_file(global->path.config, false, config_get_ptr()))
      return false;

   /* Restore the libretro_path we're using
    * since it will be overwritten by the override when reloading. */
   config_set_active_core_path(buf);
   runloop_msg_queue_push("Configuration override loaded.", 1, 100, true);

   /* Reset save paths. */
   retroarch_override_setting_set(RARCH_OVERRIDE_SETTING_STATE_PATH);
   retroarch_override_setting_set(RARCH_OVERRIDE_SETTING_SAVE_PATH);
   global->path.append_config[0] = '\0';
   return true;
}

/**
 * config_unload_override:
 *
 * Unloads configuration overrides if overrides are active.
 *
 *
 * Returns: false if there was an error.
 */
bool config_unload_override(void)
{
   global_t *global     = global_get_ptr();

   if (!global)
      return false;

   *global->path.append_config = '\0';

   /* Toggle has_save_path to false so it resets */
   retroarch_override_setting_unset(RARCH_OVERRIDE_SETTING_STATE_PATH);
   retroarch_override_setting_unset(RARCH_OVERRIDE_SETTING_SAVE_PATH);

   if (config_load_file(global->path.config, false, config_get_ptr()))
   {
      RARCH_LOG("[overrides] configuration overrides unloaded, original configuration restored.\n");

      /* Reset save paths */
      retroarch_override_setting_set(RARCH_OVERRIDE_SETTING_STATE_PATH);
      retroarch_override_setting_set(RARCH_OVERRIDE_SETTING_SAVE_PATH);

      return true;
   }

   return false;
}

/**
 * config_load_remap:
 *
 * Tries to append game-specific and core-specific remap files.
 *
 * This function only has an effect if a game-specific or core-specific
 * configuration file exists at respective locations.
 *
 * core-specific: $REMAP_DIR/$CORE_NAME/$CORE_NAME.cfg
 * game-specific: $REMAP_DIR/$CORE_NAME/$GAME_NAME.cfg
 *
 * Returns: false if there was an error or no action was performed.
 */
bool config_load_remap(void)
{
   char remap_directory[PATH_MAX_LENGTH]   = {0};    /* path to the directory containing retroarch.cfg (prefix)    */
   char core_path[PATH_MAX_LENGTH]         = {0};    /* final path for core-specific configuration (prefix+suffix) */
   char game_path[PATH_MAX_LENGTH]         = {0};    /* final path for game-specific configuration (prefix+suffix) */
   config_file_t *new_conf                 = NULL;
   const char *core_name                   = NULL;
   const char *game_name                   = NULL;
   global_t *global                        = global_get_ptr();
   settings_t *settings                    = config_get_ptr();
   rarch_system_info_t *system             = NULL;

   runloop_ctl(RUNLOOP_CTL_SYSTEM_INFO_GET, &system);

   if (system)
      core_name = system->info.library_name;
   if (global)
      game_name = path_basename(global->name.base);

   if (string_is_empty(core_name) || string_is_empty(game_name))
      return false;

   /* Remap directory: remap_directory.
    * Try remap directory setting, no fallbacks defined */
   if (string_is_empty(settings->directory.input_remapping))
      return false;

   strlcpy(remap_directory,
         settings->directory.input_remapping,
         sizeof(remap_directory));
   RARCH_LOG("Remaps: remap directory: %s\n", remap_directory);

   /* Concatenate strings into full paths for core_path, game_path */
   fill_pathname_join_special_ext(core_path,
         remap_directory, core_name,
         core_name,
         file_path_str(FILE_PATH_REMAP_EXTENSION),
         sizeof(core_path));

   fill_pathname_join_special_ext(game_path,
         remap_directory, core_name,
         game_name,
         file_path_str(FILE_PATH_REMAP_EXTENSION),
         sizeof(game_path));

   /* Create a new config file from game_path */
   new_conf = config_file_new(game_path);

   /* If a game remap file exists, load it. */
   if (new_conf)
   {
      RARCH_LOG("Remaps: game-specific remap found at %s.\n", game_path);
      if (input_remapping_load_file(new_conf, game_path))
      {
         runloop_msg_queue_push("Game remap file loaded.", 1, 100, true);
         return true;
      }
   }
   else
   {
      RARCH_LOG("Remaps: no game-specific remap found at %s.\n", game_path);
      input_remapping_set_defaults();
   }

   /* Create a new config file from core_path */
   new_conf = config_file_new(core_path);

   /* If a core remap file exists, load it. */
   if (new_conf)
   {
      RARCH_LOG("Remaps: core-specific remap found at %s.\n", core_path);
      if (input_remapping_load_file(new_conf, core_path))
      {
         runloop_msg_queue_push("Core remap file loaded.", 1, 100, true);
         return true;
      }
   }
   else
   {
      RARCH_LOG("Remaps: no core-specific remap found at %s.\n", core_path);
      input_remapping_set_defaults();
   }

   new_conf = NULL;

   return false;
}

static bool check_shader_compatibility(enum file_path_enum enum_idx)
{
   settings_t *settings = config_get_ptr();

   if (string_is_equal("vulkan", settings->video.driver))
   {
      if (enum_idx != FILE_PATH_SLANGP_EXTENSION)
         return false;
      return true;
   }

   if (string_is_equal("gl", settings->video.driver) || 
       string_is_equal("d3d9", settings->video.driver))
   {
      if (enum_idx == FILE_PATH_SLANGP_EXTENSION)
         return false;
      return true;
   }

   return false;
}

/**
 * config_load_shader_preset:
 *
 * Tries to append game-specific and core-specific shader presets.
 *
 * This function only has an effect if a game-specific or core-specific
 * configuration file exists at respective locations.
 *
 * core-specific: $SHADER_DIR/presets/$CORE_NAME/$CORE_NAME.cfg
 * game-specific: $SHADER_DIR/presets/$CORE_NAME/$GAME_NAME.cfg
 *
 * Returns: false if there was an error or no action was performed.
 */
bool config_load_shader_preset(void)
{
   unsigned idx;
   char shader_directory[PATH_MAX_LENGTH]   = {0};    /* path to the directory containing retroarch.cfg (prefix)    */
   char core_path[PATH_MAX_LENGTH]         = {0};    /* final path for core-specific configuration (prefix+suffix) */
   char game_path[PATH_MAX_LENGTH]         = {0};    /* final path for game-specific configuration (prefix+suffix) */
   const char *core_name                   = NULL;
   const char *game_name                   = NULL;
   global_t *global                        = global_get_ptr();
   settings_t *settings                    = config_get_ptr();
   rarch_system_info_t *system             = NULL;

   runloop_ctl(RUNLOOP_CTL_SYSTEM_INFO_GET, &system);

   if (system)
      core_name = system->info.library_name;
   if (global)
      game_name = path_basename(global->name.base);

   if (string_is_empty(core_name) || string_is_empty(game_name))
      return false;

   /* Shader directory: shader_directory.
    * Try shader directory setting, no fallbacks defined */
   if (string_is_empty(settings->directory.video_shader))
      return false;

   fill_pathname_join (shader_directory, settings->directory.video_shader,
       "presets", sizeof(shader_directory));

   RARCH_LOG("Shaders: preset directory: %s\n", shader_directory);

   for(idx = FILE_PATH_CGP_EXTENSION; idx < FILE_PATH_SLANGP_EXTENSION; idx++)
   {
      config_file_t *new_conf = NULL;

      if (!check_shader_compatibility((enum file_path_enum)(idx)))
         continue;
      /* Concatenate strings into full paths for core_path, game_path */
      fill_pathname_join_special_ext(core_path,
            shader_directory, core_name,
            core_name,
            file_path_str((enum file_path_enum)(idx)),
            sizeof(core_path));

      fill_pathname_join_special_ext(game_path,
            shader_directory, core_name,
            game_name,
            file_path_str((enum file_path_enum)(idx)),
            sizeof(game_path));

      /* Create a new config file from game_path */
      new_conf = config_file_new(game_path);

      if (!new_conf)
      {
         RARCH_LOG("Shaders: no game-specific preset found at %s.\n", game_path);
         continue;
      }

      /* Game shader preset exists, load it. */
      RARCH_LOG("Shaders: game-specific shader preset found at %s.\n", game_path);
      runloop_ctl(RUNLOOP_CTL_SET_DEFAULT_SHADER_PRESET, settings->path.shader);
      strlcpy(settings->path.shader, game_path, sizeof(settings->path.shader));
      config_file_free(new_conf);
      return true;
   }

   for(idx = FILE_PATH_CGP_EXTENSION; idx < FILE_PATH_SLANGP_EXTENSION; idx++)
   {
      config_file_t *new_conf = NULL;

      if (!check_shader_compatibility((enum file_path_enum)(idx)))
         continue;
      /* Concatenate strings into full paths for core_path, game_path */
      fill_pathname_join_special_ext(core_path,
            shader_directory, core_name,
            core_name,
            file_path_str((enum file_path_enum)(idx)),
            sizeof(core_path));

      /* Create a new config file from core_path */
      new_conf = config_file_new(core_path);

      if (!new_conf)
      {
         RARCH_LOG("Shaders: no core-specific preset found at %s.\n", core_path);
         continue;
      }

      /* Core shader preset exists, load it. */
      RARCH_LOG("Shaders: core-specific shader preset found at %s.\n", core_path);
      runloop_ctl(RUNLOOP_CTL_SET_DEFAULT_SHADER_PRESET, settings->path.shader);
      strlcpy(settings->path.shader, core_path, sizeof(settings->path.shader));
      config_file_free(new_conf);
      return true;
   }
   return false;
}

static void parse_config_file(void)
{
   global_t *global = global_get_ptr();
   bool         ret = config_load_file((*global->path.config)
         ? global->path.config : NULL, false, config_get_ptr());

   if (!string_is_empty(global->path.config))
   {
      RARCH_LOG("Config: loading config from: %s.\n", global->path.config);
   }
   else
   {
      RARCH_LOG("Loading default config.\n");
      if (!string_is_empty(global->path.config))
         RARCH_LOG("Config: found default config: %s.\n", global->path.config);
   }

   if (ret)
      return;

   RARCH_ERR("Config: couldn't find config at path: \"%s\"\n",
         global->path.config);
}


#if 0
static bool config_read_keybinds(const char *path)
{
   config_file_t *conf = (config_file_t*)config_file_new(path);

   if (!conf)
      return false;

   config_read_keybinds_conf(conf);
   config_file_free(conf);

   return true;
}
#endif

static void save_keybind_key(config_file_t *conf, const char *prefix,
      const char *base, const struct retro_keybind *bind)
{
   char key[64] = {0};
   char btn[64] = {0};

   fill_pathname_join_delim(key, prefix, base, '_', sizeof(key));

   input_keymaps_translate_rk_to_str(bind->key, btn, sizeof(btn));
   config_set_string(conf, key, btn);
}

static void save_keybind_hat(config_file_t *conf, const char *key,
      const struct retro_keybind *bind)
{
   char config[16]  = {0};
   unsigned hat     = GET_HAT(bind->joykey);
   const char *dir  = NULL;

   switch (GET_HAT_DIR(bind->joykey))
   {
      case HAT_UP_MASK:
         dir = "up";
         break;

      case HAT_DOWN_MASK:
         dir = "down";
         break;

      case HAT_LEFT_MASK:
         dir = "left";
         break;

      case HAT_RIGHT_MASK:
         dir = "right";
         break;

      default:
         retro_assert(0);
         break;
   }

   snprintf(config, sizeof(config), "h%u%s", hat, dir);
   config_set_string(conf, key, config);
}

static void save_keybind_joykey(config_file_t *conf, const char *prefix,
      const char *base, const struct retro_keybind *bind, bool save_empty)
{
   char key[64] = {0};

   fill_pathname_join_delim_concat(key, prefix,
         base, '_', "_btn", sizeof(key));

   if (bind->joykey == NO_BTN)
   {
       if (save_empty)
         config_set_string(conf, key, file_path_str(FILE_PATH_NUL));
   }
   else if (GET_HAT_DIR(bind->joykey))
      save_keybind_hat(conf, key, bind);
   else
      config_set_uint64(conf, key, bind->joykey);
}

static void save_keybind_axis(config_file_t *conf, const char *prefix,
      const char *base, const struct retro_keybind *bind, bool save_empty)
{
   char key[64]    = {0};
   unsigned axis   = 0;
   char dir        = '\0';

   fill_pathname_join_delim_concat(key,
         prefix, base, '_',
         "_axis",
         sizeof(key));

   if (bind->joyaxis == AXIS_NONE)
   {
      if (save_empty)
         config_set_string(conf, key, file_path_str(FILE_PATH_NUL));
   }
   else if (AXIS_NEG_GET(bind->joyaxis) != AXIS_DIR_NONE)
   {
      dir = '-';
      axis = AXIS_NEG_GET(bind->joyaxis);
   }
   else if (AXIS_POS_GET(bind->joyaxis) != AXIS_DIR_NONE)
   {
      dir = '+';
      axis = AXIS_POS_GET(bind->joyaxis);
   }

   if (dir)
   {
      char config[16] = {0};
      snprintf(config, sizeof(config), "%c%u", dir, axis);
      config_set_string(conf, key, config);
   }
}

/**
 * save_keybind:
 * @conf               : pointer to config file object
 * @prefix             : prefix name of keybind
 * @base               : base name   of keybind
 * @bind               : pointer to key binding object
 * @kb                 : save keyboard binds
 *
 * Save a key binding to the config file.
 */
static void save_keybind(config_file_t *conf, const char *prefix,
      const char *base, const struct retro_keybind *bind,
      bool save_kb, bool save_empty)
{
   if (!bind->valid)
      return;
   if (save_kb)
      save_keybind_key(conf, prefix, base, bind);
   save_keybind_joykey(conf, prefix, base, bind, save_empty);
   save_keybind_axis(conf, prefix, base, bind, save_empty);
}

/**
 * save_keybinds_user:
 * @conf               : pointer to config file object
 * @user               : user number
 *
 * Save the current keybinds of a user (@user) to the config file (@conf).
 */
static void save_keybinds_user(config_file_t *conf, unsigned user)
{
   unsigned i = 0;
   settings_t *settings = config_get_ptr();

   for (i = 0; input_config_bind_map_get_valid(i); i++)
   {
      const char *prefix = input_config_get_prefix(user,
            input_config_bind_map_get_meta(i));

      if (prefix)
         save_keybind(conf, prefix, input_config_bind_map_get_base(i),
               &settings->input.binds[user][i], true, true);
   }
}

/**
 * config_load:
 *
 * Loads a config file and reads all the values into memory.
 *
 */
void config_load(void)
{
   /* Flush out some states that could have been 
    * set by core environment variables */
   core_unset_input_descriptors();

   if (!rarch_ctl(RARCH_CTL_IS_BLOCK_CONFIG_READ, NULL))
   {
      config_set_defaults();
      parse_config_file();
   }
}

#if 0
/**
 * config_save_keybinds_file:
 * @path            : Path that shall be written to.
 *
 * Writes a keybinds config file to disk.
 *
 * Returns: true (1) on success, otherwise returns false (0).
 **/
static bool config_save_keybinds_file(const char *path)
{
   unsigned          i = 0;
   bool            ret = false;
   config_file_t *conf = config_file_new(path);

   if (!conf)
      conf = config_file_new(NULL);

   if (!conf)
      return false;

   RARCH_LOG("Saving keybinds config at path: \"%s\"\n", path);

   for (i = 0; i < MAX_USERS; i++)
      save_keybinds_user(conf, i);

   ret = config_file_write(conf, path);
   config_file_free(conf);
   return ret;
}
#endif


/**
 * config_save_autoconf_profile:
 * @path            : Path that shall be written to.
 * @user              : Controller number to save
 * Writes a controller autoconf file to disk.
 **/
bool config_save_autoconf_profile(const char *path, unsigned user)
{
   unsigned i;
   bool ret                             = false;
   char buf[PATH_MAX_LENGTH]            = {0};
   char autoconf_file[PATH_MAX_LENGTH]  = {0};
   config_file_t *conf                  = NULL;
   settings_t *settings                 = config_get_ptr();

   fill_pathname_join(buf, settings->directory.autoconfig,
         settings->input.joypad_driver, sizeof(buf));

   if(path_is_directory(buf))
   {
      char buf_new[PATH_MAX_LENGTH] = {0};

      fill_pathname_join(buf_new, buf,
            path, sizeof(buf_new));
      fill_pathname_noext(autoconf_file, buf_new,
            file_path_str(FILE_PATH_CONFIG_EXTENSION),
            sizeof(autoconf_file));
   }
   else
   {
      fill_pathname_join(buf, settings->directory.autoconfig,
            path, sizeof(buf));
      fill_pathname_noext(autoconf_file, buf,
            file_path_str(FILE_PATH_CONFIG_EXTENSION),
            sizeof(autoconf_file));
   }

   conf  = config_file_new(autoconf_file);

   if (!conf)
   {
      conf = config_file_new(NULL);
      if (!conf)
         return false;
   }

   config_set_string(conf, "input_driver",
         settings->input.joypad_driver);
   config_set_string(conf, "input_device",
         settings->input.device_names[user]);

   if(settings->input.vid[user] && settings->input.pid[user])
   {
      config_set_int(conf, "input_vendor_id",
            settings->input.vid[user]);
      config_set_int(conf, "input_product_id",
            settings->input.pid[user]);
   }

   for (i = 0; i < RARCH_FIRST_META_KEY; i++)
   {
      save_keybind(conf, "input", input_config_bind_map_get_base(i),
            &settings->input.binds[user][i], false, false);
   }

   ret = config_file_write(conf, autoconf_file);

   config_file_free(conf);

   return ret;
}


/**
 * config_save_file:
 * @path            : Path that shall be written to.
 *
 * Writes a config file to disk.
 *
 * Returns: true (1) on success, otherwise returns false (0).
 **/
bool config_save_file(const char *path)
{
   float msg_color;
   unsigned i           = 0;
   bool ret             = false;
   int bool_settings_size  = 0, int_settings_size    = 0,
       float_settings_size = 0, string_settings_size = 0,
       path_settings_size  = 0;
   struct config_bool_setting_ptr *bool_settings     = NULL;
   struct config_int_setting_ptr *int_settings       = NULL;
   struct config_float_setting_ptr *float_settings   = NULL;
   struct config_string_setting_ptr *string_settings = NULL;
   struct config_path_setting_ptr *path_settings     = NULL;
   config_file_t *conf  = config_file_new(path);
   settings_t *settings = config_get_ptr();
   global_t   *global   = global_get_ptr();

   if (!conf)
      conf = config_file_new(NULL);

   if (!conf || runloop_ctl(RUNLOOP_CTL_IS_OVERRIDES_ACTIVE, NULL))
   {
      if (conf)
         config_file_free(conf);
      return false;
   }

   bool_settings_size   = populate_settings_bool  (settings, &bool_settings);
   int_settings_size    = populate_settings_int   (settings, &int_settings);
   float_settings_size  = populate_settings_float (settings, &float_settings);
   string_settings_size = populate_settings_string(settings, &string_settings);
   path_settings_size   = populate_settings_path  (settings, &path_settings);


   /*
    * Path settings 
    *
    */

   for (i = 0; i < path_settings_size; i++)
   {
      if (path_settings[i].defaults)
         config_set_path(conf, path_settings[i].ident,
               string_is_empty(path_settings[i].value) ? "default" :
               path_settings[i].value);
      else
         config_set_path(conf, path_settings[i].ident,
               path_settings[i].value);
   }

#ifdef HAVE_MENU
   config_set_path(conf, "xmb_font",
         !string_is_empty(settings->menu.xmb.font) ? settings->menu.xmb.font : "");
#endif

   /*
    * String settings 
    *
    */

   for (i = 0; i < string_settings_size; i++)
   {
      config_set_string(conf, string_settings[i].ident,
            string_settings[i].value);
   }

   /*
    * Float settings 
    *
    */

   for (i = 0; i < float_settings_size; i++)
   {
      config_set_float(conf, float_settings[i].ident,
            *float_settings[i].ptr);
   }

   /*
    * Integer settings 
    *
    */

   for (i = 0; i < int_settings_size; i++)
   {
      config_set_int(conf, int_settings[i].ident,
            *int_settings[i].ptr);
   }

   for (i = 0; i < MAX_USERS; i++)
   {
      char cfg[64] = {0};

      snprintf(cfg, sizeof(cfg), "input_device_p%u", i + 1);
      config_set_int(conf, cfg, settings->input.device[i]);
      snprintf(cfg, sizeof(cfg), "input_player%u_joypad_index", i + 1);
      config_set_int(conf, cfg, settings->input.joypad_map[i]);
      snprintf(cfg, sizeof(cfg), "input_libretro_device_p%u", i + 1);
      config_set_int(conf, cfg, settings->input.libretro_device[i]);
      snprintf(cfg, sizeof(cfg), "input_player%u_analog_dpad_mode", i + 1);
      config_set_int(conf, cfg, settings->input.analog_dpad_mode[i]);
   }

   /*
    * Boolean settings 
    *
    */

   for (i = 0; i < bool_settings_size; i++)
   {
      config_set_bool(conf, bool_settings[i].ident,
            *bool_settings[i].ptr);
   }
#ifdef HAVE_NETWORKGAMEPAD
   for (i = 0; i < MAX_USERS; i++)
   {
      char tmp[64] = {0};
      snprintf(tmp, sizeof(tmp), "network_remote_enable_user_p%u", i + 1);
      config_set_bool(conf, tmp, settings->network_remote_enable_user[i]);
   }
#endif
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_UPS_PREF))
      config_set_bool(conf, "ups_pref", global->patch.ups_pref);
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_BPS_PREF))
      config_set_bool(conf, "bps_pref", global->patch.bps_pref);
   if (!retroarch_override_setting_is_set(RARCH_OVERRIDE_SETTING_IPS_PREF))
      config_set_bool(conf, "ips_pref", global->patch.ips_pref);
   config_set_bool(conf, "log_verbosity",
         verbosity_is_enabled());
   config_set_bool(conf, "perfcnt_enable",
         runloop_ctl(RUNLOOP_CTL_IS_PERFCNT_ENABLE, NULL));

   msg_color = (((int)(settings->video.msg_color_r * 255.0f) & 0xff) << 16) +
               (((int)(settings->video.msg_color_g * 255.0f) & 0xff) <<  8) +
               (((int)(settings->video.msg_color_b * 255.0f) & 0xff));

   /*
    * Hexadecimal settings 
    *
    */

   config_set_hex(conf, "video_message_color", msg_color);
#ifdef HAVE_MENU
   config_set_hex(conf, "menu_entry_normal_color",
         settings->menu.entry_normal_color);
   config_set_hex(conf, "menu_entry_hover_color",
         settings->menu.entry_hover_color);
   config_set_hex(conf, "menu_title_color",
         settings->menu.title_color);
#endif


   video_driver_save_settings(conf);

#ifdef HAVE_LAKKA
   if (settings->ssh_enable)
      fclose(fopen(LAKKA_SSH_PATH, "w"));
   else
      remove(LAKKA_SSH_PATH);
   if (settings->samba_enable)
      fclose(fopen(LAKKA_SAMBA_PATH, "w"));
   else
      remove(LAKKA_SAMBA_PATH);
   if (settings->bluetooth_enable)
      fclose(fopen(LAKKA_BLUETOOTH_PATH, "w"));
   else
      remove(LAKKA_BLUETOOTH_PATH);
#endif

   for (i = 0; i < MAX_USERS; i++)
      save_keybinds_user(conf, i);

   ret = config_file_write(conf, path);
   config_file_free(conf);

   free(bool_settings);
   free(int_settings);
   free(float_settings);
   free(string_settings);
   free(path_settings);

   return ret;
}

/**
 * config_save_overrides:
 * @path            : Path that shall be written to.
 *
 * Writes a config file override to disk.
 *
 * Returns: true (1) on success, otherwise returns false (0).
 **/
bool config_save_overrides(int override_type)
{
   unsigned i                                 = 0;
   int bool_settings_size   = 0, int_settings_size    = 0,
       float_settings_size  = 0, string_settings_size = 0,
       path_settings_size   = 0;
   bool ret                                    = false;
   char config_directory[PATH_MAX_LENGTH]      = {0};
   char override_directory[PATH_MAX_LENGTH]    = {0};
   char core_path[PATH_MAX_LENGTH]             = {0};
   char game_path[PATH_MAX_LENGTH]             = {0};
   const char *core_name                       = NULL;
   const char *game_name                       = NULL;
   config_file_t *conf                         = NULL;
   settings_t *settings                        = NULL;
   global_t   *global                          = global_get_ptr();
   settings_t *overrides                       = config_get_ptr();
   rarch_system_info_t *system                 = NULL;
   struct config_bool_setting_ptr *bool_settings    = NULL;
   struct config_bool_setting_ptr *bool_overrides   = NULL;
   struct config_int_setting_ptr *int_settings      = NULL;
   struct config_int_setting_ptr *int_overrides     = NULL;
   struct config_float_setting_ptr *float_settings  = NULL;
   struct config_float_setting_ptr *float_overrides = NULL;
   struct config_string_setting_ptr *string_settings = NULL;
   struct config_string_setting_ptr *string_overrides= NULL;
   struct config_path_setting_ptr *path_settings    = NULL;
   struct config_path_setting_ptr *path_overrides   = NULL;

   runloop_ctl(RUNLOOP_CTL_SYSTEM_INFO_GET, &system);

   if (system)
      core_name = system->info.library_name;
   if (global)
      game_name = path_basename(global->name.base);

   if (string_is_empty(core_name) || string_is_empty(game_name))
      return false;

   settings  = (settings_t*)calloc(1, sizeof(settings_t));

   fill_pathname_application_special(config_directory, sizeof(config_directory),
         APPLICATION_SPECIAL_DIRECTORY_CONFIG);

   fill_pathname_join(override_directory, config_directory, core_name, 
      sizeof(override_directory));

   if(!path_file_exists(override_directory))
       path_mkdir(override_directory);

   /* Concatenate strings into full paths for core_path, game_path */
   fill_pathname_join_special_ext(game_path,
         config_directory, core_name,
         game_name,
         file_path_str(FILE_PATH_CONFIG_EXTENSION),
         sizeof(game_path));

   fill_pathname_join_special_ext(core_path,
         config_directory, core_name,
         core_name,
         file_path_str(FILE_PATH_CONFIG_EXTENSION),
         sizeof(core_path));

   if (!conf)
      conf = config_file_new(NULL);

   /* Load the original config file in memory */
   config_load_file(global->path.config, false, settings);

   bool_settings_size =  populate_settings_bool(settings, &bool_settings);
   populate_settings_bool (overrides, &bool_overrides);
   int_settings_size  = populate_settings_int(settings, &int_settings);
   populate_settings_int (overrides, &int_overrides);
   float_settings_size = populate_settings_float(settings, &float_settings);
   populate_settings_float (overrides, &float_overrides);
   string_settings_size = populate_settings_string(settings, &string_settings);
   populate_settings_string (overrides, &string_overrides);
   path_settings_size = populate_settings_path(settings, &path_settings);
   populate_settings_path (overrides, &path_overrides);

   RARCH_LOG("[overrides] looking for changed settings... \n");

   for (i = 0; i < bool_settings_size; i++)
   {
      if ((*bool_settings[i].ptr) != (*bool_overrides[i].ptr))
      {
         RARCH_LOG("   original: %s=%d\n", 
            bool_settings[i].ident, (*bool_settings[i].ptr));
         RARCH_LOG("   override: %s=%d\n", 
            bool_overrides[i].ident, (*bool_overrides[i].ptr));
         config_set_bool(conf, bool_overrides[i].ident,
            (*bool_overrides[i].ptr));
      }
   }
   for (i = 0; i < int_settings_size; i++)
   {
      if ((*int_settings[i].ptr) != (*int_overrides[i].ptr))
      {
         RARCH_LOG("   original: %s=%d\n", 
            int_settings[i].ident, (*int_settings[i].ptr));
         RARCH_LOG("   override: %s=%d\n", 
            int_overrides[i].ident, (*int_overrides[i].ptr));
         config_set_int(conf, int_overrides[i].ident,
               (*int_overrides[i].ptr));
      }
   }
   for (i = 0; i < float_settings_size; i++)
   {
      if ((*float_settings[i].ptr) != (*float_overrides[i].ptr))
      {
         RARCH_LOG("   original: %s=%f\n", 
            float_settings[i].ident, *float_settings[i].ptr);
         RARCH_LOG("   override: %s=%f\n", 
            float_overrides[i].ident, *float_overrides[i].ptr);
         config_set_float(conf, float_overrides[i].ident,
            *float_overrides[i].ptr);
      }
   }
   for (i = 0; i < string_settings_size; i++)
   {
      if (!string_is_equal(string_settings[i].value, string_overrides[i].value))
      {
         RARCH_LOG("   original: %s=%s\n", 
            string_settings[i].ident, string_settings[i].value);
         RARCH_LOG("   override: %s=%s\n", 
            string_overrides[i].ident, string_overrides[i].value);
         config_set_string(conf, string_overrides[i].ident,
            string_overrides[i].value);
      }
   }
   for (i = 0; i < path_settings_size; i++)
   {
      if (!string_is_equal(path_settings[i].value, path_overrides[i].value))
      {
         RARCH_LOG("   original: %s=%s\n", 
            path_settings[i].ident, path_settings[i].value);
         RARCH_LOG("   override: %s=%s\n", 
            path_overrides[i].ident, path_overrides[i].value);
         config_set_path(conf, path_overrides[i].ident,
               path_overrides[i].value);
      }
   }

   if (override_type == OVERRIDE_CORE)
   {
      RARCH_LOG ("[overrides] path %s\n", core_path);
      /* Create a new config file from core_path */
      ret = config_file_write(conf, core_path);
      config_file_free(conf);
   }
   else if(override_type == OVERRIDE_GAME)
   {
      RARCH_LOG ("[overrides] path %s\n", game_path);
      /* Create a new config file from core_path */
      ret = config_file_write(conf, game_path);
      config_file_free(conf);
   }
   else
      ret = false;

   free(bool_settings);
   free(bool_overrides);
   free(int_settings);
   free(int_overrides);
   free(float_settings);
   free(float_overrides);
   free(string_settings);
   free(string_overrides);
   free(path_settings);
   free(path_overrides);
   free(settings);

   return ret;
}

/* Replaces currently loaded configuration file with
 * another one. Will load a dummy core to flush state
 * properly. */
bool config_replace(char *path)
{
   content_ctx_info_t content_info = {0};
   settings_t *settings            = config_get_ptr();
   global_t     *global            = global_get_ptr();

   if (!path || !global)
      return false;

   /* If config file to be replaced is the same as the
    * current config file, exit. */
   if (string_is_equal(path, global->path.config))
      return false;

   if (settings->config_save_on_exit && !string_is_empty(global->path.config))
      config_save_file(global->path.config);

   strlcpy(global->path.config, path, sizeof(global->path.config));

   rarch_ctl(RARCH_CTL_UNSET_BLOCK_CONFIG_READ, NULL);

   /* Load core in new config. */
   config_clear_active_core_path();

   if (!task_push_content_load_default(
         NULL, NULL,
         &content_info,
         CORE_TYPE_DUMMY,
         CONTENT_MODE_LOAD_NOTHING_WITH_DUMMY_CORE,
         NULL, NULL))
      return false;

   return true;
}

static char path_libretro[PATH_MAX_LENGTH];

char *config_get_active_core_path_ptr(void)
{
   return path_libretro;
}

const char *config_get_active_core_path(void)
{
   return path_libretro;
}

bool config_active_core_path_is_empty(void)
{
   return !path_libretro[0];
}

size_t config_get_active_core_path_size(void)
{
   return sizeof(path_libretro);
}

void config_set_active_core_path(const char *path)
{
   strlcpy(path_libretro, path, sizeof(path_libretro));
}

void config_clear_active_core_path(void)
{
   *path_libretro = '\0';
}

const char *config_get_active_path(void)
{
   global_t   *global          = global_get_ptr();

   if (!string_is_empty(global->path.config))
      return global->path.config;

   return NULL;
}

void config_free_state(void)
{
}