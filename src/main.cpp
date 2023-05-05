#include <SDL2/SDL.h>

// number of cells
#define CELLS_X 5
#define CELLS_Y 5
#define CELL_SIZE 20

// arrays for the cells
bool cur[CELLS_X][CELLS_Y] = {0};
bool buf[CELLS_X][CELLS_Y] = {0};

struct Color {
    Uint8 r, g, b;

    Color() {}
    Color(Uint8 r, Uint8 g, Uint8 b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
};

Color* gridColor = new Color(200, 200, 200);

void compute_buffer() {
    memset(buf, 0, sizeof(buf));

    for (int x = 0; x < CELLS_X; ++x) {
        for (int y = 0; y < CELLS_Y; ++y) {
            int cnt = 0;
            cnt += cur[(x - 1 + CELLS_X) % CELLS_X][(y - 1 + CELLS_Y) % CELLS_Y];
            cnt += cur[(x - 1 + CELLS_X) % CELLS_X][y                          ];
            cnt += cur[(x - 1 + CELLS_X) % CELLS_X][(y + 1) % CELLS_Y          ];
            cnt += cur[x                          ][(y - 1 + CELLS_Y) % CELLS_Y];
            cnt += cur[x                          ][(y + 1) % CELLS_Y          ];
            cnt += cur[(x + 1) % CELLS_X          ][(y - 1 + CELLS_Y) % CELLS_Y];
            cnt += cur[(x + 1) % CELLS_X          ][y                          ];
            cnt += cur[(x + 1) % CELLS_X          ][(y + 1) % CELLS_Y          ];

            if (cnt == 3) // birth
                buf[x][y] = 1;
            if (cur[x][y] && cnt == 2) // overcrowded / lonely
                buf[x][y] = 1;
        }
    }

    memcpy(&cur[0][0], &buf[0][0], CELLS_X * CELLS_Y * sizeof(buf[0][0]));
}

void update(SDL_Renderer *renderer, SDL_Texture *buf) {
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, buf, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void render_grid(SDL_Renderer *renderer, SDL_Texture *buf) {
    SDL_SetRenderTarget(renderer, buf);
    SDL_SetRenderDrawColor(renderer, gridColor->r, gridColor->g, gridColor->b, 255);
    for (int x = 0; x < CELLS_X; ++x) {
        SDL_RenderDrawLine(renderer, x * CELL_SIZE, 0, x * CELL_SIZE, CELLS_Y * CELL_SIZE);
        SDL_RenderDrawLine(renderer, (x + 1) * CELL_SIZE - 1, 0, (x + 1) * CELL_SIZE - 1, CELLS_Y * CELL_SIZE);
    }
    for (int y = 0; y < CELLS_Y; ++y) {
        SDL_RenderDrawLine(renderer, 0, y * CELL_SIZE, CELLS_X * CELL_SIZE, y * CELL_SIZE);
        SDL_RenderDrawLine(renderer, 0, (y + 1) * CELL_SIZE - 1, CELLS_X * CELL_SIZE, (y + 1) * CELL_SIZE - 1);
    }
}

void render(SDL_Renderer *renderer, SDL_Texture *buf) {
    SDL_SetRenderTarget(renderer, buf);
    for (int x = 0; x < CELLS_X; ++x) {
        for (int y = 0; y < CELLS_Y; ++y) {
            SDL_Rect currentCell = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            Uint8 c = 255;
            if (cur[x][y])
                c = 0;
            SDL_SetRenderDrawColor(renderer, c, c, c, 255);
            SDL_RenderFillRect(renderer, &currentCell);
        }
    }
    render_grid(renderer, buf);
}

int main() {
    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window* win = SDL_CreateWindow("Conway\'s Game of Life",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       CELLS_X * CELL_SIZE, CELLS_Y * CELL_SIZE,
                                       SDL_WINDOW_SHOWN);

    if (win == NULL)
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());

    // create renderer
    Uint32 renderFlags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, renderFlags);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // create a texture to use as a buffer
    SDL_Texture *buf = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            CELLS_X * CELL_SIZE, CELLS_Y * CELL_SIZE);
    SDL_SetRenderTarget(renderer, buf);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    cur[1][2] = 1;
    cur[2][2] = 1;
    cur[3][2] = 1;

    bool quit = false;
    Uint64 start, end;
    while (!quit) {
        start = SDL_GetPerformanceCounter();

        // event loop
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_q:
                        case SDLK_ESCAPE:
                            quit = true;
                            break;

                        default: break;
                    }

                default: break;
            }
        }

        // rendering loop
        render(renderer, buf);
        update(renderer, buf);
        compute_buffer();

        end = SDL_GetPerformanceCounter();
        float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

        // cap to 4 fps
        // TODO: find a better way of controlling the speed
        SDL_Delay(floor(250.0f - elapsedMS));
    }

    SDL_DestroyWindow(win);
    SDL_Quit();

    std::free(gridColor);

    return 0;
}
