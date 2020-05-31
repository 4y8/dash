#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>

typedef struct {
	float x, y;
} Vector2;

typedef struct {
	float   x, y, w, h, l;
	Vector2 s;
} Entity;

typedef struct {
	Entity  l[48];
	int     len;
} Entity_l;

typedef struct {
	Vector2 v;
	int     t;
} Force;

typedef struct {
	enum { SKELETON, SLIME } t;
	Entity b;
	float  d, k, r;
	int    c; /*  Has it already collided with the player */
	Force  f;
} Ennemy;

double PI = 3.14159265;

#define TRUE  1
#define FALSE 0

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

/*  Plyaer's control variables */
#define SPEED_COEF     1.7
#define PLAYER_WIDTH   10
#define PLAYER_HEIGHT  10
#define NFORCES        1
#define SWORD_LENGTH   16
#define SWORD_WIDTH    10
#define SWORD_HEIGHT   30
#define SWORD_COOLDWN  20
#define PLAYER_HEALTH  100
#define COLLISION_HP   10
#define COLLISION_COEF 17
#define COLLISION_DIV  250
#define PLAYER_DAMAGE  10
#define PLAYER_STREN   20

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

Ennemy skeleton;

int room[6][8] = {
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
make_entity(float x, float y, float w, float h, float l, Vector2 s)
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

Ennemy
make_ennemy(int t, Entity b, float d, float k, float r)
{
	Ennemy e;

	e.t = t;
	e.b = b;
	e.d = d;
	e.k = k;
	e.r = r;
	e.f = make_force(NULL_VECTOR, 0);
	e.c = FALSE;
	return e;
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

/*
 * All the draw_* functions don't apply the modifications, this has to be done,
 * by the caller. This reduces a lot flickering if managed well, for instance if
 * the SDL_RenderPresent is called only once per frame.
 */
void
draw_rectangle(int x, int y, int w, int h, int color)
{
	SDL_Rect r;

	SDL_SetRenderDrawColor(renderer, color & 0xFF0000 >> 16,
						   color & 0x00FF00 >> 8, color & 0x0000FF, 255);
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
	SDL_FillRect(s, NULL, SDL_MapRGB(s->format, color & 0xFF0000 >> 16,
									 color & 0x00FF00 >> 8,
									 color & 0x0000FF));
	t = SDL_CreateTextureFromSurface(renderer, s);
	SDL_FreeSurface(s);
	sr = make_rect(0, 0, w, h);
	dr = make_rect(x, y, w, h);
	SDL_RenderCopyEx(renderer, t, &sr, &dr, angle, NULL, SDL_FLIP_NONE);
}

void
draw_entity(Entity e, float off_x, float off_y)
{

	e.x = SCREEN_WIDTH - e.x - e.w;
	e.y = SCREEN_HEIGHT - e.y - e.h;
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
	return normalize(make_vector2(x - SCREEN_WIDTH / 2, y - SCREEN_HEIGHT / 2));
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
				l.l[++p] = make_entity(SCREEN_WIDTH  - 80 * (j + 1),
									   SCREEN_HEIGHT - 80 * (i + 1), 80, 80, -1,
									   NULL_VECTOR);
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
		if (detect_collision(walls_e.l[i], e)) return TRUE;
	return FALSE;
}

Ennemy
move_ennemy(Ennemy e)
{

	if (e.f.t) {
		e.b.x += e.f.v.x * (float) e.f.t / COLLISION_DIV;
		e.b.y += e.f.v.y * (float) e.f.t / COLLISION_DIV;
		-- e.f.t;
	}

	switch (e.t) {
		case SKELETON: {
			Vector2 v;

			v      = normalize(make_vector2(player.x - e.b.x, player.y - e.b.y));
			e.b.x += v.x;
			e.b.y += v.y;
			break;
		} case SLIME: {
			do {
				Vector2 v;
			
				v      = normalize(make_vector2(rand(), rand()));
				e.b.x += v.x;
				e.b.y += v.y;
			} while(collide_walls(e.b));
		}
	}
	return e;
}

void
update_player(float x, float y)
{
	Entity_l l;
	float    sx;
	float    sy;

	sx  = player.x;
	sy  = player.y;
	player.x = x;
	player.y = y;
	for (int i = 0; i < NFORCES; i++)
		if (forces[i].t) {
			player.x += forces[i].v.x * (float) forces[i].t / COLLISION_DIV;
			player.y += forces[i].v.y * (float) forces[i].t / COLLISION_DIV;
		}
	if (collide_walls(player)) {
		player.x = sx;
		player.y = sy;
		if (!collided) player.l -= COLLISION_HP;
		collided = TRUE;
	} else collided = FALSE;
	for (int i = 0; i < walls_e.len; i++)
		draw_entity(walls_e.l[i], x - start_x, y - start_y);
	skeleton = move_ennemy(skeleton);
	draw_entity(skeleton.b, x - start_x, y - start_y);
	draw_rectangle(start_x, start_y, PLAYER_WIDTH, PLAYER_HEIGHT, WHITE);
}

void
init()
{
	/* Setup the initial position and size of the player. */
	NULL_VECTOR = make_vector2(0, 0);
	start_x     = (SCREEN_WIDTH - PLAYER_WIDTH) / 2;
	start_y     = (SCREEN_HEIGHT - PLAYER_HEIGHT) / 2;
	walls_e     = build_walls(room);
	sw_off      = (SWORD_HEIGHT - PLAYER_HEIGHT) / 2;
	collided    = FALSE;
	skeleton    = make_ennemy(SKELETON,
							  make_entity(0, 0, 10, 20, 30, NULL_VECTOR), 5, 17, 3);
	player      = make_entity(start_x, start_y, PLAYER_WIDTH, PLAYER_HEIGHT, 
							  PLAYER_HEALTH, NULL_VECTOR);
	for (int i = 0; i < NFORCES; i++) forces[i] = make_force(NULL_VECTOR, 0);
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		printf("Error: SDL initialization error: %s\n", SDL_GetError());
	screen = SDL_CreateWindow("dash", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (!screen)
		printf("Error: Unable to create the SDL window: %s\n", 
			   SDL_GetError());
	renderer = SDL_CreateRenderer(screen, -1, 0);
	if (!renderer)
		printf("Error: Unable to create the SDL renderer: %s\n", 
			   SDL_GetError());
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	srand(time(NULL));
}

Entity
sword_entity()
{
	double a;

	a = atan2(player.s.y, player.s.x);
	a *= 180 / PI;
	if ((a > 135) || (a < -135))
		return make_entity(player.x + PLAYER_WIDTH, player.y - sw_off,
						   SWORD_WIDTH, SWORD_HEIGHT, -1, NULL_VECTOR);
	else if (a > 45)
		return make_entity(player.x - sw_off, player.y - PLAYER_WIDTH,
						   SWORD_HEIGHT, SWORD_WIDTH, -1, NULL_VECTOR);
	else if (a < -45)
		return make_entity(player.x - sw_off, player.y + PLAYER_WIDTH,
						   SWORD_HEIGHT, SWORD_WIDTH, -1, NULL_VECTOR);
	else
		return make_entity(player.x - PLAYER_WIDTH, player.y - sw_off,
						   SWORD_WIDTH, SWORD_HEIGHT, -1, NULL_VECTOR);

}

void
draw_sword()
{
	double a;

	a = atan2(player.s.y, player.s.x);
	a *= 180 / PI;
	if ((a > 135) || (a < -135)) draw_rectangle(start_x - PLAYER_WIDTH,
												start_y - sw_off,
												SWORD_WIDTH,
												SWORD_HEIGHT, WHITE);
	else if (a > 45)  draw_rectangle(start_x - sw_off,
									 start_y + PLAYER_WIDTH,
									 SWORD_HEIGHT, SWORD_WIDTH, WHITE);
	else if (a < -45) draw_rectangle(start_x - sw_off,
									 start_y - PLAYER_WIDTH,
									 SWORD_HEIGHT, SWORD_WIDTH, WHITE);
	else              draw_rectangle(start_x + PLAYER_WIDTH,
									 start_y - sw_off,
									 SWORD_WIDTH, SWORD_HEIGHT, WHITE);
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
				if ((e.button.button == SDL_BUTTON_LEFT) && (!has_sword))
					has_sword = SWORD_LENGTH + SWORD_COOLDWN;
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
		if (has_sword - SWORD_COOLDWN > 0) {
			Entity h;
			float  w;
			Entity sp;

			sp = make_entity(player.x - 2, player.y - 2,
					 player.w + 4, player.h + 4, -1, NULL_VECTOR);
			w = 2 * SWORD_WIDTH + PLAYER_HEIGHT;
			h = sword_entity();
			if (((collide_walls(h)) && (!collide_walls(sp))))
				add_force(make_vector2(COLLISION_COEF * SPEED_COEF * player.s.x,
									   COLLISION_COEF * SPEED_COEF * player.s.y),
						  50);
			if ((detect_collision(h,       skeleton.b)) &&
				(!detect_collision(player, skeleton.b))) {
				skeleton.b.l -= PLAYER_DAMAGE;
				skeleton.f = make_force(
					make_vector2(-player.s.x * (PLAYER_STREN - skeleton.r) *
								 SPEED_COEF, -player.s.y *
								 (PLAYER_STREN - skeleton.r) * SPEED_COEF), 50);
				if (skeleton.b.l <= 0)
					skeleton = make_ennemy(SKELETON,
										   make_entity(0, 0, 10, 20, 30,
													   NULL_VECTOR), 5, 17, 3);

			}
			draw_sword();
		} handle_input();
		if (has_sword) has_sword --;
		if ((detect_collision(player, skeleton.b))){
			if ((!skeleton.c)) {
				player.l -= skeleton.d;
				skeleton.c = TRUE;
				add_force(make_vector2(skeleton.k * SPEED_COEF * player.s.x,
									   skeleton.k * SPEED_COEF * player.s.y), 50);
			}
		} else skeleton.c = FALSE;
		player.s = get_mouse_v();
		update_player(player.x - SPEED_COEF * player.s.x,
					  player.y - SPEED_COEF * player.s.y);
		/* Only apply the renderings after at the end to avoid flickering. */
		SDL_RenderPresent(renderer);
		for (int i = 0; i < NFORCES; i++) if (forces[i].t) forces[i].t --;
		if (!player.l) { SDL_Quit(); exit(0); }
		SDL_Delay(5);
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
