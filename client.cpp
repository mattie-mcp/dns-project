#include <time.h>
#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <ares.h>
//for definitions of dnsclass and type see vvvv
#include <arpa/nameser.h>.

void dns_callback (void* arg, int status, int timeouts, unsigned char *abuf, int alen)
{
    if(status == ARES_SUCCESS)
        std::cout << host->h_name << "\n";
    else
        std::cout << "lookup failed: " << status << '\n';
	printf("response: %s\n", abuf);
}
void main_loop(ares_channel &channel)
{
    int nfds, count;
    fd_set readers, writers;
    timeval tv, *tvp;
    while (1) {
        FD_ZERO(&readers);
        FD_ZERO(&writers);
        nfds = ares_fds(channel, &readers, &writers);
        tvp->tv_sec = 1000;
        if (nfds == 0)
          break;
        tvp = ares_timeout(channel, NULL, &tv);
        count = select(nfds, &readers, &writers, NULL, tvp);
        ares_process(channel, &readers, &writers);
     }

}
int main(int argc, char **argv)
{
    struct in_addr ip;
    int res, i;	
	
    if(argc < 2 ) {
        std::cout << "usage: " << argv[0] << " ip.address\n";
        return 1;
    }
    inet_aton(argv[1], &ip);
    ares_channel channel;
	
    if((res = ares_init(&channel)) != ARES_SUCCESS) {
        std::cout << "ares failed: " << res << '\n';
        return 1;
    }
		
	char *qbuf;
	int buflen;
	//create query, saved in char pointer
	qbuf = ares_create_query("example.local", 0, 0, 0, qbuf, buflen, 0);
	
    for ( i=0; i<1; i++ ){
        printf("sending msg %d\n", i);
        //ares_gethostbyaddr(channel, &ip, sizeof ip, AF_INET, dns_callback, NULL);
		ares_send(channel, qbuf, buflen, dns_callback);
        main_loop(channel);
    }
	
	//clean up
	wait_ares(channel);
	ares_destroy(channel);
	ares_free_string(qbuf);
			 
    return 0;
  }
