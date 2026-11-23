/*
Squircle
Copyright (C) 2026 hjmeneses

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

/*
 * Fuente con forma de squircle: un rectángulo cuyas esquinas son cuartos de superelipse
 * |x|^n + |y|^n = 1. Con n = 2 las esquinas son arcos de círculo (rectángulo redondeado
 * normal); con n alrededor de 4-5 la curva empieza antes y se ve como un ícono de iOS.
 * Con un cuadrado y la redondez al 100 % se obtiene el squircle clásico.
 *
 * Todo se dibuja en la GPU con una función de distancia, así que las orillas salen
 * suavizadas a cualquier tamaño y escala.
 */

#include <obs-module.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>
#include <plugin-support.h>

#include <math.h>

#define S_WIDTH "width"
#define S_HEIGHT "height"
#define S_RADIUS "radius"
#define S_POWER "power"
#define S_FILL "fill_color"
#define S_BORDER_WIDTH "border_width"
#define S_BORDER_COLOR "border_color"

struct squircle {
	obs_source_t *source;
	gs_effect_t *effect;
	gs_eparam_t *p_size;
	gs_eparam_t *p_radius;
	gs_eparam_t *p_power;
	gs_eparam_t *p_border;
	gs_eparam_t *p_fill;
	gs_eparam_t *p_border_color;

	uint32_t width;
	uint32_t height;
	float radius_pct; // 0..100: % de la mitad del lado corto
	float power;      // exponente de la superelipse (2 = círculo)
	float border;     // px, hacia adentro
	uint32_t fill_color;
	uint32_t border_color;
};

static const char *squircle_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("Squircle");
}

static void squircle_update(void *data, obs_data_t *settings)
{
	struct squircle *s = data;
	s->width = (uint32_t)obs_data_get_int(settings, S_WIDTH);
	s->height = (uint32_t)obs_data_get_int(settings, S_HEIGHT);
	if (s->width < 1)
		s->width = 1;
	if (s->height < 1)
		s->height = 1;
	s->radius_pct = (float)obs_data_get_double(settings, S_RADIUS);
	s->power = (float)obs_data_get_double(settings, S_POWER);
	if (s->power < 2.0f)
		s->power = 2.0f;
	s->border = (float)obs_data_get_double(settings, S_BORDER_WIDTH);
	if (s->border < 0.0f)
		s->border = 0.0f;
	s->fill_color = (uint32_t)obs_data_get_int(settings, S_FILL);
	s->border_color = (uint32_t)obs_data_get_int(settings, S_BORDER_COLOR);
}

static void *squircle_create(obs_data_t *settings, obs_source_t *source)
{
	struct squircle *s = bzalloc(sizeof(struct squircle));
	s->source = source;

	char *path = obs_module_file("effects/squircle.effect");
	obs_enter_graphics();
	s->effect = gs_effect_create_from_file(path, NULL);
	obs_leave_graphics();
	bfree(path);

	if (!s->effect) {
		obs_log(LOG_ERROR, "could not load effects/squircle.effect");
	} else {
		s->p_size = gs_effect_get_param_by_name(s->effect, "size");
		s->p_radius = gs_effect_get_param_by_name(s->effect, "radius");
		s->p_power = gs_effect_get_param_by_name(s->effect, "power");
		s->p_border = gs_effect_get_param_by_name(s->effect, "border");
		s->p_fill = gs_effect_get_param_by_name(s->effect, "fill_color");
		s->p_border_color = gs_effect_get_param_by_name(s->effect, "border_color");
	}

	squircle_update(s, settings);
	return s;
}

static void squircle_destroy(void *data)
{
	struct squircle *s = data;
	if (s->effect) {
		obs_enter_graphics();
		gs_effect_destroy(s->effect);
		obs_leave_graphics();
	}
	bfree(s);
}

static void squircle_render(void *data, gs_effect_t *unused)
{
	UNUSED_PARAMETER(unused);
	struct squircle *s = data;
	if (!s->effect)
		return;

	const float w = (float)s->width;
	const float h = (float)s->height;
	float pct = s->radius_pct;
	if (pct < 0.0f)
		pct = 0.0f;
	if (pct > 100.0f)
		pct = 100.0f;
	const float radius = fminf(w, h) * 0.5f * pct / 100.0f;

	struct vec2 size;
	vec2_set(&size, w, h);

	// Como la fuente de color de OBS: colores en lineal y framebuffer sRGB
	struct vec4 fill, line;
	vec4_from_rgba_srgb(&fill, s->fill_color);
	vec4_from_rgba_srgb(&line, s->border_color);

	gs_effect_set_vec2(s->p_size, &size);
	gs_effect_set_float(s->p_radius, radius);
	gs_effect_set_float(s->p_power, s->power);
	gs_effect_set_float(s->p_border, s->border);
	gs_effect_set_vec4(s->p_fill, &fill);
	gs_effect_set_vec4(s->p_border_color, &line);

	const bool previous = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(true);

	while (gs_effect_loop(s->effect, "Draw"))
		gs_draw_sprite(NULL, 0, s->width, s->height);

	gs_enable_framebuffer_srgb(previous);
}

static uint32_t squircle_get_width(void *data)
{
	struct squircle *s = data;
	return s->width;
}

static uint32_t squircle_get_height(void *data)
{
	struct squircle *s = data;
	return s->height;
}

static obs_properties_t *squircle_properties(void *unused)
{
	UNUSED_PARAMETER(unused);
	obs_properties_t *props = obs_properties_create();
	obs_property_t *p;

	p = obs_properties_add_int(props, S_WIDTH, obs_module_text("Width"), 1, 16384, 1);
	obs_property_int_set_suffix(p, " px");
	p = obs_properties_add_int(props, S_HEIGHT, obs_module_text("Height"), 1, 16384, 1);
	obs_property_int_set_suffix(p, " px");

	p = obs_properties_add_float_slider(props, S_RADIUS, obs_module_text("Radius"), 0.0, 100.0, 0.1);
	obs_property_float_set_suffix(p, " %");
	obs_property_set_long_description(p, obs_module_text("Radius.Description"));

	p = obs_properties_add_float_slider(props, S_POWER, obs_module_text("Power"), 2.0, 12.0, 0.1);
	obs_property_set_long_description(p, obs_module_text("Power.Description"));

	obs_properties_add_color_alpha(props, S_FILL, obs_module_text("FillColor"));

	p = obs_properties_add_float_slider(props, S_BORDER_WIDTH, obs_module_text("BorderWidth"), 0.0, 500.0, 0.5);
	obs_property_float_set_suffix(p, " px");
	obs_properties_add_color_alpha(props, S_BORDER_COLOR, obs_module_text("BorderColor"));

	return props;
}

static void squircle_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, S_WIDTH, 400);
	obs_data_set_default_int(settings, S_HEIGHT, 400);
	obs_data_set_default_double(settings, S_RADIUS, 100.0);
	obs_data_set_default_double(settings, S_POWER, 5.0);
	obs_data_set_default_int(settings, S_FILL, 0xFFFFFFFF);
	obs_data_set_default_double(settings, S_BORDER_WIDTH, 0.0);
	obs_data_set_default_int(settings, S_BORDER_COLOR, 0xFF000000);
}

struct obs_source_info squircle_source = {
	.id = "squircle_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_SRGB,
	.icon_type = OBS_ICON_TYPE_COLOR,
	.get_name = squircle_get_name,
	.create = squircle_create,
	.destroy = squircle_destroy,
	.update = squircle_update,
	.get_defaults = squircle_defaults,
	.get_properties = squircle_properties,
	.video_render = squircle_render,
	.get_width = squircle_get_width,
	.get_height = squircle_get_height,
};
