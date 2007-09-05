
/*
   Copyright (C) 2007 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include <gl.h>
#include <SDL.h>

#include "raster.hpp"

#include <boost/shared_array.hpp>
#include <iostream>

namespace graphics
{

/* unavoidable global variable to store global clip
   rectangle changes */
std::vector<boost::shared_array<GLint> > clip_rectangles;

void prepare_raster()
{
	const SDL_Surface* fb = SDL_GetVideoSurface();
	if(fb == NULL) {
		return;
	}
	
	glViewport(0,0,fb->w,fb->h);
	glShadeModel(GL_FLAT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,fb->w,fb->h,0,-1.0,1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
}

void blit_texture(const texture& tex, int x, int y, GLfloat rotate)
{
	if(!tex.valid()) {
		return;
	}

	int h = tex.height();
	int w = tex.width();
	const int w_odd = w % 2;
	const int h_odd = h % 2;
	h /= 2;
	w /= 2;

	glPushMatrix();
	glTranslatef(x+tex.width()/2,y+tex.height()/2,0.0);
	glRotatef(rotate,0.0,0.0,1.0);
	tex.set_as_current_texture();
	glBegin(GL_QUADS);
	glColor3f(1.0,1.0,1.0);
	graphics::texture::set_coord(0.0,0.0);
	glVertex3i(-w,-h,0);
	graphics::texture::set_coord(0.0,1.0);
	glVertex3i(-w,h+h_odd,0);
	graphics::texture::set_coord(1.0,1.0);
	glVertex3i(w+w_odd,h+h_odd,0);
	graphics::texture::set_coord(1.0,0.0);
	glVertex3i(w+w_odd,-h,0);
	glEnd();
	glPopMatrix();
}

void blit_texture(const texture& tex, int x, int y, int w, int h, GLfloat rotate)
{
	if(!tex.valid()) {
		return;
	}

	const int w_odd = w % 2;
	const int h_odd = h % 2;
	 
	w /= 2;
	h /= 2;
	glPushMatrix();
	tex.set_as_current_texture();
	glTranslatef(x+w,y+h,0.0);
	glRotatef(rotate,0.0,0.0,1.0);
	glBegin(GL_QUADS);
	glColor3f(1.0,1.0,1.0);
	graphics::texture::set_coord(0.0,0.0);
	glVertex3i(-w,-h,0);
	graphics::texture::set_coord(0.0,1.0);
	glVertex3i(-w,h+h_odd,0);
	graphics::texture::set_coord(1.0,1.0);
	glVertex3i(w+w_odd,h+h_odd,0);
	graphics::texture::set_coord(1.0,0.0);
	glVertex3i(w+w_odd,-h,0);
	glEnd();
	glPopMatrix();
}

void draw_rect(const SDL_Rect& r, const SDL_Color& color,
               unsigned char alpha)
{
	glDisable(GL_TEXTURE_2D);
	glColor4ub(color.r,color.g,color.b,alpha);
	glRecti(r.x,r.y,r.x+r.w,r.y+r.h);
	glEnable(GL_TEXTURE_2D);
}

void draw_hollow_rect(const SDL_Rect& r, const SDL_Color& color,
		      unsigned char alpha) {
	
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINE_LOOP);
	glColor4ub(color.r, color.g, color.b, alpha);
	glVertex3f( r.x, r.y, 0);
	glVertex3f( r.x + r.w, r.y, 0);
	glVertex3f( r.x + r.w, r.y + r.h, 0);
	glVertex3f( r.x, r.y + r.h, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

void push_clip(const SDL_Rect& r) 
{
	const bool was_enabled_clip = glIsEnabled(GL_SCISSOR_TEST);
	bool should_enable_clip = true;

	if(r.x == 0 && r.y == 0 && r.w == screen_width() && r.h == screen_height()) {
		should_enable_clip = false;
	}

	boost::shared_array<GLint> box(new GLint[4]);

	if(was_enabled_clip) {
		glGetIntegerv(GL_SCISSOR_BOX, box.get());
	} else {
		box[0] = 0;
		box[1] = 0;
		box[2] = screen_width();
		box[3] = screen_height();
	}
	clip_rectangles.push_back(box);

	if(should_enable_clip) {
		if(!was_enabled_clip) {
			glEnable(GL_SCISSOR_TEST);
		}
		glScissor(r.x, screen_height() - (r.y + r.h - 1), r.w, r.h);
	} else if(was_enabled_clip) {
		glDisable(GL_SCISSOR_TEST);
	}
}
void pop_clip() {
	const bool was_enabled_clip = glIsEnabled(GL_SCISSOR_TEST);
	bool should_enable_clip = false;
	boost::shared_array<GLint> box;

	if(!clip_rectangles.empty()) {
		box = *clip_rectangles.rbegin();
		clip_rectangles.pop_back();

		if(box[0] != 0 || box[1] != 0 || box[2] != screen_width() || box[3] != screen_height()) {
			should_enable_clip = true;
		}
	}

	if(should_enable_clip) {
		if(!was_enabled_clip) {
			glEnable(GL_SCISSOR_TEST);
		}
		glScissor(box[0], box[1], box[2], box[3]);
	} else if(was_enabled_clip) {
		glDisable(GL_SCISSOR_TEST);
	}
}

int screen_width()
{
	return 1024;
}

int screen_height()
{
	return 768;
}

const SDL_Color& color_black()
{
	static SDL_Color res = {0,0,0,0};
	return res;
}

const SDL_Color& color_white()
{
	static SDL_Color res = {0xFF,0xFF,0xFF,0};
	return res;
}

const SDL_Color& color_red()
{
	static SDL_Color res = {0xFF,0,0,0};
	return res;
}

const SDL_Color& color_green()
{
	static SDL_Color res = {0,0xFF,0,0};
	return res;
}

const SDL_Color& color_blue()
{
	static SDL_Color res = {0,0,0xFF,0};
	return res;
}

const SDL_Color& color_yellow()
{
	static SDL_Color res = {0xFF,0xFF,0,0};
	return res;
}

}
