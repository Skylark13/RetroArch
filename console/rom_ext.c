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

#include "rom_ext.h"
#include "../boolean.h"
#include "../libsnes.hpp"
#include <string.h>

const char *ssnes_console_get_rom_ext(void)
{
   const char *id = snes_library_id();

   // SNES9x / bSNES
   if (strstr(id, "SNES"))
      return "smc|fig|sfc|gd3|gd7|dx2|bsx|swc|zip|SMC|FIG|SFC|BSX|GD3|GD7|DX2|SWC|ZIP";
   // FCEU Next
   else if (strstr(id, "FCEU"))
      return "fds|FDS|zip|ZIP|nes|NES|unif|UNIF";
   // VBA Next / Meteor
   else if (strstr(id, "VBA") || strstr(id, "Meteor"))
      return "gb|gbc|gba|GBA|GB|GBC|zip|ZIP";
   // Gambatte
   else if (strstr(id, "gambatte"))
      return "gb|gbc|GB|GBC|zip|ZIP";
   // FBA Next
   else if (strstr(id, "FBA"))
      return "zip|ZIP";
   // Genesis Plus GX/Next
   else if (strstr(id, "Genesis Plus GX"))
      return "md|smd|bin|gen|zip|MD|SMD|bin|GEN|ZIP|sms|SMS|gg|GG|sg|SG";

   return NULL;
}
