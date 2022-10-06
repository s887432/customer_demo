
//#define MACHINE_ID  "A0123"
#define MACHINE_ID  "B4567"
// datetime format
// YYYY-MM-DD HH-MM-SS
char time_buf[32];
int count;

void setup() {
  Serial.begin(115200);
  count = 0;
}

// payload
// <DATE_TIME>,<DATA_1>,<DATA_2>,<DATA_3>
void loop() {
  //Serial.print(0x55AA);
  Serial.print(MACHINE_ID);
  Serial.print(",");
  sprintf(time_buf, "2022-10-11 %02d-%02d-%02d", ((count/10000)%10000), ((count/100)%100), count % 100);
  Serial.print(time_buf);
  Serial.print(",");
  Serial.print((count/10000)%10000);
  Serial.print(",");
  Serial.print((count/100)%100);
  Serial.print(",");
  Serial.print(count%100);
  Serial.print("\n\r");
  count++;
  delay(1000);
}
