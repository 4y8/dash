#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

int WHITE = 0xFFFFFF, BLACK = 0x000000;

SDL_Surface *screen;

int player_x = 0, player_y = 0;

void
draw_rectangle (int x, int y, int w, int h, int color)
{
	SDL_Rect pos;
	pos.x = x;
	pos.y = y;
	SDL_Surface *rect = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 24, 0, 0, 0, 0);
	SDL_FillRect(rect, NULL, SDL_MapRGB
				 (screen->format,
				  color & 0xFF0000 >> 16,
				  color & 0x00FF00 >> 8,
				  color & 0x0000FF));
	SDL_BlitSurface(rect, NULL, screen, &pos);
	SDL_Flip(screen);
}

void
update_player(int x, int y)
{
	draw_rectangle(player_x, player_y, 10, 10, BLACK);
	player_x = x;
	player_y = y;
	draw_rectangle(x, y, 10, 10, WHITE);

}

int
main()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 24, SDL_HWSURFACE);
	draw_rectangle(0, 0, 222, 222, WHITE);
	draw_rectangle(30, 30, 222, 222, WHITE);
	SDL_Delay(3000);
	SDL_FreeSurface(screen);
	SDL_Quit();
	return 0;
}
