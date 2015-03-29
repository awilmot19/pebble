#include <pebble.h>
#include "pongVS.h"
  
#define KEY_OPP_POS 0
#define KEY_PLAY_POS 0

#define WIDTH 115
#define HEIGHT 120
#define HORIZ_VEL 5
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 20
#define PADDLE_DISTANCE 10
#define USER_GOAL 130
#define OPP_GOAL 10
#define BALL_RADIUS 2
#define WAIT 40
//#define CPU_VEL 3
#define PERSON_VEL 3
#define VERT_VEL 2
#define WINNING_SCORE 7
#define TOP_BORDER 10
#define LEFT_BORDER 5
#define RIGHT_BORDER 115
  
static short opp_x_pos;
  
//static Window *s_pong_window;
//static Layer *s_pong_layer;
static Layer *s_start_layer;
static Window *s_start_window;

//static AppTimer *timer;

//static bool opponent = false;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) { 
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_OPP_POS:
      APP_LOG(APP_LOG_LEVEL_ERROR, "%d", (int)t->value->int32);
      opp_x_pos = (int)t->value->int32;
      //return opp_x_pos;
      APP_LOG(APP_LOG_LEVEL_ERROR, "KEY OPP POS");
    break;
    default:
    APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
    break;
    }
  }
    // Look for next item
    t = dict_read_next(iterator);
}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_PLAY_POS:
        APP_LOG(APP_LOG_LEVEL_ERROR, "%d", (int)t->value->int32);
        //_x_pos = (int)t->value->int32;
        APP_LOG(APP_LOG_LEVEL_ERROR, "KEY OPP POS");
      break;
      default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }
  }

  // Look for next item
  t = dict_read_next(iterator);
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
  
static Window *s_pongVS_window;
static Layer *s_pongVS_layer;

static AppTimer *timer;

static short ball_x_vel = 0;
static short ball_y_vel = VERT_VEL;
static short ball_x_pos = (RIGHT_BORDER-LEFT_BORDER)/2;
static short ball_y_pos = (USER_GOAL-OPP_GOAL)/2;
static short person_vel = 0;
//static short cpu_vel = 0;
static short person_x_pos = WIDTH/2-PADDLE_WIDTH/2;
static short curr_opp_pos = WIDTH/2-PADDLE_WIDTH/2;
static short paused = 0;
static short win = 0;
static short lose = 0;
static bool gameOn = true;

static short person_score = 0;
static short opp_score = 0;

static void draw_pongVS(Layer *layer, GContext *ctx) {

  static DictionaryIterator *iter;
  //short size = dict_calc_buffer_size(1, sizeof(short));
  //uint8_t buffer[size];
  
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_text_color(ctx, GColorWhite);
  
  // Get opponents current paddle position
  //curr_opp_pos = inbox_received_callback(DictionaryIterator *iterator, void *context);
  // Send players current paddle position
  //dict_write_begin(iter, buffer, sizeof(buffer));
  app_message_outbox_begin(&iter);
  dict_write_int(iter, KEY_PLAY_POS, &person_x_pos, sizeof(person_x_pos), true);
  //outbox_sent_callback(&iter, void *context);
  app_message_outbox_send();
  
  // scores
  char person_score_char[2] = " ";
  person_score_char[0] = (char)(((int)'0')+person_score);
  graphics_draw_text(ctx, person_score_char, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(95,HEIGHT-TOP_BORDER-5,72,20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  
  char opp_score_char[2] = " ";
  opp_score_char[0] = (char)(((int)'0')+opp_score);
  graphics_draw_text(ctx, opp_score_char, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(95,TOP_BORDER,72,20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  
  // boundaries
  graphics_draw_line(ctx, GPoint(LEFT_BORDER,TOP_BORDER), GPoint(LEFT_BORDER,HEIGHT+TOP_BORDER));
  graphics_draw_line(ctx, GPoint(RIGHT_BORDER,TOP_BORDER), GPoint(RIGHT_BORDER, HEIGHT+TOP_BORDER));
  
  // ball
  if (gameOn == true)
    graphics_fill_circle(ctx, GPoint(ball_x_pos, ball_y_pos), BALL_RADIUS);
  
  // paddles
  graphics_draw_line(ctx, GPoint(person_x_pos, USER_GOAL), GPoint(person_x_pos + PADDLE_WIDTH, USER_GOAL));
  graphics_draw_line(ctx, GPoint(curr_opp_pos, OPP_GOAL), GPoint(curr_opp_pos + PADDLE_WIDTH, OPP_GOAL));
  
  // win/lose
  if (win==1) {
    graphics_draw_text(ctx, "WIN", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(52,66,40,20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  } else if (lose==1) {
    graphics_draw_text(ctx, "LOSE", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(52,66,40,20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

static void stop() {
  paused = 1;
  app_timer_cancel(timer);
}

static void move() {
  // Keep player's paddle in boundaries
  if (person_x_pos<LEFT_BORDER+PERSON_VEL && person_vel<0) {
    person_vel = 0;
  }
  if (person_x_pos>RIGHT_BORDER-PADDLE_WIDTH-PERSON_VEL && person_vel>0) {
    person_vel = 0;
  }
  
  // Move paddles and ball
  person_x_pos += person_vel;
  ball_x_pos += ball_x_vel;
  ball_y_pos += ball_y_vel;
  // Record potential scores
  if (ball_y_pos>USER_GOAL+5) {
    ball_x_vel = 2;
    ball_y_vel = -VERT_VEL;
    ball_x_pos = (RIGHT_BORDER-LEFT_BORDER)/2;
    ball_y_pos = (USER_GOAL-OPP_GOAL)/2;
    opp_score += 1;
    psleep(500);
  } else if (ball_y_pos<OPP_GOAL-5) {
    ball_x_vel = 2;
    ball_y_vel = VERT_VEL;
    ball_x_pos = (RIGHT_BORDER-LEFT_BORDER)/2;
    ball_y_pos = (USER_GOAL-OPP_GOAL)/2;
    person_score += 1;
    psleep(500);
  } 
  
  // Keep ball in boundaries
  if (ball_x_pos<LEFT_BORDER-ball_x_vel || ball_x_pos>RIGHT_BORDER-ball_x_vel) {
      ball_x_vel = -ball_x_vel;
  }  
  
  // Bounce ball off paddles
  if (ball_y_pos<=OPP_GOAL+BALL_RADIUS && ball_y_pos>OPP_GOAL+1) {
    if (ball_x_pos>=curr_opp_pos && ball_x_pos<=curr_opp_pos+PADDLE_WIDTH) {
      if (ball_y_vel < 0) {
        ball_x_vel = (ball_x_pos - curr_opp_pos - PADDLE_WIDTH/2)*HORIZ_VEL*2;
        ball_y_vel = -ball_y_vel;
        ball_x_vel = ball_x_vel/PADDLE_WIDTH;
      }
    }
  }
  if (ball_y_pos>=USER_GOAL-BALL_RADIUS && ball_y_pos<USER_GOAL-1) {
    if (ball_x_pos>=person_x_pos-2 && ball_x_pos<=person_x_pos+PADDLE_WIDTH+2) {
      if (ball_y_vel > 0) {
        ball_x_vel = (ball_x_pos - person_x_pos - PADDLE_WIDTH/2)*HORIZ_VEL*2;
        ball_y_vel = -ball_y_vel;
        ball_x_vel = ball_x_vel/PADDLE_WIDTH;
      }
    }
  }
}

static void move_with_timer() {
  move();
  timer = app_timer_register(WAIT, move_with_timer, NULL);
  if (person_score == WINNING_SCORE) {
    win = 1;
    gameOn = false;
    stop();
  }
  if (opp_score == WINNING_SCORE) {
    lose = 1;
    gameOn = false;
    stop();
  }
  layer_mark_dirty(s_pongVS_layer);
}

void reset_gameVS() {
  paused = 0;
  app_timer_cancel(timer);
  ball_x_vel = 0;
  ball_y_vel = VERT_VEL;
  ball_x_pos = WIDTH/2;
  ball_y_pos = HEIGHT/2;
  person_vel = 0;
  person_x_pos = HEIGHT/2-PADDLE_HEIGHT/2;
  opp_x_pos = HEIGHT/2-PADDLE_HEIGHT/2;
  win = 0;
  lose = 0;
  person_score = 0;
  opp_score = 0;
  gameOn = true;

  timer = app_timer_register(WAIT, move_with_timer, NULL);
}

static void end_move(ClickRecognizerRef recognizer, void *context) {
  person_vel = 0;
}

static void begin_left(ClickRecognizerRef recognizer, void *context) {
  person_vel = -PERSON_VEL;
}

static void begin_right(ClickRecognizerRef recognizer, void *context) {
  person_vel = PERSON_VEL;
}

static void back(ClickRecognizerRef recognizer, void *context) {
  reset_gameVS();
  app_timer_cancel(timer);
  window_stack_pop(true);
}

void pongVS_config_provider(Window *window) {
  // set click listeners
  window_raw_click_subscribe(BUTTON_ID_UP, begin_left, end_move, NULL);
  window_single_click_subscribe(BUTTON_ID_BACK, back);
  window_raw_click_subscribe(BUTTON_ID_DOWN, begin_right, end_move, NULL);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, reset_gameVS, NULL);
}

static void pongVS_window_load(Window *window) {
  s_pongVS_layer = layer_create(GRect(0, 0, 144, 152));
  
  layer_set_update_proc(s_pongVS_layer, draw_pongVS);
  layer_add_child(window_get_root_layer(s_pongVS_window), s_pongVS_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) pongVS_config_provider);
  
  ball_x_vel = 1;
  timer = app_timer_register(WAIT, move_with_timer, NULL);
}

static void pongVS_window_unload(Window *window) {
  layer_destroy(s_pongVS_layer);
  window_destroy(s_pongVS_window);
}

void pongVS_init() {  
  // Create main Window element and assign to pointer
  s_pongVS_window = window_create();
  
  window_set_background_color(s_pongVS_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_pongVS_window, (WindowHandlers) {
    .load = pongVS_window_load,
    .unload = pongVS_window_unload
  });
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_pongVS_window, true);
}

//static void pong_deinit() {
  // Destroy window
//  window_destroy(s_pong_window);
//}

void runPong() {
  pongVS_init();
  app_event_loop();
  //pong_deinit();
}


