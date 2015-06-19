#include <pebble.h>

static Window *s_main_window;

static BitmapLayer *s_h1_img_layer;
static BitmapLayer *s_h2_img_layer;
static BitmapLayer *s_m1_img_layer;
static BitmapLayer *s_m2_img_layer;

static TextLayer *s_time_layer_dte;

static BitmapLayer *s_warning_img_layer;

static GBitmap *s_h1_bitmap;
static GBitmap *s_h2_bitmap;
static GBitmap *s_m1_bitmap;
static GBitmap *s_m2_bitmap;
static GBitmap *s_warning_bitmap;

static bool lastBtStateConnected = false;

static Layer *s_canvas_layer;

static int chargeState = -1;

static GFont s_time_font_dte;

static int h1 = 0;
static int h2 = 0;
static int m1 = 0;
static int m2 = 0;

const int WIDTHS[10] = { 35, 15, 33, 30, 33, 32, 33, 33, 33, 33};

/*static int get_total_width() {
  int res = 0;
  
  res += WIDTHS[h1];
  res += WIDTHS[h2];
  res += WIDTHS[m1];
  res += WIDTHS[m2];
  
  return res;
}*/

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

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);
  
  // Create GFonts 
  s_time_font_dte = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_AERO_28));

  // Create and add the H1 Bitmap Layer
  s_h1_img_layer = bitmap_layer_create(GRect(2, 27, 36, 43));
  s_h1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZERO_BOLD);
  bitmap_layer_set_bitmap(s_h1_img_layer, s_h1_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_h1_img_layer));
  
  // Create and add the H2 Bitmap Layer
  s_h2_img_layer = bitmap_layer_create(GRect(37, 27, 36, 43));
  s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZERO_BOLD);
  bitmap_layer_set_bitmap(s_h2_img_layer, s_h2_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_h2_img_layer));
  
  // Create and add the M1 Bitmap Layer
  s_m1_img_layer = bitmap_layer_create(GRect(72, 28, 36, 42));
  s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZERO);
  bitmap_layer_set_bitmap(s_m1_img_layer, s_m1_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_m1_img_layer));
  
  // Create and add the M2 Bitmap Layer
  s_m2_img_layer = bitmap_layer_create(GRect(107, 28, 36, 42));
  s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZERO);
  bitmap_layer_set_bitmap(s_m2_img_layer, s_m2_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_m2_img_layer));
  
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

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer_dte);
  
  // Unload GFonts
  fonts_unload_custom_font(s_time_font_dte);
  
  // Destroy Layers
  layer_destroy(s_canvas_layer);
  
  // Destroy BitmapLayers and content
  gbitmap_destroy(s_h1_bitmap);
  gbitmap_destroy(s_h2_bitmap);
  gbitmap_destroy(s_m1_bitmap);
  gbitmap_destroy(s_m2_bitmap);
  gbitmap_destroy(s_warning_bitmap);

  bitmap_layer_destroy(s_h1_img_layer);
  bitmap_layer_destroy(s_h2_img_layer);
  bitmap_layer_destroy(s_m1_img_layer);
  bitmap_layer_destroy(s_m2_img_layer);
  bitmap_layer_destroy(s_warning_img_layer);
}

static void update_time() {
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

  // Write the current minuts into the buffer
  strftime(buffer_mm, sizeof("00"), "%M", tick_time);
  
  // Write the current date into the buffer
  strftime(buffer_dte, sizeof("ddd 00 mmm"), "%a %d %b", tick_time);
  
  // ASCII 48 = '0' ... 57 = '9'
  
  // Display h1
  gbitmap_destroy(s_h1_bitmap);
  
  switch ( buffer_hh[0] ) {
    case 48:
      h1 = 0; 
      s_h1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZERO_BOLD);
      break;
    case 49:
      h1 = 1; 
      s_h1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONE_BOLD);
      break;
    case 50:
      h1 = 2; 
      s_h1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TWO_BOLD);
      break;
    default:
      h1 = -1; 
      s_h1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLANK);
      break;
  }
  
  bitmap_layer_set_bitmap(s_h1_img_layer, s_h1_bitmap);

  // Display h2
  gbitmap_destroy(s_h2_bitmap);
  
  switch ( buffer_hh[1] ) {
    case 48:
      h2 = 0; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZERO_BOLD);
      break;
    case 49:
      h2 = 1; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONE_BOLD);
      break;
    case 50:
      h2 = 2; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TWO_BOLD);
      break;
    case 51:
      h2 = 3; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THREE_BOLD);
      break;
    case 52:
      h2 = 4; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOUR_BOLD);
      break;
    case 53:
      h2 = 5; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FIVE_BOLD);
      break;
    case 54:
      h2 = 6; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SIX_BOLD);
      break;
    case 55:
      h2 = 7; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SEVEN_BOLD);
      break;
    case 56:
      h2 = 8; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_EIGHT_BOLD);
      break;
    case 57:
      h2 = 9; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NINE_BOLD);
      break;
    default:
      h2 = -1; 
      s_h2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLANK);
      break;
  }
  
  bitmap_layer_set_bitmap(s_h2_img_layer, s_h2_bitmap);
  
  // Display m1
  gbitmap_destroy(s_m1_bitmap);

  switch ( buffer_mm[0] ) {
    case 48:
      m1 = 0; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZERO);
      break;
    case 49:
      m1 = 1; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONE);
      break;
    case 50:
      m1 = 2; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TWO);
      break;
    case 51:
      m1 = 3; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THREE);
      break;
    case 52:
      m1 = 4; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOUR);
      break;
    case 53:
      m1 = 5; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FIVE);
      break;
    case 54:
      m1 = 6; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SIX);
      break;
    case 55:
      m1 = 7; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SEVEN);
      break;
    case 56:
      m1 = 8; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_EIGHT);
      break;
    case 57:
      m1 = 9; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NINE);
      break;
    default:
      m1 = -1; 
      s_m1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLANK);
      break;
  }

  bitmap_layer_set_bitmap(s_m1_img_layer, s_m1_bitmap);
  
  // Display m2
  gbitmap_destroy(s_m2_bitmap);

  switch ( buffer_mm[1] ) {
    case 48:
      m2 = 0; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZERO);
      break;
    case 49:
      m2 = 1; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONE);
      break;
    case 50:
      m2 = 2; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TWO);
      break;
    case 51:
      m2 = 3; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THREE);
      break;
    case 52:
      m2 = 4; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOUR);
      break;
    case 53:
      m2 = 5; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FIVE);
      break;
    case 54:
      m2 = 6; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SIX);
      break;
    case 55:
      m2 = 7; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SEVEN);
      break;
    case 56:
      m2 = 8; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_EIGHT);
      break;
    case 57:
      m2 = 9; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NINE);
      break;
    default:
      m2 = -1; 
      s_m2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLANK);
      break;
  }
  bitmap_layer_set_bitmap(s_m2_img_layer, s_m2_bitmap);
  
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
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
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



