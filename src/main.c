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

// config keys and values 
#define CONFIG_KEY_HH_IN_BOLD 0
#define CONFIG_KEY_MM_IN_BOLD 1
#define CONFIG_KEY_LOCALE 2
#define CONFIG_KEY_HH_STRIP_ZERO 3
  
#define locale_en 0x0
#define locale_fr 0x1
#define locale_de 0x2
#define locale_es 0x3

#define sep_mode_none 0x0
#define sep_mode_dot 0x1
#define sep_mode_colon 0x2

static bool hh_in_bold = true;
static bool mm_in_bold = false;
static int locale = locale_en;
static bool hh_strip_zero = false;
static int sep_mode = sep_mode_dot;

// days (en, fr, de, es)
const char *DAYS[4][7] = { 
  {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},
  {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"},   
  {"Son", "Mon", "Die", "Mit", "Don", "Fre", "Sam"},
  {"Dom", "Lun", "Mar", "Mié", "Jue", "Vie", "Sab"}
};

// months (en, fr, de, es)
const char *MONTHS[4][12] = { 
  {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"},
  {"Jan", "Fev", "Mar", "Avr", "Mai", "Jui", "Jul", "Aou", "Sep", "Oct", "Nov", "Dec"},   
  {"Jan", "Feb", "Mrz", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"},
  {"Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"}
};

// in Spanish, the days of the week and the months of the year are not capitalized when spelled out or abbreviated

// images collections
const int IMAGE_BOLD_RESOURCE_IDS[10] = {
  RESOURCE_ID_ZERO_BOLD, RESOURCE_ID_ONE_BOLD, RESOURCE_ID_TWO_BOLD,
  RESOURCE_ID_THREE_BOLD, RESOURCE_ID_FOUR_BOLD, RESOURCE_ID_FIVE_BOLD,
  RESOURCE_ID_SIX_BOLD, RESOURCE_ID_SEVEN_BOLD, RESOURCE_ID_EIGHT_BOLD,
  RESOURCE_ID_NINE_BOLD
}; 

const int IMAGE_REGULAR_RESOURCE_IDS[10] = {
  RESOURCE_ID_ZERO, RESOURCE_ID_ONE, RESOURCE_ID_TWO,
  RESOURCE_ID_THREE, RESOURCE_ID_FOUR, RESOURCE_ID_FIVE,
  RESOURCE_ID_SIX, RESOURCE_ID_SEVEN, RESOURCE_ID_EIGHT,
  RESOURCE_ID_NINE
}; 

// safe image getters
static int get_image_hour(int idx) {
  if ((idx >= 0) && (idx <= 9)) 
    return hh_in_bold ? IMAGE_BOLD_RESOURCE_IDS[idx] : IMAGE_REGULAR_RESOURCE_IDS[idx];
  else
    return RESOURCE_ID_BLANK;
}

static int get_image_min(int idx) {
  if ((idx >= 0) && (idx <= 9)) 
    return mm_in_bold ? IMAGE_BOLD_RESOURCE_IDS[idx] : IMAGE_REGULAR_RESOURCE_IDS[idx];
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

  if (sep_mode == sep_mode_dot) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, GPoint(71,38), GPoint(73, 38));
    graphics_draw_line(ctx, GPoint(71,39), GPoint(73, 39));
    graphics_draw_line(ctx, GPoint(71,40), GPoint(73, 40));
    
    graphics_draw_line(ctx, GPoint(71,56), GPoint(73, 56));
    graphics_draw_line(ctx, GPoint(71,57), GPoint(73, 57));
    graphics_draw_line(ctx, GPoint(71,58), GPoint(73, 58));
  }
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
  
  // m2
  layer_remove_from_parent(bitmap_layer_get_layer(s_m2_img_layer));
  bitmap_layer_destroy(s_m2_img_layer);
  gbitmap_destroy(s_m2_bitmap);
}

static void load_time_images() {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "load_time_images");
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  
  // center align
  int total_w = get_total_width() + 4; // max is 0000 -> 4*35 + 4 = 144
  int current_x = (144 - total_w) / 2;
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "total_w = %d", total_w);
  
  // Create and add the H1 Bitmap Layer
  s_h1_img_layer = bitmap_layer_create(GRect(current_x, 27, get_width(h1), 43));
  s_h1_bitmap = gbitmap_create_with_resource(get_image_hour(h1));
  bitmap_layer_set_bitmap(s_h1_img_layer, s_h1_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_h1_img_layer));
  
  // Create and add the H2 Bitmap Layer
  current_x += get_width(h1);
  s_h2_img_layer = bitmap_layer_create(GRect(current_x, 27, get_width(h2), 43));
  s_h2_bitmap = gbitmap_create_with_resource(get_image_hour(h2));
  bitmap_layer_set_bitmap(s_h2_img_layer, s_h2_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_h2_img_layer));
  
  // Create and add the M1 Bitmap Layer
  current_x += get_width(h2);
  current_x += 4; // separation between hours and minutes
  s_m1_img_layer = bitmap_layer_create(GRect(current_x, 28, get_width(m1), 42));
  s_m1_bitmap = gbitmap_create_with_resource(get_image_min(m1));
  bitmap_layer_set_bitmap(s_m1_img_layer, s_m1_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_m1_img_layer));
  
  // Create and add the M2 Bitmap Layer
  current_x += get_width(m1);
  s_m2_img_layer = bitmap_layer_create(GRect(current_x, 28, get_width(m2), 42));
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
  s_warning_img_layer = bitmap_layer_create(GRect(0, 132, 144, 32));
  s_warning_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WARN28);
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
  
  h1 = 0; //ascii_digit_to_int(buffer_hh[0]);
  h2 = 0; //ascii_digit_to_int(buffer_hh[1]);
  
  // hide leading zero if required
  if (h1 == 0 && hh_strip_zero) h1 = -1;

  // Write the current minuts into the buffer
  strftime(buffer_mm, sizeof("00"), "%M", tick_time);
  
  m1 = 0; //ascii_digit_to_int(buffer_mm[0]);
  m2 = 0; //ascii_digit_to_int(buffer_mm[1]);
  
  // Write the current date into the buffer
  if (locale != locale_en) {
    // Ddd 00 Mmm
    //strftime(buffer_dte, sizeof("ddd 00 mmm"), "%a %d %b", tick_time);
    snprintf(buffer_dte, sizeof("ddd 00 mmm"), "%s %d %s", DAYS[locale][tick_time->tm_wday], tick_time->tm_mday, MONTHS[locale][tick_time->tm_mon]);
  }
  else {
    // Ddd Mmm 00
    //strftime(buffer_dte, sizeof("ddd mmm 00"), "%a %b %d", tick_time);
    snprintf(buffer_dte, sizeof("ddd mmm 00"), "%s %s %d", DAYS[0][tick_time->tm_wday], MONTHS[0][tick_time->tm_mon], tick_time->tm_mday);
  }
  
  // undload and relaod time images
  unload_time_images();
  load_time_images();
  
  // Display values in TextLayers
  text_layer_set_text(s_time_layer_dte, buffer_dte);
  
  if (bluetooth_connection_service_peek()) {
    // phone is connected
    layer_set_hidden(bitmap_layer_get_layer(s_warning_img_layer), true); 
    //layer_set_hidden(text_layer_get_layer(s_time_layer_dte), false); 
    lastBtStateConnected = true;
  } else {
    // phone is not connected
    //layer_set_hidden(text_layer_get_layer(s_time_layer_dte), true); 
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

void read_configuration(void)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "read_configuration");
  
  if (persist_exists(CONFIG_KEY_HH_IN_BOLD))
  {
    hh_in_bold = persist_read_bool(CONFIG_KEY_HH_IN_BOLD);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "hh_in_bold = %d", hh_in_bold);
  }
  
  if (persist_exists(CONFIG_KEY_MM_IN_BOLD))
  {
    mm_in_bold = persist_read_bool(CONFIG_KEY_MM_IN_BOLD);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "mm_in_bold = %d", mm_in_bold);
  }
  
  if (persist_exists(CONFIG_KEY_LOCALE))
  {
    locale = persist_read_int(CONFIG_KEY_LOCALE);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "using config locale (0=en, 1=fr, 2=de, 3=es) = %d", locale);
  }
  else {
    // use default / system locale
    char *sys_locale = setlocale(LC_ALL, "");
    
    if (strcmp(sys_locale, "fr_FR") == 0) {
      locale = locale_fr;
    } else if (strcmp(sys_locale, "de_DE") == 0) {
      locale = locale_de;
    } else if (strcmp(sys_locale, "es_ES") == 0) {
      locale = locale_es;
    } else {
      locale = locale_en; // default
    }
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "using default locale = %s -> %d", sys_locale, locale);
  }
  
  if (persist_exists(CONFIG_KEY_HH_STRIP_ZERO))
  {
    hh_strip_zero = persist_read_bool(CONFIG_KEY_HH_STRIP_ZERO);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "hh_strip_zero = %d", hh_strip_zero);
  }
}

void in_received_handler(DictionaryIterator *received, void *context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler");
  
  Tuple *hh_in_bold_tuple = dict_find(received, CONFIG_KEY_HH_IN_BOLD);
  if (hh_in_bold_tuple)
  {
    app_log(APP_LOG_LEVEL_DEBUG,
            __FILE__,
            __LINE__,
            "hh_in_bold=%s",
            hh_in_bold_tuple->value->cstring);

    if (strcmp(hh_in_bold_tuple->value->cstring, "0") == 0)
    {
      persist_write_bool(CONFIG_KEY_HH_IN_BOLD, false);
    }
    else
    {
      persist_write_bool(CONFIG_KEY_HH_IN_BOLD, true);
    }
  }

  Tuple *mm_in_bold_tuple = dict_find(received, CONFIG_KEY_MM_IN_BOLD);
  if (mm_in_bold_tuple)
  {
    app_log(APP_LOG_LEVEL_DEBUG,
            __FILE__,
            __LINE__,
            "mm_in_bold=%s",
            mm_in_bold_tuple->value->cstring);

    if (strcmp(mm_in_bold_tuple->value->cstring, "1") == 0)
    {
      persist_write_bool(CONFIG_KEY_MM_IN_BOLD, true);
    }
    else
    {
      persist_write_bool(CONFIG_KEY_MM_IN_BOLD, false);
    }
  }
  
  Tuple *locale_tuple = dict_find(received, CONFIG_KEY_LOCALE);
  if (locale_tuple)
  {
    app_log(APP_LOG_LEVEL_DEBUG,
            __FILE__,
            __LINE__,
            "locale=%s",
            locale_tuple->value->cstring);

    if (strcmp(locale_tuple->value->cstring, "default") == 0)
    {
      if (persist_exists(CONFIG_KEY_LOCALE)) 
      {
        persist_delete(CONFIG_KEY_LOCALE);
      }
    }
    else if (strcmp(locale_tuple->value->cstring, "fr") == 0)
    {
      persist_write_int(CONFIG_KEY_LOCALE, locale_fr);
    }
    else if (strcmp(locale_tuple->value->cstring, "de") == 0)
    {
      persist_write_int(CONFIG_KEY_LOCALE, locale_de);
    }
    else if (strcmp(locale_tuple->value->cstring, "es") == 0)
    {
      persist_write_int(CONFIG_KEY_LOCALE, locale_es);
    }
    else
    {
      persist_write_int(CONFIG_KEY_LOCALE, locale_en);
    }
  }
  
  Tuple *hh_strip_zero_tuple = dict_find(received, CONFIG_KEY_HH_STRIP_ZERO);
  if (hh_strip_zero_tuple)
  {
    app_log(APP_LOG_LEVEL_DEBUG,
            __FILE__,
            __LINE__,
            "hh_strip_zero=%s",
            hh_strip_zero_tuple->value->cstring);

    if (strcmp(hh_strip_zero_tuple->value->cstring, "0") == 0)
    {
      persist_write_bool(CONFIG_KEY_HH_STRIP_ZERO, false);
    }
    else
    {
      persist_write_bool(CONFIG_KEY_HH_STRIP_ZERO, true);
    }
  }

  read_configuration();
  update_display();
}


void in_dropped_handler(AppMessageResult reason, void *ctx)
{
    app_log(APP_LOG_LEVEL_WARNING,
            __FILE__,
            __LINE__,
            "Message dropped, reason code %d",
            reason);
}


static void init() {
  // read configuration 
  read_configuration();
    
  // register configurable messages
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_open(64, 64);
  
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
  //tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  // unregister messages handling
  app_message_deregister_callbacks();
  
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



