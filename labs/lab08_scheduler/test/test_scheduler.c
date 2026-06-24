/**
 * @file test_scheduler.c
 * @brief Lab 8 测试套件。无需修改本文件；实现 scheduler.c 使其全部通过即可。
 */
#include "test_framework.h"
#include "scheduler.h"

static void task_inc(void *arg) {
    (*(int *)arg)++;
}

static int test_init_null(void) {
    sch_task_t slots[4];
    scheduler_t s;
    int dummy = 0;
    ASSERT_EQ_INT(SCH_ERR_NULL, sch_init(NULL, slots, 4));
    ASSERT_EQ_INT(SCH_ERR_NULL, sch_init(&s, NULL, 4));
    ASSERT_EQ_INT(SCH_ERR_NULL, sch_init(&s, slots, 0));
    ASSERT_EQ_INT(0, (int)sch_now(NULL));
    ASSERT_EQ_SIZE(0u, sch_active_count(NULL));
    ASSERT_EQ_SIZE(0u, sch_capacity(NULL));
    ASSERT_EQ_INT(-1, sch_add(NULL, task_inc, &dummy, 1, 1));
    ASSERT_EQ_INT(SCH_ERR_NULL, sch_remove(NULL, 0));
    ASSERT_EQ_SIZE(0u, sch_tick(NULL));
    ASSERT_EQ_SIZE(0u, sch_advance(NULL, 5));
    ASSERT_FALSE(sch_is_active(NULL, 0));
    sch_reset(NULL);

    sch_init(&s, slots, 4);
    ASSERT_EQ_INT(-1, sch_add(&s, NULL, &dummy, 1, 1)); /* fn 为 NULL */
    ASSERT_EQ_SIZE(4u, sch_capacity(&s));
    ASSERT_FALSE(sch_is_active(&s, -1));                /* id 越界 */
    ASSERT_FALSE(sch_is_active(&s, 100));
    return 0;
}

static int test_periodic_single(void) {
    sch_task_t slots[4];
    scheduler_t s;
    sch_init(&s, slots, 4);

    int a = 0;
    int id = sch_add(&s, task_inc, &a, 2, 2);  /* 每 2 tick 一次，首发在 tick 2 */
    ASSERT_TRUE(id >= 0);
    ASSERT_EQ_SIZE(1u, sch_active_count(&s));

    size_t total_fired = 0;
    for (int i = 0; i < 10; i++) total_fired += sch_tick(&s);

    ASSERT_EQ_INT(5, a);                 /* tick 2,4,6,8,10 各一次 */
    ASSERT_EQ_SIZE(5u, total_fired);
    ASSERT_EQ_INT(10, (int)sch_now(&s));
    return 0;
}

static int test_one_shot(void) {
    sch_task_t slots[4];
    scheduler_t s;
    sch_init(&s, slots, 4);

    int a = 0;
    int id = sch_add(&s, task_inc, &a, 0, 3); /* period==0：一次性，tick 3 触发 */
    ASSERT_TRUE(id >= 0);
    ASSERT_EQ_SIZE(1u, sch_active_count(&s));

    for (int i = 0; i < 3; i++) sch_tick(&s);
    ASSERT_EQ_INT(1, a);
    ASSERT_EQ_SIZE(0u, sch_active_count(&s)); /* 触发后自动失效 */

    for (int i = 0; i < 5; i++) sch_tick(&s);
    ASSERT_EQ_INT(1, a);                       /* 不再触发 */
    return 0;
}

static int test_two_tasks(void) {
    sch_task_t slots[4];
    scheduler_t s;
    sch_init(&s, slots, 4);

    int a = 0, b = 0;
    sch_add(&s, task_inc, &a, 2, 2);  /* tick 2,4,6,8,10,12 */
    sch_add(&s, task_inc, &b, 3, 3);  /* tick 3,6,9,12       */
    ASSERT_EQ_SIZE(2u, sch_active_count(&s));

    size_t total = 0;
    for (int i = 0; i < 12; i++) total += sch_tick(&s);

    ASSERT_EQ_INT(6, a);
    ASSERT_EQ_INT(4, b);
    ASSERT_EQ_SIZE(10u, total);  /* 6 + 4 */
    return 0;
}

/* ---- sch_advance：批量推进等价于多次 tick ---- */

static int test_advance(void) {
    sch_task_t slots[4];
    scheduler_t s;
    sch_init(&s, slots, 4);

    int a = 0;
    sch_add(&s, task_inc, &a, 2, 2);   /* tick 2,4,6,8,10 */
    size_t fired = sch_advance(&s, 10);
    ASSERT_EQ_INT(5, a);
    ASSERT_EQ_SIZE(5u, fired);
    ASSERT_EQ_INT(10, (int)sch_now(&s));
    /* advance 0 不应触发任何任务、不推进时钟 */
    ASSERT_EQ_SIZE(0u, sch_advance(&s, 0));
    ASSERT_EQ_INT(10, (int)sch_now(&s));
    return 0;
}

/* ---- sch_is_active：跟随 add / remove / 一次性失效 ---- */

static int test_is_active(void) {
    sch_task_t slots[4];
    scheduler_t s;
    sch_init(&s, slots, 4);

    int a = 0;
    int id = sch_add(&s, task_inc, &a, 0, 2);   /* 一次性，tick 2 */
    ASSERT_TRUE(sch_is_active(&s, id));
    sch_advance(&s, 2);
    ASSERT_EQ_INT(1, a);
    ASSERT_FALSE(sch_is_active(&s, id));         /* 触发后失效 */
    return 0;
}

/* ---- sch_reset：清空任务并归零时钟 ---- */

static int test_reset(void) {
    sch_task_t slots[4];
    scheduler_t s;
    sch_init(&s, slots, 4);

    int a = 0;
    sch_add(&s, task_inc, &a, 1, 1);
    sch_advance(&s, 5);
    ASSERT_EQ_INT(5, a);
    ASSERT_TRUE(sch_now(&s) == 5);

    sch_reset(&s);
    ASSERT_EQ_SIZE(0u, sch_active_count(&s));
    ASSERT_EQ_INT(0, (int)sch_now(&s));          /* 时钟归零 */

    /* reset 后还能正常用 */
    int b = 0;
    int id = sch_add(&s, task_inc, &b, 1, 1);
    ASSERT_TRUE(id >= 0);
    sch_advance(&s, 3);
    ASSERT_EQ_INT(3, b);
    return 0;
}

static int test_remove(void) {
    sch_task_t slots[4];
    scheduler_t s;
    sch_init(&s, slots, 4);

    int a = 0;
    int id = sch_add(&s, task_inc, &a, 2, 2);
    sch_tick(&s);
    sch_tick(&s);            /* tick 2：触发一次 */
    ASSERT_EQ_INT(1, a);

    ASSERT_EQ_INT(SCH_OK, sch_remove(&s, id));
    ASSERT_EQ_SIZE(0u, sch_active_count(&s));
    for (int i = 0; i < 6; i++) sch_tick(&s);
    ASSERT_EQ_INT(1, a);     /* 移除后不再触发 */

    ASSERT_EQ_INT(SCH_ERR_ARG, sch_remove(&s, id)); /* 重复移除 */
    return 0;
}

static int test_full_and_reuse(void) {
    sch_task_t slots[2];
    scheduler_t s;
    sch_init(&s, slots, 2);

    int a = 0;
    int id0 = sch_add(&s, task_inc, &a, 1, 1);
    int id1 = sch_add(&s, task_inc, &a, 1, 1);
    ASSERT_TRUE(id0 >= 0 && id1 >= 0);
    ASSERT_EQ_INT(-1, sch_add(&s, task_inc, &a, 1, 1)); /* 槽满 */

    ASSERT_EQ_INT(SCH_OK, sch_remove(&s, id0));
    int id2 = sch_add(&s, task_inc, &a, 1, 1);          /* 复用空槽 */
    ASSERT_TRUE(id2 >= 0);
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_null);
    RUN_TEST(test_periodic_single);
    RUN_TEST(test_one_shot);
    RUN_TEST(test_two_tasks);
    RUN_TEST(test_advance);
    RUN_TEST(test_is_active);
    RUN_TEST(test_reset);
    RUN_TEST(test_remove);
    RUN_TEST(test_full_and_reuse);
    TEST_END();
}
