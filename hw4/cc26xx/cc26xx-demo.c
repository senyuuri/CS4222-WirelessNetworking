/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-platforms
 * @{
 *
 * \defgroup cc26xx-examples CC26xx Example Projects
 *
 * Example projects for CC26xx-based platforms.
 * @{
 *
 * \defgroup cc26xx-demo CC26xx Demo Project
 *
 *   Example project demonstrating the CC13xx/CC26xx platforms
 *
 *   This example will work for the following boards:
 *   - srf06-cc26xx: SmartRF06EB + CC13xx/CC26xx EM
 *   - CC2650 and CC1350 SensorTag
 *   - CC1310, CC1350, CC2650 LaunchPads
 *
 *   This is an IPv6/RPL-enabled example. Thus, if you have a border router in
 *   your installation (same RDC layer, same PAN ID and RF channel), you should
 *   be able to ping6 this demo node.
 *
 *   This example also demonstrates CC26xx BLE operation. The process starts
 *   the BLE beacon daemon (implemented in the RF driver). The daemon will
 *   send out a BLE beacon periodically. Use any BLE-enabled application (e.g.
 *   LightBlue on OS X or the TI BLE Multitool smartphone app) and after a few
 *   seconds the cc26xx device will be discovered.
 *
 * - etimer/clock : Every CC26XX_DEMO_LOOP_INTERVAL clock ticks the LED defined
 *                  as CC26XX_DEMO_LEDS_PERIODIC will toggle and the device
 *                  will print out readings from some supported sensors
 * - sensors      : Some sensortag sensors are read asynchronously (see sensor
 *                  documentation). For those, this example will print out
 *                  readings in a staggered fashion at a random interval
 * - Buttons      : CC26XX_DEMO_SENSOR_1 button will toggle CC26XX_DEMO_LEDS_BUTTON
 *                - CC26XX_DEMO_SENSOR_2 turns on LEDS_REBOOT and causes a
 *                  watchdog reboot
 *                - The remaining buttons will just print something
 *                - The example also shows how to retrieve the duration of a
 *                  button press (in ticks). The driver will generate a
 *                  sensors_changed event upon button release
 * - Reed Relay   : Will toggle the sensortag buzzer on/off
 *
 * @{
 *
 * \file
 *     Example demonstrating the cc26xx platforms
 */
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "random.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "rf-core/rf-ble.h"

#include "ti-lib.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define CC26XX_DEMO_LOOP_INTERVAL       (CLOCK_SECOND * 20)
#define CC26XX_DEMO_LEDS_PERIODIC       LEDS_YELLOW
#define CC26XX_DEMO_LEDS_BUTTON         LEDS_RED
#define CC26XX_DEMO_LEDS_REBOOT         LEDS_ALL
/*---------------------------------------------------------------------------*/
#define CC26XX_DEMO_SENSOR_NONE         (void *)0xFFFFFFFF

#define CC26XX_DEMO_SENSOR_1     &button_left_sensor
#define CC26XX_DEMO_SENSOR_2     &button_right_sensor

#if BOARD_SENSORTAG
#define CC26XX_DEMO_SENSOR_3     CC26XX_DEMO_SENSOR_NONE
#define CC26XX_DEMO_SENSOR_4     CC26XX_DEMO_SENSOR_NONE
#define CC26XX_DEMO_SENSOR_5     &reed_relay_sensor
#elif BOARD_LAUNCHPAD
#define CC26XX_DEMO_SENSOR_3     CC26XX_DEMO_SENSOR_NONE
#define CC26XX_DEMO_SENSOR_4     CC26XX_DEMO_SENSOR_NONE
#define CC26XX_DEMO_SENSOR_5     CC26XX_DEMO_SENSOR_NONE
#else
#define CC26XX_DEMO_SENSOR_3     &button_up_sensor
#define CC26XX_DEMO_SENSOR_4     &button_down_sensor
#define CC26XX_DEMO_SENSOR_5     &button_select_sensor
#endif
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS(cc26xx_demo_process, "cc26xx demo process");
AUTOSTART_PROCESSES(&cc26xx_demo_process);
/*---------------------------------------------------------------------------*/
#if BOARD_SENSORTAG
/*---------------------------------------------------------------------------*/
/*
 * Update sensor readings in a staggered fashion every SENSOR_READING_PERIOD
 * ticks + a random interval between 0 and SENSOR_READING_RANDOM ticks
 */
#define SENSOR_READING_PERIOD (CLOCK_SECOND * 10)
#define SENSOR_READING_RANDOM (CLOCK_SECOND << 4)

static struct ctimer bmp_timer, opt_timer, hdc_timer, tmp_timer, mpu_timer;
/*---------------------------------------------------------------------------*/
static void init_bmp_reading(void *not_used);
static void init_opt_reading(void *not_used);
static void init_hdc_reading(void *not_used);
static void init_tmp_reading(void *not_used);
static void init_mpu_reading(void *not_used);

int count = 0;
/*---------------------------------------------------------------------------*/
static void
print_mpu_reading(int reading, bool isLast)
{
  if(reading < 0) {
    printf("-");
    reading = -reading;
  }
  if(isLast){
    printf("%d.%02d\n", reading / 100, reading % 100);
  }else{
    printf("%d.%02d", reading / 100, reading % 100);
  }
}
/*---------------------------------------------------------------------------*/
static void
get_bmp_reading()
{
  int value;
  clock_time_t next = SENSOR_READING_PERIOD +
    (random_rand() % SENSOR_READING_RANDOM);

  count++;
  value = bmp_280_sensor.value(BMP_280_SENSOR_TYPE_PRESS);
  if(value != CC26XX_SENSOR_READING_ERROR) {
    printf("%d,b,%d.%02d,,\n", count, value / 100, value % 100);
  } else {
    printf("BAR: Pressure Read Error\n");
  }

  SENSORS_DEACTIVATE(bmp_280_sensor);

  ctimer_set(&bmp_timer, next, init_bmp_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void
get_tmp_reading()
{
  int value;
  clock_time_t next = SENSOR_READING_PERIOD +
    (random_rand() % SENSOR_READING_RANDOM);

  value = tmp_007_sensor.value(TMP_007_SENSOR_TYPE_ALL);

  if(value == CC26XX_SENSOR_READING_ERROR) {
    printf("TMP: Ambient Read Error\n");
    return;
  }

  count++;
  value = tmp_007_sensor.value(TMP_007_SENSOR_TYPE_AMBIENT);
  printf("%d,t,%d.%03d,,\n",count, value / 1000, value % 1000);

  SENSORS_DEACTIVATE(tmp_007_sensor);

  ctimer_set(&tmp_timer, next, init_tmp_reading, NULL);
}

/*---------------------------------------------------------------------------*/
static void
get_hdc_reading()
{
  int value;
  clock_time_t next = SENSOR_READING_PERIOD +
    (random_rand() % SENSOR_READING_RANDOM);

  count++;
  value = hdc_1000_sensor.value(HDC_1000_SENSOR_TYPE_HUMIDITY);
  if(value != CC26XX_SENSOR_READING_ERROR) {
    printf("%d,h,%d.%02d,,\n", count, value / 100, value % 100);
  } else {
    printf("HDC: Humidity Read Error\n");
  }

  ctimer_set(&hdc_timer, next, init_hdc_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void
get_light_reading()
{
  int value;
  clock_time_t next = SENSOR_READING_PERIOD +
    (random_rand() % SENSOR_READING_RANDOM);

  count++;
  value = opt_3001_sensor.value(0);
  if(value != CC26XX_SENSOR_READING_ERROR) {
    printf("%d,l,%d.%02d,,\n", count, value / 100, value % 100);
  } else {
    printf("OPT: Light Read Error\n");
  }

  /* The OPT will turn itself off, so we don't need to call its DEACTIVATE */
  ctimer_set(&opt_timer, next, init_opt_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void get_mpu_reading(){
  int x, y, z;
  count++;
  printf("%d,a,",count);
  x = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X);
  print_mpu_reading(x, false);
  printf(",");
  y = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y);
  print_mpu_reading(y, false);
  printf(",");
  z = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);
  print_mpu_reading(z, true);

  init_mpu_reading(NULL);
}

/*---------------------------------------------------------------------------*/
static void
init_bmp_reading(void *not_used)
{
  SENSORS_ACTIVATE(bmp_280_sensor);
}
/*---------------------------------------------------------------------------*/
static void
init_opt_reading(void *not_used)
{
  SENSORS_ACTIVATE(opt_3001_sensor);
}
/*---------------------------------------------------------------------------*/
static void
init_hdc_reading(void *not_used)
{
  SENSORS_ACTIVATE(hdc_1000_sensor);
}
/*---------------------------------------------------------------------------*/
static void
init_tmp_reading(void *not_used)
{
  SENSORS_ACTIVATE(tmp_007_sensor);
}
/*---------------------------------------------------------------------------*/
static void
init_mpu_reading(void *not_used)
{
  mpu_9250_sensor.configure(SENSORS_ACTIVE, MPU_9250_SENSOR_TYPE_ALL);
}
#endif
/*---------------------------------------------------------------------------*/
static void
get_sync_sensor_readings(void)
{
  int value;

  // value = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
  // count++;
  // printf("%d,t,%d,,\n", count, value);

  return;
}
/*---------------------------------------------------------------------------*/
static void
init_sensors(void)
{
#if BOARD_SENSORTAG
  SENSORS_ACTIVATE(reed_relay_sensor);
#endif

  SENSORS_ACTIVATE(batmon_sensor);
}
/*---------------------------------------------------------------------------*/
static void
init_sensor_readings(void)
{
#if BOARD_SENSORTAG
  SENSORS_ACTIVATE(hdc_1000_sensor);
  SENSORS_ACTIVATE(tmp_007_sensor);
  SENSORS_ACTIVATE(opt_3001_sensor);
  SENSORS_ACTIVATE(bmp_280_sensor);

  init_mpu_reading(NULL);
#endif
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc26xx_demo_process, ev, data)
{

  PROCESS_BEGIN();

  printf("CC26XX demo\n");

  init_sensors();

  /* Init the BLE advertisement daemon */
  rf_ble_beacond_config(0, BOARD_STRING);
  rf_ble_beacond_start();

  etimer_set(&et, CC26XX_DEMO_LOOP_INTERVAL);
  init_mpu_reading(NULL);
  get_sync_sensor_readings();
  init_sensor_readings();

  while(1) {

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER) {
      if(data == &et) {
        leds_toggle(CC26XX_DEMO_LEDS_PERIODIC);

        get_sync_sensor_readings();

        etimer_set(&et, CC26XX_DEMO_LOOP_INTERVAL);
      }
    } else if(ev == sensors_event) {
      if(data == CC26XX_DEMO_SENSOR_1) {
        printf("Left: Pin %d, press duration %d clock ticks\n",
               (CC26XX_DEMO_SENSOR_1)->value(BUTTON_SENSOR_VALUE_STATE),
               (CC26XX_DEMO_SENSOR_1)->value(BUTTON_SENSOR_VALUE_DURATION));

        if((CC26XX_DEMO_SENSOR_1)->value(BUTTON_SENSOR_VALUE_DURATION) >
           CLOCK_SECOND) {
          printf("Long button press!\n");
        }

        leds_toggle(CC26XX_DEMO_LEDS_BUTTON);
      } else if(data == CC26XX_DEMO_SENSOR_2) {
        leds_on(CC26XX_DEMO_LEDS_REBOOT);
        watchdog_reboot();
      } else if(data == CC26XX_DEMO_SENSOR_3) {
        printf("Up\n");
      } else if(data == CC26XX_DEMO_SENSOR_4) {
        printf("Down\n");
      } else if(data == CC26XX_DEMO_SENSOR_5) {
#if BOARD_SENSORTAG
        if(buzzer_state()) {
          buzzer_stop();
        } else {
          buzzer_start(1000);
        }
      } else if(ev == sensors_event && data == &bmp_280_sensor) {
        get_bmp_reading();
      } else if(ev == sensors_event && data == &opt_3001_sensor) {
        get_light_reading();
      } else if(ev == sensors_event && data == &hdc_1000_sensor) {
        get_hdc_reading();
      } else if(ev == sensors_event && data == &tmp_007_sensor) {
        get_tmp_reading();
      } else if(ev == sensors_event && data == &mpu_9250_sensor) {
        get_mpu_reading();
#elif BOARD_SMARTRF06EB
        printf("Sel: Pin %d, press duration %d clock ticks\n",
               button_select_sensor.value(BUTTON_SENSOR_VALUE_STATE),
               button_select_sensor.value(BUTTON_SENSOR_VALUE_DURATION));
#endif
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
