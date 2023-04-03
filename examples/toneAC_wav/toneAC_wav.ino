// For best results use 8-bit 16KHZ mono original sound file
// Ensure the original sound file is VERY loud before conversion.
// Converted with "xxd -i data.wav > data.h" (linux)
// Default resonant freq is for those tiny passive motherboard speakers (~20KHZ).
//
// NOTES:
// This dual GPIO pin AC driver is much louder and clearer than single pin GPIO DC drivers.
// Even at 16KHZ, speech quality is not great but wav sound effects are ok.
// While you *can* play wavs and tones at the same time, the result is predictably not great.

#include "tos_intercom.h";

#include "tos_working.h"
#include "tos_data.h"
#include "connected.h"
#include "disconnected.h"

#include <toneAC.h>


void setup() {
  // No set up required!
}


void loop() {

  // This is a conversation bewtween two famous computers ...

  wavAC(tos_intercom_wav, tos_intercom_wav_len, RESONANT_FREQ, RATE_16KHZ);
  wavAC(tos_data_wav, tos_data_wav_len, RESONANT_FREQ, RATE_16KHZ);
  wavAC(tos_working_wav, tos_working_wav_len, RESONANT_FREQ, RATE_16KHZ);
  wavAC(connected_wav, connected_wav_len, RESONANT_FREQ, RATE_16KHZ);
  for (int i = 0; i < 10; i++)
  {
    toneAC(random(2000) + 2000, 10);
    delay(100);
    noToneAC();
  }
  
  wavAC(disconnected_wav, disconnected_wav_len, RESONANT_FREQ, RATE_16KHZ);

  // Long pause, and loop.
  delay(1000);

}
