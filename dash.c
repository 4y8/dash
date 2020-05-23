#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>

typedef struct Vector {
	float x, y;
} vector;

typedef struct Entity {
	int    x, y, w, h;
	vector s;
} entity;

typedef struct Entity_l {
	entity  l[48];
	int     len;
} entity_l;

typedef struct Force {
	vector v;
	int    t;
} force;

double PI = 3.14159265;

int SCREEN_WIDTH  = 640;
int SCREEN_HEIGHT = 480;
int SPEED_COEF    = 3;
int PLAYER_WIDTH  = 10;
int PLAYER_HEIGHT = 10;
int NFORCES       = 3;

int WHITE = 0xFFFFFF, BLACK = 0x000000;

int start_x;
int start_y;


SDL_Window   *screen;
SDL_Renderer *renderer;

force forces[3];

entity_l walls_e;

entity player;

int walls[6][8] = {
{1, 1, 1, 1, 1, 1, 1, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 1, 1, 1, 1, 1, 1, 1}};

vector
make_vector(int x, int y)
{
	vector v;

	v.x = x;
	v.y = y;
	return v;
}

vector NULL_VECTOR;

entity
make_entity(int x, int y, int w, int h, vector s)
{
	entity e;

	e.x = x;
	e.y = y;
	e.w = w;
	e.h = h;
	e.s = s;
	return e;
}

force
make_force(vector v, int t)
{
	force f;

	f.v = v;
	f.t = t;
	return f;
}

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
	if (n == 0) return NULL_VECTOR;
	v.x /= n;
	v.y /= n;
	return v;

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
draw_rectangle_a(int x, int y, int h, int w, int color, double angle)
{
	SDL_Surface *s;
	SDL_Texture *t;
	SDL_Rect    sr;
	SDL_Rect    dr;

	s = SDL_CreateRGBSurface(0, w, h, 24, 0, 0, 0, 0);
	SDL_FillRect(s, NULL, SDL_MapRGB(s->format,
									 color & 0xFF0000 >> 16,
									 color & 0x00FF00 >> 8,
									 color & 0x0000FF));
	t = SDL_CreateTextureFromSurface(renderer, s);
	SDL_FreeSurface(s);
	sr = make_rect(0, 0, w, h);
	dr = make_rect(x, y, w, h);
	SDL_RenderCopyEx(renderer, t, &sr, &dr, angle, NULL, SDL_FLIP_NONE);
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
get_mouse_v()
{
	SDL_Event e;
	int       x;
	int       y;

	x = mouseX();
	y = mouseY();
	return normalize(make_vector(x - 320, y - 240));
}

entity_l
build_walls(int w[6][8])
{
	entity_l l;
	int      p;
	vector   nv;

	p  = -1;
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 8; j++)
			if (w[i][j])
				l.l[++p] = make_entity(80 * j, 80 * i, 80, 80, NULL_VECTOR);
	l.len = ++p;
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

int
collide_walls (entity e)
{
	for (int i = 0; i < walls_e.len; i++)
		if (detect_collision(walls_e.l[i], e)) return 1;
	return 0;
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
	player.x = x;
	player.y = y;
	if (collide_walls(player)) {
		player.x   = sx;
		player.y   = sy;
		player.s.x = 0;
		player.s.y = 0;
	}
	for (int i = 0; i < walls_e.len; i++)
		draw_entity(walls_e.l[i], x - start_x, y - start_y);
	draw_rectangle(start_x,
				   start_y,
				   PLAYER_WIDTH,
				   PLAYER_HEIGHT,
				   WHITE);
	/* Only apply the renderings after at the end to avoid flickering. */
}

void
init()
{
	/* Setup the initial position and size of the player. */
	NULL_VECTOR = make_vector(0, 0);
	start_x     = 320 - PLAYER_WIDTH / 2;
	start_y     = 240 - PLAYER_HEIGHT / 2;
	player      = make_entity(start_x,
							  start_y,
							  PLAYER_WIDTH,
							  PLAYER_HEIGHT,
							  NULL_VECTOR);
	walls_e     = build_walls(walls);
	for (int i = 0; i < NFORCES; i++) forces[i] = make_force(NULL_VECTOR, 0);
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

void
draw_sword()
{
	double a;
	entity hitbox;

	a = atan2(player.s.y, player.s.x);
	a *= 180 / PI;
	if ((a > 135) || (a < -135))
		draw_rectangle(start_x - 10, start_y - 10, 10, 30, WHITE);
	else if (a > 45)  draw_rectangle(start_x - 10, start_y + 10, 30, 10, WHITE);
	else if (a < -45) draw_rectangle(start_x - 10, start_y - 10, 30, 10, WHITE);
	else              draw_rectangle(start_x + 10, start_y - 10, 10, 30, WHITE);

}

void
add_force(vector v, int t)
{
	for (int i = 0; i < NFORCES; i ++)
		if (forces[i].t <= 0) {
			forces[i] = make_force(v, t);
			return;
		}
}

void
handle_input()
{
	SDL_Event e;

	if (SDL_PollEvent(&e))
		switch (e.type){
			case SDL_QUIT:
				SDL_Quit();
				exit(0);
				break;
			case SDL_MOUSEBUTTONDOWN: {
				if (e.button.button == SDL_BUTTON_LEFT) {
					entity h;

					h = make_entity(player.x - 10, player.y - 10, 30, 30, NULL_VECTOR);
					draw_sword();
					if (collide_walls(h))
						add_force(make_vector(-1 * player.s.x, -1 * player.s.y), 500);
				}
				break;
			}
			default:
				break;
		}

}

void
main_loop()
{
	for (;;) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		handle_input();
		player.s = get_mouse_v();
		update_player(player.x - SPEED_COEF * player.s.x,
					  player.y - SPEED_COEF * player.s.y);
		SDL_RenderPresent(renderer);
		for (int i = 0; i < NFORCES; i++) forces[i].t --;
		SDL_Delay(10);
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
