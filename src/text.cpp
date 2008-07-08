/***************************************************************************
 *  Copyright (C) 2008 by Sergey Popov <loonycyborg@gmail.com>             *
 *                                                                         *
 *  This file is part of Silver Tree.                                      *
 *                                                                         *
 *  Silver Tree is free software; you can redistribute it and/or modify    *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  Silver Tree is distributed in the hope that it will be useful,         *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftbitmap.h>
#include <pango/pangoft2.h>
#include <boost/lexical_cast.hpp>
#include <string>

#include "text.hpp"
#include "gl_utils.hpp"

using std::string;

namespace
{

FT_Library library;
PangoFT2FontMap* font_map;
PangoContext* context;

}

namespace graphics
{

namespace text
{

boost::scoped_ptr<renderer> renderer::the_renderer;

renderer& renderer::instance()
{
	if(!the_renderer) {
		the_renderer.reset(new renderer);
	}

	return *the_renderer;
}

renderer::renderer()
{
	FT_Init_FreeType(&library);
	font_map = (PangoFT2FontMap*)pango_ft2_font_map_new();
	context = pango_ft2_font_map_create_context(font_map);
	pango_ft2_font_map_set_resolution(PANGO_FT2_FONT_MAP(font_map), 96, 96);
}

renderer::~renderer()
{
	g_object_unref(context);
	g_object_unref(font_map);
}

gl::texture2d_ptr renderer::render_text(std::string text, int& width, int& height, int size, bool markup)
{
	FT_Bitmap bitmap;

	PangoLayout* layout = pango_layout_new(context);
	if(markup)
		pango_layout_set_markup(layout, text.data(), text.size());
	else
		pango_layout_set_text(layout, text.data(), text.size());

	PangoFontDescription *desc = pango_font_description_new();
	pango_font_description_set_family_static(desc, "FreeSans");
	pango_font_description_set_size(desc, size * PANGO_SCALE);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	std::cout << text << std::endl;

	PangoRectangle rect;
	pango_layout_get_pixel_extents(layout, NULL, &rect);

	bitmap.width = rect.width;
	bitmap.rows = rect.height;
	bitmap.pitch = rect.width;
	bitmap.num_grays = 256;
	bitmap.pixel_mode = FT_PIXEL_MODE_GRAY;
	bitmap.buffer = new unsigned char[bitmap.pitch * bitmap.rows];
	memset(bitmap.buffer, 0x00, bitmap.pitch * bitmap.rows);

	pango_ft2_render_layout(&bitmap, layout, 0, 0);

	gl::texture2d_ptr texture(new gl::texture2d);
	texture->bind();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA, bitmap.width, bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap.buffer);
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, bitmap.width, bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap.buffer);

	delete[] bitmap.buffer;
	g_object_unref(layout);

	width = bitmap.width;
	height = bitmap.rows;
	std::cout << width << " " << height << std::endl;
	return texture;
}

}

}
