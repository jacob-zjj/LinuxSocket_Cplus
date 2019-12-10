#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void display_usage(void)
{
    printf("please usage\n");
    printf("./a.out -f -t time -n num -ddate\n");
}

int main(int argc, char *argv[])
{
    char *optstring = "ft:n:d::?";
    int opt;
    int flag = 0;
    int num = 0;
    int time = 0;
    int date= 0;

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
            case 'f':flag = 1; break;
            case 'n':num = atoi(optarg);break;
            case 't':time = atoi(optarg);break;
            case 'd':date = atoi(optarg);break;
            case '?':display_usage();exit(0);
            default:display_usage();exit(0);
        }
    }

    printf("flag = %d\tnum=%d\ttime=%d\tdate=%d\n", flag, num, time, date);

    return 0;
}
