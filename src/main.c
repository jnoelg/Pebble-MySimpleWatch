#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer_hh;
static TextLayer *s_time_layer_mm;
static TextLayer *s_time_layer_dte;

static GFont s_time_font_hh;
static GFont s_time_font_mm;
static GFont s_time_font_dte;

static void main_window_load(Window *window) {
  // Create GFonts 
  s_time_font_hh = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_AERO_64));
  s_time_font_mm = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_AERO_LIGHT_64));
  s_time_font_dte = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_AERO_LIGHT_28));

  // Create time TextLayer HH
  s_time_layer_hh = text_layer_create(GRect(0, 16, 70, 64));
  text_layer_set_background_color(s_time_layer_hh, GColorBlack);
  text_layer_set_text_color(s_time_layer_hh, GColorWhite);
  text_layer_set_text(s_time_layer_hh, "00");
  text_layer_set_font(s_time_layer_hh, s_time_font_hh);
  text_layer_set_text_alignment(s_time_layer_hh, GTextAlignmentRight);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer_hh));
  
  // Create time TextLayer MM
  s_time_layer_mm = text_layer_create(GRect(74, 16, 70, 64));
  text_layer_set_background_color(s_time_layer_mm, GColorBlack);
  text_layer_set_text_color(s_time_layer_mm, GColorWhite);
  text_layer_set_text(s_time_layer_mm, "00");
  text_layer_set_font(s_time_layer_mm, s_time_font_mm);
  text_layer_set_text_alignment(s_time_layer_mm, GTextAlignmentLeft);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer_mm));
  
  // Create time TextLayer DTE
  s_time_layer_dte = text_layer_create(GRect(0, 96, 144, 32));
  text_layer_set_background_color(s_time_layer_dte, GColorBlack);
  text_layer_set_text_color(s_time_layer_dte, GColorWhite);
  text_layer_set_text(s_time_layer_dte, "00");
  text_layer_set_font(s_time_layer_dte, s_time_font_dte);
  text_layer_set_text_alignment(s_time_layer_dte, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer_dte));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer_hh);
  text_layer_destroy(s_time_layer_mm);
  text_layer_destroy(s_time_layer_dte);
  
  // Unload GFonts
  fonts_unload_custom_font(s_time_font_hh);
  fonts_unload_custom_font(s_time_font_mm);
  fonts_unload_custom_font(s_time_font_dte);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffers
  static char buffer_hh[] = "00";
  static char buffer_mm[] = "00";
  static char buffer_dte[] = "ddd 00 mmm";
  
  // Write the current hours into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer_hh, sizeof("00:00"), "%H", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer_hh, sizeof("00:00"), "%I", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer_hh, buffer_hh);
  
  // Write the current minuts into the buffer
  strftime(buffer_mm, sizeof("00"), "%M", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer_mm, buffer_mm);
  
  // Write the current date into the buffer
  strftime(buffer_dte, sizeof("ddd 00 mmm"), "%a %d %b", tick_time);
  
  // Display this date on the TextLayer
  text_layer_set_text(s_time_layer_dte, buffer_dte);
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  // Reset locale to the one selected by the user (en_US by default)
  setlocale(LC_TIME, ""); 
  
  // Create main Window element
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}



