#ifndef UNIT_TEST_H_
#define UNIT_TEST_H_

#ifdef __GNUC__
	#define __unused__ __attribute__ ((unused))
#else
	#define __unused__
#endif

#define UNIT_TEST_START(name) \
	__unused__ static void unit_test_function_##name() { \
		printf("* " #name ": "); \

#define UNIT_TEST_END() \
		printf("PASS\n"); \
	}

#define UNIT_TEST_SKIP() \
		printf("SKIPPED\n"); \
		return;

#define UNIT_TEST_RUN(name) \
	unit_test_function_##name();

#define UNIT_TEST_ASSERT(expr) \
		if(!(expr)) { \
			printf("FAIL (%s:%d)\n", __FILE__, __LINE__); \
			return; \
		}

#endif /* UNIT_TEST_H_ */
