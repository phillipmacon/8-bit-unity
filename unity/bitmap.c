/*
 * Copyright (c) 2018 Anthony Beaucamp.
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
 */

#include "unity.h"

#ifdef __APPLE2__
  #pragma code-name("LC")
#endif

#ifdef __ATARIXL__
  #pragma code-name("SHADOW_RAM")
#endif

// Apple specific variables & functions
#ifdef __APPLE2__
  extern void RestoreSprLine(unsigned char x, unsigned char y);
#endif

// Lynx specific variables & functions
#ifdef __LYNX__   
  unsigned char videoInit = 0;	 
  static int palette[] =  { 0x01ca, 0x03b4, 0x08c4, 0x0cc3, 0x0c53, 0x0822, 0x0552, 0x0527, 
							0x075e, 0x0e0f, 0x09af, 0x034d, 0x0248, 0x0fff, 0x0888, 0x0000 };
  void __fastcall__ SuzyInit(void);
#endif

// C64 specific variables & functions
#ifdef __CBM__
  int vicconf[3];
  unsigned char bg = 0;
  void SwitchBank(char bank) {
	// notice that 0 selects vic bank 3, 1 selects vic bank 2, etc.
	POKE(0xDD00, (PEEK(0xDD00) & 252 | (3 - bank))); // switch VIC base
  }
#endif

// Initialize Bitmap Screen
void InitBitmap() 
{
#if defined __APPLE2__
	// Prepare Double Hi-Res Mode
	asm("sta $c052"); // TURN ON FULLSCREEN       
	asm("sta $c057"); // TURN ON HI-RES           
	asm("sta $c001"); // TURN ON 80 STORE
#elif defined __ATARI__
	// Set default palette
	POKE(PALETTERAM+0, 0x00);
	POKE(PALETTERAM+1, 0x24);
	POKE(PALETTERAM+2, 0x86);
	POKE(PALETTERAM+3, 0xd8);
#elif defined __ORIC__
	// Switch to Hires mode
	if PEEK((char*)0xC800) {
		asm("jsr $EC33");	// Atmos (ROM 1.1)
	} else {
		asm("jsr $E9BB");	// Oric-1 (ROM 1.0)
	}
	memset((char*)0xBF68, 0, 120);	// Clear text area
#elif defined __CBM__
	// Backup VIC config
	vicconf[0] = PEEK(53272);
	vicconf[1] = PEEK(53265);
	vicconf[2] = PEEK(53270);
#elif defined __LYNX__
	// Did we already initialize?
	unsigned char i;
	if (videoInit) { return; }
	
	// Install drivers (and set interrupts)
	SuzyInit();
	lynx_snd_init(); 	
	__asm__("cli");	
	
	// Reset palette
	for (i=0; i<16; i++) {
		POKE(0xFDA0+i, palette[i] >> 8);
		POKE(0xFDB0+i, palette[i] & 0xFF);
	}
	
	// Set flag
	videoInit = 1;
#endif	
}

// Switch from Text mode to Bitmap mode
void EnterBitmapMode()
{		
#if defined __CBM__
	// Set data direction flag and Switch bank
	POKE(0xDD02, (PEEK(0xDD02) | 3));
	SwitchBank(VIDEOBANK);

	// Enter multicolor mode
	POKE(0xD018, SCREENLOC*16 + BITMAPLOC);	// 53272: address of screen and bitmap RAM
	POKE(0xD011, PEEK(0xD011) | 32);		// 53265: set bitmap mode
	POKE(0xD016, PEEK(0xD016) | 16);		// 53270: set multicolor mode
#elif defined __ATARI__
	// Assign palette
	POKE(0x02c8, PEEK(PALETTERAM+0));
	POKE(0x02c4, PEEK(PALETTERAM+1));
	POKE(0x02c5, PEEK(PALETTERAM+2));
	POKE(0x02c6, PEEK(PALETTERAM+3));	
	
	// Switch ON graphic mode and antic
	__asm__("jsr %w", STARTBMP);			
	POKE(559, 32+16+8+4+2); // ANTIC: DMA Screen + Enable P/M + DMA Players + DMA Missiles + Single resolution
#elif defined __APPLE2__
	// Switch ON Double Hi-Res Mode
	asm("sta $c00d"); // TURN ON 80 COLUMN MODE	  
    asm("sta $c050"); // TURN ON GRAPHICS         
    asm("sta $c05e"); // TURN ON DOUBLE HI-RES
#endif
}

// Switch from Bitmap mode to Text mode
void ExitBitmapMode()
{
#if defined __CBM__
	// Return VIC and Bank back to previous state
	POKE(53272, vicconf[0]);
	POKE(53265, vicconf[1]);
	POKE(53270, vicconf[2]);
	SwitchBank(0);
#elif defined __ATARI__
    // Switch OFF graphic mode and antic
	__asm__("jsr %w", STOPBMP);
	POKE(559, 2);
#elif defined __APPLE2__
    // Switch OFF Double Hi-Res Mode
    asm("sta $c051"); // TEXT - HIDE GRAPHICS
    asm("sta $c05f"); // TURN OFF DOUBLE RES
	asm("sta $c00c"); // TURN OFF 80 COLUMN MODE	  
#endif
}

// Clear entire bitmap screen
void ClearBitmap()
{
#if defined __APPLE2__
    // clear main and aux screen memory	
	*dhraux = 0;
    bzero((char *)BITMAPRAM, 8192);
	*dhrmain = 0;
    bzero((char *)BITMAPRAM, 8192);
#elif defined __ATARI__
	bzero((char*)BITMAPRAM1, 8000);
	bzero((char*)BITMAPRAM2, 8000);
#elif defined __ORIC__
	// reset pixels and set AIC Paper/Ink
	unsigned char y;
	memset((char*)BITMAPRAM, 64, 8000);	
    for (y=0; y<200; y++) {
		POKE((char*)BITMAPRAM+y*40, (y%2) ? 6 : 3);
	}	
#elif defined __CBM__
	bzero((char*)BITMAPRAM, 8000);
	bzero((char*)SCREENRAM, 1000);
	bzero((char*)COLORRAM,  1000);
#elif defined __LYNX__
	unsigned int i;
	memset(BITMAPRAM, 0xff, 8364); 
	for (i=0; i<102; i++) { 
		POKE((char*)BITMAPRAM+i*82,    0x52); 
		POKE((char*)BITMAPRAM+i*82+81, 0x00); 
	}
	UpdateDisplay();
#endif
}

// Load bitmap from file
void LoadBitmap(char *filename) 
{
#if defined __ORIC__
	// Load directly to bitmap ram
	FileRead(filename, (void*)(BITMAPRAM));
#elif defined __LYNX__
	// Load from CART file system
	if (FileLoad(filename) && autoRefresh) 
		UpdateDisplay();	
#else	
	// Open Map File
	FILE* fp;
	fp = fopen(filename, "rb");	
  #if defined __CBM__
	// Consume two bytes of header
	fgetc(fp); fgetc(fp);
	
	// 8000 bytes bitmap ram
	fread((char*)(BITMAPRAM), 1, 8000, fp);

	// 1000 bytes char ram
	fread((char*)(SCREENRAM), 1, 1000, fp);
	
	// 1000 bytes color ram
	fread((char*)(COLORRAM), 1, 1000, fp);
	
	// 1 byte background color
	bg = (char) fgetc(fp);	
  #elif defined __ATARI__	
	// 4 bytes palette ram
	fread((char*)PALETTERAM, 1, 4, fp);
	
	// 8000 bytes RAM1 (color 1)
	fread((char*)BITMAPRAM1, 1, 8000, fp);
	
	// 8000 bytes RAM2 (color 2)
	fread((char*)BITMAPRAM2, 1, 8000, fp);
  #elif defined __APPLE2__
	// Read 8192 bytes to AUX
	*dhraux = 0;
	fread((char*)BITMAPRAM, 1, 8192, fp);
	
	// Read 8192 bytes to MAIN
	*dhrmain = 0;
	fread((char*)BITMAPRAM, 1, 8192, fp);
  #endif
	// Close file
	fclose(fp);
#endif
}

// Location of current pixel 
unsigned char pixelX, pixelY;

void LocatePixel(unsigned int x, unsigned int y)
{
// This function maps pixel coordinates from a 320x200 screen definition
// It can be by-passed by assigning pixelX, pixelY directly in your code
#if defined __APPLE2__	// DHR Mode: 140 x 192
	pixelX = (x*140)/320;
	pixelY = (y*192)/200;
#elif defined __ATARI__	// INP Mode: 160 x 200
	pixelX = x/2;
	pixelY = y;
#elif defined __ORIC__	// AIC Mode: 117 x 100 equivalent pixels
	pixelX = (x*117)/320;	
	pixelY = y/2;
#elif defined __CBM__	// MLC Mode: 160 x 200
	pixelX = x/2;
	pixelY = y;
#elif defined __LYNX__	// STD Mode: 160 x 102
	pixelX = x/2;
	pixelY = (y*102)/200;
#endif
}

unsigned char GetPixel()
{
#if defined __CBM__
	unsigned char index;
	unsigned int addr, offset;
	
	// Check color index
	DisableRom();
	addr = BITMAPRAM + 40*(pixelY&248)+(pixelY&7)+((pixelX*2)&504);
	index = (PEEK(addr) >> (2*(3-(pixelX%4)))) & 3;
	EnableRom();
	
	// Is background color?
	if (index==0) { return bg; }
	
	// Analyze color index
	offset = (pixelY/8)*40+(pixelX/4);
	if (index==1) {	// Upper bits of screen RAM
		addr = SCREENRAM + offset;
		return (PEEK(addr) & 0xF0) >> 4;		
	}
	if (index==2) {	// Lower bits of screen RAM
		addr = SCREENRAM + offset;
		return (PEEK(addr) & 0x0F);
	}
	if (index==3) { // Lower bits of color RAM
		addr = COLORRAM + offset;
		return (PEEK(addr) & 0x0F);
	}
#elif defined __ATARI__
	unsigned int offset;
	unsigned char val1, val2, shift;
	
	// Compute pixel location
	offset = 40*pixelY+pixelX/4;
	shift = 6 - 2*(pixelX%4);

	// Dual buffer (colour/shade)
	val1 = (PEEK((char*)BITMAPRAM1+offset) & ( 3 << shift )) >> shift;
	val2 = (PEEK((char*)BITMAPRAM2+offset) & ( 3 << shift )) >> shift;
	if (val1 > val2) {
		return val1*4 + val2;
	} else {
		return val2*4 + val1;
	}
#elif defined __APPLE2__
	// Use DHR routines
	RestoreSprLine(pixelX,pixelY);
	SetDHRPointer(pixelX,pixelY);
	return GetDHRColor();
#elif defined __ORIC__
	unsigned int addr;
	unsigned char i, pX, pY, xO, yO, occlusion = 0;
	unsigned char byte1, byte2, shift, color = 0;
	extern unsigned char sprDrawn[SPRITE_NUM];
	extern unsigned char* sprBG[SPRITE_NUM];
	extern unsigned char sprX[SPRITE_NUM];
	extern unsigned char sprY[SPRITE_NUM];
	extern unsigned char frameROWS;

	// Scale to block coordinates (6x2)
	pX = pixelX/3+1; pY = pixelY*2;
	
	// Check for sprite occlusion
	for (i=0; i<SPRITE_NUM; i++) {
		if (sprDrawn[i]) {
			xO = pX - (sprX[i]);
			yO = pY - (sprY[i]);
			if (xO<3 && yO<frameROWS) {
				addr = sprBG[i]+yO*4+xO;
				byte1 = PEEK(addr);
				byte2 = PEEK(addr+4);	
				occlusion = 1;
				break;
			}
		}
	}
	
	// Get 2 bytes from Bitmap RAM (interlaced lines)
	if (!occlusion) {
		addr = (char*)BITMAPRAM + pY*40 + pX;		
		byte1 = PEEK(addr);
		byte2 = PEEK(addr+40);	
	}
	
	// Get PAPER/INK inversion group
	if (byte1 & 128) { color += 5; }
	if (byte2 & 128) { color += 10; }
		
	// Get pixels state
	shift = 2 * (pixelX%3);
	byte1 = (byte1 << shift) & 48;
	byte2 = (byte2 << shift) & 48;
	switch (byte1) {
	case 0:
		if (byte2 == 48) { color += 3; }
		break;
	case 32:
		color += 1;
		break;
	case 48:
		if (byte2 == 48) { color += 4; } else { color += 2; }
		break;	
	}
	return color;
#elif defined __LYNX__
	unsigned int addr;
	addr = BITMAPRAM + pixelY*82 + pixelX/2 + 1;
	if (pixelX%2) { 
		return (PEEK((char*)addr) & 15);
	} else {
		return (PEEK((char*)addr) & 240) >> 4;
	}	
#endif	
}

void SetPixel(unsigned char color)
{
#if defined __CBM__
	unsigned int offset;
	unsigned char shift;
	
	// Set index to 3
	DisableRom();
	offset = 40*(pixelY&248)+(pixelY&7)+((pixelX*2)&504);
	shift = (2*(3-(pixelX%4)));
	POKE(BITMAPRAM+offset, PEEK(BITMAPRAM+offset) | 3 << shift);
	EnableRom();
	
	// Set color in COLORAM
	offset = (pixelY/8)*40+(pixelX/4);
	POKE(COLORRAM+offset, color);
#elif defined __ATARI__
	unsigned int offset;
	unsigned char shift, mask, col1, col2;	

	// Compute pixel location
	offset = 40*pixelY + pixelX/4;
	shift = 6 - 2*(pixelX%4);
	mask = 255 - (3 << shift);
	if ((pixelY+pixelX)%2) {
		col2 = (color%4) << shift;
		col1 = (color/4) << shift;
	} else {
		col1 = (color%4) << shift;
		col2 = (color/4) << shift;
	}

	// Dual buffer (colour/shade)
	POKE((char*)BITMAPRAM1+offset, (PEEK((char*)BITMAPRAM1+offset) & mask) | col1);
	POKE((char*)BITMAPRAM2+offset, (PEEK((char*)BITMAPRAM2+offset) & mask) | col2);
#elif defined __APPLE2__
	// Use DHR routines
	SetDHRPointer(pixelX,pixelY);
	SetDHRColor(color);	
#elif defined __ORIC__
	unsigned int offset;
	unsigned char byte1, byte2, shift;
	
	// Compute pixel offset
	offset = pixelY*80 + pixelX/3 + 1;
	
	// Get bytes from Bitmap RAM
	byte1 = PEEK((char*)BITMAPRAM+offset) & 63;
	byte2 = PEEK((char*)BITMAPRAM+offset+40) & 63;	
	
	// Set PAPER/INK inversion
	switch (color/5) {
	case 0:
		byte1 |= 64;
		byte2 |= 64;
		break;
	case 1:
		byte1 |= 192;
		byte2 |= 64;
		break;
	case 2:
		byte1 |= 64;
		byte2 |= 192;
		break;
	case 3:
		byte1 |= 192;
		byte2 |= 192;	
		break;
	}
	
	// Set pixels
	shift = 2 * (pixelX%3);
	byte1 &= ~(48 >> shift);
	byte2 &= ~(48 >> shift);
	switch (color%5) {
    case 1:
		byte1 |= 32 >> shift;
		byte2 |= 16 >> shift;
		break;
	case 2:
		byte1 |= 48 >> shift;
		break;
	case 3:
		byte2 |= 48 >> shift;
		break;
	case 4:
		byte1 |= 48 >> shift;
		byte2 |= 48 >> shift;
		break;
	}
	
	// Assign bytes in Bitmap RAM
	POKE((char*)BITMAPRAM+offset,    byte1);
	POKE((char*)BITMAPRAM+offset+40, byte2);
#elif defined __LYNX__
	unsigned char* addr = (char*)BITMAPRAM + pixelY*82 + pixelX/2 + 1;
	if (pixelX%2) { 
		*addr &= 240;
		*addr |= color;
	} else {
		*addr &= 15;
		*addr |= color << 4;
	}
#endif
}
