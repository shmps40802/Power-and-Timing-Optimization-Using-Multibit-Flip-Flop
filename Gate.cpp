#include "Gate.h"
Gate::Gate() : Cell() {
}
Gate::Gate(int width, int height, int pincount, vector<Point> pin)
    : Cell(width, height, pincount) {
    setPin(pin);
}