#include "FRM_Convert.h"
#include "FRM_Animate.h"
#include "B_Endian.h"

#include <cstdint>
#include <fstream>
#include <vector>
#include <SDL.h>

uint8_t convert_colors(uint8_t bytes);

// Used to convert Fallout's palette colors to normal values
uint8_t convert_colors(uint8_t bytes) {
	if (bytes < 64) {
		return 4 * bytes;
	}
	else {
		return bytes;
	}
}

SDL_Color PaletteColors[256];
SDL_Color* loadPalette(char * name)
{
	std::ifstream f("color.pal", 
		std::ios::in | std::ios::binary);
	if (!f.is_open()) {
		printf("Error opening color.pal.\n");
		return false;
	}

	uint8_t r, g, b;
	printf("Palette size: %d\n", 256);
	for (int i = 0; i < 256; i++)
	{
		uint8_t bytes[4];

		f.read((char*)bytes, 3);
		r = convert_colors(bytes[0]);
		g = convert_colors(bytes[1]);
		b = convert_colors(bytes[2]);
		//r = B_Endian::read_u8(f);
		//g = B_Endian::read_u8(f);
		//b = B_Endian::read_u8(f);
		//printf("RGB: %d, %d, %d\n", r, g, b);
		PaletteColors[i] = SDL_Color{ r, g, b };
	}
	return PaletteColors;
}

// Converts the color space to Fallout's paletted format
SDL_Surface* FRM_Color_Convert(SDL_Surface *surface)
{
	struct abgr
	{
		uint8_t a;
		uint8_t b;
		uint8_t g;
		uint8_t r;
	} abgr_;
	
	SDL_Palette myPalette;
	SDL_PixelFormat pxlFMT;
	SDL_Surface* Temp_Surface;
	myPalette.ncolors = 256;
	myPalette.colors = loadPalette("default");

	pxlFMT = *SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);
	pxlFMT.palette = &myPalette;
	pxlFMT.BitsPerPixel = 8;
	pxlFMT.BytesPerPixel = 1;
	int k = (surface->w)*(surface->h);

	// TODO: need to figure out what command is best to create a blank surface
	//Temp_Surface = SDL_CreateRGBSurfaceFrom(surface->pixels, surface->w, surface->h, surface->format->BitsPerPixel, surface->pitch, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
	Temp_Surface = SDL_ConvertSurface(surface, &pxlFMT, 0);
	//Temp_Surface = SDL_CreateRGBSurface(0, surface->w, surface->h, 8, 0, 0, 0, 0);

	if (!Temp_Surface) {
		printf("Error: %s", SDL_GetError());
	}

	for (int i = 0; i < k; i++) 
	{
		memcpy(&abgr_, (abgr*)surface->pixels + i, sizeof(abgr));
		int w_smallest = INT_MAX;
		uint8_t w_PaletteColor;
		w_PaletteColor = SDL_MapRGBA(&pxlFMT, abgr_.r, abgr_.g, abgr_.b, abgr_.a);
		//for (int j = 0; j < 256; j++)
		//{
		//	int s = (abgr_.r - PaletteColors[j].r);
		//	int t = (abgr_.g - PaletteColors[j].g);
		//	int u = (abgr_.b - PaletteColors[j].b);
		//	int v = (abgr_.a - PaletteColors[j].a);
		//	s *= s;
		//	t *= t;
		//	u *= u;
		//	v *= v;
		//	int w = sqrt(s + t + u + v);
		//	if (w < w_smallest) { 
		//		w_smallest = w; 
		//		w_PaletteColor = j;
		//	}
		//	//printf("stuv: %d %d %d %d", s, t, u, v);
		//}
		if (i == 100)		{ printf("loop #: %d\n", i); }
		if (i == 1000)		{ printf("loop #: %d\n", i); }
		if (i == 10000)		{ printf("loop #: %d\n", i); }
		if (i == 100000)	{ printf("loop #: %d\n", i); }
		if (i == 1000000)	{ printf("loop #: %d\n", i); }
		if (i == 2000000)	{ printf("loop #: %d\n", i); }
		((uint8_t*)Temp_Surface->pixels)[i]  = w_PaletteColor;
		//printf("w_PaletteColor: %d\n", w_PaletteColor);
	}
	return Temp_Surface;
}

// Convert FRM color to standard 32bit? maybe?
SDL_Surface* BMP_Color_Convert(LF *F_Prop)
{
	struct abgr
	{
		uint8_t a;
		uint8_t b;
		uint8_t g;
		uint8_t r;
	} abgr_;
	SDL_Palette myPalette;
	SDL_PixelFormat pxlFMT_Pal;
	SDL_PixelFormat pxlFMT_UnPal;

	myPalette.ncolors = 256;
	myPalette.colors  = loadPalette("default");
	
	// 8 bit palleted stuff here
	pxlFMT_Pal = *SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);
	pxlFMT_Pal.palette = &myPalette;
	pxlFMT_Pal.BitsPerPixel = 8;
	pxlFMT_Pal.BytesPerPixel = 1;

	FILE *ptr = fopen(F_Prop->Opened_File, "rb");
	fseek(ptr, 0x3E, SEEK_SET);

	unsigned short frame_width = 0;
	fread(&frame_width, 2, 1, ptr);
	frame_width = B_Endian::write_u16(frame_width);

	unsigned short frame_height = 0;
	fread(&frame_height, 2, 1, ptr);
	frame_height = B_Endian::write_u16(frame_height);

	unsigned int frame_size = 0;
	fread(&frame_size, 4, 1, ptr);
	frame_size = B_Endian::write_u32(frame_size);

	fseek(ptr, 0x4A, SEEK_SET);
	SDL_Surface* Paletted_Surface = SDL_CreateRGBSurface(0, frame_width, frame_height, 8, 0,0,0,0);
	if (!Paletted_Surface)
	{
	printf("Error: %s\n", SDL_GetError());
	}
	SDL_Surface* Output_Surface   = SDL_CreateRGBSurface(0, frame_width, frame_height, 32, 0,0,0,0);
	printf("Error: %s\n", SDL_GetError());
	fread(Paletted_Surface->pixels, frame_size, 1, ptr);


	// 32 bit pixel format stuff below
	pxlFMT_UnPal = *SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
	pxlFMT_UnPal.palette = NULL;
	pxlFMT_UnPal.BitsPerPixel = 32;
	// BytesPerPixel = BitsPerPixel/4 (raised to nearest power of 2)
	pxlFMT_UnPal.BytesPerPixel = 4;

	int k = (frame_width)*(frame_height);

	// TODO: need to figure out what command is best to create a blank surface
	//Temp_Surface = SDL_CreateRGBSurfaceFrom(surface->pixels, surface->w, surface->h, surface->format->BitsPerPixel, surface->pitch, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
	Output_Surface = SDL_ConvertSurface(Paletted_Surface, &pxlFMT_UnPal, 0);
	//Temp_Surface = SDL_CreateRGBSurface(0, surface->w, surface->h, 8, 0, 0, 0, 0);

	if (!Output_Surface) {
		printf("Error: %s", SDL_GetError());
	}

	for (int i = 0; i < k; i++)
	{
		memcpy(&abgr_, (abgr*)Paletted_Surface->pixels + i, sizeof(abgr));
		int w_smallest = INT_MAX;
		uint8_t w_PaletteColor;
		w_PaletteColor = SDL_MapRGBA(&pxlFMT_Pal, abgr_.r, abgr_.g, abgr_.b, abgr_.a);
		//for (int j = 0; j < 256; j++)
		//{
		//	int s = (abgr_.r - PaletteColors[j].r);
		//	int t = (abgr_.g - PaletteColors[j].g);
		//	int u = (abgr_.b - PaletteColors[j].b);
		//	int v = (abgr_.a - PaletteColors[j].a);
		//	s *= s;
		//	t *= t;
		//	u *= u;
		//	v *= v;
		//	int w = sqrt(s + t + u + v);
		//	if (w < w_smallest) { 
		//		w_smallest = w; 
		//		w_PaletteColor = j;
		//	}
		//	//printf("stuv: %d %d %d %d", s, t, u, v);
		//}
		if (i == 100) { printf("loop #: %d\n", i); }
		if (i == 1000) { printf("loop #: %d\n", i); }
		if (i == 10000) { printf("loop #: %d\n", i); }
		if (i == 100000) { printf("loop #: %d\n", i); }
		if (i == 1000000) { printf("loop #: %d\n", i); }
		if (i == 2000000) { printf("loop #: %d\n", i); }
		((uint8_t*)Output_Surface->pixels)[i] = w_PaletteColor;
		//printf("w_PaletteColor: %d\n", w_PaletteColor);
	}
	return Output_Surface;
}














void Palette_to_Texture()
{
	char buffer[512];
	FILE *palette_test;
	palette_test = fopen("Palette_Shader.vert", "rb");

	fread(buffer, sizeof(palette_test), 1, palette_test);
}