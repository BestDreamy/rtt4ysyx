#include <am.h>
#include <klib.h>
#include <rtthread.h>

Context *cp_to = NULL;
Context *cp_from = NULL;

static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
    case EVENT_YIELD:
      cp_from = c;
      c = cp_to;
      break;
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch_to(rt_ubase_t to) {
  cp_to = *((Context **)to);
  yield();
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  cp_to = *((Context **)to);
  yield();
  from = (rt_ubase_t)&cp_from;
}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

typedef struct {
  void *entry;
  void *parameter;
  void *exit;
} rt_arg_t;

static void rt_f(void *arg)
{
  rt_arg_t *rt_arg = arg;
  void *rt_entry = rt_arg->entry;
  void *rt_parameter = rt_arg->parameter;
  void *rt_exit = rt_arg->exit;

  void (*entry)(void *parameter) = rt_entry;
  entry(rt_parameter);

  void (*exit)() = rt_exit;
  exit();

  while(1);
}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  uintptr_t kstack = (uintptr_t)stack_addr & ~(sizeof(uintptr_t) - 1);

  rt_arg_t *rt_arg = (rt_arg_t*)(kstack - sizeof(Context) - sizeof(rt_arg_t));
  rt_arg->entry = tentry;
  rt_arg->parameter = parameter;
  rt_arg->exit = texit;

  Context *cp = kcontext((Area){.end = (void*)kstack}, rt_f, (void *)rt_arg);

  return (rt_uint8_t*)cp;
}
