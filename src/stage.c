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

 */

#include "stage.h"
#include "engine.h"
#include "fuzzone.h"
#include "mutator.h"

afl_ret_t afl_stage_init(stage_t *stage, engine_t *engine) {

  stage->engine = engine;

  // We also add this stage to the engine's fuzzone
  if (engine) { engine->fuzz_one->funcs.add_stage(engine->fuzz_one, stage); }

  stage->funcs.iterations = afl_iterations_stage_default;

  return AFL_RET_SUCCESS;

}

void afl_stage_deinit(stage_t *stage) {

  stage->engine = NULL;

}

afl_ret_t afl_fuzzing_stage_init(fuzzing_stage_t *fuzz_stage,
                                 engine_t *       engine) {

  if (afl_stage_init(&(fuzz_stage->base), engine) != AFL_RET_SUCCESS) {

    return AFL_RET_ERROR_INITIALIZE;

  }

  fuzz_stage->funcs.add_mutator_to_stage = afl_add_mutator_to_stage_default;
  fuzz_stage->base.funcs.perform = afl_perform_stage_default;

  return AFL_RET_SUCCESS;

}

void afl_fuzz_stage_deinit(fuzzing_stage_t *fuzz_stage) {

  size_t i;
  /* We deinitialize the mutators associated with the stage here */

  afl_stage_deinit(&(fuzz_stage->base));

  for (i = 0; i < fuzz_stage->mutators_count; ++i) {

    afl_mutator_deinit(fuzz_stage->mutators[i]);

  }

}

afl_ret_t afl_add_mutator_to_stage_default(fuzzing_stage_t *stage,
                                           mutator_t *      mutator) {

  if (!stage || !mutator) { return AFL_RET_NULL_PTR; }

  if (stage->mutators_count >= MAX_STAGE_MUTATORS) { return AFL_RET_ARRAY_END; }

  stage->mutators[stage->mutators_count] = mutator;
  stage->mutators_count++;

  mutator->stage = (stage_t *)stage;

  return AFL_RET_SUCCESS;

}

size_t afl_iterations_stage_default(stage_t *stage) {

  return (1 + afl_rand_below(&stage->engine->rnd, 128));

}

/* Perform default for fuzzing stage */
afl_ret_t afl_perform_stage_default(stage_t *stage, raw_input_t *input) {

  size_t i;
  // This is to stop from compiler complaining about the incompatible pointer
  // type for the function ptrs. We need a better solution for this to pass the
  // scheduled_mutator rather than the mutator as an argument.
  fuzzing_stage_t *fuzz_stage = (fuzzing_stage_t *)stage;

  size_t num = fuzz_stage->base.funcs.iterations(stage);

  for (i = 0; i < num; ++i) {

    raw_input_t *copy = input->funcs.copy(input);
    if (!copy) { return AFL_RET_ERROR_INPUT_COPY; }

    size_t j;
    for (j = 0; j < fuzz_stage->mutators_count; ++j) {

      mutator_t *mutator = fuzz_stage->mutators[j];
      if (mutator->funcs.custom_queue_get) {

        mutator->funcs.custom_queue_get(mutator, copy);
        continue;

      }  // If the mutator decides not to fuzz this input, don't fuzz it. This

      // is to support the custom mutator API of AFL++

      if (mutator->funcs.trim) {

        size_t orig_len = copy->len;
        size_t trim_len = mutator->funcs.trim(mutator, copy);

        if (trim_len > orig_len) { return AFL_RET_TRIM_FAIL; }

      }

      mutator->funcs.mutate(mutator, copy);

    }

    /* Let's post process the mutated data now. */
    for (j = 0; j < fuzz_stage->mutators_count; ++j) {

      mutator_t *mutator = fuzz_stage->mutators[j];

      if (mutator->funcs.post_process) {

        mutator->funcs.post_process(mutator, copy);

      }

    }

    afl_ret_t ret = stage->engine->funcs.execute(stage->engine, copy);
    /* Let's collect some feedback on the input now */

    bool add_to_queue = false;

    for (i = 0; i < stage->engine->feedbacks_num; ++i) {

      add_to_queue = add_to_queue ||
                     stage->engine->feedbacks[i]->funcs.is_interesting(
                         stage->engine->feedbacks[i], stage->engine->executor);

    }

    /* If the input is interesting and there is a global queue add the input to
     * the queue */
    if (add_to_queue && stage->engine->global_queue) {

      raw_input_t *input_copy = copy->funcs.copy(copy);

      if (!input_copy) { return AFL_RET_ERROR_INPUT_COPY; }

      queue_entry_t *entry = afl_queue_entry_create(input_copy);

      if (!entry) { return AFL_RET_ALLOC; }

      global_queue_t *queue = stage->engine->global_queue;

      queue->base.funcs.add_to_queue((base_queue_t *)queue, entry);

    }

    afl_input_delete(copy);

    switch (ret) {

      case AFL_RET_SUCCESS:
        continue;
      // We'll add more cases here based on the type of exit_ret value given by
      // the executor.Those will be handled in the engine itself.
      default:
        return ret;

    }

  }

  return AFL_RET_SUCCESS;

}

