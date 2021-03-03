#ifndef TESTBED_H_
#define TESTBED_H_
void assert_execution_on_testbed();
void handle_container_stop_signals(int sig);
int is_executing_on_testbed();
#endif