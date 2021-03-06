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

   The Engine is the main and central part of the fuzzer. It contains the
   queues, feedbacks, executor and the fuzz_one (which in turn has stages)

 */

#ifndef LIBENGINE_H
#define LIBENGINE_H

#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "queue.h"
#include "feedback.h"
#include "afl-returns.h"
#include "xxh3.h"
#include "xxhash.h"
#include "afl-rand.h"
#include "llmp.h"

#define MAX_FEEDBACKS 10

struct engine_functions {

  global_queue_t *(*get_queue)(engine_t *);
  fuzz_one_t *(*get_fuzz_one)(engine_t *);
  u64 (*get_execs)(engine_t *);
  u64 (*get_start_time)(engine_t *);

  void (*set_fuzz_one)(engine_t *, fuzz_one_t *);
  int (*add_feedback)(engine_t *, feedback_t *);
  void (*set_global_queue)(engine_t *, global_queue_t *);

  u8 (*execute)(engine_t *, raw_input_t *);
  void (*handle_new_message)(engine_t *, llmp_message_t *);
  afl_ret_t (*load_testcases_from_dir)(
      engine_t *, char *, raw_input_t *(*custom_input_init)(u8 *buf));
  void (*load_zero_testcase)(size_t);

  afl_ret_t (*loop)(engine_t *);

};

struct engine {

  fuzz_one_t *      fuzz_one;
  global_queue_t *  global_queue;
  executor_t *      executor;
  feedback_queue_t *current_feedback_queue;
  feedback_t
      *feedbacks[MAX_FEEDBACKS];  // We're keeping a pointer of feedbacks here
                                  // to save memory, consideting the original
                                  // feedback would already be allocated
  u64   executions, start_time, crashes, feedbacks_num;
  u32   id;
  char *in_dir;  // Input corpus directory

  afl_rand_t rnd;

  u8 *                    buf;  // Reusable buf for realloc
  struct engine_functions funcs;
  llmp_client_state_t *   llmp_client;  // Our IPC for fuzzer communication

};

/* TODO: Add default implementations for load_testcases and execute */
global_queue_t *afl_get_queue_default(engine_t *);
fuzz_one_t *    afl_get_fuzz_one_default(engine_t *);
u64             afl_get_execs_defualt(engine_t *);
u64             afl_get_start_time_default(engine_t *);

void afl_set_fuzz_one_default(engine_t *, fuzz_one_t *);
int  afl_add_feedback_default(engine_t *, feedback_t *);
void afl_set_global_queue_default(engine_t *      engine,
                                  global_queue_t *global_queue);

u8        afl_execute_default(engine_t *, raw_input_t *);
afl_ret_t afl_load_testcases_from_dir_default(
    engine_t *, char *, raw_input_t *(*custom_input_init)());
void afl_load_zero_testcase_default(size_t);
void afl_handle_new_message_default(engine_t *, llmp_message_t *);

afl_ret_t afl_loop_default(engine_t *);  // Not sure about this functions
                                         // use-case. Was in FFF though.

afl_ret_t afl_engine_init(engine_t *, executor_t *, fuzz_one_t *,
                          global_queue_t *);
void      afl_engine_deinit(engine_t *);

static inline engine_t *afl_engine_create(executor_t *    executor,
                                          fuzz_one_t *    fuzz_one,
                                          global_queue_t *global_queue) {

  engine_t *engine = calloc(1, sizeof(engine_t));
  if (!engine) return NULL;
  if (afl_engine_init(engine, executor, fuzz_one, global_queue) !=
      AFL_RET_SUCCESS) {

    free(engine);
    return NULL;

  }

  return engine;

}

static inline void afl_engine_delete(engine_t *engine) {

  afl_engine_deinit(engine);
  free(engine);

}

#endif

