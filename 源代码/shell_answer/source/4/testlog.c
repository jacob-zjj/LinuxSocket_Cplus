#include <stdio.h>
#include <unistd.h>
#include <syslog.h>

int main(void)
{
    openlog("xwptest", LOG_PID, LOG_USER);
    syslog(LOG_INFO|LOG_LOCAL2, "xwp info log OK");
    syslog(LOG_NOTICE|LOG_LOCAL2, "xwp notice log OK");
    syslog(LOG_DEBUG|LOG_LOCAL2, "xwp debug log OK");
    closelog();

    return 0;
}
