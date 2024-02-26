#include "color.hpp"

bool i3neostatus::color::rgb_model::operator==(const rgb &color_1,
                                               const rgb &color_2) {
  return ((color_1.r == color_2.r) && (color_1.g == color_2.g) &&
          (color_1.b == color_2.b));
}

bool i3neostatus::color::rgb_model::operator==(const rgba &color_1,
                                               const rgba &color_2) {
  return ((color_1.a == color_2.a) &&
          (static_cast<rgb>(color_1) == static_cast<rgb>(color_2)));
}
