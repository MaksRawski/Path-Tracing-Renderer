#ifndef WINDOW_COORDINATE_H_
#define WINDOW_COORDINATE_H_

typedef struct {
  double x, y;
} WindowCoordinate;

inline static WindowCoordinate WindowCoordinate_sub(WindowCoordinate a,
                                                    WindowCoordinate b) {
  return (WindowCoordinate){a.x - b.x, a.y - b.y};
}

#endif // WINDOW_COORDINATE_H_
