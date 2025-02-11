#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int DEFAULT_LENGTH = 100; /*stack length is 100*/

/*stack structure*/
typedef struct stack {
    int* data;
    int length;
    int count;
} stack;

/*stack creation function*/
stack* create() {
    stack* new_stack = (stack*)(malloc(sizeof(stack))); /*create stack object*/
    new_stack->data = (int*)(malloc(sizeof(int) * DEFAULT_LENGTH)); /*create data array*/
    new_stack->length = DEFAULT_LENGTH;
    new_stack->count = 0;
    return new_stack; /*return stack object*/
}

/*stack destroy function*/
void destroy(stack * s) {
    free(s->data);
    free(s);
}

int stack_push(stack* s, int num) {
    if(s->count == s->length) {
        return 0; /*stack is full return failure*/
    }
    /*shift data down*/
    s->count++; /*increment*/
    for (int i = (s->count - 1); i != 0; i--) {
        s->data[i] = s->data[i - 1];
    }
    s->data[0] = num; /*set head value*/

    return 1; /*success*/
}

int stack_pop(stack* s) {
    if(s->count == 0) {
        return -1; /*stack is empty*/
    }
    int temp = s->data[0]; /*store head of stack*/
    for(int i=0; i<(s->length -1); i++) {
        s->data[i] = s->data[i + 1]; /*shift data up*/
    }
    s->count--; /*decrement*/
    return temp; /*return value*/
}

/*stack printing*/
void stack_print(stack* s) {
    printf("HEAD->\n");
    for(int i=0; i<s->length; i++) {
        printf("%d | ", s->data[i]);
        if(((i+1) % 10) == 0) { /*new line every 10 digits*/
            printf("\n");
        }
    }
    printf("<-TAIL\n");
}

int main() {
    stack* Test = create();
    /*push some numbers to the stack*/
    for(int i=1; i<10; i++) {
        stack_push(Test, i);
        printf("Pushing to stack: %d\n", i);
    }
    /*pop all values and print them*/
    int head;
    while((head = stack_pop(Test)) != -1) {
        printf("Pop: %d\n", head);
    }
    /*fill stack to max*/
    printf("FILLING STACK...");
    for(int i=1; i<DEFAULT_LENGTH+1; i++) {
        stack_push(Test, i);
    }
    /*print stack*/
    stack_print(Test);
    /*clean*/
    destroy(Test);
    return 0;
}