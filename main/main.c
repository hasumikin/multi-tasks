/* main/main.c */

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "mrubyc.h"

#include "models/thermistor.h"
#include "models/led.h"
#include "loops/master.h"
#include "loops/slave.h"

#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   64
#define MEMORY_SIZE (1024*40)

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0; //GPIO4
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_2;


static uint8_t memory_pool[MEMORY_SIZE];

static void c_gpio_init_output(mrb_vm *vm, mrb_value *v, int argc) {
  int pin = GET_INT_ARG(1);
  console_printf("init pin %d\n", pin);
  gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

static void c_gpio_set_level(mrb_vm *vm, mrb_value *v, int argc){
  int pin = GET_INT_ARG(1);
  int level = GET_INT_ARG(2);
  gpio_set_level(pin, level);
}

static void c_init_adc(mrb_vm *vm, mrb_value *v, int argc){
  adc2_config_channel_atten((adc2_channel_t)channel, atten);
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}

static void c_read_adc(mrb_vm *vm, mrb_value *v, int argc){
  uint32_t adc_reading = 0;
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    int raw;
    adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
    adc_reading += raw;
  }
  adc_reading /= NO_OF_SAMPLES;
  uint32_t millivolts = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
  SET_INT_RETURN(millivolts);
}

void app_main(void) {
  mrbc_init(memory_pool, MEMORY_SIZE);

  mrbc_define_method(0, mrbc_class_object, "gpio_init_output", c_gpio_init_output);
  mrbc_define_method(0, mrbc_class_object, "gpio_set_level", c_gpio_set_level);
  mrbc_define_method(0, mrbc_class_object, "init_adc", c_init_adc);
  mrbc_define_method(0, mrbc_class_object, "read_adc", c_read_adc);

  mrbc_create_task( thermistor, 0 );
  mrbc_create_task( led, 0 );
  mrbc_create_task( master, 0 );
  mrbc_create_task( slave, 0 );
  mrbc_run();
}
