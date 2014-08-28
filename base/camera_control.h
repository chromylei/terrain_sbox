#pragma once

#include "azer/render/render.h"

void UpdatedownCamera(azer::Camera* camera, azer::Radians speed, float delta_time);
void MovableControl(azer::MovableObject* obj, azer::Radians speed, float delta_time);
