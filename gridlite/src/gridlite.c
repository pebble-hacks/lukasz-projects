#include <pebble.h>

Window *window;
TextLayer* time_layer[16];
char time_text[] = "1234";
char month_text[] = "DECEMBER    ";
char weekday_text[] = "THURDSDAY    ";
char day_text[] = "23RD";
char grid[16][2];
GColor foregroundcolor;
GColor backgroundcolor;

void draw_time(){
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  
  if(clock_is_24h_style()==true){
    strftime(time_text, sizeof(time_text), "%H%M", tick_time);
  }else{
    strftime(time_text, sizeof(time_text), "%I%M", tick_time);
  }
  
  for(int i=0; i<4; i++){
    char s[] = " ";
    s[0] = time_text[i];
    strcpy(grid[i], s);
    text_layer_set_text(time_layer[i], grid[i]);
  }
}

void draw_date(){
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  
  strftime(weekday_text, sizeof(weekday_text), "%A   ", tick_time);
  for(int i=0; i<(int)(strlen(weekday_text)); i++){
	if(weekday_text[i]>96)weekday_text[i]-=32;
  }
    
  for(int i=0; i<4; i++){
    char s[] = " ";
    s[0] = weekday_text[i];
    strcpy(grid[4+i], s);
    text_layer_set_text(time_layer[4+i], grid[4+i]);
  }
    
  strftime(day_text, sizeof(day_text), "%dTH", tick_time);
  if(day_text[1]=='1'&&day_text[0]!='1'){
    day_text[2]='S';
    day_text[3]='T';
  }else if(day_text[1]=='2'&&day_text[0]!='1'){
    day_text[2]='N';
    day_text[3]='D';
  }else if(day_text[1]=='3'&&day_text[0]!='1'){
    day_text[2]='R';
    day_text[3]='D';
  }else{
    day_text[2]='T';
    day_text[3]='H';
  }
    
  for(int i=0; i<4; i++){
    char s[] = " ";
    s[0] = day_text[i];
    strcpy(grid[8+i], s);
    text_layer_set_text(time_layer[8+i], grid[8+i]);
  }
  
  strftime(month_text, sizeof(month_text), "%B   ", tick_time);
  for(int i=0; i<(int)(strlen(month_text)); i++){
	if(month_text[i]>96)month_text[i]-=32;
  }
    
  for(int i=0; i<4; i++){
    char s[] = " ";
    s[0] = month_text[i];
    strcpy(grid[12+i], s);
    text_layer_set_text(time_layer[12+i], grid[12+i]);
  }
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  
  draw_time();
  
  if(tick_time->tm_min==0&&tick_time->tm_hour==0){
    draw_date();
  }
}

static void init() {
  backgroundcolor = GColorBlack;
  foregroundcolor = GColorWhite;

  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, backgroundcolor);
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  GFont time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SCIFLY_47));
  
  for(int i = 0; i<4; i++){
    for(int j = 0; j<4; j++){
      time_layer[i*4+j] = text_layer_create(GRect(j*36-4, i*42-8, 44, 50));
      text_layer_set_text_color(time_layer[i*4+j], foregroundcolor);
      text_layer_set_font(time_layer[i*4+j], time_font);
      text_layer_set_text_alignment(time_layer[i*4+j], GTextAlignmentCenter);
      text_layer_set_background_color(time_layer[i*4+j], GColorClear);
      layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer[i*4+j]));
    }
  }
  for(int i=0; i<16; i++){
    strcpy(grid[i], " ");
  }
  
  draw_time();
  draw_date();

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit() {
  for(int i = 0; i<16; i++){
    text_layer_destroy(time_layer[i]);
  }
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}   
