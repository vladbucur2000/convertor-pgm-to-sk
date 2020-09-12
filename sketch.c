// Basic program skeleton for a Sketch File (.sk) Viewer
#include "displayfull.h"
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Allocate memory for a drawing state and initialise it
state *newState() {
  //TO DO
  state *x = (state *) malloc(sizeof(state));
  
  x -> start = 0, x -> end = false, x -> tool = LINE, x -> data = 0;
  x -> x = x -> y = x -> tx = x -> ty = 0;

  return x; // this is a placeholder only
}

// Release all memory associated with the drawing state
void freeState(state *s) {
  //TO DO
  free(s);
}

// Extract an opcode from a byte (two most significant bits).
int getOpcode(byte b) {
  //TO DO
  return b >> 6;
}

// Extract an operand (-32..31) from the rightmost 6 bits of a byte.
int getOperand(byte b) {
  //TO DO
  int sign;
  sign = (b >> 5) % 2;

  b = b & (255 - (1 << 5));
  b = b & (255 - (1 << 6));
  b = b & (255 - (1 << 7));

  if (sign) return (b - (1 << 5));
  
  return b;
}

void opTool(display *d, state *s, int opperand) {
  if (opperand == NONE || opperand == LINE || opperand == BLOCK) s -> tool = opperand;
  if (opperand == COLOUR) colour(d, s -> data);
  if (opperand == TARGETX) s -> tx = s -> data;
  if (opperand == TARGETY) s -> ty = s -> data;
  if (opperand == SHOW) show(d);
  if (opperand == PAUSE) pause(d, s -> data);
  if (opperand == NEXTFRAME) s -> end = true;
  s -> data = 0;
}

void opDY(display *d, state *s, int opperand) {
  s -> ty += opperand;
  if (s -> tool == LINE) line(d, s -> x, s -> y, s -> tx, s -> ty);
  if (s -> tool == BLOCK) block(d, s -> x, s -> y, s -> tx - s -> x, s -> ty - s -> y);
  s -> x = s -> tx, s -> y = s -> ty; 
}

void opDATA(display *d, state *s, int opperand) {
  opperand = opperand & 0x3F;
  s -> data <<= 6;
  s -> data += opperand;
}

// Execute the next byte of the command sequence.
void obey(display *d, state *s, byte op) {
  //TO DO
  int opcode = getOpcode(op), opperand = getOperand(op);
  
  if (opcode == TOOL) opTool(d, s, opperand);

  if (opcode == DX) s -> tx += opperand;

  if (opcode == DY) opDY(d, s, opperand);

  if (opcode == DATA) opDATA(d, s, opperand);
}

// Draw a frame of the sketch file. For basic and intermediate sketch files
// this means drawing the full sketch whenever this function is called.
// For advanced sketch files this means drawing the current frame whenever
// this function is called.
bool processSketch(display *d, void *data, const char pressedKey) {

    //TO DO: OPEN, PROCESS/DRAW A SKETCH FILE BYTE BY BYTE, THEN CLOSE IT
    //NOTE: CHECK DATA HAS BEEN INITIALISED... if (data == NULL) return (pressedKey == 27);
    //NOTE: TO GET ACCESS TO THE DRAWING STATE USE... state *s = (state*) data;
    //NOTE: TO GET THE FILENAME... char *filename = getName(d);
    //NOTE: DO NOT FORGET TO CALL show(d); AND TO RESET THE DRAWING STATE APART FROM
    //      THE 'START' FIELD AFTER CLOSING THE FILE


  if (data == NULL) return (pressedKey == 27);
  
  state *s = (state *) data;
    
  char *filename = getName(d);
  FILE *in = fopen(filename, "rb");
    
  char b = fgetc(in);
  int i = 0;

  while (! feof(in) && !s -> end) {
    if (i > s -> start || s -> start == 0) obey(d, s, b);
    if (s -> end) s -> start = i;
    ++i;
    b = getc(in);
  }

  if (feof(in)) s -> start = 0;
  
  fclose(in);
  show(d);
  
  s -> tool = LINE, s -> tx = 0, s -> ty = 0, s -> x = 0, s -> y = 0;
  s -> end = false;

  
  return (pressedKey == 27);
}

// View a sketch file in a 200x200 pixel window given the filename
void view(char *filename) {
  display *d = newDisplay(filename, 200, 200);
  state *s = newState();
  run(d, s, processSketch);
  freeState(s);
  freeDisplay(d);
}

// Include a main function only if we are not testing (make sketch),
// otherwise use the main function of the test.c file (make test).
#ifndef TESTING
int main(int n, char *args[n]) {
  if (n != 2) { // return usage hint if not exactly one argument
    printf("Use ./sketch file\n");
    exit(1);
  } else view(args[1]); // otherwise view sketch file in argument
  return 0;
}
#endif
