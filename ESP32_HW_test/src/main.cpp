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

#if HW_REVISION == 1
  const uint8_t I2C_SDA_PIN = 2;
  const uint8_t I2C_SCL_PIN = 4;
  const uint8_t I2S_LRCK_PIN = 27;
  const uint8_t I2S_DATA_PIN = 26;
  const uint8_t I2S_BCK_PIN = 25;
  const uint8_t SD_DI_PIN = 23;
  const uint8_t SD_DO_PIN = 19;
  const uint8_t SD_SCK_PIN = 18;
  const uint8_t SD_CS_PIN = 5;

#elif HW_REVISION == 2
  const uint8_t I2C_SDA_PIN = 32;
  const uint8_t I2C_SCL_PIN = 33;
  const uint8_t I2S_LRCK_PIN = 25;
  const uint8_t I2S_DATA_PIN = 26;
  const uint8_t I2S_BCK_PIN = 27;
  const uint8_t SD_DI_PIN = 19;
  const uint8_t SD_DO_PIN = 5;
  const uint8_t SD_SCK_PIN = 18;
  const uint8_t SD_CS_PIN = 21;

#else
  #error "HW_REVISION undefined or invalid. Should be 1 or 2"
#endif


PCM51xx pcm(Wire); //Using the default I2C address 0x74

bool sd_setup() {
  SPI.begin(SD_SCK_PIN, SD_DO_PIN, SD_DI_PIN);
  if (!SD.begin(SD_CS_PIN)) {
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
  out->SetPinout(I2S_BCK_PIN, I2S_LRCK_PIN, I2S_DATA_PIN); 
  out->SetBitsPerSample(16);
  out->SetRate(44100);

  // PCM5141 start
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN); 

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
