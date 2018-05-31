#include <cutter.h>
#include <stack.h>

void
test_new_stack (void)
{
    Stack *stack;
    stack = stack_new();
    if (stack_is_empty(stack))
        PASS;
    else
        FAIL;
}
