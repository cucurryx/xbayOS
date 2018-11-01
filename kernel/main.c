#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"

int main() {
    put_str("I am kernel\n");
    init_all();
    put_str("init_all done\n");
    // ASSERT(1==2);
    // uint32_t a;
    // memset((void*)&a, 0x1, 4);
    // put_int(a);
    // put_char('\n');

    // uint16_t b;
    // memcpy((void*)&b, (void*)&a, 2);
    // put_int(b);
    // put_char('\n');

    // put_int(memcmp((void*)&a, (void*)&b, 2));

    char *s1 = "hello";
    char *s2 = "     ";
    char *s3 = "world";

    put_int(strlen(s1));
    put_char('\n');

    strcpy(s2, s1);
    put_int(strcmp(s1, s2));
    put_char('\n');

    put_int(strncmp(s1, s2, 2));
    put_char('\n');

    put_int(strcmp(s1, s3));
    put_char('\n');

    put_str(strchr(s1, 'h'));
    put_char('\n');

    put_str(s1);
    put_char('\n');
    put_str(s2);
    put_char('\n');
    put_str(s3);
    put_char('\n');

    put_int(strcmp(strcat(s1, s2), "helloworld"));
    put_char('\n');

    put_int(strcnt(s1, 'o'));

    while (1);
}