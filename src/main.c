//Main.c

#include <pebble.h>
#include "pong.h"
#include "pongVS.h"

#define WIDTH 115
#define HEIGHT 120
#define MAX_HORIZ_VEL 5
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 20
#define PADDLE_DISTANCE 10
#define USER_GOAL 130
#define CPU_GOAL 10
#define BALL_RADIUS 2
#define WAIT 40
#define CPU_VEL 3
#define PERSON_VEL 3
#define VERT_VEL 2
#define WINNING_SCORE 7
#define TOP_BORDER 10
#define LEFT_BORDER 5
#define RIGHT_BORDER 115
#define KEY_START 0
#define KEY_OPP_POS 1
  

static short cpu_x_pos;
  
//static Window *s_pong_window;
//static Layer *s_pong_layer;
static Layer *s_start_layer;
static Window *s_start_window;

//static AppTimer *timer;

static bool opponent = false;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) { 
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_START:
      APP_LOG(APP_LOG_LEVEL_ERROR, "%d", (int)t->value->int32);
      APP_LOG(APP_LOG_LEVEL_ERROR, "KEY START");
      if (((int)t->value->int32) == 1) {
        pongVS_init();
      }
    break;
    case KEY_OPP_POS:
      APP_LOG(APP_LOG_LEVEL_ERROR, "%d", (int)t->value->int32);
      cpu_x_pos = (int)t->value->int32;
      APP_LOG(APP_LOG_LEVEL_ERROR, "KEY OPP POS");
    break;
    default:
    APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
    break;
  }

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



/*void tennis_init() {  
  // Create main Window element and assign to pointer
  s_pong_window = window_create();
  
  window_set_background_color(s_pong_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_pong_window, (WindowHandlers) {
    .load = pong_window_load,
    .unload = pong_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_tennis_window, true);
}

static void tennis_deinit() {
  // Destroy window
  window_destroy(s_tennis_window);
}*/

/******************************************** Start Window **********************************************************/

static void draw_start(Layer *layer, GContext *ctx) {
  graphics_draw_text(ctx, "Looking for Game", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0,0,144,20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, "SELECT for CPU", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0,66,144,20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

//game is reset but you're destroying the window and timer?
static void start(ClickRecognizerRef recognizer, void *context) {
  pong_init();
}

void start_config_provider(Window *window) {
  // set click listeners
  window_single_click_subscribe(BUTTON_ID_SELECT, start);
}

static void start_window_load(Window *window) {
  s_start_layer = layer_create(GRect(0, 0, 144, 152));
  
  layer_set_update_proc(s_start_layer, draw_start);
  layer_add_child(window_get_root_layer(s_start_window), s_start_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) start_config_provider);
}

static void start_window_unload(Window *window) {
  layer_destroy(s_start_layer);
}

/*static void pongRun(ClickRecognizerRef recognizer, void *context) {
  pong_init();
}*/

/*void menu_config_provider(Window *window) {
  // set click listeners
  window_single_click_subscribe(BUTTON_ID_SELECT, pongRun);
}*/

void start_init() {  
  // Create main Window element and assign to pointer
  s_start_window = window_create();

  window_set_background_color(s_start_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_start_window, (WindowHandlers) {
    .load = start_window_load,
    .unload = start_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_start_window, true);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
//  window_set_click_config_provider(s_start_window, (ClickConfigProvider) menu_config_provider);
}

static void start_deinit() {
  // Destroy window
  window_destroy(s_start_window);
}


int main() {
  start_init();
  app_event_loop();
  start_deinit();
}