#include <pebble.h>

Window *window;

Layer *display_layer;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *headline_layer;
char time_text[] = "12:34";
char date_text[] = "Jul  4, Tue   ";
char headline_text[] = "This headline is very long because it has so many words in it and stuff to show to the user (Pebble sports)(Even more sports)(Sports sports sports)";

GColor foregroundcolor;
GColor backgroundcolor;

AppSync sync;
uint8_t sync_buffer[128];

enum WeatherKey {
  HEADLINE_KEY = 0x0,         // TUPLE_CSTRING
};

static void update_display_layer(Layer *me, GContext* ctx) {
  (void)me;

  graphics_context_set_fill_color(ctx, foregroundcolor);
  graphics_context_set_stroke_color(ctx, foregroundcolor);

  graphics_fill_rect(ctx, GRect(3, 61, 138, 1), 0, GCornerNone);
}

void draw_time(){
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  
  if(clock_is_24h_style()==true){
    strftime(time_text, sizeof(time_text), "%H:%M", tick_time);
  }else{
    strftime(time_text, sizeof(time_text), "%I:%M", tick_time);
  }
  
  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }
  
  text_layer_set_text(time_layer, time_text);
}

void draw_date(){
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  
  strftime(date_text, sizeof(date_text), "%b %d, %a", tick_time);
  text_layer_set_text(date_layer, date_text);
}

void draw_headline(){ 
  text_layer_set_text(headline_layer, headline_text);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  
  draw_time();
  
  if(tick_time->tm_min==0&&tick_time->tm_hour==0){
    draw_date();
  }
}

void set_error_message(char * msg){
  /*strcpy(headline_text, msg);
  text_layer_set_text(headline_layer, headline_text);*/
}

void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
  switch(app_message_error){
    case APP_MSG_OK:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "All good, operation was successful.");
      break;

    case APP_MSG_SEND_TIMEOUT:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "The other end did not confirm receiving the sent data with an (n)ack in time.");
      set_error_message("TIMEOUT");
      break;

     case APP_MSG_SEND_REJECTED:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "The other end rejected the sent data, with a \"nack\" reply.");
      set_error_message(" PHONE-2");
      break;

    case APP_MSG_NOT_CONNECTED:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "The other end was not connected.");
      set_error_message(" PHONE");
      break;

    case APP_MSG_APP_NOT_RUNNING:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "The local application was not running.");
      set_error_message(" PHONE-3");
      break;

    case APP_MSG_INVALID_ARGS:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "The function was called with invalid arguments.");
      set_error_message(" PHONE-4");
      break;

    case APP_MSG_BUSY:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "There are pending (in or outbound) messages that need to be processed first before new ones can be received or sent.");
      set_error_message(" PHONE BUSY");
      break;

    case APP_MSG_BUFFER_OVERFLOW:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "The buffer was too small to contain the incoming message.");
      set_error_message(" PHONE-5");
      break;

    case APP_MSG_ALREADY_RELEASED:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "The resource had already been released.");
      set_error_message(" PHONE-6");
      break;

    case APP_MSG_CALLBACK_ALREADY_REGISTERED:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "The callback node was already registered, or its ListNode has not been initialized.");
      set_error_message(" PHONE-7");
      break;

    case APP_MSG_CALLBACK_NOT_REGISTERED:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "The callback could not be deregistered, because it had not been registered before.");
      set_error_message(" PHONE-8");
      break;

    case APP_MSG_OUT_OF_MEMORY:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "The support library did not have sufficient application memory to perform the requested operation.");
      set_error_message(" PHONE-9");
      break;

    case APP_MSG_CLOSED:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "App message was closed.");
      set_error_message(" PHONE10");
      break;

    case APP_MSG_INTERNAL_ERROR:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "An internal OS error prevented APP_MSG from completing an operation.");
      set_error_message(" PHONE11");
      break;
  }
}

void change_headline(const char * msg){
  strcpy(headline_text, msg);
  text_layer_set_text(headline_layer, headline_text);
}

void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case HEADLINE_KEY:
      change_headline(new_tuple->value->cstring);
      break;
  }
}

static void init() {

  const uint32_t inbound_size = 512;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  backgroundcolor = GColorBlack;
  foregroundcolor = GColorWhite;

  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, backgroundcolor);
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

//background
  display_layer = layer_create(bounds);
  layer_set_update_proc(display_layer, update_display_layer);
  layer_add_child(window_layer, display_layer);

//time
  GFont time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SANS_BOLDITALIC_47));
  time_layer = text_layer_create(GRect(-3, -8, 144, 65));
  text_layer_set_text_color(time_layer, foregroundcolor);
  text_layer_set_font(time_layer, time_font);
  //text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(time_layer, GColorClear);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
  draw_time();

//date
  GFont date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SANS_BOLDITALIC_16));
  date_layer = text_layer_create(GRect(0, 40, 144, 40));
  text_layer_set_text_color(date_layer, foregroundcolor);
  text_layer_set_font(date_layer, date_font);
  //text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_background_color(date_layer, GColorClear);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
  draw_date();

//headline
  headline_layer = text_layer_create(GRect(0, 63, 144, 105));
  text_layer_set_text_color(headline_layer, foregroundcolor);
  text_layer_set_font(headline_layer, date_font);
  text_layer_set_text_alignment(headline_layer, GTextAlignmentLeft);
  text_layer_set_background_color(headline_layer, GColorClear);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(headline_layer));
  draw_headline();

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  
  Tuplet initial_values[] = {
    TupletCString(HEADLINE_KEY, "Refreshing..."),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
}

static void deinit() {
  accel_data_service_unsubscribe();
  text_layer_destroy(date_layer);
  text_layer_destroy(time_layer);
  text_layer_destroy(headline_layer);
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}   
