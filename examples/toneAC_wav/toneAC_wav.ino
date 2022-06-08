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
#include "tos_data.h"
#include "tos_working.h"

#include <toneAC.h>


void setup() {
  // No set up required!
}


void loop() {

  // This is a conversation bewtween two famous computers ...
  
  // Play sound effect (non-blocking)
  wavAC(tos_intercom_wav, tos_intercom_wav_len, RESONANT_FREQ, RATE_8KHZ, true);

  // Pause to let the effect play ...
  delay(1500);
  
  // Say something (blocking) ...
  wavAC(tos_data_wav, tos_data_wav_len, RESONANT_FREQ, RATE_16KHZ, false);

  // Short pause ...
  delay(300);
  
  //Play some beep-boops (non-blocking) ...
  for (int i = 0; i < 10; i++)
  {
    toneAC(random(5000) + 1000, 9);
    delay(100);
    noToneAC();
  }

  // Short pause ...
  delay(300);

  // Say something (non-blocking) ...
  wavAC(tos_working_wav, tos_working_wav_len, RESONANT_FREQ, RATE_8KHZ, true);

  // Long pause, and loop.
  delay(3000);

}
