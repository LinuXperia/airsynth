#include "synth.hpp"
#include <cmath>

using namespace std;

AirSynth::AirSynth(const char *device)
{
   audio = unique_ptr<AudioDriver>(new ALSADriver(device, 44100, 1));
   dead.store(false);
   mixer_thread = thread(&AirSynth::mixer_loop, this);
}

AirSynth::~AirSynth()
{
   dead.store(true);
   if (mixer_thread.joinable())
      mixer_thread.join();
}

void AirSynth::set_note(unsigned note, unsigned velocity)
{
   omega.store(2.0f * M_PI * 440.0f * pow(2.0f, (note - 69.0f) / 12.0f) / 44100.0f);
   vel.store(velocity / 128.0f);
}

void AirSynth::set_sustain(bool sustain)
{
   this->sustain.store(sustain);
}

void AirSynth::mixer_loop()
{
   int16_t buffer[64];
   double angle = 0.0;

   while (!dead.load())
   {
      double om = omega.load();
      double v = vel.load();

      for (unsigned p = 0; p < 64; p++, angle += om)
         buffer[p] = int16_t(v * sin(angle) * 0x7fff);

      audio->write(buffer, 64);
   }
}

