/* ---------------------------------------------------------------------------
Created by Tim Eckel - teckel@leethost.com
Copyright 2019 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html

See "toneAC.h" for purpose, syntax, version history, links, and more.
--------------------------------------------------------------------------- */

#include "toneAC_internal.h"

#if defined(ESP32)

#include <mutex>
#include <esp32-hal-cpu.h>
#include <driver/mcpwm.h>

#define MCPWM0APIN 25
#define MCPWM0BPIN 26

static hw_timer_t *_tAC_pcm = NULL;
static void IRAM_ATTR onPCM();

static std::once_flag _tAC_init;

volatile unsigned char*   _pcm_data    = NULL;
volatile unsigned int     _pcm_length  = 0;
volatile unsigned int     _pcm_index   = 0;
volatile          bool    _pcm_playing = false;

void toneAC_init() {
	std::call_once(_tAC_init, [](){
		// Initialize MCPWM
		mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MCPWM0APIN);
		mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, MCPWM0BPIN);
		mcpwm_config_t pwm_config;
		pwm_config.frequency = 1;
		pwm_config.cmpr_a = 0.0;
		pwm_config.cmpr_b = 0.0;
		pwm_config.counter_mode = MCPWM_UP_COUNTER;
		pwm_config.duty_mode = MCPWM_DUTY_MODE_1;
		mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
		mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_LOW_COMPLIMENT_MODE, 0, 0);
		mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);

		_tAC_pcm   = timerBegin(0, ESP.getCpuFreqMHz(), true);
		timerAttachInterrupt(_tAC_pcm,   &onPCM,   true);


	});
}

void toneAC_playNote(unsigned long frequency, uint8_t volume) {
	float duty = 100.0 / _tAC_volume[volume - 1];

	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MCPWM0APIN);
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, MCPWM0BPIN);

	mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, frequency);
	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty);
	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty);
	mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
}



void toneAC_playWAV(unsigned char* data, unsigned long size, unsigned long resonant_freq, unsigned long rate) {

	//TODO: pass resonant freq and bitrate

	_pcm_data   = data;
	_pcm_length = size;
	_pcm_index  = 0;

	//This is to eliminate 10Hz buzz  (attach pins when MCPWM not idle - detached in noToneC below)
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MCPWM0APIN);
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, MCPWM0BPIN);

	mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, resonant_freq); //common resonant freq
	mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);

	timerAlarmWrite(_tAC_pcm, rate, true);
	timerRestart(_tAC_pcm);
	timerAlarmEnable(_tAC_pcm);

	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 50);
	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 50);

	_pcm_playing = true;

	while(_pcm_index < _pcm_length)
	{
	}

	noToneAC();
}


void noToneAC() {

	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 50);
	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 50);

	_pcm_playing = false;
	timerAlarmDisable(_tAC_pcm);
	mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);

}


uint32_t cp0_regs[18];

static void IRAM_ATTR onPCM() {

	// FPU state must be restored or it panics.
	// Save FPU state
	xthal_set_cpenable(1);
	xthal_save_cp0(cp0_regs);



	//Stream 8-bit PCM data.
	//TODO - provide composite buffer for concurrent sounds.

	if (_pcm_index >= _pcm_length){

		noToneAC();
		//This is to eliminate 10Hz buzz (detach pins when MCPWM is idle)
		gpio_reset_pin((gpio_num_t)MCPWM0APIN);
		gpio_reset_pin((gpio_num_t)MCPWM0BPIN);

	}

	if (_pcm_playing){

		uint8_t  d = _pcm_data[_pcm_index];
		double duty =  ((double)d)/255.0*100.0;

		mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty);
		mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty);

		_pcm_index++;
	}

	// Restore FPU state
	xthal_restore_cp0(cp0_regs);
	xthal_set_cpenable(0);
}

#endif
