#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <lwip/sockets.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>

#define zero(S) memset(&S, 0, sizeof(S))

static xTaskHandle CLIENT_TASK_HANDLE;
const char *TARGET_IPV6 = "fe80::2de1:9dc5:388a:b33f";
const char *SOURCE_IPV6 = "fe80::b60e:cfff:fe11:8e75";


static void client_task(void *arg)
{

    vTaskDelay(2000);
    (void)arg; // unused

    int result;
    struct ip6_addr target_ipv6_addr;

    // should intialize `target_ipv6_addr`
    result = ip6addr_aton(TARGET_IPV6, &target_ipv6_addr);
    if (result == 0) {
        printf("Invalid ipv6 address\n");
        goto failure;
    }

    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("socket creation failed\n");
        goto failure;
    }

    printf("socket created\n");

    {
        struct sockaddr_in6 sock_addr;
        struct ip6_addr target_ipv6_addr;
        zero(sock_addr);
        sock_addr.sin6_family = AF_INET6;

        result = ip6addr_aton(SOURCE_IPV6, &target_ipv6_addr);

        if (result == 0) {
            printf("Invalid ipv6 address\n");
            goto failure;
        }

        struct in6_addr *ref_s6_addr = &sock_addr.sin6_addr;
        struct ip6_addr *ref_target_ipv6_addr = &target_ipv6_addr;
        inet6_addr_from_ip6addr(ref_s6_addr, ref_target_ipv6_addr);

        sock_addr.sin6_port = htons(23333);
        result = bind(socket_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
        printf("bind result is %d\r\n", result);
    }
    // prepare an `struct sockaddr_in6` connection to port 18080
    struct sockaddr_in6 sock_addr;
    zero(sock_addr);
    sock_addr.sin6_family = AF_INET6;

    struct in6_addr *ref_s6_addr = &sock_addr.sin6_addr;
    struct ip6_addr *ref_target_ipv6_addr = &target_ipv6_addr;
    inet6_addr_from_ip6addr(ref_s6_addr, ref_target_ipv6_addr);

    sock_addr.sin6_port = htons(18080);


    int retry = 0;

    result = connect(socket_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
    while (result != 0) {
        if (result != 0) {
            printf("connected failed with %d\n", result);
            vTaskDelay(100);

            if (retry > 5) {
                goto failure;
            }
        }
        retry += 1;
    }
    printf("connected \n");

    char msg[] = "abc\n";

    result = send(socket_fd, msg, strlen(msg), 0);
    if (result == -1) {
        printf("Send failed\n");
    }


failure:
    vTaskDelete(NULL);
    return;
}

void user_task()
{
    int ret;
    const char *const task_name = "client task";

    ret = xTaskCreate(client_task, task_name, 1024, NULL, 6, &CLIENT_TASK_HANDLE);
    if (ret != pdPASS) {
        printf("unable to start task client task");
        return;
    }
}

