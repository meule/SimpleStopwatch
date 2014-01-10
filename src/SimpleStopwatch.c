#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static ActionBarLayer *action_bar;
static GBitmap *my_icon_pause, *my_icon_reset, *my_icon_lap, *my_icon_start;
static int seconds = 0;
static int minutes = 0;
static int hours = 0;
static bool paused = true;


static char * vtom(int sec, int min, int hour)
{
  int time = (hour * 10000) + (min * 100) + sec;
  static char arr[] = "00h\n00m\n00s";
  int i = 9;
  while (time > 0)
  {
    arr[i] = (time % 10) + '0';
    time /= 10;
    if (i % 4 == 1)
      i--;
    else if (i % 4 == 0)
      i-=3;
  }
  return arr;
}

static void switch_resume_icon()
{
  if (paused)
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, my_icon_start);
  else
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, my_icon_pause);
}


static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed)
{
  if (paused)
    return;

  seconds++;

  /* now we need to handle minutes and hours */
  if (seconds >= 60)
  {
    minutes++;
    seconds = 0;
  }
  if (minutes >= 60)
  {
    hours++;
    minutes = 0;
  }
  text_layer_set_text(text_layer, vtom(seconds,minutes,hours));
}

void lap_handle(ClickRecognizerRef recognizer, void *context)
{
  text_layer_set_text(text_layer, "LAP!");
}

void pause_handle(ClickRecognizerRef recognizer, void *context)
{
  paused = !paused;
  switch_resume_icon();
}

void reset_handle(ClickRecognizerRef recognizer, void *context)
{
  seconds = 0;
  minutes = 0;
  hours = 0;
  text_layer_set_text(text_layer, "00h\n00m\n00s");
}

void click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) lap_handle);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) pause_handle);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) reset_handle);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  text_layer = text_layer_create(GRect(5, 0, 100, 160));
  text_layer_set_text(text_layer, "00h\n00m\n00s");
  text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  //initialize images
  my_icon_start = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RESUME);
  my_icon_reset = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RESET);
  my_icon_lap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LAP);
  my_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);

  //initialize side action bar
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, my_icon_start);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, my_icon_reset);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, my_icon_lap);
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);

}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
   //deinit images
  gbitmap_destroy(my_icon_start);
  gbitmap_destroy(my_icon_reset);
  gbitmap_destroy(my_icon_lap);
  gbitmap_destroy(my_icon_pause);

  window_destroy(window);
}

int main(void) {
  init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  app_event_loop();
  deinit();
}

