#ifndef FRAME_H
#define FRAME_H

struct Frame
{
  int lowerBound;
  int upperBound;
  int pageNumber;
  int pid;

  Frame();
  Frame(int x, int y);
};

Frame::Frame()
{
}

Frame::Frame(int x, int y)
{
  lowerBound = x;
  upperBound = y;
}

#endif
