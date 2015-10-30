//  file: eyeball.c
//  Copyright (C)2015 Matthew Clark, SetPebble

#include <pebble.h>

//  constants

#define  kDebug  0
const int kPupilInterval = 100;               //  interval (ms)
const int kEyelidInterval = 40;               //  interval (ms)
const int kPupilSize = 124;                   //  pupil width and height
const int kBlinkFrequency = 50;               //  blink frequency
const int kIrisFrequency = 80;                //  iris frequency
const int kEyelidSpeed = 50;                  //  speed of eyelid dropping
const int kSpeedMax = 10;                     //  maximum pupil speed
const int kPupilFrequency = 50;               //  frequency for changing pupil
const int kPupilMax = 50;                     //  maximum pupil displacement
const GSize kEyelidSize = { 150, 180 } ;      //  eyelid size

//  macro functions

#define  ABS(a,b)  (((a)>=(b))?(a)-(b):(b)-(a))

//  static variables

static Window* window;
static Layer* layer_screen;
static AppTimer* timer_screen;
static GBitmap* bitmap_pupils[3], * bitmap_eyelid;
static int n_pupil, n_eyelid, n_speed;
static GPoint pt_pupil, pt_destination;

//  functions

void screen_update(Layer* layer, GContext* context) {
  GRect rect = layer_get_bounds(layer);
  //GPoint pt_center = grect_center_point(&rect);
  //  draw pupil
  graphics_context_set_compositing_mode(context, GCompOpSet);
  graphics_draw_bitmap_in_rect(context, bitmap_pupils[n_pupil], GRect(pt_pupil.x + (rect.size.w - kPupilSize) / 2, pt_pupil.y + (rect.size.h - kPupilSize) / 2, kPupilSize, kPupilSize));
  //  eyelid
  if (n_eyelid) {
    int n_position = (n_eyelid <= kEyelidSize.h) ? n_eyelid - kEyelidSize.h : kEyelidSize.h - n_eyelid;
    graphics_draw_bitmap_in_rect(context, bitmap_eyelid, GRect(n_position, 0, kEyelidSize.w, kEyelidSize.h));
  }
}

void timer_callback(void* data) {
  //  clear timer
  timer_screen = NULL;
  //  eyelid animation
  if (n_eyelid) {
    n_eyelid += kEyelidSpeed;
    if (n_eyelid >= 2 * kEyelidSize.h)
      n_eyelid = 0;
  } else if (!(rand() % kBlinkFrequency))
    n_eyelid = kEyelidSpeed;
  //  iris color
  if (!(rand() % kIrisFrequency))
    n_pupil = rand() % ARRAY_LENGTH(bitmap_pupils);
  //  move pupil
  if (!gpoint_equal(&pt_pupil, &pt_destination)) {
    if (ABS(pt_pupil.x, pt_destination.x) < n_speed)
      pt_pupil.x = pt_destination.x;
    else
      pt_pupil.x += ((pt_destination.x - pt_pupil.x) * n_speed) / kSpeedMax;
    if (ABS(pt_pupil.y, pt_destination.y) < n_speed)
      pt_pupil.y = pt_destination.y;
    else
      pt_pupil.y += ((pt_destination.y - pt_pupil.y) * n_speed) / kSpeedMax;
  }
  //  relocate pupil
  if (!(rand() % kPupilFrequency)) {
    if (rand() % 2) {
      pt_destination.x = (rand() % (2 * kPupilMax)) - kPupilMax;
      pt_destination.y = (rand() % (2 * kPupilMax)) - kPupilMax;
    } else
      pt_destination.x = pt_destination.y = 0;
    n_speed = (rand() % (kSpeedMax / 2)) + (kSpeedMax / 2);
  }
  /*
   * TODO:
  */
  //  reset timer
  timer_screen = app_timer_register(n_eyelid ? kEyelidInterval : kPupilInterval, timer_callback, NULL);
  //  mark dirty
  layer_mark_dirty(layer_screen);
}

//  window

void window_load(Window* window) {
  //  get root layer
  Layer* layer_root = window_get_root_layer(window);
  GRect rect = layer_get_bounds(layer_root);
  //  images
  bitmap_pupils[0] = gbitmap_create_with_resource(RESOURCE_ID_PUPIL_BLUE);
  bitmap_pupils[1] = gbitmap_create_with_resource(RESOURCE_ID_PUPIL_GREEN);
  bitmap_pupils[2] = gbitmap_create_with_resource(RESOURCE_ID_PUPIL_RED);
  bitmap_eyelid = gbitmap_create_with_resource(RESOURCE_ID_EYELID);
  //  screen
  layer_screen = layer_create(rect);
  layer_set_update_proc(layer_screen, screen_update);
  layer_add_child(layer_root, layer_screen);
  //  light
  light_enable(true);
  //  start timer
  timer_screen = app_timer_register(kPupilInterval, timer_callback, NULL);
}

void window_unload(Window* window) {
  //  light
  light_enable(false);
  light_enable_interaction();
  //  timers
  if (timer_screen)
    app_timer_cancel(timer_screen);
  //  images
  for (int i = 0;  i < 3;  i++)
    gbitmap_destroy(bitmap_pupils[i]);
  //gbitmap_destroy(bitmap_eyelid);
  //  layers
  layer_destroy(layer_screen);
}

void init(void) {
  //  variables
  n_pupil = n_eyelid = n_speed = 0;
  pt_pupil = pt_destination = GPoint(0, 0);
  timer_screen = NULL;
  //  seed random number generator
  srand(time(NULL));
  //  create window
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_background_color(window, GColorWhite);
  window_stack_push(window, true);
}

void deinit(void) {
  //  destroy window
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
