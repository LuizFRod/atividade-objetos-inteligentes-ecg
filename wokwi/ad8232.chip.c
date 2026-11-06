// Modelo didatico de interface do AD8232. NAO e simulacao eletrica/fisiologica.
// O ECG sintetico e gerado neste componente; o ESP32 le a saida por ADC.
#include "wokwi-api.h"
#include <math.h>
#include <stdlib.h>

typedef struct {
  pin_t output, lo_plus, lo_minus, shutdown, disconnected;
  uint32_t bpm;
  double phase;
} chip_state_t;

static float gaussian(double phase, double center, double width, double scale) {
  double z = (phase - center) / width;
  return (float)(scale * exp(-z * z));
}

static void update(void *user_data) {
  chip_state_t *chip = (chip_state_t *)user_data;
  // Apenas interface funcional: alimentacao/impedancias/filtros nao modelados.
  bool off = pin_read(chip->shutdown) == LOW;
  bool lead_off = pin_read(chip->disconnected) == HIGH;
  pin_write(chip->lo_plus, lead_off ? HIGH : LOW);
  pin_write(chip->lo_minus, lead_off ? HIGH : LOW);
  float bpm = attr_read_float(chip->bpm);
  if (bpm < 30) bpm = 30;
  if (bpm > 180) bpm = 180;
  chip->phase += bpm / 60.0 * 0.001; // timer de 1 ms
  if (chip->phase >= 1.0) chip->phase -= 1.0;
  float ecg = gaussian(chip->phase, .18, .04, .12)
            + gaussian(chip->phase, .37, .012, -.15)
            + gaussian(chip->phase, .40, .010, 1.00)
            + gaussian(chip->phase, .43, .015, -.25)
            + gaussian(chip->phase, .68, .09, .30);
  float volts = off ? 0.0f : (lead_off ? 1.65f : 1.65f + ecg);
  pin_dac_write(chip->output, volts);
}

void chip_init(void) {
  chip_state_t *chip = calloc(1, sizeof(chip_state_t));
  chip->output = pin_init("OUTPUT", ANALOG);
  chip->lo_plus = pin_init("LOP", OUTPUT_LOW);
  chip->lo_minus = pin_init("LON", OUTPUT_LOW);
  chip->shutdown = pin_init("SDN", INPUT_PULLUP);
  chip->disconnected = pin_init("DESC", INPUT_PULLDOWN);
  chip->bpm = attr_init_float("bpm", 75);
  const timer_config_t config = {.callback = update, .user_data = chip};
  timer_t timer = timer_init(&config);
  pin_dac_write(chip->output, 1.65f);
  timer_start(timer, 1000, true);
}
