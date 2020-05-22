/**
* iperf-liked network performance tool
*
*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <lwip/sockets.h>
#include <aos/kernel.h>

#include <cli.h>
#include <netutils/netutils.h>

#define IPERF_PORT_LOCAL    5002
#define IPERF_PORT          5001
#define IPERF_BUFSZ         (4 * 1300)
#define IPERF_BUFSZ_UDP     (1 * 1300)
#define DEBUG_HEADER        "[NET] [IPC] "
#define DEFAULT_HOST_IP     "192.168.11.1"

typedef struct UDP_datagram { 
    uint32_t id;
    uint32_t tv_sec;
    uint32_t tv_usec;
} UDP_datagram;

static void iperf_client_tcp(void *arg)
{
    int i;
    int sock;
    int ret;

    uint8_t *send_buf;
    int sentlen;
    uint32_t tick0, tick1, tick2;
    struct sockaddr_in addr;
    char *host = (char*) arg;
    uint64_t bytes_transfered = 0;

    char speed[32] = { 0 };
    float f_min = 8000.0, f_max = 0.0;

    send_buf = (uint8_t *) pvPortMalloc (IPERF_BUFSZ);
    if (!send_buf) {
        vPortFree(arg);
        return;
    }

    for (i = 0; i < IPERF_BUFSZ; i ++) {
        send_buf[i] = i & 0xff;
    }

    while (1) {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0) 
        {
            printf("create socket failed!\r\n");
            vTaskDelay(1000);
            continue;
        }

        addr.sin_family = PF_INET;
        addr.sin_port = htons(IPERF_PORT);
        addr.sin_addr.s_addr = inet_addr(host);

        ret = connect(sock, (const struct sockaddr*)&addr, sizeof(addr));
        if (ret == -1) 
        {
            printf("Connect failed!\r\n");
            closesocket(sock);

            vTaskDelay(1000);
            continue;
        }

        printf("Connect to iperf server successful!\r\n");

        {
            int flag = 1;

            setsockopt(sock,
                IPPROTO_TCP,     /* set option at TCP level */
                TCP_NODELAY,     /* name of option */
                (void *) &flag,  /* the cast is historical cruft */
                sizeof(int));    /* length of option value */
        }

        sentlen = 0;

        tick0 = xTaskGetTickCount();
        tick1 = tick0;
        while(1) {
            tick2 = xTaskGetTickCount();
            if (tick2 - tick1 >= 1000 * 5)
            {
                float f_now, f_avg;

                f_now = (float)(sentlen)  / 125 / (((int32_t)tick2 - (int32_t)tick1)) * 1000;
                f_now /= 1000.0f;
                bytes_transfered += sentlen;
                f_avg = (float)(bytes_transfered)  / 125 / (((int32_t)tick2 - (int32_t)tick0)) * 1000;
                f_avg /= 1000.0f;

                if (f_now < f_min) {
                    f_min = f_now;
                }
                if (f_max < f_now) {
                    f_max = f_now;
                }
                snprintf(speed, sizeof(speed), "%.4f(%.4f %.4f %.4f) Mbps!\r\n",
                        f_now,
                        f_min,
                        f_avg,
                        f_max
                );
                printf("%s", speed);
                tick1 = tick2;
                sentlen = 0;
            }

            ret = send(sock, send_buf, IPERF_BUFSZ, 0);
            if (ret > 0) 
            {
                sentlen += ret;
            }

            if (ret < 0) break;
        }

        closesocket(sock);

        vTaskDelay(1000*2);
        printf("disconnected!\r\n");
    }
    printf("iper stop\r\n");
    vPortFree(send_buf);
}

static void iperf_client_tcp_entry(const char *name)
{
    int host_len;
    char *host;

    host_len = strlen(name) + 4;
    host = pvPortMalloc(host_len);//mem will be free in tcpc_entry
    strcpy(host, name);
    aos_task_new("ipc", iperf_client_tcp, host, 4096);
}

static void iperf_client_udp(void *arg)
{
    int i;
    int sock;
    int ret;

    uint8_t *send_buf;
    int sentlen;
    uint64_t bytes_transfered = 0;
    uint32_t tick0, tick1, tick2;
    struct sockaddr_in laddr, raddr;
    UDP_datagram udp_header, *udp_header_buf;
    char *host = (char*) arg;

    char speed[64] = { 0 };
    float f_min = 8000.0, f_max = 0.0;

    send_buf = (uint8_t *) pvPortMalloc (IPERF_BUFSZ_UDP);
    if (!send_buf) {
        vPortFree(arg);
        return;
    }

    for (i = 0; i < IPERF_BUFSZ_UDP; i ++) {
        send_buf[i] = i & 0xff;
    }

        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock < 0) 
        {
            printf("create socket failed!\r\n");
            vTaskDelay(1000);
            vPortFree(arg);
            return;
        }

        memset(&laddr, 0, sizeof(struct sockaddr_in));
        laddr.sin_family = PF_INET;
        laddr.sin_port = htons(IPERF_PORT_LOCAL);
        laddr.sin_addr.s_addr = inet_addr("0.0.0.0");

        ret = bind(sock, (struct sockaddr*)&laddr, sizeof(laddr));
        if (ret == -1) 
        {
            printf("Bind failed!\r\n");
            lwip_close(sock);

            vTaskDelay(1000);
            vPortFree(arg);
            return;
        }

        printf("bind UDP socket successfully!\r\n");

        memset(&raddr, 0, sizeof(struct sockaddr_in));
        raddr.sin_family = PF_INET;
        raddr.sin_port = htons(IPERF_PORT);
        raddr.sin_addr.s_addr = inet_addr(host);

        sentlen = 0;

        udp_header_buf = (UDP_datagram*)send_buf;
        udp_header.id = 0;
        udp_header.tv_sec = 0;
        udp_header.tv_usec = 0;
        tick0 = xTaskGetTickCount();
        tick1 = tick0;
        while (1) {
            tick2 = xTaskGetTickCount();
            if (tick2 - tick1 >= 1000 * 5)
            {
                float f_now, f_avg;

                f_now = (float)(sentlen)  / 125 / (((int32_t)tick2 - (int32_t)tick1)) * 1000;
                f_now /= 1000.0f;
                bytes_transfered += sentlen;
                f_avg = (float)(bytes_transfered)  / 125 / (((int32_t)tick2 - (int32_t)tick0)) * 1000;
                f_avg /= 1000.0f;

                if (f_now < f_min) {
                    f_min = f_now;
                }
                if (f_max < f_now) {
                    f_max = f_now;
                }
                snprintf(speed, sizeof(speed), "%.4f(%.4f %.4f %.4f) Mbps!\r\n",
                        f_now,
                        f_min,
                        f_avg,
                        f_max
                );
                printf("%s", speed);
                tick1 = tick2;
                sentlen = 0;
            }

            udp_header.id++;
            udp_header_buf->id = htonl(udp_header.id);
            udp_header_buf->tv_sec = 0;
            udp_header_buf->tv_usec = 0;
retry:
            ret = sendto(sock, send_buf, IPERF_BUFSZ_UDP, 0, (const struct sockaddr*)&raddr, sizeof(raddr));
            if (ret > 0) {
                sentlen += ret;
            }

            if (ret < 0) {
                if (ERR_MEM == ret) {
                    goto retry;
                }
                break;
            }
        }

        lwip_close(sock);

        vTaskDelay(1000*2);
        printf("disconnected! ret %d\r\n",  ret);
        vTaskDelete(NULL);
}

static void iperf_client_udp_entry(const char *name)
{
    int host_len;
    char *host;

    host_len = strlen(name) + 4;
    host = pvPortMalloc(host_len);//mem will be free in tcpc_entry
    strcpy(host, name);
    aos_task_new("ipu", iperf_client_udp, host, 4096);
}

static void iperf_server(void *arg)
{
    uint8_t *recv_data;
    uint32_t sin_size;
    uint32_t tick0, tick1, tick2;
    int sock = -1, connected, bytes_received, recvlen;
    struct sockaddr_in server_addr, client_addr;
    char speed[32] = { 0 };
    char *host = (char*)arg;
    uint64_t bytes_transfered = 0;
    float f_min = 8000.0, f_max = 0.0;

    recv_data = (uint8_t *)pvPortMalloc(IPERF_BUFSZ);
    if (recv_data == NULL)
    {
        printf("No memory\r\n");
        goto __exit;
    }

    (void) host;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket error\r\n");
        goto __exit;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(IPERF_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), 0x0, sizeof(server_addr.sin_zero));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        printf("Unable to bind\r\n");
        goto __exit;
    }

    if (listen(sock, 5) == -1) {
        printf("Listen error\r\n");
        goto __exit;
    }

    while (1) {
        sin_size = sizeof(struct sockaddr_in);

        connected = accept(sock, (struct sockaddr *)&client_addr, (socklen_t *)&sin_size);

        printf("new client connected from (%s, %d)\r\n",
                  inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        {
            int flag = 1;

            setsockopt(connected,
                IPPROTO_TCP,     /* set option at TCP level */
                TCP_NODELAY,     /* name of option */
                (void *) &flag,  /* the cast is historical cruft */
                sizeof(int));    /* length of option value */
        }

        recvlen = 0;
        tick0 = xTaskGetTickCount();
        tick1 = tick0;
        while (1) {
            bytes_received = recv(connected, recv_data, IPERF_BUFSZ, 0);
            if (bytes_received <= 0) break;

            recvlen += bytes_received;

            tick2 = xTaskGetTickCount();
            if (tick2 - tick1 >= 1000 * 5)
            {
                float f_now, f_avg;

                f_now = (float)(recvlen)  / 125 / (((int32_t)tick2 - (int32_t)tick1)) * 1000;
                f_now /= 1000.0f;
                bytes_transfered += recvlen;
                f_avg = (float)(bytes_transfered)  / 125 / (((int32_t)tick2 - (int32_t)tick0)) * 1000;
                f_avg /= 1000.0f;

                if (f_now < f_min) {
                    f_min = f_now;
                }
                if (f_max < f_now) {
                    f_max = f_now;
                }
                snprintf(speed, sizeof(speed), "%.4f(%.4f %.4f %.4f) Mbps!\r\n",
                        f_now,
                        f_min,
                        f_avg,
                        f_max
                );
                printf("%s", speed);
                tick1 = tick2;
                recvlen = 0;
            }
        }

        if (connected >= 0) closesocket(connected);
        connected = -1;
    }

__exit:
    if (sock >= 0) closesocket(sock);
    if (recv_data) vPortFree(recv_data);
    if (arg) vPortFree(arg);
}

static void iperf_server_entry(const char *name)
{
    int host_len;
    char *host;

    host_len = strlen(name) + 4;
    host = pvPortMalloc(host_len);//mem will be free in tcpc_entry
    strcpy(host, name);
    aos_task_new("ips", iperf_server, host, 4096);
}

static void ipc_test_cmd(char *buf, int len, int argc, char **argv)
{
    if (1 == argc) {
        printf(DEBUG_HEADER "[IPC] Connecting with default address " DEFAULT_HOST_IP "\r\n");
        iperf_client_tcp_entry(DEFAULT_HOST_IP);
    } else if (2 == argc) {
        iperf_client_tcp_entry(argv[1]);
    } else {
        printf(DEBUG_HEADER  "[IPC] illegal address\r\n");
    }
}

static void ips_test_cmd(char *buf, int len, int argc, char **argv)
{
    if (1 == argc) {
        puts(DEBUG_HEADER "[IPS] Starting iperf server on 0.0.0.0\r\n");
        iperf_server_entry(DEFAULT_HOST_IP);
    } else if (2 == argc) {
        iperf_server_entry(argv[1]);
    } else {
        printf(DEBUG_HEADER  "[IPS] illegal address\r\n");
    }
}

static void ipu_test_cmd(char *buf, int len, int argc, char **argv)
{
    if (1 == argc) {
        printf(DEBUG_HEADER "[IPU] Connecting with default address " DEFAULT_HOST_IP "\r\n");
        iperf_client_udp_entry(DEFAULT_HOST_IP);
    } else if (2 == argc) {
        iperf_client_udp_entry(argv[1]);
    } else {
        printf(DEBUG_HEADER  "[IPU] illegal address\r\n");
    }
}

// STATIC_CLI_CMD_ATTRIBUTE makes this(these) command(s) static
const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
    { "ipc", "iperf TCP client", ipc_test_cmd},
    { "ips", "iperf TCP server", ips_test_cmd},
    { "ipu", "iperf UDP client", ipu_test_cmd},
};                                                                                   

int network_netutils_iperf_cli_register()
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));          
    return 0;
}
