#include "tersbox/base/camera_control.h"

void UpdatedownCamera(azer::Camera* camera, azer::Radians speed,
                      float delta_time) {
  MovableControl(camera, speed, delta_time);
  camera->Update();
}

void MovableControl(azer::MovableObject* obj, azer::Radians speed,
                    float delta_time) {
  if( ::GetAsyncKeyState('A') & 0x8000f ) {
    obj->strafe(-speed.value()* delta_time);
  }
  if( ::GetAsyncKeyState('D') & 0x8000f ) {
    obj->strafe(speed.value()* delta_time);
  }
  if( ::GetAsyncKeyState('Q') & 0x8000f ) {
    obj->yaw(azer::Radians(speed.value()* delta_time));
  }
  if( ::GetAsyncKeyState('E') & 0x8000f ) {
    obj->yaw(azer::Radians(-speed.value()* delta_time));
  }
  if( ::GetAsyncKeyState('R') & 0x8000f ) {
    obj->pitch(azer::Radians(speed.value()* delta_time));
  }
  if( ::GetAsyncKeyState('T') & 0x8000f ) {
    obj->pitch(azer::Radians(-speed.value()* delta_time));
  }
  if( ::GetAsyncKeyState('F') & 0x8000f ) {
    obj->roll(azer::Radians(speed.value()* delta_time));
  }
  if( ::GetAsyncKeyState('G') & 0x8000f ) {
    obj->roll(azer::Radians(-speed.value()* delta_time));
  }
  
  if( ::GetAsyncKeyState('W') & 0x8000f ) {
    obj->walk(+speed.value()* delta_time);
  }
  if( ::GetAsyncKeyState('S') & 0x8000f ) {
    obj->walk(-speed.value()* delta_time);
  }
  if( ::GetAsyncKeyState('H') & 0x8000f ) {
    obj->fly(+speed.value()* delta_time);
  }
  if( ::GetAsyncKeyState('J') & 0x8000f ) {
    obj->fly(-speed.value()* delta_time);
  }
}

