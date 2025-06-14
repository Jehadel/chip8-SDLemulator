#include <string.h>
#include <SDL2/SDL.h>

// MEMORY 
#define MEMORY_SIZE 4096
#define START_ADDRESS 512

// DISPLAY
#define BLACK SDL_FALSE
#define WHITE SDL_TRUE
#define PIXEL_BY_WIDTH 64
#define PIXEL_BY_HEIGHT 32
#define PIXEL_DIM 8
#define WIDTH  PIXEL_BY_WIDTH*PIXEL_DIM 
#define HEIGHT PIXEL_BY_HEIGHT*PIXEL_DIM

typedef SDL_bool s_pixel;

////////////////
// GFX FUNCTIONS
////////////////
struct s_screen
{
    SDL_Window *w;
    SDL_Renderer *r;
    s_pixel pixels[PIXEL_BY_WIDTH][PIXEL_BY_HEIGHT];
    Uint32 pixel_height;
    Uint32 pixel_width;
};

void clear_screen(struct s_screen *screen)
{
    memset(screen->pixels, BLACK, sizeof(screen->pixels));  
}

void destroy_screen(struct s_screen *screen)
{
    SDL_DestroyRenderer(screen->r);
    SDL_DestroyWindow(screen->w);
}

int initialize_screen(struct s_screen *screen)
{
    screen->w = SDL_CreateWindow(
        "CHIP8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (screen->w == NULL)
        {
            fprintf(stderr, "Error SDL_CreateWindow: %s\n", SDL_GetError());
            return -1;
        }
    screen->r = SDL_CreateRenderer(screen->w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (screen->r ==NULL)
    {
        fprintf(stderr, "Error SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(screen->w);
        return -2;
    }
    
    clear_screen(screen);
    screen->pixel_width = PIXEL_DIM;
    screen->pixel_height = PIXEL_DIM;
    return 0;
}

void update_screen(struct s_screen *screen)
{
    SDL_SetRenderDrawColor(screen->r, 0, 0, 0, 255);
    SDL_RenderClear(screen->r);
    SDL_SetRenderDrawColor(screen->r, 255, 255, 255, 255);
    for(size_t i = 0; i< PIXEL_BY_WIDTH; i++){
        for(size_t j = 0; j< PIXEL_BY_HEIGHT; j++)
        {
            if (screen->pixels[i][j] == WHITE)
            {
                SDL_Rect pixel_rect =
                {
                    screen->pixel_width * i,
                    screen->pixel_height * j,
                    screen->pixel_width,
                    screen->pixel_height
                };
                SDL_RenderFillRect(screen->r, &pixel_rect);
            }
        }
    }
    SDL_RenderPresent(screen->r);
}

////////////////
// CPU FUNCTIONS
////////////////
struct s_cpu {
    Uint8 memory[MEMORY_SIZE];
    Uint16 pc;
    Uint8 V[16];
    Uint16 I;
    Uint16 jump[16];
    Uint8 jump_nb;
    Uint8 sys_counter;
    Uint8 sound_counter;
};
void initialize_cpu(struct  s_cpu *cpu)
{
    memset(cpu, 0, sizeof(*cpu));
    cpu -> pc = START_ADDRESS;
}

void clock(struct s_cpu *cpu)
{
    if(cpu->sys_counter > 0)
        cpu->sys_counter--;
    if(cpu->sound_counter > 0)
        cpu->sound_counter--;
}

int load_rom(struct s_cpu *cpu, const char path[])
{
    FILE *rom = fopen(path, "rb");
    if(!rom)
    {
        perror("Error fopen!");
        return -1;
    }
    fread(&cpu->memory[START_ADDRESS], sizeof(Uint8) *(MEMORY_SIZE - START_ADDRESS), 1, rom);
    fclose( rom);
    return 0;
}

//////////////////
// INPUT FUNCTIONS
//////////////////
struct s_input {
    SDL_bool key[SDL_NUM_SCANCODES];
    SDL_bool quit;
    int x, y, xrel, yrel;
    int xwheel, ywheel;
    SDL_bool mouse[6];
    SDL_bool resize;
};

void update_event(struct s_input *input)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_QUIT)
            input->quit = SDL_TRUE;
        else if (event.type == SDL_KEYDOWN)
            input->key[event.key.keysym.scancode] = SDL_TRUE;
        else if (event.type == SDL_KEYUP)
            input->key[event.key.keysym.scancode] = SDL_FALSE;
        else if (event.type == SDL_MOUSEMOTION)
        {
            input->x = event.motion.x;
            input->y = event.motion.y;
            input->xrel = event.motion.xrel;
            input->yrel = event.motion.yrel;
        }
        else if (event.type == SDL_MOUSEWHEEL)
        {
            input->xwheel = event.wheel.x;
            input->ywheel = event.wheel.y;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
            input->mouse[event.button.button] = SDL_TRUE;
        else if (event.type == SDL_MOUSEBUTTONUP)
            input->mouse[event.button.button] = SDL_FALSE;
        else if (event.type == SDL_WINDOWEVENT)
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                input->resize = SDL_TRUE;
    }
}

/////////////////////
// EMULATOR FUNCTIONS
/////////////////////
struct s_emulator{
    struct s_cpu cpu;
    struct s_screen screen;
    struct s_input input;
};

int initialize_SDL(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Error SDL_Init: %s.\n", SDL_GetError());
        return -1;
    }
    return 0;
}

void destroy_SDL(void)
{
    SDL_Quit();
}

int initialize_emulator(struct s_emulator *emulator)
{
    int status = -1;
    initialize_cpu(&emulator->cpu);
    memset(&emulator->input, 0, sizeof(emulator->input));
    if(0 == initialize_SDL())
    {
        status = initialize_screen(&emulator->screen);
        if(0 > status)
            destroy_SDL();
    }
    return status;
}

void emulate(struct s_emulator *emulator)
{
    while(!emulator->input.quit)
    {
        update_event(&emulator->input);
        update_screen(&emulator->screen);
        SDL_Delay(20);
    }
}

void destroy_emulator(struct s_emulator *emulator)
{
    destroy_screen(&emulator->screen);
    destroy_SDL();
}

///////
// MAIN
///////
int main(int argc, char *argv[])
{
    struct s_emulator emulator = {0};
    int status = -1;
    if(!initialize_emulator(&emulator))
    {
        status = 0;
        emulate(&emulator);
        destroy_emulator(&emulator);
    }
    printf("Status : %i", status);
    return status;
}