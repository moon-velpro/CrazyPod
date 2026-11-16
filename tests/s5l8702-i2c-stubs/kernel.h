#ifndef TEST_S5L8702_KERNEL_H
#define TEST_S5L8702_KERNEL_H

struct mutex { int depth; };

static inline void mutex_init(struct mutex *m) { m->depth = 0; }
static inline void mutex_lock(struct mutex *m) { ++m->depth; }
static inline void mutex_unlock(struct mutex *m) { --m->depth; }

#endif
