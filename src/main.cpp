/*
  Copyright (C) 2021 Alessandro Orlando

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

//Library
#include <Arduino.h>
#include <AccelStepper.h>
#include <nRF24L01.h>
#include <RF24.h>

//Radio Chip Select & Chip Enable pins
const unsigned int radioCS = 2;
const unsigned int radioCE = 3;

//Motor X dir & step pins
const unsigned int dirX = 6;
const unsigned int stepX = 7;

//Motor Y dir & step pins
const unsigned int dirY = 8;
const unsigned int stepY = 9;

//Motor X dir & step pins
const unsigned int dirZ = 4;
const unsigned int stepZ = 5;

//Driver enable pin
const unsigned int pinEnable = 10;

// Radio address — do NOT use all-zero address (causes broadcast collisions)
// Must match transmitter exactly.
const byte address[6] = "00001";

// Packet structure — packed and using 16-bit speeds to match transmitter
struct __attribute__((packed)) Packet {
  bool    moveX;
  int16_t speedX;
  bool    moveY;
  int16_t speedY;
  bool    moveZ;
  int16_t speedZ;
  bool    enable;
};

//AccelStepper fixed variables
const unsigned int maxSpeed = 1000;
const unsigned int minSpeed = 0;
const float acceleration = 20.0;

//Step motors variables
AccelStepper motorX(AccelStepper::DRIVER, stepX, dirX);
AccelStepper motorY(AccelStepper::DRIVER, stepY, dirY);
AccelStepper motorZ(AccelStepper::DRIVER, stepZ, dirZ);

//Radio pins connected to CE and CSN of the module
RF24 radio(radioCE, radioCS);

// Initialize packet to safe stopped state
Packet pkt = { false, 0, false, 0, false, 0, false };

void setup() {

  //Driver enable pin
  pinMode(pinEnable, OUTPUT);
  
  //Set motors variables
  motorX.setMaxSpeed(maxSpeed);
  motorX.setSpeed(minSpeed);
  motorX.setAcceleration(acceleration);
  
  motorY.setMaxSpeed(maxSpeed);
  motorY.setSpeed(minSpeed);
  motorY.setAcceleration(acceleration);

  motorZ.setMaxSpeed(maxSpeed);
  motorZ.setSpeed(minSpeed);
  motorZ.setAcceleration(acceleration);

  //Initially the drivers are disabled
  digitalWrite(pinEnable, !pkt.enable);

  //Radio initialize
  radio.begin();
 
  //Radio output power, in my case at LOW
  radio.setPALevel(RF24_PA_LOW);

  //Radio receiver channel on the specified address
  radio.openReadingPipe(1,address);

  //Radio on listen
  radio.startListening();
}
  
//Values contained in the Packet structure

void drivemotors(Packet pkt) {

  //Enable or disable the step motors
  digitalWrite(pinEnable, !pkt.enable);
  
  //Move the motors
  if (pkt.moveX) {
    motorX.setSpeed(pkt.speedX);
    motorX.run();
  } else {
    motorX.stop();
  }

  if (pkt.moveY) {
    motorY.setSpeed(pkt.speedY);
    motorY.run();
  } else {
    motorY.stop();
  }

  if (pkt.moveZ) {
    motorZ.setSpeed(pkt.speedZ);
    motorZ.run();
  } else {
    motorZ.stop();
  }

}
void loop() {

  //If radio is receiving
  if (radio.available()) {

  //Radio buffer data read & write on Packet
  radio.read(&pkt, sizeof(pkt));
  } else {

  //If there are no data, Packet is resetted
    pkt = {
      false,
      0,
      false,
      0,
      false,
      0,
      false
    };

  }

  //Decode the received values and operate the motors accordingly
  drivemotors(pkt);
  delay(18);
  
}