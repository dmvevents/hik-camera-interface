#pragma once
#include "pipe_image.h"
#include <string>

class PipeImageVideo : public PipeImage {
public:
  PipeImageVideo():_video_path(""){}
  ~PipeImageVideo(){}

  void set_video_path(const std::string &video_path) {
    _video_path = video_path;
  }

protected:
  virtual void update();

private:
  std::string _video_path;
};
