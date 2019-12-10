#include <stdio.h>
#include <arpa/inet.h>

int main(void)
{
    short tmp = 0x1234;

    tmp = htons(tmp);

    if ((*(char *)&tmp) == 0x34) {
        printf("It's little\n");
    } else if ((*(char *)&tmp) == 0x12) {
        printf("It's big\n");
    }

    return 0;
}
