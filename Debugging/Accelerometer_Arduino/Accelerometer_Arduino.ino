#include <Wire.h>

const int MPU = 0x68;

int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

float Ax_g, Ay_g, Az_g;
float Gx_dps, Gy_dps, Gz_dps;

void setup() {
  Wire.begin();

  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.begin(9600);
}

void loop() {

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU,14,true);

  AcX = Wire.read()<<8 | Wire.read();
  AcY = Wire.read()<<8 | Wire.read();
  AcZ = Wire.read()<<8 | Wire.read();

  Tmp = Wire.read()<<8 | Wire.read();

  GyX = Wire.read()<<8 | Wire.read();
  GyY = Wire.read()<<8 | Wire.read();
  GyZ = Wire.read()<<8 | Wire.read();

  // Convert accelerometer to g
  Ax_g = AcX / 16384.0;
  Ay_g = AcY / 16384.0;
  Az_g = AcZ / 16384.0;

  // Convert gyro to deg/s
  Gx_dps = GyX / 131.0;
  Gy_dps = GyY / 131.0;
  Gz_dps = GyZ / 131.0;

  Serial.print("Accelerometer (g): ");
  Serial.print("X = "); Serial.print(Ax_g,3);
  Serial.print(" | Y = "); Serial.print(Ay_g,3);
  Serial.print(" | Z = "); Serial.println(Az_g,3);

  Serial.print("Gyroscope (deg/s): ");
  Serial.print("X = "); Serial.print(Gx_dps,2);
  Serial.print(" | Y = "); Serial.print(Gy_dps,2);
  Serial.print(" | Z = "); Serial.println(Gz_dps,2);

  Serial.println();

  delay(250);
}