#include <pebble.h>
#include <pebble_fonts.h>

#include "datastream_layer.h"

Window *window;	
TextLayer* datetime;

static char time_str[16] = "12:12\0";

#define DATASTREAM_PORTS 0x03
static DataStreamLayer* ds[DATASTREAM_PORTS];

DataStreamLayer* find_datastream(uint32_t feed, const char* channel)
{
	if (feed == 0 || channel == NULL)
		return NULL;
	
	for (int idx = 0; idx < DATASTREAM_PORTS; idx++)
	{
		if (ds[idx]->feed == feed && strcmp(ds[idx]->channel, channel) == 0)
			return ds[idx];
	}
	
	return NULL;
}

DataStreamLayer* datastream_layer_next_layer(DataStreamLayer* layer)
{
	for (int idx = 0; idx < DATASTREAM_PORTS; idx++)
	{
		if (ds[idx] == layer && idx + 1 < DATASTREAM_PORTS)
			return ds[idx+1];
	}
	
	return NULL;
}


// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *dict, void *context)
{
	uint32_t feed = datastream_layer_extract_feed(dict);
	const char* channel = datastream_layer_extract_channel(dict);
	DataStreamLayer* layer = find_datastream(feed, channel);

	if (layer != NULL)
	{
		const char* data = datastream_layer_extract_data(dict);
		const char* value = datastream_layer_extract_value(dict);
		const char* valueMin = datastream_layer_extract_valueMin(dict);
		const char* valueMax = datastream_layer_extract_valueMax(dict);

		datastream_layer_set_graph(layer, data);
		datastream_layer_set_text(layer, value, valueMin, valueMax);
		
		datastream_layer_request_data(datastream_layer_next_layer(layer));
	}
	else
	{
		const char* channel_1 = datastream_layer_extract_channel_1(dict);
		const char* channel_2 = datastream_layer_extract_channel_2(dict);
		const char* channel_3 = datastream_layer_extract_channel_3(dict);
		const char* channel_4 = datastream_layer_extract_channel_4(dict);

		// Add datastream layer
		for (int idx = 0; idx < DATASTREAM_PORTS; idx++)
		{
			channel = NULL;
			uint8_t icon = 0;

			if (idx == 0 && channel_1 != NULL && strlen(channel_1) > 0)
			{
				channel = channel_1;
				icon = RESOURCE_ID_ICON_BBQ;
			}
			else if (idx == 1 && channel_2 != NULL && strlen(channel_2) > 0)
			{
				channel = channel_2;
				icon = RESOURCE_ID_ICON_TEMPERATURE;
			}
			else if (idx == 2 && channel_3 != NULL && strlen(channel_3) > 0)
			{
				channel = channel_3;
				icon = RESOURCE_ID_ICON_HUMIDITY;
			}
			else if (idx == 4 && channel_4 != NULL && strlen(channel_4) > 0)
			{
				channel = channel_4;
				icon = RESOURCE_ID_ICON_BBQ;
			}

			if (channel != NULL)
			{
				datastream_layer_destroy(ds[idx]);

				ds[idx] = datastream_layer_create(feed, channel, GPoint(0, (LAYER_HEIGHT + 8)*idx + 25));
				datastream_layer_set_icon(ds[idx], icon);

				layer_add_child(window_get_root_layer(window), ds[idx]->layer);
				datastream_layer_set_text(ds[idx], "N/A", "", "");
			}
		}
	
		// datastream_layer_request_data(ds[0]);
	}
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context)
{
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context)
{
}

void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{
	strftime (time_str, sizeof(time_str), "%R", tick_time);
	text_layer_set_text(datetime, time_str);
	
	for (int idx = 0; idx < DATASTREAM_PORTS; idx++)
	{
		datastream_layer_request_data(ds[idx]);
	}

}

void init(void)
{
	for (int idx = 0; idx < DATASTREAM_PORTS; idx++)
	{
		ds[idx] = NULL;
	}

	window = window_create();
	window_set_fullscreen(window, true);
	window_set_background_color(window, GColorBlack);

	window_stack_push(window, true);
		
	datetime = text_layer_create(GRect(80, 0, 64, 20));
	text_layer_set_text_alignment(datetime, GTextAlignmentCenter);
	text_layer_set_font(datetime, fonts_get_system_font (FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_background_color(datetime, GColorBlack);
	text_layer_set_text_color(datetime, GColorWhite);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(datetime));
	text_layer_set_text(datetime, time_str);
	
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
		
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

void deinit(void)
{
	tick_timer_service_unsubscribe();

	app_message_deregister_callbacks();
	
	layer_remove_from_parent(text_layer_get_layer(datetime));
	text_layer_destroy(datetime);
	
	for (int idx = 0; idx < DATASTREAM_PORTS; idx++)
	{
		datastream_layer_destroy(ds[idx]);
	}

	window_destroy(window);
}

int main( void )
{
	init();
	app_event_loop();
	deinit();
}