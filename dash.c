#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>

typedef struct Vector2 {
	float x, y;
} Vector2;

typedef struct Entity {
	float   x, y, w, h, l;
	Vector2 s;
} Entity;

typedef struct Entity_l {
	Entity  l[48];
	int     len;
} Entity_l;

typedef struct Force {
	Vector2 v;
	int    t;
} Force;

double PI = 3.14159265;

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
/*  Plyaer's control variables */
#define SPEED_COEF    4
#define PLAYER_WIDTH  10
#define PLAYER_HEIGHT 10
#define NFORCES       1
#define SWORD_LENGTH  16
#define SWORD_WIDTH   10
#define SWORD_HEIGHT  30
#define PLAYER_HEALTH 100
#define COLLISION_HP  10

#define WHITE 0xFFFFFF
#define BLACK 0x000000

int start_x;
int start_y;
int sw_off;
int has_sword;
int collided;

SDL_Window   *screen;
SDL_Renderer *renderer;

Force forces[NFORCES];

Entity_l walls_e;

Entity player;

int walls[6][8] = {
{1, 1, 1, 1, 1, 1, 1, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 0, 0, 0, 0, 0, 0, 1},
{1, 1, 1, 1, 1, 1, 1, 1}};

Vector2
make_vector2(int x, int y)
{
	Vector2 v;

	v.x = x;
	v.y = y;
	return v;
}

Vector2 NULL_VECTOR;

Entity
make_entity(float x, float y, float w, float h, Vector2 s, float l)
{
	Entity e;

	e.x = x;
	e.y = y;
	e.w = w;
	e.h = h;
	e.s = s;
	e.l = l;
	return e;
}

Force
make_force(Vector2 v, int t)
{
	Force f;

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
norm(Vector2 v)
{
	return (sqrtf (v.x * v.x + v.y * v.y));
}

Vector2
normalize(Vector2 v)
{
	float n;

	n = norm(v);
	if (n == 0) return NULL_VECTOR;
	v.x /= n;
	v.y /= n;
	return v;

}

/*  All the draw_* functions don't apply the modifications, this has to be done,
 * by the caller. This reduces a lot flickering if managed well
 */
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
draw_entity(Entity e, int off_x, int off_y)
{
	draw_rectangle(e.x + off_x, e.y + off_y, e.w, e.h, WHITE);
}

int
mouseX()
{
	int x;

	SDL_GetMouseState(&x, NULL);
	return x;
}

int
mouseY()
{
	int y;

	SDL_GetMouseState(NULL, &y);
	return y;
}

Vector2
get_mouse_v()
{
	SDL_Event e;
	int       x;
	int       y;

	SDL_PumpEvents();
	x = mouseX();
	y = mouseY();
	return normalize(make_vector2(x - 320, y - 240));
}

Entity_l
build_walls(int w[6][8])
{
	Entity_l l;
	int      p;
	Vector2   nv;

	p = -1;
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 8; j++)
			if (w[i][j])
				l.l[++p] = make_entity(80 * j, 80 * i, 80, 80, NULL_VECTOR, -1);
	l.len = ++p;
	return l;
}

int
detect_collision(Entity e1, Entity e2)
{
	return
		(e1.x <= e2.x + e2.w) &&
		(e1.x + e1.w >= e2.x) &&
		(e1.y <= e2.y + e2.h) &&
		(e1.y + e1.h >= e2.y);
}

int
collide_walls (Entity e)
{
	for (int i = 0; i < walls_e.len; i++)
		if (detect_collision(walls_e.l[i], e)) return 1;
	return 0;
}

void
update_player(int x, int y)
{
	Entity_l l;
	int      col;
	int      sx;
	int      sy;

	sx  = player.x;
	sy  = player.y;
	col = 0;
	player.x = x;
	player.y = y;
	for (int i = 0; i < NFORCES; i++)
		if (forces[i].t) {
			player.x += forces[i].v.x * (float) forces[i].t / 250;
			player.y += forces[i].v.y * (float) forces[i].t / 250;
		}
	if (collide_walls(player)) {
		player.x = sx;
		player.y = sy;
		if(!collided) player.l -= COLLISION_HP;
		collided = 1;
	} else collided = 0;
	for (int i = 0; i < walls_e.len; i++)
		draw_entity(walls_e.l[i], x - start_x, y - start_y);
	draw_rectangle(start_x,
				   start_y,
				   PLAYER_WIDTH,
				   PLAYER_HEIGHT,
				   WHITE);
}

void
init()
{
	/* Setup the initial position and size of the player. */
	NULL_VECTOR = make_vector2(0, 0);
	start_x     = 320 - PLAYER_WIDTH / 2;
	start_y     = 240 - PLAYER_HEIGHT / 2;
	walls_e     = build_walls(walls);
	sw_off      = (SWORD_HEIGHT - PLAYER_HEIGHT) / 2;
	collided    = 0;
	player      = make_entity(start_x,
							  start_y,
							  PLAYER_WIDTH,
							  PLAYER_HEIGHT,
							  NULL_VECTOR,
							  PLAYER_HEALTH);
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
	Entity hitbox;

	a = atan2(player.s.y, player.s.x);
	a *= 180 / PI;
	if ((a > 135) || (a < -135)) draw_rectangle(start_x - PLAYER_WIDTH,
												start_y - sw_off,
												SWORD_WIDTH,
												SWORD_HEIGHT,
												WHITE);
	else if (a > 45)  draw_rectangle(start_x - sw_off,
									 start_y + PLAYER_WIDTH,
									 SWORD_HEIGHT,
									 SWORD_WIDTH,
									 WHITE);
	else if (a < -45) draw_rectangle(start_x - sw_off,
									 start_y - PLAYER_WIDTH,
									 SWORD_HEIGHT,
									 SWORD_WIDTH,
									 WHITE);
	else              draw_rectangle(start_x + PLAYER_WIDTH,
									 start_y - sw_off,
									 SWORD_WIDTH,
									 SWORD_HEIGHT,
									 WHITE);
}

void
add_force(Vector2 v, int t)
{
	forces[0] = make_force(v, t);
	return;
}

void
handle_input()
{
	SDL_Event e;

	if (SDL_PollEvent(&e))
		switch (e.type){
			case SDL_QUIT: SDL_Quit(); exit(0);
			case SDL_MOUSEBUTTONDOWN: {
				if (e.button.button == SDL_BUTTON_LEFT)
					has_sword = SWORD_LENGTH;
				break;
			}
			default: break;
		}

}

void
main_loop()
{
	for (;;) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		if (has_sword) {
			Entity h;
			float  w;
			Entity sp;

			sp = make_entity(player.x - 2,
							 player.y - 2,
							 player.w + 4,
							 player.h + 4,
							 NULL_VECTOR,
							 -1);
			w = 2 * SWORD_WIDTH + PLAYER_HEIGHT;
			h = make_entity(player.x - sw_off,
							player.y - sw_off,
							w,
							w,
							NULL_VECTOR,
							-1);
			if ((collide_walls(h) && (!collide_walls(sp))))
				add_force(make_vector2(15 * SPEED_COEF * player.s.x,
									   15 * SPEED_COEF * player.s.y), 50);
			has_sword --;
			draw_sword();
		} handle_input();
		player.s = get_mouse_v();
		update_player(player.x - SPEED_COEF * player.s.x,
					  player.y - SPEED_COEF * player.s.y);

		/* Only apply the renderings after at the end to avoid flickering. */
		SDL_RenderPresent(renderer);
		for (int i = 0; i < NFORCES; i++) if (forces[i].t) forces[i].t --;
		if (!player.l) {SDL_Quit(); exit(0);}
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
