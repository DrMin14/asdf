#define SDL_MAIN_HANDLED
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#pragma comment(lib, "./lib/SDL2.lib")
#pragma comment(lib, "./lib/SDL2_image.lib")
#pragma comment(lib, "./lib/SDL2_mixer.lib")

#include "../include/SDL/SDL.h"
#include "../include/SDL/SDL_image.h"
#include "../include/SDL/SDL_mixer.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>

#define OK 10
#define NO 11

#define MISS	-500
#define BAD		 100
#define NICE	 500
#define PERFECT 1000

#define find_ed(_str) (info.find(_str) + sizeof(_str) - 1)

using _p_window_t	= SDL_Window*;
using _p_renderer_t = SDL_Renderer*;
using _event_t		= SDL_Event;
using _surface_t	= SDL_Surface;
using _p_texture_t	= SDL_Texture*;
using _point_t		= SDL_Point;
using _fpoint_t		= SDL_FPoint;
using _rect_t		= SDL_Rect;
using _frect_t		= SDL_FRect;
using _p_music_t	= Mix_Music*;


struct
{
	bool			is_run		= false;
	_p_window_t		window		= nullptr;
	_p_renderer_t	renderer	= nullptr;
	_p_texture_t	font		= nullptr;
	_p_texture_t	resource	= nullptr;
	_event_t		ev			= {};

	void(*update)	(void)		= nullptr;
	void(*rendering)(void)		= nullptr;

	bool		is_pause	= false;
	int64_t		paused_time	= 0;
	void(*paused_update)	(void)	= nullptr;
	void(*paused_rendering)	(void)	= nullptr;

	float fps;

	struct
	{
		int			key  [4] = {};
		uint8_t		input[4] = {};
		int64_t		time [4] = { -1, -1, -1, -1 };
		uint64_t	combo		= 0;
		int64_t		score		= 0;
		float		note_speed	= 0.5f;
		int64_t		delay		= 0;
		int			fullscreen	= 0;
		int64_t		scope		= 0;
		std::vector<std::string> track_list;
		int64_t		waiting		= 3000;
		const uint8_t* kb_state = nullptr;
	}user;
	struct
	{
		uint64_t st_time = 0;
		uint64_t lt_time = 0;

		int64_t ready = 3000;

		const uint64_t range_bad		= 177	/ 2;
		const uint64_t range_nice		= 110	/ 2;
		const uint64_t range_perfect	= 84	/ 2;

		int32_t	hit		= NULL;
		int64_t	score	= NULL;
		
		int32_t	pad			= 0;
		bool	is_played	= false;
		bool	is_paused	= false;
	}game;
}app_info;

enum
{
	  line0 = 0x01
	, line1 = 0x02
	, line2 = 0x04
	, line3 = 0x08
};

struct
{
	std::string title;
	uint64_t	len;
	int64_t		now;
	uint64_t	size;
	uint64_t	nc;
	std::vector<int32_t>	time;
	std::vector<uint8_t>	notes;
	_p_music_t	music;
}track;

inline void render_bar();
void		render_font(char _char, const _frect_t* _dst);
inline void render_scoreboard();
inline void	render_str(const char* _str, _fpoint_t _pos, float _scale);
inline void render_notes();

void		load_track_list();

int			load_track(const char* _file);
void		unload_track();

void		rendering_menu();
void		rendering_game();
void		rendering_ready_game();
void		rendering_sub_menu();
void		rendering_setting_menu();

void		hit_test();
void		recording_score();

void		game();
void		game_set();
void		game_quit();

void		pause();
void		resume();

void		sub_menu();
void		setting_menu();
void		menu();

void		user_init();
bool		init();
void		quit();

void		sys_porc();
void		process();



int 
main(int argc, char* argv[])
{
	if (init()) return -1;

lp: process();
	app_info.update();
	app_info.rendering();
	if (app_info.is_run) 
		goto lp;
	
	quit();
	return 0;
}

inline void 
render_bar()
{
	static const _rect_t src = { 97, 582, 320, 47 };
	static const _rect_t dst = {  0, 529, 320, 47 };
	SDL_RenderCopy(app_info.renderer, app_info.resource, &src, &dst);

	static const _rect_t ht_src  = {   0, 220, 73, 33 };
	static const _rect_t ht_dst0 = {   4, 533, 73, 33 };
	static const _rect_t ht_dst1 = {  83, 533, 73, 33 };
	static const _rect_t ht_dst2 = { 162, 533, 73, 33 };
	static const _rect_t ht_dst3 = { 241, 533, 73, 33 };
	if (app_info.user.input[0]) SDL_RenderCopy(app_info.renderer, app_info.resource, &ht_src, &ht_dst0);
	if (app_info.user.input[1]) SDL_RenderCopy(app_info.renderer, app_info.resource, &ht_src, &ht_dst1);
	if (app_info.user.input[2]) SDL_RenderCopy(app_info.renderer, app_info.resource, &ht_src, &ht_dst2);
	if (app_info.user.input[3]) SDL_RenderCopy(app_info.renderer, app_info.resource, &ht_src, &ht_dst3);
	
	float len = track.now / (float)track.len;		
	_frect_t bar = { 0, 529, 320 * len, 2 };
	SDL_SetRenderDrawColor	(app_info.renderer, 0xf0, 0xff, 0xff, 0xff);
	SDL_RenderFillRectF		(app_info.renderer, &bar);
	SDL_SetRenderDrawColor	(app_info.renderer, 0x00, 0x00, 0x00, 0xff);
}
void		
render_font(char _char, const _frect_t* _dst)
{
	_rect_t result = {};
	switch (_char)
	{
	case  OK: result = { 0x10 * 0x00, 0x10 * 0x05, 0x10, 0x10 }; break;
	case  NO: result = { 0x10 * 0x01, 0x10 * 0x05, 0x10, 0x10 }; break;
	case '!': result = { 0x10 * 0x06, 0x10 * 0x03, 0x10, 0x10 }; break;
	case ',': result = { 0x10 * 0x01, 0x10 * 0x04, 0x10, 0x10 }; break;
	case '-': result = { 0x10 * 0x06, 0x10 * 0x01, 0x10, 0x10 }; break; // n
	case '.': result = { 0x10 * 0x00, 0x10 * 0x04, 0x10, 0x10 }; break;
	case '0': result = { 0x10 * 0x06, 0x10 * 0x05, 0x10, 0x10 }; break;
	case '1': result = { 0x10 * 0x02, 0x10 * 0x04, 0x10, 0x10 }; break;
	case '2': result = { 0x10 * 0x03, 0x10 * 0x04, 0x10, 0x10 }; break;
	case '3': result = { 0x10 * 0x04, 0x10 * 0x04, 0x10, 0x10 }; break;
	case '4': result = { 0x10 * 0x05, 0x10 * 0x04, 0x10, 0x10 }; break;
	case '5': result = { 0x10 * 0x06, 0x10 * 0x04, 0x10, 0x10 }; break;
	case '6': result = { 0x10 * 0x02, 0x10 * 0x05, 0x10, 0x10 }; break;
	case '7': result = { 0x10 * 0x03, 0x10 * 0x05, 0x10, 0x10 }; break;
	case '8': result = { 0x10 * 0x04, 0x10 * 0x05, 0x10, 0x10 }; break;
	case '9': result = { 0x10 * 0x05, 0x10 * 0x05, 0x10, 0x10 }; break;
	case '?': result = { 0x10 * 0x05, 0x10 * 0x03, 0x10, 0x10 }; break;
	case 'a': result = { 0x10 * 0x00, 0x10 * 0x00, 0x10, 0x10 }; break;
	case 'b': result = { 0x10 * 0x01, 0x10 * 0x00, 0x10, 0x10 }; break;
	case 'c': result = { 0x10 * 0x02, 0x10 * 0x00, 0x10, 0x10 }; break;
	case 'd': result = { 0x10 * 0x03, 0x10 * 0x00, 0x10, 0x10 }; break;
	case 'e': result = { 0x10 * 0x04, 0x10 * 0x00, 0x10, 0x10 }; break;
	case 'f': result = { 0x10 * 0x05, 0x10 * 0x00, 0x10, 0x10 }; break;
	case 'g': result = { 0x10 * 0x06, 0x10 * 0x00, 0x10, 0x10 }; break;
	case 'h': result = { 0x10 * 0x00, 0x10 * 0x01, 0x10, 0x10 }; break;
	case 'i': result = { 0x10 * 0x01, 0x10 * 0x01, 0x10, 0x10 }; break;
	case 'j': result = { 0x10 * 0x02, 0x10 * 0x01, 0x10, 0x10 }; break;
	case 'k': result = { 0x10 * 0x03, 0x10 * 0x01, 0x10, 0x10 }; break;
	case 'l': result = { 0x10 * 0x04, 0x10 * 0x01, 0x10, 0x10 }; break;
	case 'm': result = { 0x10 * 0x05, 0x10 * 0x01, 0x10, 0x10 }; break;
	case 'n': result = { 0x10 * 0x06, 0x10 * 0x01, 0x10, 0x10 }; break;
	case 'o': result = { 0x10 * 0x00, 0x10 * 0x02, 0x10, 0x10 }; break;
	case 'p': result = { 0x10 * 0x01, 0x10 * 0x02, 0x10, 0x10 }; break;
	case 'q': result = { 0x10 * 0x02, 0x10 * 0x02, 0x10, 0x10 }; break;
	case 'r': result = { 0x10 * 0x03, 0x10 * 0x02, 0x10, 0x10 }; break;
	case 's': result = { 0x10 * 0x04, 0x10 * 0x02, 0x10, 0x10 }; break;
	case 't': result = { 0x10 * 0x05, 0x10 * 0x02, 0x10, 0x10 }; break;
	case 'u': result = { 0x10 * 0x06, 0x10 * 0x02, 0x10, 0x10 }; break;
	case 'v': result = { 0x10 * 0x00, 0x10 * 0x03, 0x10, 0x10 }; break;
	case 'w': result = { 0x10 * 0x01, 0x10 * 0x03, 0x10, 0x10 }; break;
	case 'x': result = { 0x10 * 0x02, 0x10 * 0x03, 0x10, 0x10 }; break;
	case 'y': result = { 0x10 * 0x03, 0x10 * 0x03, 0x10, 0x10 }; break;
	case 'z': result = { 0x10 * 0x04, 0x10 * 0x03, 0x10, 0x10 }; break;
	default: break;
	}
	SDL_RenderCopyF(app_info.renderer, app_info.font, &result, _dst);
}
inline void 
render_scoreboard()
{
	static const _rect_t src_perfect	= { 0, 176, 82, 43 };
	static const _rect_t src_nice		= { 0, 132, 82, 43 };
	static const _rect_t src_bad		= { 0,  88, 82, 43 };
	static const _rect_t src_miss		= { 0,  44, 82, 43 };
	static const _rect_t dst			= { 119, 400, 82, 43 };

	if		(PERFECT	== app_info.game.hit) 
		SDL_RenderCopy(app_info.renderer, app_info.resource, &src_perfect,	&dst);
	else if (NICE		== app_info.game.hit) 
		SDL_RenderCopy(app_info.renderer, app_info.resource, &src_nice,		&dst);
	else if (BAD		== app_info.game.hit) 
		SDL_RenderCopy(app_info.renderer, app_info.resource, &src_bad,		&dst);
	else if (MISS		== app_info.game.hit)
		SDL_RenderCopy(app_info.renderer, app_info.resource, &src_miss,		&dst);
	else;

	render_str(std::to_string(app_info.game.score).c_str(), { 119, 450 }, 16);


	// !miss!
	// !bad!
	// !nice!
	// !perfect!
}
inline void 
render_str(const char* _str, _fpoint_t _pos, float _scale)
{
	_frect_t dst = { _pos.x, _pos.y, 1 * _scale, 1 * _scale };
	for (int i = 0; _str[i]; ++i)
	{
		if ('\n' == _str[i])
		{
			dst.y += dst.h;
			dst.x = _pos.x;
		} else
		{
			if ('A' <= _str[i] && 'Z' >= _str[i])
				render_font(_str[i] + 32, &dst);
			else
				render_font(_str[i], &dst);
			dst.x += dst.w;
		}
	}
}
inline void 
render_notes()
{
	static const _rect_t	ht_src  = {   0, 0, 82, 43 };
	static _frect_t			ht_dst0 = {   0, 4, 82, 43 };
	static _frect_t			ht_dst1 = {  79, 4, 82, 43 };
	static _frect_t			ht_dst2 = { 158, 4, 82, 43 };
	static _frect_t			ht_dst3 = { 237, 4, 82, 43 };

	for (uint64_t i = track.nc; track.size > i; ++i)
	{
		const uint8_t& note = track.notes[i];
		float offset = (track.time[i] - track.now);
		float y = 533.f - offset * app_info.user.note_speed;
		if (y < -43) break;
		if ((note & 0b00001000)) ht_dst0.y = y,
			SDL_RenderCopyF(app_info.renderer, app_info.resource, &ht_src, &ht_dst0);
		if ((note & 0b00000100)) ht_dst1.y = y,
			SDL_RenderCopyF(app_info.renderer, app_info.resource, &ht_src, &ht_dst1);
		if ((note & 0b00000010)) ht_dst2.y = y,
			SDL_RenderCopyF(app_info.renderer, app_info.resource, &ht_src, &ht_dst2);
		if ((note & 0b00000001)) ht_dst3.y = y,
			SDL_RenderCopyF(app_info.renderer, app_info.resource, &ht_src, &ht_dst3);
	}	
}

void		
load_track_list()
{
	std::string path = "./game/track";
	app_info.user.track_list.clear();
	for (auto& file : std::filesystem::directory_iterator(path))
		app_info.user.track_list.push_back(file.path().filename().string());
}

int			
load_track(const char* _file)
{
	std::string	path("./game/track/" + std::string(_file));
	{
		std::ifstream	finfo	(path + "/info.txt");
		std::string		info;
		finfo	.seekg	(0, std::ios::end);
		info	.resize	(finfo.tellg());
		finfo	.seekg	(0, std::ios::beg);
		finfo	.read	(&info[0], info.size());

		for (size_t len = sizeof(" ") - 1;;)
		{	
			size_t itr = info.find(" ");
			if (std::string::npos == itr) break;
			info.erase(itr, len); 
		}	
		for (size_t len = sizeof("\t") - 1;;)
		{	
			size_t itr = info.find("\t");
			if (std::string::npos == itr) break;
			info.erase(itr, len); 
		}	

		for (size_t i = find_ed("title=");	info[i] && '\n' != info[i]; ++i)
			track.title.push_back(info[i]);
		
		std::string len;
		for (size_t i = find_ed("len=");	info[i] && '\n' != info[i]; ++i)
			len.push_back(info[i]);
		track.len = std::stoi(len.c_str());
	}

	{
		std::ifstream	fnote	(path + "/notes.txt");
		std::string		note;
		fnote	.seekg	(0, std::ios::end);
		note	.resize	(fnote.tellg());
		fnote	.seekg	(0, std::ios::beg);
		fnote	.read	(&note[0], note.size());

		for (size_t len = sizeof(" ") - 1;;)
		{	
			size_t itr = note.find(" ");
			if (std::string::npos == itr) break;
			note.erase(itr, len); 
		}
		for (size_t len = sizeof("\t") - 1;;)
		{	
			size_t itr = note.find("\t");
			if (std::string::npos == itr) break;
			note.erase(itr, len); 
		}

		size_t	size	= 0;
		bool	is_first = true;
		bool	is_note = false;
		std::string	_time;
		std::string	_note;
		
		

		for (size_t i = 0; ; ++i)
		{	
			if ('\n' == note[i] || '\0' == note[i])
			{	
				if (is_note) 
				{	
					const char* data =  _note.c_str();
					uint8_t		notes = 0;
					if (data[0] - '0') notes += 0b00001000;
					if (data[1] - '0') notes += 0b00000100;
					if (data[2] - '0') notes += 0b00000010;
					if (data[3] - '0') notes += 0b00000001;
					_note.clear();
					track.notes.push_back(notes);
					is_note = false;
					++size;
				}
				if (!note[i]) break;
				continue; 
			} else if ('=' == note[i]) 
			{	
				is_note = true; 
				int64_t time = std::stoi(_time.c_str());
				_time.clear();
				if (is_first)
				{
					is_first = false;
					app_info.game.pad = 576 / app_info.user.note_speed; // todo: write
					if (time > app_info.game.pad)
						app_info.game.pad = 0;
					else
						app_info.game.pad = app_info.game.pad - time;
				}
				track.time.push_back(time + app_info.game.pad);
			} else
			{	
				if (is_note)	_note.push_back(note[i]);
				else			_time.push_back(note[i]);
			}
		}
		track.size = size;
	}
	track.music = Mix_LoadMUS((path + "/music.wav").c_str());
	return 0;
}
void		
unload_track()
{
	track.title.clear();
	track.size	= 0;
	track.nc	= 0;
	track.time.clear();
	track.notes.clear();

	Mix_FreeMusic(track.music);
}

void		
rendering_menu()
{
	SDL_RenderClear(app_info.renderer);

	static const _rect_t background = { 418, 0, 320, 576 };
	SDL_RenderCopy(app_info.renderer, app_info.resource, &background, NULL);

	int scope_lv = app_info.user.scope / 12;
	int pos = scope_lv * 12;
	int len = 12;
	if (len > app_info.user.track_list.size() - scope_lv * 12) len = app_info.user.track_list.size() - scope_lv * 12;
	int local_scope = app_info.user.scope - scope_lv * 12;
	for (int i = pos; pos + len > i; ++i)
		render_str(app_info.user.track_list[i].c_str(), { 10, 47.f * (i - pos) + 4}, 16);

	static const _rect_t src = { 418, 582, 320, 47 };
	_rect_t dst = { 0, 47 * local_scope, 320, 47};
	SDL_RenderCopy(app_info.renderer, app_info.resource, &src, &dst);

	SDL_RenderPresent(app_info.renderer);
}
void		
rendering_game()
{
	SDL_RenderClear(app_info.renderer);

	static const _rect_t background = { 97, 0, 320, 576 };
	SDL_RenderCopy(app_info.renderer, app_info.resource, &background, NULL);
	render_notes();
	render_bar();
	render_scoreboard();
	if (app_info.game.is_paused)
		render_str("paused", { 10, 10 }, 16);
	SDL_RenderPresent(app_info.renderer);
}
void
rendering_ready_game()
{
	SDL_RenderClear(app_info.renderer);

	static const _rect_t background = { 97, 0, 320, 576 };
	SDL_RenderCopy(app_info.renderer, app_info.resource, &background, NULL);

	render_str(track.title.c_str(), { 20, 20 }, 16);
	if (-100 < app_info.game.ready)
		render_str(std::to_string((app_info.game.ready / 1000) + 1).c_str(), { 20, 40 }, 16);
	uint64_t now_time = SDL_GetTicks64();
	app_info.game.ready -= now_time - app_info.game.lt_time;
	app_info.game.lt_time = now_time;

	render_bar();

	SDL_RenderPresent(app_info.renderer);
}
void
rendering_sub_menu()
{
	SDL_RenderClear(app_info.renderer);

	render_str("setting", {}, 24);

	SDL_RenderPresent(app_info.renderer);
}
void
rendering_setting_menu()
{
	SDL_RenderClear(app_info.renderer);

	render_str("setting", {}, 24);

	SDL_RenderPresent(app_info.renderer);
}

void
hit_test()
{
	if (track.size == track.nc)
		return;

	if (	track.now >= track.time[track.nc] - app_info.game.range_bad
		&&	track.now <= track.time[track.nc] + app_info.game.range_bad
		&& ((0 != (track.notes[track.nc] & 0b00001000) == app_info.user.input[0])
		&&  (0 != (track.notes[track.nc] & 0b00000100) == app_info.user.input[1])
		&&  (0 != (track.notes[track.nc] & 0b00000010) == app_info.user.input[2])
		&&  (0 != (track.notes[track.nc] & 0b00000001) == app_info.user.input[3])))
	{	
		int64_t total = app_info.user.time[0]
			+ app_info.user.time[1]
			+ app_info.user.time[2]
			+ app_info.user.time[3];
		float count = (0 != (track.notes[track.nc] & 0b00001000))
			+ (0 != (track.notes[track.nc] & 0b00000100))
			+ (0 != (track.notes[track.nc] & 0b00000010))
			+ (0 != (track.notes[track.nc] & 0b00000001));
		float avg = total / count;

		if (	avg >= track.time[track.nc] - app_info.game.range_bad
			&&  avg <= track.time[track.nc] + app_info.game.range_bad)
		{
			float gap = abs(avg - (float)track.time[track.nc]);
			if			(app_info.game.range_perfect >= gap)
			{
				app_info.game.hit = PERFECT;
			} else if	(app_info.game.range_nice >= gap)
			{
				app_info.game.hit = NICE;
			} else if	(app_info.game.range_bad >= gap)
			{
				app_info.game.hit = BAD;
			} else
			{
				app_info.game.hit = MISS;
			}
			recording_score();
			if (track.size > track.nc) 
				++track.nc;
		}		
	} else if(track.now > track.time[track.nc] + app_info.game.range_bad)
	{	
		app_info.game.hit = MISS;
		recording_score();
		if (track.size > track.nc) 
			++track.nc; 
	} 
}
void 
recording_score()
{
	app_info.game.score += app_info.game.hit;
}

void
game()
{
	uint64_t now_time = SDL_GetTicks64();
	app_info.fps = 1000.f / (float)(now_time - app_info.game.lt_time);
	app_info.game.lt_time = now_time;
	track.now = (app_info.game.lt_time - app_info.game.st_time);
	hit_test();
	if (app_info.game.is_played && !Mix_PlayingMusic())
	{
		app_info.update = game_quit;
	} else if (!app_info.game.is_played && app_info.game.pad <= track.now)
	{
		Mix_PlayMusic(track.music, 0);
		app_info.game.is_played = true;
	}
}
void		
game_set()
{
	if (0 > app_info.game.ready)
	{
		app_info.update			= game;
		app_info.rendering		= rendering_game;
		app_info.game.hit		= NULL;
		app_info.game.score		= NULL;
		app_info.game.is_played = false;
		app_info.game.st_time	= SDL_GetTicks64();
	}
}
void		
game_quit()
{
	unload_track();
	app_info.update		= menu;
	app_info.rendering	= rendering_menu;
	app_info.game.is_played = false;
	app_info.game.is_paused = false;
}

void
pause()
{
	if (app_info.game.is_played)
	{
		Mix_PauseMusic();
		app_info.paused_time = SDL_GetTicks64();
		app_info.game.is_paused = true;
	}

	app_info.paused_update		= app_info.update;
	app_info.paused_rendering	= app_info.rendering;

	app_info.update		= sub_menu;
	app_info.rendering	= rendering_sub_menu;
	app_info.is_pause	= true;
}
void
resume()
{
	app_info.update		= app_info.paused_update;
	app_info.rendering	= app_info.paused_rendering;

	app_info.is_pause = false;

	if (app_info.game.is_played)
	{
		// printf("%d", SDL_GetTicks64() - app_info.paused_time);
		app_info.game.st_time += SDL_GetTicks64() - app_info.paused_time; // hmm...
		Mix_ResumeMusic();
	}
}

void
sub_menu()
{
	
}
void
setting_menu()
{

}
void		
menu()
{
	if (app_info.user.input[0])
	{
		load_track(app_info.user.track_list[app_info.user.scope].c_str());
		app_info.game.ready = app_info.user.waiting;
		app_info.game.lt_time = SDL_GetTicks64();
		app_info.update		= game_set;
		app_info.rendering	= rendering_ready_game;
	}
	if (app_info.user.input[1]);

	static bool K2_RELASE = true;
	if (app_info.user.input[2] && K2_RELASE) K2_RELASE = false,
		(app_info.user.track_list.size() > ++app_info.user.scope)		? app_info.user.scope 
		: app_info.user.scope = 0;
	else if (!app_info.user.input[2]) K2_RELASE = true;
	
	static bool K3_RELASE = true;
	if (app_info.user.input[3] && K3_RELASE) K3_RELASE = false,
		(0 <= --app_info.user.scope)									? app_info.user.scope 
		: app_info.user.scope = app_info.user.track_list.size() - 1;
	else if (!app_info.user.input[3]) K3_RELASE = true;

	static bool F5_RELASE = true;
	if (app_info.user.kb_state[SDL_SCANCODE_F5] && F5_RELASE) F5_RELASE = false,
		user_init(), load_track_list();
	else if (!app_info.user.input[3]) F5_RELASE = true;
}

void
user_init()
{
	_surface_t* suf;

	suf = IMG_Load("game/resource/evt.png");
	SDL_DestroyTexture(app_info.resource);
	app_info.resource = SDL_CreateTextureFromSurface(app_info.renderer, suf);
	SDL_FreeSurface(suf);

	suf = IMG_Load("game/resource/font.png");
	SDL_DestroyTexture(app_info.font);
	app_info.font = SDL_CreateTextureFromSurface(app_info.renderer, suf);
	SDL_FreeSurface(suf);

	app_info.user.key[0] = SDL_GetScancodeFromKey('q');
	app_info.user.key[1] = SDL_GetScancodeFromKey('w');
	app_info.user.key[2] = SDL_GetScancodeFromKey('o');
	app_info.user.key[3] = SDL_GetScancodeFromKey('p');

	app_info.user.kb_state = SDL_GetKeyboardState(NULL);
}
bool		
init()
{
	bool ninit	= true;
	ninit		= SDL_Init(SDL_INIT_EVERYTHING);
	ninit		= !IMG_Init(IMG_INIT_PNG);
	ninit		= Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	if (ninit) return -1;

	app_info.window		= SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 576, SDL_WINDOW_RESIZABLE);
	if (nullptr == app_info.window)		return -1;
	app_info.renderer	= SDL_CreateRenderer(app_info.window, -1, NULL);
	if (nullptr == app_info.renderer)	return -1;
	
	SDL_RenderSetLogicalSize(app_info.renderer, 320, 576);

	app_info.is_run		= true;
	app_info.update		= menu;
	app_info.rendering	= rendering_menu;

	user_init();
	load_track_list();

	return 0;
}
void		
quit()
{
	SDL_DestroyTexture(app_info.resource);
	SDL_DestroyTexture(app_info.font);

	SDL_DestroyRenderer	(app_info.renderer);
	SDL_DestroyWindow	(app_info.window);

	Mix_CloseAudio();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

void		
sys_porc()
{
	for (; SDL_PollEvent(&app_info.ev);)
	{
		switch (app_info.ev.type)
		{
		case SDL_QUIT:
			app_info.is_run = false;
			break;

		case SDL_KEYDOWN:
			if (app_info.user.time[0] == -1 && app_info.user.kb_state[app_info.user.key[0]]) 
				app_info.user.time[0] = app_info.ev.key.timestamp - app_info.game.st_time;
			if (app_info.user.time[1] == -1 && app_info.user.kb_state[app_info.user.key[1]]) 
				app_info.user.time[1] = app_info.ev.key.timestamp - app_info.game.st_time;
			if (app_info.user.time[2] == -1 && app_info.user.kb_state[app_info.user.key[2]]) 
				app_info.user.time[2] = app_info.ev.key.timestamp - app_info.game.st_time;
			if (app_info.user.time[3] == -1 && app_info.user.kb_state[app_info.user.key[3]]) 
				app_info.user.time[3] = app_info.ev.key.timestamp - app_info.game.st_time;
			break;

		case SDL_KEYUP:
			if (!app_info.user.kb_state[app_info.user.key[0]]) 
				app_info.user.time[0] = -1;
			if (!app_info.user.kb_state[app_info.user.key[1]]) 
				app_info.user.time[1] = -1;
			if (!app_info.user.kb_state[app_info.user.key[2]]) 
				app_info.user.time[2] = -1;
			if (!app_info.user.kb_state[app_info.user.key[3]]) 
				app_info.user.time[3] = -1;
			break;

		default:
			break;
		}
	}
}
void		
process()
{
	sys_porc();
	app_info.user.input[0] = app_info.user.kb_state[app_info.user.key[0]];
	app_info.user.input[1] = app_info.user.kb_state[app_info.user.key[1]];
	app_info.user.input[2] = app_info.user.kb_state[app_info.user.key[2]];
	app_info.user.input[3] = app_info.user.kb_state[app_info.user.key[3]];

	// printf("%d %d %d %d \n", app_info.user.time[0], app_info.user.time[1], app_info.user.time[2], app_info.user.time[3]);

	static bool F11_RELASE = true;
	if (app_info.user.kb_state[SDL_SCANCODE_F11] && F11_RELASE) F11_RELASE = false
		, SDL_SetWindowFullscreen(app_info.window
			, app_info.user.fullscreen = app_info.user.fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
	else if (!app_info.user.kb_state[SDL_SCANCODE_F11]) F11_RELASE = true;

	static bool ESC_RELASE = true;
	if (app_info.user.kb_state[SDL_SCANCODE_ESCAPE] && ESC_RELASE) ESC_RELASE = false
		, app_info.is_pause ? resume() : pause();
	else if (!app_info.user.kb_state[SDL_SCANCODE_ESCAPE]) ESC_RELASE = true;	
}