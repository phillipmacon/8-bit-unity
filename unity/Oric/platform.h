/*
 *	API of the "8bit-Unity" SDK for CC65
 *	All functions are cross-platform for the Apple IIe, Atari XL/XE, and C64/C128
 *	
 *	Last modified: 2019/07/12
 *	
 * Copyright (c) 2019 Anthony Beaucamp.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented * you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 *	Credits: 
 *		* Oliver Schmidt for his IP65 network interface
 *		* Christian Groessler for helping optimize the memory maps on Commodore and Atari
 *		* Bill Buckels for his Apple II Double Hi-Res bitmap code
 */
 
// Memory Map
#define MUSICBUF   (0x8200) // 8200-8fff (music buffer used by player)
#define MUSICRAM   (0x9000) // 9000-97ff (compressed music data)
#define SPRITERAM  (0x9C00)	// 9C00-9fff (location of sprite frames)
#define BITMAPRAM  (0xA000) // A000-Bfff (address where bitmaps are loaded)

// Character Mode
#define CHR_COLS 39
#define CHR_ROWS 25

// Bitmap Mode (AIC)
#define BMP_COLS 117
#define BMP_ROWS 100
#define BMP_PALETTE 20

// Bitmap Palette
#define BLACK   0
#define MGREEN	1
#define GREEN	1
#define DGREEN	2
#define CYAN	3
#define LGREEN	4
#define AIC		4
#define GREY	5
#define MBLUE	6
#define DBLUE	7
#define LBLUE 	8
#define AQUA    9
#define GREY2	10
#define BROWN	11
#define YELLOW	12
#define RED		13
#define ORANGE  14
#define WHITE   15
#define DPINK   16
#define LPURPLE 17
#define LPINK   18
#define PURPLE  19

// Workaround for file reading
#if defined __ATMOS__
  int SedoricWrite(const char* fname, void* buf, int len);
  int SedoricRead(const char* fname, void* buf);
#endif