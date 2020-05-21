#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>

int SCREEN_WIDTH  = 640;
int SCREEN_HEIGHT = 480;

int WHITE = 0xFFFFFF, BLACK = 0x000000;

SDL_Window   *screen;
SDL_Renderer *renderer;

typedef struct Vector {
	float x, y;
} vector;

float
norm(vector v)
{
	return (sqrtf (v.x * v.x + v.y * v.y));
}

vector
normalize(vector v)
{
	float n;

	n = norm(v);
	v.x /= n;
	v.y /= n;
	return v;

}

int player_x = 0, player_y = 0;
vector player_speed;

vector
get_mouse(vector v)
{
	SDL_Event e;

	while (SDL_PollEvent(&e))
		if (e.type == SDL_MOUSEMOTION) {
			v.x = e.motion.x - player_x;
			v.y = e.motion.y - player_y;
			return normalize(v);
		} else {
			return v;
		}
	return v;
}

void
draw_rectangle (int x, int y, int w, int h, int color)
{
	SDL_Rect rect;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer,
						   color & 0xFF0000 >> 16,
						   color & 0x00FF00 >> 8,
						   color & 0x0000FF,
						   255);
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_RenderFillRect(renderer, &rect);
	SDL_RenderPresent(renderer);
}

void
update_player(int x, int y)
{
	draw_rectangle(player_x, player_y, 10, 10, BLACK);
	player_x = x;
	player_y = y;
	draw_rectangle(player_x, player_y, 10, 10, WHITE);

}

void
init()
{
	player_speed.x = 0;
	player_speed.y = 0;
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_CreateWindow("dash", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	renderer = SDL_CreateRenderer(screen, -1, 0);
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

int
main()
{
	init();
	int v = 5;
	while (1) {
		player_speed = get_mouse(player_speed);
		update_player(player_x + v * player_speed.x, player_y + v * player_speed.y);
		SDL_Delay(100);
	}
	SDL_Delay(3000);
	SDL_Quit();
	return 0;
}
