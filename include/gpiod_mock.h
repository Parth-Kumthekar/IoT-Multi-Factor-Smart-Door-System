#pragma once

struct gpiod_chip {};
struct gpiod_line {};

inline gpiod_chip* gpiod_chip_open_by_name(const char*) { return new gpiod_chip(); }
inline gpiod_line* gpiod_chip_get_line(gpiod_chip*, unsigned int) { return new gpiod_line(); }
inline int  gpiod_line_request_output(gpiod_line*, const char*, int) { return 0; }
inline int  gpiod_line_set_value(gpiod_line*, int) { return 0; }
inline void gpiod_line_release(gpiod_line*) {}
inline void gpiod_chip_close(gpiod_chip*) {}