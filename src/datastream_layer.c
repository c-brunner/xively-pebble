#include <pebble.h>
#include "datastream_layer.h"

/*static uint8_t DATASTREAM_ICONS[] =
{
	RESOURCE_ID_ICON_LOADING,
	RESOURCE_ID_ICON_LOADING,
	RESOURCE_ID_ICON_TEMPERATURE,
	RESOURCE_ID_ICON_HUMIDITY,
	RESOURCE_ID_ICON_BBQ,
	RESOURCE_ID_ICON_MEAT
};*/

void copy_string (const char* source, char* destination, uint8_t maxLength)
{
	int length = strlen(source);
	if (length >= maxLength && length > 0)
		length = maxLength - 1;
	
	if (length > 0)
		memcpy(destination, source, length);

	destination[length] = '\0';
}

void datastream_layer_draw_graph(struct Layer *layer, GContext *ctx)
{
	if (layer == NULL)
		return;

//	APP_LOG(APP_LOG_LEVEL_DEBUG, "-> datastream_layer_draw_graph");

	graphics_context_set_stroke_color(ctx, GColorWhite);

	DataStreamLayer** datastream_layer = layer_get_data(layer);

	for (int idx = 1; idx < GRAPH_WIDTH; idx++)
	{
		if ((*datastream_layer)->pixel[idx-1] >= 0  && (*datastream_layer)->pixel[idx] >= 0 &&
		    (*datastream_layer)->pixel[idx-1] < LAYER_HEIGHT && (*datastream_layer)->pixel[idx] < LAYER_HEIGHT)
			graphics_draw_line(ctx, GPoint(idx-1, (*datastream_layer)->pixel[idx-1]), GPoint(idx, (*datastream_layer)->pixel[idx]));
	}

	graphics_draw_line(ctx, GPoint(0, 0), GPoint(0, LAYER_HEIGHT - 1));
	graphics_draw_line(ctx, GPoint(0, LAYER_HEIGHT - 1), GPoint(GRAPH_WIDTH - 1, LAYER_HEIGHT - 1));
	
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "<- datastream_layer_draw_graph");
}

DataStreamLayer* datastream_layer_create(uint32_t feed, const char* channel, GPoint pos)
{
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "-> datastream_layer_create %u %s", (unsigned int)feed, channel);
	
	DataStreamLayer* layer = malloc (sizeof(DataStreamLayer));
	
	layer->resource = 0;
	layer->feed = feed;
	copy_string (channel, layer->channel, sizeof(layer->channel));

	layer->layer = layer_create(GRect(pos.x, pos.y, 144, LAYER_HEIGHT));
	
	layer->textMax = text_layer_create(GRect(144 - TEXT_WIDTH, -4, TEXT_WIDTH, 14));
	text_layer_set_text_alignment(layer->textMax, GTextAlignmentRight);
	text_layer_set_font(layer->textMax, fonts_get_system_font (FONT_KEY_GOTHIC_14));
	text_layer_set_background_color(layer->textMax, GColorClear);
	text_layer_set_text_color(layer->textMax, GColorWhite);
	layer_add_child(layer->layer, text_layer_get_layer(layer->textMax));

	layer->text = text_layer_create(GRect(144 - TEXT_WIDTH, 4, TEXT_WIDTH, 24));
	text_layer_set_text_alignment(layer->text, GTextAlignmentRight);
	text_layer_set_font(layer->text, fonts_get_system_font (FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_background_color(layer->text, GColorClear);
	text_layer_set_text_color(layer->text, GColorWhite);
	layer_add_child(layer->layer, text_layer_get_layer(layer->text));

	layer->textMin = text_layer_create(GRect(144 - TEXT_WIDTH, 27, TEXT_WIDTH, 14));
	text_layer_set_text_alignment(layer->textMin, GTextAlignmentRight);
	text_layer_set_font(layer->textMin, fonts_get_system_font (FONT_KEY_GOTHIC_14));
	text_layer_set_background_color(layer->textMin, GColorClear);
	text_layer_set_text_color(layer->textMin, GColorWhite);
	layer_add_child(layer->layer, text_layer_get_layer(layer->textMin));

	layer->graph = layer_create_with_data(GRect(ICON_WIDTH, 0, GRAPH_WIDTH, LAYER_HEIGHT), sizeof(DataStreamLayer*));
	DataStreamLayer** target = layer_get_data(layer->graph);
	*target = layer;
	
	layer_set_update_proc(layer->graph, datastream_layer_draw_graph);
	layer_add_child(layer->layer, layer->graph);

	for (int idx = 0; idx < GRAPH_WIDTH; idx++)
	{
		layer->pixel[idx] = LAYER_HEIGHT - 1;
	}
	
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "<- datastream_layer_create");

	return layer;
}

void datastream_layer_set_icon(DataStreamLayer* layer, uint8_t resource)
{
	if (layer == NULL)
		return;

//	APP_LOG(APP_LOG_LEVEL_DEBUG, "-> datastream_layer_set_icon %u", resource);
	
	if (layer->resource != resource)
	{
		if(layer->resource != 0)
		{
			layer_remove_from_parent(bitmap_layer_get_layer(layer->icon));
			gbitmap_destroy((GBitmap *)bitmap_layer_get_bitmap(layer->icon));
			bitmap_layer_destroy(layer->icon);
		}
		
		layer->resource = resource;
		
		if (resource != 0)
		{
			layer->icon = bitmap_layer_create(GRect(0, 0, ICON_WIDTH, LAYER_HEIGHT));
			bitmap_layer_set_bitmap(layer->icon, gbitmap_create_with_resource(layer->resource));
			layer_add_child(layer->layer, bitmap_layer_get_layer(layer->icon));
		}
	}

//	APP_LOG(APP_LOG_LEVEL_DEBUG, "<- datastream_layer_set_icon");
}

void datastream_layer_set_text(DataStreamLayer* layer, const char* value, const char* valueMin, const char* valueMax)
{
	if (layer == NULL)
		return;

//	APP_LOG(APP_LOG_LEVEL_DEBUG, "-> datastream_layer_set_text %s", value);

	if (value != NULL)
	{
		copy_string (value, layer->value, sizeof(layer->value));
		text_layer_set_text(layer->text, layer->value);
	}
	
	if (valueMin != NULL)
	{
		copy_string (valueMin, layer->valueMin, sizeof(layer->valueMin));
		text_layer_set_text(layer->textMin, layer->valueMin);
	}
	
	if (valueMax != NULL)
	{
		copy_string (valueMax, layer->valueMax, sizeof(layer->valueMax));	
		text_layer_set_text(layer->textMax, layer->valueMax);
	}
	
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "<- datastream_layer_set_text");
}

void datastream_layer_set_graph(DataStreamLayer* layer, const char* data)
{
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "-> datastream_layer_set_graph");

	if (layer == NULL || data == NULL)
		return;

	int length = strlen(data);

	if (length > GRAPH_WIDTH)
		length = GRAPH_WIDTH;

	for (int idx = 0; idx < length; idx++)
	{
		layer->pixel[idx] = LAYER_HEIGHT - (data[idx] - 64);

	}
	
	for (int idx = length; idx < GRAPH_WIDTH; idx++)
	{
		layer->pixel[idx] = LAYER_HEIGHT - 1;
	}

	layer_mark_dirty(layer->graph);

//	APP_LOG(APP_LOG_LEVEL_DEBUG, "<- datastream_layer_set_graph");
}

void datastream_layer_request_data(DataStreamLayer* layer)
{
	if (layer == NULL)
		return;

//	APP_LOG(APP_LOG_LEVEL_DEBUG, "-> request_datastream %u, %s", (unsigned int)layer->feed, layer->channel);

	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
		dict_write_uint32(iter, FEED_KEY, layer->feed);
		dict_write_cstring(iter, CHANNEL_KEY, layer->channel);
		dict_write_uint8(iter, CHART_WIDTH_KEY, GRAPH_WIDTH);
		dict_write_uint8(iter, CHART_HEIGHT_KEY, LAYER_HEIGHT);
	dict_write_end(iter);

	app_message_outbox_send();

//	APP_LOG(APP_LOG_LEVEL_DEBUG, "<- request_datastream");
}

const char* datastream_layer_extract_apiKey(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, API_KEY_KEY);
	if(tuple)
		return tuple->value->cstring;

	return NULL;
}

uint32_t datastream_layer_extract_feed(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, FEED_KEY);
	if(tuple && tuple->type == TUPLE_CSTRING)
		return atoi(tuple->value->cstring);
	else if (tuple)
		return tuple->value->uint32;

	return 0;
}

const char* datastream_layer_extract_channel(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, CHANNEL_KEY);
	if(tuple)
		return tuple->value->cstring;

	return NULL;
}

const char* datastream_layer_extract_channel_1(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, CHANNEL_1_KEY);
	if(tuple)
		return tuple->value->cstring;

	return NULL;
}

const char* datastream_layer_extract_channel_2(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, CHANNEL_2_KEY);
	if(tuple)
		return tuple->value->cstring;

	return NULL;
}

const char* datastream_layer_extract_channel_3(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, CHANNEL_3_KEY);
	if(tuple)
		return tuple->value->cstring;

	return NULL;
}

const char* datastream_layer_extract_channel_4(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, CHANNEL_4_KEY);
	if(tuple)
		return tuple->value->cstring;

	return NULL;
}

const char* datastream_layer_extract_data(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, DATA_KEY);
	if(tuple)
		return tuple->value->cstring;

	return NULL;
}

const char* datastream_layer_extract_value(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, VALUE_KEY);
	if(tuple)
		return tuple->value->cstring;

	return NULL;
}

const char* datastream_layer_extract_valueMin(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, VALUE_MIN_KEY);
	if(tuple)
		return tuple->value->cstring;

	return NULL;
}

const char* datastream_layer_extract_valueMax(DictionaryIterator *dict)
{
	Tuple* tuple = dict_find(dict, VALUE_MAX_KEY);
	if(tuple)
		return tuple->value->cstring;

	return NULL;
}

void datastream_layer_destroy(DataStreamLayer* layer)
{
	if (layer == NULL)
		return;

//	APP_LOG(APP_LOG_LEVEL_DEBUG, "-> datastream_layer_destroy");

	layer_remove_from_parent(layer->layer);
	
	layer_remove_child_layers(layer->layer);
	
	text_layer_destroy(layer->text);
	text_layer_destroy(layer->textMin);
	text_layer_destroy(layer->textMax);
	layer_destroy(layer->graph);

	if(layer->icon != 0)
	{
		layer_remove_from_parent(bitmap_layer_get_layer(layer->icon));
		gbitmap_destroy((GBitmap *)bitmap_layer_get_bitmap(layer->icon));
	}	bitmap_layer_destroy(layer->icon);

	layer_destroy(layer->layer);
	
	free(layer);

//	APP_LOG(APP_LOG_LEVEL_DEBUG, "<- datastream_layer_destroy");
}
