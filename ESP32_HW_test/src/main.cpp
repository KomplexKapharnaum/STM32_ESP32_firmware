#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include "PCM51xx.h"
#include "Wire.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;

PCM51xx pcm(Wire); //Using the default I2C address 0x74

bool sd_setup() {
  // CS / SS  GPIO for SD module
  if (!SD.begin(5)) {
    Serial.println("SD card error");
    return false;
  }
  else {
    Serial.println("SD card OK");
    return true;
  }
}

void setup()
{
  // Wifi OFF
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);
  delay(100);

  // SD start
  sd_setup();

  // DAC start
  out = new AudioOutputI2S();
  out->SetPinout(25, 27, 26); //HW dependent ! BCK, LRCK, DATA
  out->SetBitsPerSample(16);
  out->SetRate(44100);

  // PCM5141 start
  Wire.begin(2, 4); //HW dependent ! SDA, SCL

  if (pcm.begin(PCM51xx::SAMPLE_RATE_44_1K, PCM51xx::BITS_PER_SAMPLE_16))
    Serial.println("PCM51xx initialized successfully.");
  else
  {
    Serial.println("Failed to initialize PCM51xx.");
    uint8_t powerState = pcm.getPowerState();
    if (powerState == PCM51xx::POWER_STATE_I2C_FAILURE)
    {
      Serial.print("No answer on I2C bus at address ");
      Serial.println(pcm.getI2CAddr());
    }
    else
    {
      Serial.print("Power state : ");
      Serial.println(pcm.getPowerState());
      Serial.println("Check that the sample rate / bit depth combination is supported.");
    }
  }

  //Set volume
  pcm.setVolume(127);

  // MP3 Decoder
  mp3 = new AudioGeneratorMP3();

  // FILE open
  file = new AudioFileSourceSD("/test.mp3");

  // START playback
  mp3->begin(file, out);
}

void loop()
{
  if (mp3->isRunning()) {
    if (!mp3->loop()) file->seek(0,SEEK_SET); // LOOP
  }

  static uint8_t nb = 0;
  if (Serial.available())
  {
    Serial.readStringUntil('/n');
    Serial.write(nb);
    nb++;
  }
}
