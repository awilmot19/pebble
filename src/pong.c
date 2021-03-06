#include <pebble.h>
#include "pong.h"

#define WIDTH 115
#define HEIGHT 120
#define HORIZ_VEL 5
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
#define RIGHT_BORDER 117
#define MAX_PORTAL_SIZE 30
#define MIN_PORTAL_SIZE 30
#define MIN_PORTAL_TOP 30
#define MAX_PORTAL_TOP 110
  
static Window *s_pong_window;
static Layer *s_pong_layer;

static AppTimer *timer;

static short ball_x_vel = 0;
static short ball_y_vel = VERT_VEL;
static short ball_x_pos = (RIGHT_BORDER-LEFT_BORDER)/2;
static short ball_y_pos = (USER_GOAL-CPU_GOAL)/2;
static short person_vel = 0;
static short cpu_vel = 0;
static short person_x_pos = WIDTH/2-PADDLE_WIDTH/2;
static short cpu_x_pos = WIDTH/2-PADDLE_WIDTH/2;
static short paused = 0;
static short win = 0;
static short lose = 0;
static bool gameOn = true;
static short portal_size = 30;
static short portal_left = 55;
static short portal_right = 55;
static short roundStart = 0;

static short person_score = 0;
static short cpu_score = 0;

static void draw_pong(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_text_color(ctx, GColorWhite);
  
  // scores
  char person_score_char[2] = " ";
  person_score_char[0] = (char)(((int)'0')+person_score);
  graphics_draw_text(ctx, person_score_char, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(95,HEIGHT-TOP_BORDER-5,72,20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  
  char cpu_score_char[2] = " ";
  cpu_score_char[0] = (char)(((int)'0')+cpu_score);
  graphics_draw_text(ctx, cpu_score_char, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(95,TOP_BORDER,72,20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  
  // boundaries
  graphics_draw_line(ctx, GPoint(LEFT_BORDER, TOP_BORDER), GPoint(LEFT_BORDER, portal_left));
  graphics_draw_line(ctx, GPoint(LEFT_BORDER, portal_left+portal_size), GPoint(LEFT_BORDER, HEIGHT+CPU_GOAL));
  graphics_draw_line(ctx, GPoint(RIGHT_BORDER, TOP_BORDER), GPoint(RIGHT_BORDER, portal_right));
  graphics_draw_line(ctx, GPoint(RIGHT_BORDER, portal_right+portal_size), GPoint(RIGHT_BORDER, HEIGHT+CPU_GOAL));
  
  graphics_draw_line(ctx, GPoint(LEFT_BORDER-1, portal_left), GPoint(LEFT_BORDER-1, portal_left+portal_size));
  graphics_draw_line(ctx, GPoint(LEFT_BORDER-2, portal_left), GPoint(LEFT_BORDER-2, portal_left+portal_size));
  graphics_draw_line(ctx, GPoint(LEFT_BORDER-3, portal_left), GPoint(LEFT_BORDER-3, portal_left+portal_size));
  graphics_draw_line(ctx, GPoint(LEFT_BORDER-4, portal_left), GPoint(LEFT_BORDER-4, portal_left+portal_size));
  
  graphics_draw_line(ctx, GPoint(RIGHT_BORDER+1, portal_right), GPoint(RIGHT_BORDER+1, portal_right+portal_size));
  graphics_draw_line(ctx, GPoint(RIGHT_BORDER+2, portal_right), GPoint(RIGHT_BORDER+2, portal_right+portal_size));
  graphics_draw_line(ctx, GPoint(RIGHT_BORDER+3, portal_right), GPoint(RIGHT_BORDER+3, portal_right+portal_size));
  graphics_draw_line(ctx, GPoint(RIGHT_BORDER+4, portal_right), GPoint(RIGHT_BORDER+4, portal_right+portal_size));
  
  // ball
  if (gameOn == true)
    graphics_fill_circle(ctx, GPoint(ball_x_pos, ball_y_pos), BALL_RADIUS);
  
  // paddles
  graphics_draw_line(ctx, GPoint(person_x_pos, USER_GOAL), GPoint(person_x_pos + PADDLE_WIDTH, USER_GOAL));
  graphics_draw_line(ctx, GPoint(cpu_x_pos, CPU_GOAL), GPoint(cpu_x_pos + PADDLE_WIDTH, CPU_GOAL));
  
  // win/lose
  if (win==1) {
    graphics_draw_text(ctx, "WIN", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(52,66,40,20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  } else if (lose==1) {
    graphics_draw_text(ctx, "LOSE", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(52,66,40,20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
  if (roundStart == 2) {
    psleep(1500);
  } else roundStart++;
  if (roundStart > 2) {
  } else roundStart++;
}

static void stop() {
  paused = 1;
  app_timer_cancel(timer);
}

static void move() {
  // Move computer
  if (ball_x_pos>(cpu_x_pos+PADDLE_WIDTH)-2) {
    cpu_vel = CPU_VEL;
  } else if (ball_x_pos<(cpu_x_pos)+2) {
    cpu_vel = -CPU_VEL;
  } else {
    cpu_vel = 0;
  }
  
  // Keep paddles in boundaries
  if (person_x_pos<LEFT_BORDER+PERSON_VEL && person_vel<0) {
    person_vel = 0;
  }
  if (person_x_pos>RIGHT_BORDER-PADDLE_WIDTH-PERSON_VEL && person_vel>0) {
    person_vel = 0;
  }
  if (cpu_x_pos<LEFT_BORDER+CPU_VEL && cpu_vel<0) {
    cpu_vel = 0;
  }
  if (cpu_x_pos>RIGHT_BORDER-PADDLE_WIDTH-CPU_VEL && cpu_vel>0) {
    cpu_vel = 0;
  }
  
  // Move paddles and ball
  person_x_pos += person_vel;
  cpu_x_pos += cpu_vel;
  ball_x_pos += ball_x_vel;
  ball_y_pos += ball_y_vel;
  // Record potential scores
  if (ball_y_pos>USER_GOAL+5) {
    ball_x_vel = 2;
    ball_y_vel = -VERT_VEL;
    ball_x_pos = (RIGHT_BORDER-LEFT_BORDER)/2 + LEFT_BORDER;
    ball_y_pos = (USER_GOAL-CPU_GOAL)/2 +CPU_GOAL;
    cpu_score += 1;
    roundStart = 0;
    portal_left = MIN_PORTAL_TOP + (rand() % 50);
    portal_right = MIN_PORTAL_TOP + (rand() % 50);
    //psleep(1000);
  } else if (ball_y_pos<CPU_GOAL-5) {
    ball_x_vel = 2;
    ball_y_vel = VERT_VEL;
    ball_x_pos = (RIGHT_BORDER-LEFT_BORDER)/2;
    ball_y_pos = (USER_GOAL-CPU_GOAL)/2;
    person_score += 1;
    roundStart = 0;
    portal_left = MIN_PORTAL_TOP + (rand() % 50);
    portal_right = MIN_PORTAL_TOP + (rand() % 50);
    //psleep(1000);
  } 
  
  // Keep ball in boundaries
  if ((ball_x_pos<LEFT_BORDER-ball_x_vel && (ball_y_pos>=portal_left+portal_size || ball_y_pos<=portal_left)) || (ball_x_pos>RIGHT_BORDER-ball_x_vel 
                                                                                                                 && (ball_y_pos>=portal_right+portal_size || ball_y_pos<=portal_right))) {
    ball_x_vel = -ball_x_vel;
  }  
  
  // Send through portals
  if (ball_x_pos<LEFT_BORDER-3) {
    ball_x_pos = RIGHT_BORDER;
    ball_y_pos = (ball_y_pos-portal_left) + portal_right;
  }
  
  if (ball_x_pos>RIGHT_BORDER+3) {
    ball_x_pos = LEFT_BORDER;
    ball_y_pos = (ball_y_pos-portal_right) + portal_left;
  }
  
  // Bounce ball off paddles
  if (ball_y_pos<=CPU_GOAL+BALL_RADIUS && ball_y_pos>CPU_GOAL) {
    if (ball_x_pos>=cpu_x_pos && ball_x_pos<=cpu_x_pos+PADDLE_WIDTH) {
      if (ball_y_vel < 0) {
        ball_x_vel = (ball_x_pos - cpu_x_pos - PADDLE_WIDTH/2)*HORIZ_VEL*2;
        ball_y_vel = -ball_y_vel;
        ball_x_vel = ball_x_vel/PADDLE_WIDTH;
      }
    }
  }
  if (ball_y_pos>=USER_GOAL-BALL_RADIUS && ball_y_pos<USER_GOAL) {
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
  if (cpu_score == WINNING_SCORE) {
    lose = 1;
    gameOn = false;
    stop();
  }
  layer_mark_dirty(s_pong_layer);
}

void reset_game() {
  paused = 0;
  app_timer_cancel(timer);
  ball_x_vel = 0;
  ball_y_vel = VERT_VEL;
  ball_x_pos = WIDTH/2;
  ball_y_pos = HEIGHT/2;
  person_vel = 0;
  cpu_vel = 0;
  person_x_pos = HEIGHT/2-PADDLE_HEIGHT/2;
  cpu_x_pos = HEIGHT/2-PADDLE_HEIGHT/2;
  win = 0;
  lose = 0;
  person_score = 0;
  cpu_score = 0;
  gameOn = true;
  roundStart = true;
  

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

static void pause(ClickRecognizerRef recognizer, void *context) {
  if (win==0 && lose==0) {
    if (paused==0) {
      paused = 1;
      app_timer_cancel(timer);  
    } else {
      paused = 0;
      timer = app_timer_register(WAIT, move_with_timer, NULL);
    }
  } else {
    reset_game();
  }
}

static void back(ClickRecognizerRef recognizer, void *context) {
  reset_game();
  app_timer_cancel(timer);
  window_stack_pop(true);
}

void pong_config_provider(Window *window) {
  // set click listeners
  window_raw_click_subscribe(BUTTON_ID_UP, begin_left, end_move, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, pause);
  window_single_click_subscribe(BUTTON_ID_BACK, back);
  window_raw_click_subscribe(BUTTON_ID_DOWN, begin_right, end_move, NULL);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, reset_game, NULL);
}

static void pong_window_load(Window *window) {
  s_pong_layer = layer_create(GRect(0, 0, 144, 152));
  
  layer_set_update_proc(s_pong_layer, draw_pong);
  layer_add_child(window_get_root_layer(s_pong_window), s_pong_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) pong_config_provider);
  
  ball_x_vel = 1;
  timer = app_timer_register(WAIT, move_with_timer, NULL);
}

static void pong_window_unload(Window *window) {
  layer_destroy(s_pong_layer);
  window_destroy(s_pong_window);
}

void pong_init() {  
  // Create main Window element and assign to pointer
  s_pong_window = window_create();
  
  window_set_background_color(s_pong_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_pong_window, (WindowHandlers) {
    .load = pong_window_load,
    .unload = pong_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_pong_window, true);
}

//static void pong_deinit() {
  // Destroy window
//  window_destroy(s_pong_window);
//}

/*void runPong() {
  pong_init();
  app_event_loop();
  //pong_deinit();
}*/

