#include <stdio.h>
#include <stdint.h>
#include <nvboard.h>
#include "Vtop.h"
#define _CONCAT(x, y) x ## y
#define CONCAT(x, y)  _CONCAT(x, y)
#define BITMASK(bits) ((1ull << (bits)) - 1)
// similar to x[hi:lo] in verilog
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1))
#define DEF_WIRE(name, w) uint64_t name : w
#define DEF_REG(name, w)  uint64_t name : w; \
                          uint64_t CONCAT(name, _next) : w; \
                          uint64_t CONCAT(name, _update) : 1
#define EVAL(c, name, val) do { \
                             c->CONCAT(name, _next) = (val); \
                             c->CONCAT(name, _update) = 1; \
                           } while (0)
#define UPDATE(c, name)    do { \
                             if (c->CONCAT(name, _update)) { \
                               c->name = c->CONCAT(name, _next); \
                             } \
                           } while (0)
static TOP_NAME top;
void nvboard_bind_all_pins(TOP_NAME *top);
void single_cycle() {
  top.clk = 0; top.eval();
  top.clk = 1; top.eval();
}

void reset(int n) {
  top.rst = 1;
  while (n -- > 0) single_cycle();
  top.rst = 0;
}
int main() {
  reset(10);
  nvboard_bind_all_pins(&top);
  nvboard_init();
  while (1) {
    single_cycle();
    nvboard_update();
  }
  return 0;
}
