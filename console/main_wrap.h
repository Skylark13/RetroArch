/*  SSNES - A Super Nintendo Entertainment System (SNES) Emulator frontend for libsnes.
 *  Copyright (C) 2010-2012 - Hans-Kristian Arntzen
 *
 *  Some code herein may be based on code found in BSNES.
 * 
 *  SSNES is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  SSNES is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with SSNES.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAIN_WRAP_H__
#define MAIN_WRAP_H__

#ifndef __cplusplus
#include <stdbool.h>
#endif

// Builds argc/argv and calls ssnes_main_init().

struct ssnes_main_wrap
{
   const char *rom_path;
   const char *sram_path;
   const char *state_path;
   const char *config_path;
   bool verbose;
};

int ssnes_main_init_wrap(const struct ssnes_main_wrap *args);

#endif
