#include <pebble.h>
	
#define KEY_WORD 0
#define KEY_DEFINITION 1
#define KEY_EXAMPLE 2
	
#define FONT_INSTRUCTIONS FONT_KEY_GOTHIC_24
#define FONT_DICTIONARY FONT_KEY_GOTHIC_18

static Window *window;

static ScrollLayer *s_scroll_layer;
static TextLayer *text_layer;

static char ud_word_buffer[128];
static char ud_definition_buffer[1024];
static char ud_example_buffer[1024];
static char ud_term_buffer[2048];

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	snprintf(ud_term_buffer, sizeof(ud_term_buffer), "Loading...");
	text_layer_set_size(text_layer, GSize(144, 168));
	scroll_layer_set_content_size(s_scroll_layer, GSize(144, 168));
	text_layer_set_text(text_layer, ud_term_buffer);
	scroll_layer_set_content_offset(s_scroll_layer, GPointZero, false);
	
	// Begin dictionary
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	// Add a key-value pair
	dict_write_uint8(iter, 0, 0);

	// Send the message!
	app_message_outbox_send();
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	GRect max_text_bounds = GRect(2, 2, bounds.size.w-4, 2000);
	
 	s_scroll_layer = scroll_layer_create(bounds);
	scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
	scroll_layer_set_callbacks(s_scroll_layer, (ScrollLayerCallbacks){.click_config_provider=click_config_provider});
	
	text_layer = text_layer_create(max_text_bounds);
	text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
	text_layer_set_text(text_layer, "WARNING: URBAN DICTIONARY CONTAINS ADULT CONTENT\n\nPress SELECT for a random definiton");
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_INSTRUCTIONS));
	text_layer_set_size(text_layer, GSize(144, 168));
	scroll_layer_set_content_size(s_scroll_layer, GSize(144, 168));
	
	// Add the layers for display
	scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(text_layer));
	layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void update_layer_sizes_for_content() {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	GRect max_text_bounds = GRect(2, 2, bounds.size.w-2, 2000);

	GSize gr_content_size = graphics_text_layout_get_content_size(
		ud_term_buffer,
		fonts_get_system_font(FONT_DICTIONARY),
		GRect(0,0,144,1000),
		GTextOverflowModeWordWrap,
		GTextAlignmentLeft );
	
	text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_DICTIONARY));
	text_layer_set_size(text_layer, GSize(max_text_bounds.size.w, gr_content_size.h));
	scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, gr_content_size.h+4));

}

static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    switch(t->key) {
    case KEY_WORD:
		snprintf(ud_word_buffer, sizeof(ud_word_buffer), "%s", t->value->cstring);
		break;
    case KEY_DEFINITION:
		snprintf(ud_definition_buffer, sizeof(ud_definition_buffer), "%s", t->value->cstring);
		break;
	case KEY_EXAMPLE:
		snprintf(ud_example_buffer, sizeof(ud_example_buffer), "%s", t->value->cstring);
		break;
    default:
		APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
		break;
    }
	  
	snprintf(ud_term_buffer, sizeof(ud_term_buffer), "%s\n\nDefinition:\n%s\n\nExample:\n%s",
			 ud_word_buffer,
			 ud_definition_buffer,
			 ud_example_buffer
	);
	text_layer_set_text(text_layer, ud_term_buffer);
	update_layer_sizes_for_content();
	scroll_layer_set_content_offset(s_scroll_layer, GPointZero, false);

    // Look for next item
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init(void) {
	window = window_create();
	window_set_click_config_provider(window, click_config_provider);
	
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	const bool animated = true;
	window_stack_push(window, animated);
	
	// register callbacks to talk to phone
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}