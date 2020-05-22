#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>

int SCREEN_WIDTH  = 640;
int SCREEN_HEIGHT = 480;
int SPPED_COEF    = 4;
int PLAYER_WIDTH  = 10;
int PLAYER_HEIGHT = 10;

int start_x;
int start_y;

int WHITE = 0xFFFFFF, BLACK = 0x000000;

SDL_Window   *screen;
SDL_Renderer *renderer;

typedef struct Vector {
	float x, y;
} vector;

typedef struct Entity {
	int    x, y, w, h;
	vector speed;
} entity;

typedef struct Entity_l {
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

SDL_Rect
make_rect(int x, int y, int w, int h)
{
	SDL_Rect r;

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return r;
}

SDL_Point
make_point(int x, int y)
{
	SDL_Point p;

	p.x = x;
	p.y = y;
	return p;
}

void
draw_rectangle(int x, int y, int w, int h, int color)
{
	SDL_Rect r;

	SDL_SetRenderDrawColor(renderer,
						   color & 0xFF0000 >> 16,
						   color & 0x00FF00 >> 8,
						   color & 0x0000FF,
						   255);
	r = make_rect(x, y, w, h);
	SDL_RenderFillRect(renderer, &r);
}

void
draw_rectangle_a(int x, int y, int h, int w, int color, int angle)
{
	SDL_Surface *s;
	SDL_Texture *t;
	SDL_Rect    sr;
	SDL_Rect    dr;
	SDL_Point   p;

	s = SDL_CreateRGBSurface(0, w, h, 24, 0, 0, 0, 0);
	SDL_FillRect(s, NULL, SDL_MapRGB(s->format,
									 color & 0xFF0000 >> 16,
									 color & 0x00FF00 >> 8,
									 color & 0x0000FF));
	t = SDL_CreateTextureFromSurface(renderer, s);
	SDL_FreeSurface(s);
	puts("ee");
	sr = make_rect(0, 0, w, h);
	dr = make_rect(x, y, w, h);
	p  = make_point(0, 0);
	SDL_RenderCopyEx(renderer, t, &sr, &dr, angle, &p, SDL_FLIP_NONE);
}

void
draw_entity(entity e, int off_x, int off_y)
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
build_walls(int w[6][8])
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
detect_collision(entity e1, entity e2)
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
		draw_entity(l.l[i], x - start_x, y - start_y);
	draw_rectangle(start_x,
				   start_y,
				   PLAYER_WIDTH,
				   PLAYER_HEIGHT,
				   WHITE);
	/* Only apply the renderings after at the end to avoid flickering. */
	SDL_RenderPresent(renderer);
}

void
init()
{
	/* Setup the initial position and size of the player. */
	start_x  = 320 - PLAYER_WIDTH / 2;
	start_y  = 240 - PLAYER_HEIGHT / 2;
	player.x = start_x;
	player.y = start_y;
	player.w = PLAYER_WIDTH;
	player.h = PLAYER_HEIGHT;
	player.speed.x = 0;
	player.speed.y = 0;
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		printf("Error: SDL initialization error: %s\n", SDL_GetError());
	screen = SDL_CreateWindow("dash", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (!screen)
		printf("Error: Unable to create the SDL window: %s\n", SDL_GetError());
	renderer = SDL_CreateRenderer(screen, -1, 0);
	if (!renderer)
		printf("Error: Unable to create the SDL renderer: %s\n", SDL_GetError());
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}

int
main_loop()
{
	for (;;) {
		player.speed = get_mouse_v(player.speed);
		update_player(player.x - SPPED_COEF * player.speed.x,
					  player.y - SPPED_COEF * player.speed.y);
		SDL_Delay(16);
	}
}

int
main()
{
	init();
	main_loop();
	SDL_Quit();
	return 0;
}
