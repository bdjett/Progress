#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static InverterLayer *inverter_layer;
char buffer[] = "00:00";
int hour;
int minute;
int total;
float percent;
int pixels;
int percent_int;

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	// strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	hour = tick_time->tm_hour;
	minute = tick_time->tm_min;
	total = (hour * 60) + minute;
	percent = total / 1440.0;
	pixels = percent * 168;
	percent_int = percent * 100;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "%d:%d, total: %d, pixels: %d", hour, minute, total, pixels);
	inverter_layer_destroy(inverter_layer);
	inverter_layer = inverter_layer_create(GRect(0, (168 - pixels), 148, pixels));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inverter_layer));
	if (!clock_is_24h_style()) {
		hour = (hour % 12);
		if (hour == 0) {
			hour = 12;
		}
		if (minute < 10) {
			snprintf(buffer, sizeof("00:00"), "%d:0%d", hour, minute);
		} else {
			snprintf(buffer, sizeof("00:00"), "%d:%d", hour, minute);
		}
	} else {
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	}
	text_layer_set_text(text_layer, buffer);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	inverter_layer = inverter_layer_create(GRect(0, 0, 148, 0));
	layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));
	text_layer = text_layer_create(GRect(0, 113, 130, 50));
	text_layer_set_text_alignment(text_layer, GTextAlignmentRight);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
	layer_add_child(window_layer, text_layer_get_layer(text_layer));
	struct tm *t;
	time_t temp;
	temp = time(NULL);
	t = localtime(&temp);
	tick_handler(t, MINUTE_UNIT);
}

static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
	inverter_layer_destroy(inverter_layer);
}

static void init(void) {
	tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	const bool animated = true;
	window_stack_push(window, animated);
}

static void deinit(void) {
	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}