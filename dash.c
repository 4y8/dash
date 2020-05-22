#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>

int SCREEN_WIDTH  = 640;
int SCREEN_HEIGHT = 480;
int SPPED_COEF    = 4;

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
	entity  l[48];
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
}

void
draw_entity (entity e, int off_x, int off_y)
{
	draw_rectangle(e.x + off_x, e.y + off_y, e.w, e.h, WHITE);
}

int
mouseX()
{
	SDL_PumpEvents();
	int x;
	SDL_GetMouseState(&x, NULL);
	return x;
}

int
mouseY()
{
	SDL_PumpEvents();
	int y;
	SDL_GetMouseState(NULL, &y);
	return y;
}

vector
get_mouse_v(vector v)
{
	SDL_Event e;
	int x;
	int y;

	x = mouseX();
	y = mouseY();
	v.x = x - 320;
	v.y = y - 240;
	return normalize(v);
}


entity_l
build_walls (int w[6][8])
{
	entity_l l;
	int      p;
	vector   nv;

	p    = 0;
	nv.x = 0;
	nv.y = 0;
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 8; j++) {
			if (w[i][j]) {
				l.l[p].x     = 80 * j;
				l.l[p].y     = 80 * i;
				l.l[p].w     = 80;
				l.l[p].h     = 80;
				l.l[p].speed = nv;
				p ++;
			}
		}
	l.len = p;
	return l;
}

int
detect_collision (entity e1, entity e2)
{
	return
		(e1.x <= e2.x + e2.w) &&
		(e1.x + e1.w >= e2.x) &&
		(e1.y <= e2.y + e2.h) &&
		(e1.y + e1.h >= e2.y);
}

void
update_player(int x, int y)
{
	entity_l l;
	int      col;
	int      sx;
	int      sy;

	sx  = player.x;
	sy  = player.y;
	col = 0;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	l = build_walls(walls);
	player.x = x;
	player.y = y;
	for (int i = 0; i < l.len; i++)
		col += detect_collision(l.l[i], player);
	if (col) {
		player.x = sx;
		player.y = sy;
	}
	for (int i = 0; i < l.len; i++)
		draw_entity(l.l[i], x - 315, y - 235);
	draw_rectangle(315, 235, 10, 10, WHITE);
	SDL_RenderPresent(renderer);
}

void
init()
{
	player.x = 315;
	player.y = 235;
	player.w = 10;
	player.h = 10;
	player.speed.x = 0;
	player.speed.y = 0;
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_CreateWindow("dash", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	renderer = SDL_CreateRenderer(screen, -1, 0);
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

int
main()
{
	init();
	entity e1;
	e1.x = 0;
	e1.y = 0;
	e1.w = 40;
	e1.h = 40;
	entity e2;
	e2.x = 0;
	e2.y = 100;
	e2.w = 40;
	e2.h = 40;
	printf("%d\n", detect_collision(e1, e2));
	SDL_Delay(3000);
	while (1) {
		player.speed = get_mouse_v(player.speed);
		update_player(player.x - SPPED_COEF * player.speed.x,
					  player.y - SPPED_COEF * player.speed.y);
		SDL_Delay(16);
	}
	SDL_Quit();
	return 0;
}
