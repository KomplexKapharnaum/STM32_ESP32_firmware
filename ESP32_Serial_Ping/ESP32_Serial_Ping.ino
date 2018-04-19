
void setup()
{
  Serial.begin(115200);
}

void loop()
{
  static uint8_t nb = 0;
  Serial.readStringUntil('/n');
  Serial.write(nb);
  nb++;
}
