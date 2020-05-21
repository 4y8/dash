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

typedef struct entity {
	int    x, y, w, h;
	vector speed;
} entity;

typedef struct entity_l {
	entity *l;
	int     len;
} entity_l;

float
norm(vector v)
{
	return (sqrtf (v.x * v.x + v.y * v.y));
}

int walls[6][8] = {
{1, 1, 1, 1, 1, 1, 1, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 1, 1, 1, 1, 1, 1, 1}};

vector
normalize(vector v)
{
	float n;

	n = norm(v);
	if (n == 0) {
		v.x = 0;
		v.y = 0;
		return v;
	}
	v.x /= n;
	v.y /= n;
	return v;

}

entity player;

vector
get_mouse(vector v)
{
	SDL_Event e;

	while (SDL_PollEvent(&e))
		if (e.type == SDL_MOUSEMOTION) {
			v.x = e.motion.x - player.x;
			v.y = e.motion.y - player.y;
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
draw_entity (entity e)
{
	draw_rectangle(e.x, e.y, e.w, e.h, WHITE);
}

entity_l
build_walls (int w[6][8])
{
	entity   e[48];
	entity_l l;
	int      p;
	vector   nv;

	p    = 0;
	nv.x = 0;
	nv.y = 0;
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 8; j++) {
			if (w[i][j]) {
				e[p].x     = 60 * j;
				e[p].y     = 60 * i;
				e[p].w     = 60;
				e[p].h     = 60;
				e[p].speed = nv;
				p ++;
			}
		}
	l.len = p;
	l.l   = e;
	return l;
}

void
update_player(int x, int y)
{
	draw_rectangle(player.x, player.y, 10, 10, BLACK);
	player.x = x;
	player.y = y;
	draw_rectangle(player.x, player.y, 10, 10, WHITE);

}

void
init()
{
	player.speed.x = 0;
	player.speed.y = 0;
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
	entity_l l = build_walls(walls);

		for (int i = 0; i < l.len; i++)
			draw_entity(* (l.l + i));
		int v = 2;
	SDL_Delay(3000);
	while (1) {
		player.speed = get_mouse(player.speed);
		update_player(player.x + v * player.speed.x, player.y + v * player.speed.y);
		SDL_Delay(10);

	}
	SDL_Quit();
	return 0;
}
