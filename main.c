#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <SDL3/SDL.h>

#include "font.h"
#include "stocks.h"
#include "image.h"
#include "log.h"

/* Window size */
#define SCREEN_WIDTH  341
#define SCREEN_HEIGHT 270

/* Footer layout */
#define FOOTER_X         21
#define FOOTER_Y        222
#define FOOTER_MAX_WIDTH 292

/* Helper macros */
#define SAFE_TEXT(s, fallback) ((s) && (s)[0] ? (s) : (fallback))

/* Global SDL objects */
SDL_Renderer *renderer;
static SDL_Window *window;
static SDL_Texture *bg_tex;

/* Fonts */
static TTF_Font *font_13pt;
static TTF_Font *font_16pt;
static TTF_Font *font_18pt;
static TTF_Font *font_40pt;

/* Colors */
static SDL_Color color_green = {0,255,0,SDL_ALPHA_OPAQUE};
static SDL_Color color_red   = {255,0,0,SDL_ALPHA_OPAQUE};
static SDL_Color color_white = {255,255,255,SDL_ALPHA_OPAQUE};
static SDL_Color color_gray  = {193,195,195,SDL_ALPHA_OPAQUE};

/* Status indicator textures */
static SDL_Texture *arrow_up_tex   = NULL;
static SDL_Texture *arrow_down_tex = NULL;
static SDL_Texture *arrow_flat_tex = NULL;

/* Rendered text objects */
struct rendered_text txt_main_percent;
struct rendered_text txt_main_name;
struct rendered_text txt_footer;
struct rendered_text txt_price;

struct rendered_text txt_sub_symbol[MAX_SUB];
struct rendered_text txt_sub_percent[MAX_SUB];
struct rendered_text txt_sub_price[MAX_SUB];

/* Stocks info (populated externally) */
static struct stocks_info si = {0};

/* Command-line arguments */
static struct args {
	const char *execute_command;
	Uint32 update_stocks_time_ms;
	int x;
	int y;
} args = {
	.execute_command = NULL,
	.update_stocks_time_ms = 300 * 1000,
	.x = -1,
	.y = -1
};

/* Forward declarations */
static void create_texts(void);
static Uint32 update_stocks_cb(void *userdata, SDL_TimerID timerID, Uint32 interval);

/* Render full frame (background, arrows, texts) */
static inline void update_frame(void)
{
	SDL_RenderClear(renderer);
	SDL_RenderTexture(renderer, bg_tex, NULL, NULL);

	if (arrow_up_tex)   image_render(arrow_up_tex, 0, 0);
	if (arrow_down_tex) image_render(arrow_down_tex, 0, 0);
	if (arrow_flat_tex) image_render(arrow_flat_tex, 0, 0);

	int margin = 30;
	font_render_text(&txt_main_percent, SCREEN_WIDTH - txt_main_percent.width - margin, 20);
	font_render_text(&txt_price,        SCREEN_WIDTH - txt_price.width        - margin, 70);
	font_render_text(&txt_main_name,    SCREEN_WIDTH - txt_main_name.width    - margin, 115);
	font_render_text(&txt_footer, FOOTER_X, FOOTER_Y);

	for (int i = 0; i < si.sub_count; ++i) {
		font_render_text(&txt_sub_symbol[i],    20 + 110 * i, 150);
		font_render_text(&txt_sub_percent[i],   20 + 110 * i, 170);
		font_render_text(&txt_sub_price[i], 20 + 110 * i, 190);
	}

	SDL_RenderPresent(renderer);
}

/* Create window and renderer */
static int create_sdl_window(int w, int h, int flags)
{
	if (!SDL_CreateWindowAndRenderer("stocks-widget", w, h, flags, &window, &renderer))
		log_panic("Unable to create window and renderer!\n");

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	if (args.x >= 0 && args.y >= 0)
		SDL_SetWindowPosition(window, args.x, args.y);

	return 0;
}

/* Update stock info and trigger redraw */
static void update_stocks_info(void)
{
	log_info("Updating stocks info...\n");

	if (stocks_get(args.execute_command, &si) < 0)
		log_err_to(out, "Unable to get stocks info!\n");

	image_load(&bg_tex, "assets/bg_dark.png");
	create_texts();

out:
	SDL_AddTimer(args.update_stocks_time_ms, update_stocks_cb, NULL);
}

/* Load all fonts */
static void load_fonts(void)
{

	font_13pt = font_open("assets/fonts/NotoSans-Regular.ttf", 14);
	if (!font_13pt) log_panic("Unable to open font with size 14pt!\n");

	font_16pt = font_open("assets/fonts/NotoSans-Regular.ttf", 16);
	if (!font_16pt) log_panic("Unable to open font with size 16pt!\n");

	font_18pt = font_open("assets/fonts/NotoSans-Regular.ttf", 18);
	if (!font_18pt) log_panic("Unable to open font with size 18pt!\n");

	font_40pt = font_open("assets/fonts/NotoSans-Regular.ttf", 40);
	if (!font_40pt) log_panic("Unable to open font with size 40pt!\n");
}

/* Create and update rendered texts/arrows */
void create_texts(void)
{
	char buff[128];

	SDL_Color percent_col;
	if (si.percentChange == 0)
		percent_col = color_gray;
	else if (si.percentChange > 0)
		percent_col = color_green;
	else
		percent_col = color_red;

	snprintf(buff, sizeof buff, "%.2f%%", si.percentChange);
	font_create_text(&txt_main_percent, font_40pt, buff, &percent_col, 0);

	image_free(&arrow_up_tex);
	image_free(&arrow_down_tex);
	image_free(&arrow_flat_tex);

	if (si.percentChange > 0.01f)
		image_load(&arrow_up_tex, "assets/arrow_up_green.png");
	else if (si.percentChange < -0.01f)
		image_load(&arrow_down_tex, "assets/arrow_down_red.png");
	else
		image_load(&arrow_flat_tex, "assets/arrow_flat_gray.png");

	snprintf(buff, sizeof buff, "%.2f %s", si.price, SAFE_TEXT(si.currency, ""));
	font_create_text(&txt_price, font_18pt, buff, &color_white, 0);

	font_create_text(&txt_main_name, font_18pt, SAFE_TEXT(si.longName, "N/A"), &color_white, 0);
	font_create_text(&txt_footer,    font_16pt, SAFE_TEXT(si.provider, "Dummy"), &color_gray, 0);

	for (int i = 0; i < si.sub_count; ++i) {
		font_create_text(&txt_sub_symbol[i], font_16pt, SAFE_TEXT(si.sub[i].symbol, "?"), &color_white, 0);

		SDL_Color sub_col = (si.sub[i].percentChange >= 0) ? color_green : color_red;
		snprintf(buff, sizeof buff, "%.2f%%", si.sub[i].percentChange);
		font_create_text(&txt_sub_percent[i], font_16pt, buff, &sub_col, 0);

		snprintf(buff, sizeof buff, "%.2f %s", si.sub[i].price, SAFE_TEXT(si.sub[i].currency, ""));
		font_create_text(&txt_sub_price[i], font_13pt, buff, &color_gray, 0);
	}
}

/* Free textures, fonts and stock data */
void free_resources(void)
{
	image_free(&bg_tex);
	image_free(&arrow_up_tex);
	image_free(&arrow_down_tex);
	image_free(&arrow_flat_tex);

	font_destroy_text(&txt_main_percent);
	font_destroy_text(&txt_main_name);
	font_destroy_text(&txt_price);
	font_destroy_text(&txt_footer);

	for (int i = 0; i < MAX_SUB; ++i) {
		font_destroy_text(&txt_sub_symbol[i]);
		font_destroy_text(&txt_sub_percent[i]);
		font_destroy_text(&txt_sub_price[i]);
	}

	font_close(font_13pt);
	font_close(font_16pt);
	font_close(font_18pt);
	font_close(font_40pt);

	stocks_free(&si);
}

/* SDL timer callback to trigger updates via event */
static Uint32 update_stocks_cb(void *userdata, SDL_TimerID timerID, Uint32 interval)
{
	(void)userdata; (void)timerID; (void)interval;

	SDL_Event event;
	event.type = SDL_EVENT_USER;
	event.user.data1 = NULL;
	SDL_PushEvent(&event);
	return 0;
}

/* Help Page */
void usage(const char *prgname)
{
	fprintf(stderr,
		"Usage: %s [options] -c <command-to-run>\n\n"
		"Options:\n"
		"  -t <secs>    Interval (seconds) for stock updates (default: 300)\n"
		"  -c <command> Command to execute for updates (required)\n"
		"  -x <pos>     Window X coordinate\n"
		"  -y <pos>     Window Y coordinate\n"
		"  -h           Help\n\n"
		"Example:\n"
		"  %s -t 300 -c \"python3 stocks_request.py\"\n\n",
		prgname, prgname);
	exit(EXIT_FAILURE);
}

/* Parse command-line arguments */
void parse_args(int argc, char **argv)
{
	int c;
	while ((c = getopt(argc, argv, "t:c:x:y:h")) != -1) {
		switch (c) {
		case 'h': usage(argv[0]); break;
		case 't':
			args.update_stocks_time_ms = atoi(optarg) * 1000;
			if (!args.update_stocks_time_ms) {
				log_info("Invalid -t value!\n");
				usage(argv[0]);
			}
			break;
		case 'c': args.execute_command = optarg; break;
		case 'x': args.x = atoi(optarg); break;
		case 'y': args.y = atoi(optarg); break;
		default: usage(argv[0]); break;
		}
	}

	if (!args.execute_command) {
		log_info("Option -c is required!\n");
		usage(argv[0]);
	}
}

/* Main entry point */
int main(int argc, char **argv)
{
	parse_args(argc, argv);

	SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");

	if (!SDL_Init(SDL_INIT_VIDEO))
		log_panic("SDL could not initialize!: %s\n", SDL_GetError());
	if (font_init() < 0)
		log_panic("Unable to initialize SDL_ttf!\n");

	const char *base_path = SDL_GetBasePath();
	if (!base_path)
		log_panic("Unable to get program base path!\n");
	chdir(base_path);

	create_sdl_window(
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_TRANSPARENT | SDL_WINDOW_BORDERLESS | SDL_WINDOW_UTILITY);

	image_load(&bg_tex, "assets/bg_dark.png");
	load_fonts();
	update_stocks_info();

	SDL_SetEventEnabled(SDL_EVENT_CLIPBOARD_UPDATE, 0);

	SDL_Event event;
	while (SDL_WaitEvent(&event) != 0) {
		if (event.type == SDL_EVENT_QUIT)
			break;
		else if (event.type == SDL_EVENT_USER) {
			update_stocks_info();
			update_frame();
		} else if ((event.type >= SDL_EVENT_WINDOW_FIRST  && event.type <= SDL_EVENT_WINDOW_LAST) ||
		           (event.type >= SDL_EVENT_DISPLAY_FIRST && event.type <= SDL_EVENT_DISPLAY_LAST)) {
			update_frame();
		}
	}

	free_resources();

	if (renderer) SDL_DestroyRenderer(renderer);
	if (window)   SDL_DestroyWindow(window);

	font_quit();
	SDL_Quit();
	return 0;
}
