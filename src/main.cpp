#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <string>

// default number of cells
#define CELLS_X 40
#define CELLS_Y 30
#define CELL_SIZE 20

// update rate
#define UPDATE 500.0f

// keybinds
#define K_QUIT SDLK_q
#define K_CLEAR SDLK_c
#define K_PAUSE SDLK_SPACE
#define K_SAVE SDLK_s
#define K_OPEN SDLK_o

// arrays for the cells
bool cur[CELLS_X][CELLS_Y] = {0};
bool swap[CELLS_X][CELLS_Y] = {0};

struct Color {
    Uint8 r, g, b;

    Color() {}
    Color(Uint8 r, Uint8 g, Uint8 b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
};

Color *gridColor = new Color(200, 200, 200);
Color *backgroundColor = new Color(255, 255, 255);
Color *foregroundColor = new Color(0, 0, 0);

void compute_buffer() {
    memset(swap, 0, sizeof(swap));

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
                swap[x][y] = 1;
            if (cur[x][y] && cnt == 2) // overcrowded / lonely
                swap[x][y] = 1;
        }
    }

    memcpy(&cur[0][0], &swap[0][0], CELLS_X * CELLS_Y * sizeof(swap[0][0]));
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
            Color *c = backgroundColor;
            if (cur[x][y])
                c = foregroundColor;
            SDL_SetRenderDrawColor(renderer, c->r, c->g, c->b, 255);
            SDL_RenderFillRect(renderer, &currentCell);
        }
    }
    render_grid(renderer, buf);
}

void color_cell(int x, int y, bool color) {
    int cell_x = x / CELL_SIZE;
    int cell_y = y / CELL_SIZE;
    cur[cell_x][cell_y] = color;
}

void clear() {
    memset(cur, 0, sizeof(cur));
}

void save_to_file() {
    std::ofstream file;
    std::string filename;
    std::cout << "Please enter the name of the file you want to write to: ";
    std::cin >> filename;
    file.open(filename);
    file << CELLS_X << " " << CELLS_Y << std::endl;
    for (int x = 0; x < CELLS_X; ++x) {
        for (int y = 0; y < CELLS_Y; ++y) {
            if (cur[x][y])
                file << x << " " << y << std::endl;
        }
    }
    file.close();
}

void open_from_file() {
    std::ifstream file;
    std::string filename;
    bool compatible = true;
    std::cout << "Please enter the name of the file you want to read from: ";
    std::cin >> filename;
    file.open(filename);
    int cells_x, cells_y;
    file >> cells_x >> cells_y;
    if (cells_x > CELLS_X) {
        printf("Please increment CELLS_X to be greater than %d", cells_x);
        compatible = false;
    }
    if (cells_y > CELLS_Y) {
        printf("Please increment CELLS_Y to be greater than %d", cells_y);
        compatible = false;
    }
    if (!compatible)
        return;

    int x, y;
    while (file >> x >> y)
        cur[x][y] = 1;

    file.close();
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

    bool quit = false;
    bool rendering = false;
    Uint64 start, end;
    int mouse_x, mouse_y;
    start = SDL_GetPerformanceCounter();
    render(renderer, buf);
    update(renderer, buf);
    compute_buffer();
    while (!quit) {
        // event loop
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case K_QUIT:
                        case SDLK_ESCAPE:
                            quit = true;
                            break;

                        case K_CLEAR:
                            if (!rendering) {
                                clear();
                                render(renderer, buf);
                                update(renderer, buf);
                            }
                            break;

                        case K_PAUSE:
                            rendering = !rendering;
                            break;

                        case K_SAVE:
                            if (rendering)
                                break;
                            save_to_file();
                            break;

                        case K_OPEN:
                            if (rendering)
                                break;
                            clear();
                            open_from_file();
                            render(renderer, buf);
                            update(renderer, buf);
                            break;

                        default: break;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (rendering)
                        break;
                    switch (event.button.button) {
                        case SDL_BUTTON_LEFT:
                            SDL_GetMouseState(&mouse_x, &mouse_y);
                            color_cell(mouse_x, mouse_y, 1);
                            render(renderer, buf);
                            update(renderer, buf);
                            break;

                        case SDL_BUTTON_RIGHT:
                            SDL_GetMouseState(&mouse_x, &mouse_y);
                            color_cell(mouse_x, mouse_y, 0);
                            render(renderer, buf);
                            update(renderer, buf);
                            break;

                        default: break;
                    }

                case SDL_MOUSEMOTION:
                    if (rendering)
                        break;
                    Uint32 state;
                    state = SDL_GetMouseState(&mouse_x, &mouse_y);
                    if (state & SDL_BUTTON_LMASK) {
                        color_cell(mouse_x, mouse_y, 1);
                        render(renderer, buf);
                        update(renderer, buf);
                    }
                    if (state & SDL_BUTTON_RMASK) {
                        color_cell(mouse_x, mouse_y, 0);
                        render(renderer, buf);
                        update(renderer, buf);
                    }
                    break;

                default: break;
            }
        }

        // rendering loop
        end = SDL_GetPerformanceCounter();
        float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        if (rendering && elapsed > UPDATE) {
            render(renderer, buf);
            update(renderer, buf);
            compute_buffer();
            start = SDL_GetPerformanceCounter();
            end = start;
        }
    }

    SDL_DestroyWindow(win);
    SDL_Quit();

    std::free(gridColor);
    std::free(backgroundColor);
    std::free(foregroundColor);

    return 0;
}
