/*
   american fuzzy lop++ - fuzzer header
   ------------------------------------

   Originally written by Michal Zalewski

   Now maintained by Marc Heuse <mh@mh-sec.de>,
                     Heiko Eißfeldt <heiko.eissfeldt@hexco.de>,
                     Andrea Fioraldi <andreafioraldi@gmail.com>,
                     Dominik Maier <mail@dmnk.co>

   Copyright 2016, 2017 Google Inc. All rights reserved.
   Copyright 2019-2020 AFLplusplus Project. All rights reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at:

     http://www.apache.org/licenses/LICENSE-2.0

   This is the Library based on AFL++ which can be used to build
   customized fuzzers for a specific target while taking advantage of
   a lot of features that AFL++ already provides.

 */

#include "lib-common.h"

#define DEFAULT_INPUT_LEN 100

typedef struct raw_input {

  u8 *   bytes;  // Raw input bytes
  size_t len;  // Length of the input field. C++ had strings, we have to make do
               // with storing the lengths :/

  struct raw_input_operations *operations;

} raw_input_t;

typedef struct raw_input_operations {

  void (*deserialize)(raw_input_t *, u8 *, size_t);
  u8 *(*serialize)(raw_input_t *);
  raw_input_t *(*copy)(raw_input_t *);
  raw_input_t *(*empty)(raw_input_t *);
  void (*restore)(raw_input_t *, raw_input_t *);
  u8 (*load_from_file)(raw_input_t *, u8 *);
  u8 (*save_to_file)(raw_input_t *, u8 *);
  void (*clear)(raw_input_t *);
  u8 *(*get_bytes)(raw_input_t *);

} raw_input_operations_t;

raw_input_t *afl_input_init();
void         afl_input_deinit(raw_input_t *);

// Default implementations of the functions for raw input vtable
void         _raw_inp_deserialize_(raw_input_t *, u8 *, size_t);
u8 *         _raw_inp_serialize_(raw_input_t *);
raw_input_t *_raw_inp_copy_(raw_input_t *);
raw_input_t *_raw_inp_empty_(raw_input_t *);
void         _raw_inp_restore_(raw_input_t *, raw_input_t *);
u8           _raw_inp_load_from_file_(raw_input_t *, u8 *);
u8           _raw_inp_save_to_file_(raw_input_t *, u8 *);
void         _raw_inp_clear_(raw_input_t *);
u8 *         _raw_inp_get_bytes_(raw_input_t *);

// input_clear and empty functions... difference??
// serializing and deserializing would be done on the basis of some structure
// right??
