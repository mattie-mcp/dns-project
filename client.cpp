#include <ares.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/nameser.h>
#include <sys/time.h>

struct DNS_HEADER{
    unsigned short id :16;          // identification number

    unsigned char aa :1;        // authoritative answer
    unsigned char tc :1;        // truncated msg
    unsigned char rd :1;        // recursion desired
    unsigned char ra :1;        // 
    unsigned char z :3;        // query/response flag

    unsigned char qr :1;        // query/response flag
    unsigned char opcode :4;    // purpose of msg

    unsigned char rcode :4;     // r code

    unsigned short qd_count :16;     // number of question entries
    unsigned short an_count :16;   // number of answer entries
    unsigned short ns_count :16;  // number of authority entries
    unsigned short ar_count :16;   // number of resource records
};

int truncated_count;
int qtyreceived_count

static void state_cb(void *data, int s, int read, int write)
{
    //printf("Change state fd %d read:%d write:%d\n", s, read, write);
}


static void callback(void *arg, int status, int timeouts, struct hostent *host)
{

    if(!host || status != ARES_SUCCESS){
        printf("Failed to lookup %s\n", ares_strerror(status));
        return;
    }

    printf("Found address name %s\n", host->h_name);
    char ip[INET6_ADDRSTRLEN];
    int i = 0;

    for (i = 0; host->h_addr_list[i]; ++i) {
        inet_ntop(host->h_addrtype, host->h_addr_list[i], ip, sizeof(ip));
        printf("%s\n", ip);
    }
}

void query_callback(void* arg, int status, int timeouts, unsigned char *abuf, int alen){
	if (status == ARES_SUCCESS){
        struct DNS_HEADER *dns_hdr = (struct DNS_HEADER*) abuf;
        qtyreceived_count++;
		// printf("success, packet is %i bytes\n", alen);
        // printf("id num: 0x%X\n", dns_hdr->id);
		// printf("truncated response : %d\n", dns_hdr->tc);
        if (dns_hdr->tc == 1){
            //printf("truncated reponse\n");
            //printf("id num: %d\n", dns_hdr->id);
            truncated_count++;
        }
        
	}
	else
		printf("lookup failed: %d\n", status);
}

static void wait_ares(ares_channel channel)
{
    int timeout = 500;
    for(;;){
        struct timeval *tvp, tv, *max_t;
        fd_set read_fds, write_fds;
        int nfds;

        max_t->tv_usec = (suseconds_t) timeout;
       
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        nfds = ares_fds(channel, &read_fds, &write_fds);
        if(nfds == 0){
            break;
        }
        tvp = ares_timeout(channel, max_t, &tv);
        select(nfds, &read_fds, &write_fds, NULL, tvp);
        ares_process(channel, &read_fds, &write_fds);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2){
		printf("Usage: client packets_to_send\n");
		exit(1);
	}

    ares_channel channel;
    struct ares_options options;
    int optmask = 0;
	int status, i;
    int packetsToSend = atoi(argv[1]);

    status = ares_library_init(ARES_LIB_INIT_ALL);
    if (status != ARES_SUCCESS){
        printf("ares_library_init: %s\n", ares_strerror(status));
        return 1;
    }
    options.sock_state_cb = state_cb;
    optmask |= ARES_OPT_SOCK_STATE_CB;

    status = ares_init_options(&channel, &options, optmask);
    if(status != ARES_SUCCESS) {
        printf("ares_init_options: %s\n", ares_strerror(status));
        return 1;
    }
    
    printf("ares initialized\n");
	unsigned char **qbuf = malloc(sizeof(unsigned char **));
	int *buflen = malloc(sizeof( int*));

	for ( i=0; i<packetsToSend; i++ ){
        if ( i % 500 == 0)
            printf("message count: %d\n", i);
        // printf("creating query...\n"); // ns_c_in = 1 (internet); ns_t_a = 1 (host addr)
	    ares_create_query("example.local", ns_c_in, ns_t_a, i, 0, qbuf, buflen, 0);
		ares_send(channel, *qbuf, *buflen, query_callback, NULL);
		wait_ares(channel);
	}
    
    printf("sent %d packets\n", packetsToSend);	

    wait_ares(channel);

    printf("received %d packets     | %d were truncated\n", qtyreceived_count, truncated_count)

    ares_destroy(channel);
    ares_library_cleanup();
    printf("end\n");
    return 0;
}
