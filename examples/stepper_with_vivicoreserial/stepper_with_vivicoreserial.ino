// This code name is steppingmotorcontrolsample
// Generated by VIVIWARE developer v1.3.0 on https://dev.developer.viviware.com/hard/4158
// Generated at 2022/8/4 19:02:28
//
// Copyright (C) 2022 Takashi Ando
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
// USA.

#include "Stepper.h"
#include "DataCode.h"

////////////////////////////////////////////////////////////////////////////////
// USER CODE WHICH SHOULD BE IMPLEMENTED
////////////////////////////////////////////////////////////////////////////////

// User defined version number
const uint16_t USER_FW_VER = 0x0001;

const int maxSteps = 2048;
Stepper *stepper = nullptr;

void setup() {
  initializeLibrary(USER_FW_VER);

  stepper = new Stepper(maxSteps, 4, 7, 5, 8);
  stepper->setSpeedPps(0);
  stepper->step(0);

  DBG_SERIAL_INSTANCE.println("Start");
}

void loop() {
  // Recommended delay is up to 30 in single loop. May encounter data loss from
  // App if delay is too long.
  // Stepper::move() needs 2ms to wait at max speed 512[pps].

  // 最速の512[pps]でおよそ2ms間隔でStepper内のstateを切り替える必要がある.
  // Coreとの通信は 18Bytes * 8bit / 50Kbps = 約3ms を要するため,
  // 最速回転中に通信すると回転速度が一定でない場合がある.
  if (!timeout(2)) {
    return;
  }

  // Receiver operation
  const receiveStruct received_values = receiveFromApp();
  bool started = static_cast<bool>(received_values.val_dc1);
  long speed = static_cast<long>(received_values.val_dc2);
  int steps = static_cast<int>(received_values.val_dc3);
  bool moving = false;

  static bool cur_started = false;
  static long cur_speed = 0;
  static int cur_steps = 0;
  static bool cur_moving = false;

  if (cur_speed != speed) {
    cur_speed = speed;
    DBG_SERIAL_INSTANCE.print("speed [pps]: "); DBG_SERIAL_INSTANCE.println(cur_speed);
  }

  if (cur_steps != steps) {
    cur_steps = steps;
    DBG_SERIAL_INSTANCE.print("steps: "); DBG_SERIAL_INSTANCE.println(cur_steps);
  }

  if (cur_started != started) {
    cur_started = started;
    if (cur_started) {
      stepper->setSpeedPps(cur_speed);
      stepper->step(cur_steps);
    }
    DBG_SERIAL_INSTANCE.print("started: "); DBG_SERIAL_INSTANCE.println(cur_started);
  }

  if (cur_started) {
    const int left_steps = stepper->move();
    moving = left_steps > 0;
    //DBG_SERIAL_INSTANCE.print("left steps: "); DBG_SERIAL_INSTANCE.println(left_steps);
  } else {
    moving = false;
    //DBG_SERIAL_INSTANCE.println("stopped");
  }

  // Sender operation
  if (cur_moving != moving) {
    cur_moving = moving;
    const sendStruct sendValue = {cur_moving};
    sendToApp(sendValue);
    DBG_SERIAL_INSTANCE.print("moving: "); DBG_SERIAL_INSTANCE.println(cur_moving);
  }
}
