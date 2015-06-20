#include <pebble.h>

static Window *s_main_window;

// graphical elements
static BitmapLayer *s_h1_img_layer;
static BitmapLayer *s_h2_img_layer;
static BitmapLayer *s_m1_img_layer;
static BitmapLayer *s_m2_img_layer;
static GBitmap *s_h1_bitmap;
static GBitmap *s_h2_bitmap;
static GBitmap *s_m1_bitmap;
static GBitmap *s_m2_bitmap;

static BitmapLayer *s_warning_img_layer;
static GBitmap *s_warning_bitmap;

static TextLayer *s_time_layer_dte;

static Layer *s_canvas_layer;

static GFont s_time_font_dte;

// states
static bool lastBtStateConnected = false;
static int chargeState = -1;

// time decomposition
static int h1 = 0;
static int h2 = 0;
static int m1 = 0;
static int m2 = 0;

// images collections
const int IMAGE_HOUR_RESOURCE_IDS[10] = {
  RESOURCE_ID_ZERO_BOLD, RESOURCE_ID_ONE_BOLD, RESOURCE_ID_TWO_BOLD,
  RESOURCE_ID_THREE_BOLD, RESOURCE_ID_FOUR_BOLD, RESOURCE_ID_FIVE_BOLD,
  RESOURCE_ID_SIX_BOLD, RESOURCE_ID_SEVEN_BOLD, RESOURCE_ID_EIGHT_BOLD,
  RESOURCE_ID_NINE_BOLD
}; 

const int IMAGE_MIN_RESOURCE_IDS[10] = {
  RESOURCE_ID_ZERO, RESOURCE_ID_ONE, RESOURCE_ID_TWO,
  RESOURCE_ID_THREE, RESOURCE_ID_FOUR, RESOURCE_ID_FIVE,
  RESOURCE_ID_SIX, RESOURCE_ID_SEVEN, RESOURCE_ID_EIGHT,
  RESOURCE_ID_NINE
}; 

// safe getters
static int get_image_hour(int idx) {
  if ((idx >= 0) && (idx <= 9)) 
    return IMAGE_HOUR_RESOURCE_IDS[idx];
  else
    return RESOURCE_ID_BLANK;
}

static int get_image_min(int idx) {
  if ((idx >= 0) && (idx <= 9)) 
    return IMAGE_MIN_RESOURCE_IDS[idx];
  else
    return RESOURCE_ID_BLANK;
}

// widths
const int WIDTHS[10] = { 35, 15, 33, 30, 33, 32, 33, 33, 33, 33};

// safe width getter
static int get_width(int idx) {
  if ((idx >= 0) && (idx <= 9)) 
    return WIDTHS[idx];
  else
    return 0;
}

// calc total width
static int get_total_width() {
  int res = 0;
  
  res += get_width(h1);
  res += get_width(h2);
  res += get_width(m1);
  res += get_width(m2);
  
  return res;
}

// convert ascii digit to its value
static int ascii_digit_to_int(int ascii_val) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "ascii_digit_to_int: ascii_val = %d", ascii_val);
  // ASCII 48 = '0' ... 57 = '9'
  if ((ascii_val >= 48) && (ascii_val <= 57)) 
    return (ascii_val - 48);
  else 
    return -1;
}

// color bar drawing
static void layer_update_callback(Layer *me, GContext *ctx) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Getting chargeState = %d", chargeState);
  
  #ifdef PBL_COLOR
    //GColor color = GColorFromRGB(rand() % 255, rand() % 255, rand() % 255);
    GColor color = GColorFromRGB(255, 255, 255);
  
    if (chargeState >= 80) {
        color = GColorFromRGB(0, 255, 0);
    } else if (chargeState >= 50) {
        color = GColorFromRGB(0, 0, 255);
    } else if (chargeState >= 30) {
        color = GColorFromRGB(255, 255, 0);
    } else if (chargeState > -1) {
        color = GColorFromRGB(255, 0, 0);
    }    
    
    graphics_context_set_stroke_color(ctx, color);
  #else
    graphics_context_set_stroke_color(ctx, GColorWhite);
  #endif
 
  graphics_draw_line(ctx, GPoint(24,88), GPoint(120, 88));
  graphics_draw_line(ctx, GPoint(24,89), GPoint(120, 89));
  graphics_draw_line(ctx, GPoint(24,90), GPoint(120, 90));
  graphics_draw_line(ctx, GPoint(24,91), GPoint(120, 91));
}

static void unload_time_images() {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "unload_time_images");
  
  // h1
  layer_remove_from_parent(bitmap_layer_get_layer(s_h1_img_layer));
  bitmap_layer_destroy(s_h1_img_layer);
  gbitmap_destroy(s_h1_bitmap);
  
  // h2
  layer_remove_from_parent(bitmap_layer_get_layer(s_h2_img_layer));
  bitmap_layer_destroy(s_h2_img_layer);
  gbitmap_destroy(s_h2_bitmap);
  
  // m1
  layer_remove_from_parent(bitmap_layer_get_layer(s_m1_img_layer));
  bitmap_layer_destroy(s_m1_img_layer);
  gbitmap_destroy(s_m1_bitmap);
  
  // h1
  layer_remove_from_parent(bitmap_layer_get_layer(s_m2_img_layer));
  bitmap_layer_destroy(s_m2_img_layer);
  gbitmap_destroy(s_m2_bitmap);
}

static void load_time_images() {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "load_time_images");
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  
  // center align
  int total_w = get_total_width();
  int current_x = (144 - total_w) / 2;
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "total_w = %d", total_w);
  
  // Create and add the H1 Bitmap Layer
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "current_x = %d, h1 = %d", current_x, h1);
  s_h1_img_layer = bitmap_layer_create(GRect(current_x, 27, get_width(h1), 43));
  //layer_set_bounds(bitmap_layer_get_layer(s_h1_img_layer), GRect(current_x, 27, get_width(h1), 43));
  s_h1_bitmap = gbitmap_create_with_resource(get_image_hour(h1));
  bitmap_layer_set_bitmap(s_h1_img_layer, s_h1_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_h1_img_layer));
  
  // Create and add the H2 Bitmap Layer
  current_x += get_width(h1);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "current_x = %d, h2 = %d", current_x, h2);
  s_h2_img_layer = bitmap_layer_create(GRect(current_x, 27, get_width(h2), 43));
  //layer_set_bounds(bitmap_layer_get_layer(s_h2_img_layer), GRect(current_x, 27, get_width(h2), 43));
  s_h2_bitmap = gbitmap_create_with_resource(get_image_hour(h2));
  bitmap_layer_set_bitmap(s_h2_img_layer, s_h2_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_h2_img_layer));
  
  // Create and add the M1 Bitmap Layer
  current_x += get_width(h2);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "current_x = %d, m1 = %d", current_x, m1);
  s_m1_img_layer = bitmap_layer_create(GRect(current_x, 28, get_width(m1), 42));
  //layer_set_bounds(bitmap_layer_get_layer(s_m1_img_layer), GRect(current_x, 28, get_width(m1), 42));
  s_m1_bitmap = gbitmap_create_with_resource(get_image_min(m1));
  bitmap_layer_set_bitmap(s_m1_img_layer, s_m1_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_m1_img_layer));
  
  // Create and add the M2 Bitmap Layer
  current_x += get_width(m1);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "current_x = %d, m2 = %d", current_x, m2);
  s_m2_img_layer = bitmap_layer_create(GRect(current_x, 28, get_width(m2), 42));
  //layer_set_bounds(bitmap_layer_get_layer(s_m2_img_layer), GRect(current_x, 28, get_width(m2), 42));
  s_m2_bitmap = gbitmap_create_with_resource(get_image_min(m2));
  bitmap_layer_set_bitmap(s_m2_img_layer, s_m2_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_m2_img_layer));
}

// main window loading (initialisation)
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);
  
  // Create GFonts 
  s_time_font_dte = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_AERO_28));

  // Initial time (00:00)
  load_time_images();
  
  // Create and add the time TextLayer DTE
  s_time_layer_dte = text_layer_create(GRect(0, 96, 144, 32));
  text_layer_set_background_color(s_time_layer_dte, GColorBlack);
  text_layer_set_text_color(s_time_layer_dte, GColorWhite);
  text_layer_set_text(s_time_layer_dte, "Ddd 00 Mmm");
  text_layer_set_font(s_time_layer_dte, s_time_font_dte);
  text_layer_set_text_alignment(s_time_layer_dte, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer_dte));
  
  // Create and add a line Canvas Layer
  s_canvas_layer = layer_create(frame);
  layer_set_update_proc(s_canvas_layer, layer_update_callback);
  layer_add_child(window_layer, s_canvas_layer);
  
  // Create and add a Bitmap Layer for BT Signal warning
  s_warning_img_layer = bitmap_layer_create(GRect(0, 96, 144, 64));
  s_warning_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WARNING_IMAGE);
  bitmap_layer_set_bitmap(s_warning_img_layer, s_warning_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_warning_img_layer));
}

// main window unloading (destruction)
static void main_window_unload(Window *window) {
  // Unload font
  fonts_unload_custom_font(s_time_font_dte);

  // Destroy text layer
  text_layer_destroy(s_time_layer_dte);
  
  // Destroy canvas layer
  layer_destroy(s_canvas_layer);
  
  // Remove and destroy bitmap layers and images
  unload_time_images();
  
  layer_remove_from_parent(bitmap_layer_get_layer(s_warning_img_layer));
  bitmap_layer_destroy(s_warning_img_layer);
  gbitmap_destroy(s_warning_bitmap);
}

// update display
static void update_display() {
  // Getting Battery State
  BatteryChargeState current_battery_charge_state = battery_state_service_peek();
  
  if (current_battery_charge_state.is_charging) {
    chargeState = -1;
  }
  else {
    chargeState = current_battery_charge_state.charge_percent;
  }
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting chargeState = %d", chargeState);
  
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
  
  h1 = ascii_digit_to_int(buffer_hh[0]);
  h2 = ascii_digit_to_int(buffer_hh[1]);

  // Write the current minuts into the buffer
  strftime(buffer_mm, sizeof("00"), "%M", tick_time);
  
  m1 = ascii_digit_to_int(buffer_mm[0]);
  m2 = ascii_digit_to_int(buffer_mm[1]);
  
  // Write the current date into the buffer
  strftime(buffer_dte, sizeof("ddd 00 mmm"), "%a %d %b", tick_time);
  
  // undload and relaod time images
  unload_time_images();
  load_time_images();
  
  // Display values in TextLayers
  text_layer_set_text(s_time_layer_dte, buffer_dte);
  
  if (bluetooth_connection_service_peek()) {
    // phone is connected
    layer_set_hidden(bitmap_layer_get_layer(s_warning_img_layer), true); 
    layer_set_hidden(text_layer_get_layer(s_time_layer_dte), false); 
    lastBtStateConnected = true;
  } else {
    // phone is not connected
    layer_set_hidden(text_layer_get_layer(s_time_layer_dte), true); 
    layer_set_hidden(bitmap_layer_get_layer(s_warning_img_layer), false); 
    
    // if we just lost the connection, vibe twice
    if (lastBtStateConnected) {
      lastBtStateConnected = false;
      vibes_double_pulse();
    }
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_display();
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
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_display();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Untergister services
  tick_timer_service_unsubscribe();
  
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}



