#ifndef DATASTREAM_LAYER_H
#define DATASTREAM_LAYER_H

#include <pebble.h>

#define TEXT_WIDTH 48
#define ICON_WIDTH 32
#define GRAPH_WIDTH 64
#define LAYER_HEIGHT 42

// Key values for AppMessage Dictionary
enum {
	FEED_KEY = 0,
	CHANNEL_KEY = 1,
	ICON_KEY = 2,
	VALUE_KEY = 3,
	VALUE_MIN_KEY = 4,
	VALUE_MAX_KEY = 5,
	DATA_KEY = 6,
	CHART_WIDTH_KEY = 7,
	CHART_HEIGHT_KEY = 8,
	API_KEY_KEY = 9,
	CHANNEL_1_KEY = 10,
	CHANNEL_2_KEY = 11,
	CHANNEL_3_KEY = 12,
	CHANNEL_4_KEY = 13
};

typedef struct {
	Layer* graph;
	BitmapLayer* icon;
	TextLayer* text;
	TextLayer* textMin;
	TextLayer* textMax;
	Layer* layer;
	uint8_t resource;
	char value[8];
	char valueMin[8];
	char valueMax[8];
	uint32_t feed;
	char channel[32];
	int8_t pixel[GRAPH_WIDTH];
} DataStreamLayer;


DataStreamLayer* datastream_layer_create(uint32_t feed, const char* channel, GPoint pos);
void datastream_layer_destroy(DataStreamLayer* layer);
void datastream_layer_set_icon(DataStreamLayer* layer, uint8_t resource);
void datastream_layer_set_text(DataStreamLayer* layer, const char* value, const char* valueMin, const char* valueMax);
void datastream_layer_set_graph_dict(DataStreamLayer* layer, DictionaryIterator* received);
void datastream_layer_set_graph(DataStreamLayer* layer, const char* pGraph);
void datastream_layer_request_data(DataStreamLayer* layer);
const char* datastream_layer_extract_apiKey(DictionaryIterator *dict);
uint32_t datastream_layer_extract_feed(DictionaryIterator *dict);
const char* datastream_layer_extract_channel(DictionaryIterator *dict);
const char* datastream_layer_extract_channel_1(DictionaryIterator *dict);
const char* datastream_layer_extract_channel_2(DictionaryIterator *dict);
const char* datastream_layer_extract_channel_3(DictionaryIterator *dict);
const char* datastream_layer_extract_channel_4(DictionaryIterator *dict);
const char* datastream_layer_extract_data(DictionaryIterator *dict);
const char* datastream_layer_extract_value(DictionaryIterator *dict);
const char* datastream_layer_extract_valueMin(DictionaryIterator *dict);
const char* datastream_layer_extract_valueMax(DictionaryIterator *dict);

#endif
