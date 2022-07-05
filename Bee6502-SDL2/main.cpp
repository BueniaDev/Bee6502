#include <bee6502.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <vector>
#include <array>
#include <cassert>
using namespace bee6502;
using namespace std;
using namespace std::placeholders;

class Easy6502Interface : public Bee6502Interface
{
    struct Easy6502RGB
    {
	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
    };

    public:
	Easy6502Interface(Bee6502 &cb) : core(cb)
	{
	    core.setinterface(this);
	}

	~Easy6502Interface()
	{

	}

	bool load_file(string filename)
	{
	    ifstream file(filename, ios::in | ios::binary | ios::ate);

	    if (!file.is_open())
	    {
		cout << "Could not open " << filename << endl;
		return false;
	    }

	    streampos size = file.tellg();
	    rom_data.resize(size, 0);
	    file.seekg(0, ios::beg);
	    file.read((char*)rom_data.data(), rom_data.size());
	    cout << filename << " succesfully loaded." << endl;
	    file.close();

	    load_code(rom_data);

	    return true;
	}

	void load_code(vector<uint8_t> mach_code)
	{
	    // Copy code to offset of 0x600
	    copy(mach_code.begin(), mach_code.end(), (memory.begin() + 0x600));
	    // Initialize reset vector to 0x600
	    memory[0xFFFC] = 0x00;
	    memory[0xFFFD] = 0x06;
	}

	bool init()
	{
	    if (SDL_Init(SDL_INIT_VIDEO) < 0)
	    {
		return sdl_error("SDL2 could not be initialized!");
	    }

	    window = SDL_CreateWindow("Bee6502-SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		(width * scale), (height * scale), SDL_WINDOW_SHOWN);

	    if (window == NULL)
	    {
		return sdl_error("Window could not be created!");
	    }

	    render = SDL_CreateRenderer(window, -1, 0);

	    if (render == NULL)
	    {
		return sdl_error("Renderer could not be created!");
	    }

	    texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
		width, height);

	    if (texture == NULL)
	    {
		return sdl_error("Texture could not be created!");
	    }

	    SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	    core.init();
	    is_stopped = false;

	    return true;
	}

	void run()
	{
	    while (!quit)
	    {
		handle_events();
		run_machine();
		render_pixels();
	    }
	}

	void handle_events()
	{
	    while (SDL_PollEvent(&event))
	    {
		switch (event.type)
		{
		    case SDL_QUIT: quit = true; break;
		}
	    }
	}

	void render_pixels()
	{
	    assert(render && texture);
	    SDL_UpdateTexture(texture, NULL, framebuffer.data(), (width * sizeof(Easy6502RGB)));
	    SDL_RenderClear(render);
	    SDL_RenderCopy(render, texture, NULL, NULL);
	    SDL_RenderPresent(render);
	}

	void run_machine()
	{
	    if (!is_stopped)
	    {
		int cycles = core.runinstruction();
		core.debugoutput();
		cout << "Cycles: " << dec << cycles << endl;
		cout << endl;
	    }

	    if ((core.getStatus().pc == 0) && !is_stopped)
	    {
		uint16_t addr = (core.getStatus().sp + 2);
		uint16_t pc_val = (((memory[addr + 1] << 8) | memory[addr]) - 2);
		cout << "Program stopped at $" << hex << int(pc_val) << endl;
		is_stopped = true;
	    }

	    update_pixels();
	}

	void shutdown()
	{
	    if (texture != NULL)
	    {
		SDL_DestroyTexture(texture);
		texture = NULL;
	    }

	    if (render != NULL)
	    {
		SDL_DestroyRenderer(render);
		render = NULL;
	    }

	    if (window != NULL)
	    {
		SDL_DestroyWindow(window);
		window = NULL;
	    }

	    SDL_Quit();
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0;
	    if ((addr >= 0x200) && (addr < 0x600))
	    {
		data = pixel_ram[(addr - 0x200)];
	    }
	    else
	    {
		data = memory[addr];
	    }

	    return data;
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    if ((addr >= 0x200) && (addr < 0x600))
	    {
		pixel_ram[(addr - 0x200)] = data;
	    }
	    else
	    {
		memory[addr] = data;
	    }
	}

	void update_pixels()
	{
	    for (int i = 0; i < 0x400; i++)
	    {
		int color_val = (pixel_ram[i] & 0xF);
		int ypos = (i / 32);
		int xpos = (i % 32);

		fillRect((xpos * 5), (ypos * 5), 5, 5, get_color(color_val));
	    }
	}

	void fillRect(int x, int y, int w, int h, Easy6502RGB color)
	{
	    for (int i = 0; i < h; i++)
	    {
		for (int j = 0; j < w; j++)
		{
		    int screen_pos = ((x + j) + ((y + i) * width));
		    framebuffer[screen_pos] = color;
		}
	    }
	}

	Easy6502RGB get_color(int color_val)
	{
	    color_val &= 0xF;

	    switch (color_val)
	    {
		case 0: return {0x00, 0x00, 0x00}; break;
		case 1: return {0xFF, 0xFF, 0xFF}; break;
		case 2: return {0x88, 0x00, 0x00}; break;
		case 3: return {0xAA, 0xFF, 0xEE}; break;
		case 4: return {0xCC, 0x44, 0xCC}; break;
		case 5: return {0x00, 0xCC, 0x55}; break;
		case 6: return {0x00, 0x00, 0xAA}; break;
		case 7: return {0xEE, 0xEE, 0x77}; break;
		case 8: return {0xDD, 0x88, 0x55}; break;
		case 9: return {0x66, 0x44, 0x00}; break;
		case 10: return {0xFF, 0x77, 0x77}; break;
		case 11: return {0x33, 0x33, 0x33}; break;
		case 12: return {0x77, 0x77, 0x77}; break;
		case 13: return {0xAA, 0xFF, 0x66}; break;
		case 14: return {0x00, 0x88, 0xFF}; break;
		case 15: return {0xBB, 0xBB, 0xBB}; break;
	    }

	    return {0x00, 0x00, 0x00};
	}

    private:
	SDL_Window *window = NULL;
	SDL_Renderer *render = NULL;
	SDL_Texture *texture = NULL;

	Bee6502 &core;

	bool quit = false;
	SDL_Event event;

	bool is_stopped = false;

	const int width = 160;
	const int height = 160;
	const int scale = 2;

	array<Easy6502RGB, (160 * 160)> framebuffer;

	vector<uint8_t> rom_data;
	array<uint8_t, 0x10000> memory;
	array<uint8_t, 0x400> pixel_ram;

	bool sdl_error(string message)
	{
	    cout << message << " SDL_Error: " << SDL_GetError() << endl;
	    return false;
	}
};

int main(int argc, char *argv[])
{
    /*
    if (argc < 2)
    {
	cout << "Usage: Bee6502-SDL2 <ROM>" << endl;
	return 1;
    }

    string filename = argv[1];
    */
    Bee6502 core;
    Easy6502Interface *inter = new Easy6502Interface(core);

    /*
    if (!inter->load_file(filename))
    {
	return 1;
    }
    */

    inter->load_code({
	0x20, 0x09, 0x06,
	0x20, 0x0C, 0x06,
	0x20, 0x12, 0x06,
	0xA2, 0x00,
	0x60,
	0xE8,
	0xE0, 0x05,
	0xD0, 0xFB,
	0x60,
	0x00
    });

    if (!inter->init())
    {
	inter->shutdown();
	return 1;
    }

    inter->run();
    core.shutdown();
    inter->shutdown();
    inter = NULL;
    return 0;
}