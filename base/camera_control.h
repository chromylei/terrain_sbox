#pragma once

#include <vector>
#include "azer/render/render.h"

void UpdatedownCamera(azer::Camera* camera, azer::Radians speed,
                      azer::Radians rspeed, float delta_time);
void UpdatedownCamera(azer::Camera* camera, azer::Radians speed, float delta_time);
void MovableControl(azer::MovableObject* obj, azer::Radians speed, float delta_time);
void RotateControl(azer::MovableObject* obj, azer::Radians speed, float delta_time);
void RendererControl(azer::Renderer* renderer, double time);

class FreeCamera : public azer::Camera {
 public:
  FreeCamera() {}

  class Observer {
   public:
    Observer(FreeCamera* camera) {
      observered_ = camera;
      camera->AddObserver(this);
    }
    
    ~Observer() {observered_->RemoveObserver(this);}
    virtual void Action() = 0;
   private:
    FreeCamera* observered_;
  };

  void AddObserver(Observer* ob);
  void RemoveObserver(Observer* ob);
 private:
  friend class Observer;
  virtual void GenMatrices();
  std::vector<Observer*> observer_;
  DISALLOW_COPY_AND_ASSIGN(FreeCamera);
};

class MirrorCamera : public azer::Camera, FreeCamera::Observer {
 public:
  MirrorCamera(FreeCamera* camera, const azer::Plane& plane);

  virtual void Action();
 private:
  virtual void GenMatrices();
  FreeCamera* camera_;
  azer::Matrix4 mirror_;
  DISALLOW_COPY_AND_ASSIGN(MirrorCamera);
};
