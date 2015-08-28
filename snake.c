#include <SDL.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 810
#define SCREEN_HEIGHT 630
#define TILE 45
#define FPS 7

float brown[3] = {0.647059, 0.164706, 0.164706};
float green[3] = {0.0, 1.0, 0.0};
float red[3] = {1.0, 0.0, 0.0};

enum direction { UP, DOWN, LEFT, RIGHT };

struct square {
    int x;
    int y;
    int size;
    float *color;
};

struct body {
    struct square *part;
    struct body *rest;
};

struct square * make_square(int x, int y, int size, float *color);
struct body * make_body(int x, int y);
void reposition_fruit(struct square *f);
int is_intersect(struct square *s1, struct square *s2);
int is_intersect_body(struct square *s, struct body *b);
void add_body(struct body **b, int x, int y);
void step_body(struct body **b, int x, int y);
void clean_body(struct body *b);
void clear_screen(void);
void display_square(struct square *s);
void display_body(struct body *b);
void display(struct square *h, struct body *sb, struct square *f);
void cap_game_fps(void (*d)(struct square *h, struct body *sb, struct square *f), SDL_Window *w, struct square *h, struct body *sb, struct square *f);

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    SDL_Window *win = SDL_CreateWindow("snake", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    if (!win) {
        fprintf(stderr, "Unable to initialize window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    SDL_GLContext gl_con = SDL_GL_CreateContext(win);
    struct square *head = make_square(0, 0, TILE, brown);
    struct square *fruit = make_square(0, 0, TILE, red);
    if (!head || !fruit) {
        fprintf(stderr, "Unable to allocate memory for square struct.");
        if (head) {
            free(head);
        }
    }
    struct body *snake_body = NULL;
    reposition_fruit(fruit);
    enum direction snake_d = LEFT;
    int previousx;
    int previousy;
    if (gl_con == NULL) {
        fprintf(stderr, "Unable to initialize gl context: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return -1;
    }
    SDL_Event event;
    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                goto end;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    goto end;
                    break;
                case SDLK_UP:
                    snake_d = UP;
                    break;
                case SDLK_DOWN:
                    snake_d = DOWN;
                    break;
                case SDLK_LEFT:
                    snake_d = LEFT;
                    break;
                case SDLK_RIGHT:
                    snake_d = RIGHT;
                    break;
                }
            }
        }
        previousx = head->x;
        previousy = head->y;
        switch (snake_d) {
        case UP:
            head->y += TILE;
            break;
        case DOWN:
            head->y -= TILE;
            break;
        case LEFT:
            head->x -= TILE;
            break;
        case RIGHT:
            head->x += TILE;
            break;
        }
        if (is_intersect_body(head, snake_body)) {
            goto end;
        }
        step_body(&snake_body, previousx, previousy);
        if (head->y > SCREEN_HEIGHT - TILE || head->y < -1 * SCREEN_HEIGHT || head->x > SCREEN_WIDTH - TILE || head->x < -1 * SCREEN_WIDTH) {
            goto end;
        }
        else if (is_intersect(head, fruit)) {
            add_body(&snake_body, previousx, previousy);
            do {
                reposition_fruit(fruit);
            } while (is_intersect(head, fruit) || is_intersect_body(fruit, snake_body));
        }
	cap_game_fps(display, win, head, snake_body, fruit);
    }
end:
    free(head);
    clean_body(snake_body);
    free(fruit);
    SDL_GL_DeleteContext(gl_con);
    SDL_Quit();
    return 0;
}

struct square * make_square(int x, int y, int size, float *color)
{
    struct square *s = malloc(sizeof(struct square));
    if (s) {
        s->x = x;
        s->y = y;
        s->size = size;
        s->color = color;
    }
    return s;
}

struct body * make_body(int x, int y)
{
    struct body *b = malloc(sizeof(struct body));
    if (b) {
        b->part = make_square(x, y, TILE, green);
        b->rest = NULL;
        if (!(b->part)) {
            free(b);
            b = NULL;
        }
    }
    return b;
}

void reposition_fruit(struct square *f)
{
    do {
        f->x = rand() % ((SCREEN_WIDTH - TILE) + 1);
        f->y = rand() % ((SCREEN_HEIGHT - TILE) + 1);
    } while (f->x % TILE != 0 || f->y % TILE != 0);
    if (rand() > RAND_MAX / 2) {
        f->x *= -1;
    }
    if (rand() > RAND_MAX / 2) {
        f->y *= -1;
    }
}

int is_intersect(struct square *s1, struct square *s2)
{
    if (s1->x == s2->x && s1->y == s2->y) {
        return 1;
    }
    else {
        return 0;
    }
}

int is_intersect_body(struct square *s, struct body *b)
{
    if (!s || !b) {
        return 0;
    }
    else if (is_intersect(s, b->part)) {
        return 1;
    }
    else {
        return is_intersect_body(s, b->rest);
    }
}

void add_body(struct body **b, int x, int y)
{
    if (!(*b)) {
        *b = make_body(x, y);
        (**b).rest = NULL;
    }
    else {
        add_body(&((*b)->rest), x, y);
    }
}

void step_body(struct body **b, int x, int y)
{
    struct body *temp = NULL;
    if (*b) {
        temp = (**b).rest;
        free(*b);
        *b = temp;
        add_body(b, x, y);
    }
}

void clean_body(struct body *b)
{
    struct body *rest = NULL;
    if (b) {
        rest = b->rest;
        free(b->part);
        clean_body(rest);
    }
}

void clear_screen(void)
{
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void display_square(struct square *s)
{
    float left = (float)s->x / SCREEN_WIDTH;
    float right = left + (float)s->size / SCREEN_WIDTH;
    float bottom = (float)s->y / SCREEN_HEIGHT;
    float top = bottom + (float)s->size / SCREEN_HEIGHT;
    
    glBegin(GL_QUADS);
       glColor3fv(s->color);
       glVertex2f(left, top);
       glVertex2f(right, top);
       glVertex2f(right, bottom);
       glVertex2f(left, bottom);
    glEnd();
}

void display_body(struct body *b)
{
    if (b) {
        display_square(b->part);
        display_body(b->rest);
    }
}

void display(struct square *h, struct body *sb, struct square *f)
{
    clear_screen();
    display_square(h);
    display_body(sb);
    display_square(f);
}

void cap_game_fps(void (*d)(struct square *h, struct body *sb, struct square *f), SDL_Window *w, struct square *h, struct body *sb, struct square *f)
{
    Uint32 start, end;
    start = SDL_GetTicks();
    d(h, sb, f);
    SDL_GL_SwapWindow(w);
    do {
	end = SDL_GetTicks();
    } while (end - start < 1000 / FPS);
}
