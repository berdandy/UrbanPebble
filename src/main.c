#include <pebble.h>
	
#define KEY_WORD 0
#define KEY_DEFINITION 1
#define KEY_EXAMPLE 2

static Window *window;

static ScrollLayer *s_scroll_layer;
static TextLayer *text_layer;

static char ud_word_buffer[32];
static char ud_definition_buffer[1024];
static char ud_example_buffer[1024];
static char ud_term_buffer[2048];

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	APP_LOG(APP_LOG_LEVEL_INFO, "WAT");
	
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
	GRect max_text_bounds = GRect(2, 2, bounds.size.w-2, 2000);
	
 	s_scroll_layer = scroll_layer_create(bounds);
	scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
	
	text_layer = text_layer_create(max_text_bounds);
	text_layer_set_text(text_layer, "Press select for a random definiton");
	// text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	
//	GSize max_size = text_layer_get_content_size(text_layer);
	text_layer_set_size(text_layer, GSize(max_text_bounds.size.w, max_text_bounds.size.h));
	scroll_layer_set_content_size(s_scroll_layer, GSize(max_text_bounds.size.w, max_text_bounds.size.h+4));

	// Add the layers for display
	scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(text_layer));
	layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
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
	  
	snprintf(ud_term_buffer, sizeof(ud_term_buffer), "%s\n\n%s\n\n%s",
			 ud_word_buffer,
			 ud_definition_buffer,
			 ud_example_buffer
	);
	text_layer_set_text(text_layer, ud_term_buffer);

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