#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<stdbool.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[512];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    // you don't need to change this.
    int id;
    int wait_for_write;  // used by handle_read to know if the header is read or not.
    int fd;//store it's opened fd 
} request;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* accept_read_header = "ACCEPT_FROM_READ";
const char* accept_write_header = "ACCEPT_FROM_WRITE";
const char* read_path =  "./registerRecord";
const char* fail_msg = "[Error] Operation failed. Please try again.\n";
const char* lock_msg = "Locked.\n";
const char* ask_client_to_write = "Please input your preference order respectively(AZ,BNT,Moderna):\n";
const char* modify_msg = "data has been modify\n";

bool online[20];


static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

typedef struct {
    int id;          //902001-902020
    int AZ;          
    int BNT;         
    int Moderna;     
}registerRecord;

int handle_read(request* reqP) {
    int r;
    char buf[512];

    // Read in request from client
    r = read(reqP->conn_fd, buf, sizeof(buf));
    if (r < 0) return -1;
    if (r == 0) return 0;
    char* p1 = strstr(buf, "\015\012");
    int newline_len = 2;
    if (p1 == NULL) {
       p1 = strstr(buf, "\012");
        if (p1 == NULL) {
            ERR_EXIT("this really should not happen...");
        }
    }
    size_t len = p1 - buf + 1;
    memmove(reqP->buf, buf, len);
    reqP->buf[len - 1] = '\0';
    reqP->buf_len = len-1;
    return 1;
}

void print_order(char *buf, registerRecord* reg){
    switch (reg->AZ)
    {
    case 1:
        if ((reg->BNT) < (reg-> Moderna)){
            sprintf(buf, "Your preference order is AZ > BNT > Moderna.\n");
        }
        else{
            sprintf(buf, "Your preference order is AZ > Moderna > BNT.\n");
        }
        break;
    case 2:
        if (reg->BNT < reg-> Moderna){
            sprintf(buf, "Your preference order is BNT > AZ > Moderna.\n");
        }
        else{
            sprintf(buf, "Your preference order is Moderna > AZ > BNT.\n");
        }
        break;
    case 3:
        if (reg->BNT < reg-> Moderna){
            sprintf(buf, "Your preference order is Moderna > BNT > AZ.\n");
        }
        else{
            sprintf(buf, "Your preference order is BNT > Moderna > AZ.\n");
        }
        break;
    
    default:
        break;
    }
}

void print_order_with_id(char *buf, registerRecord* reg, int id ){
    switch (reg->AZ)
    {
    case 1:
        if ((reg->BNT) < (reg-> Moderna)){
            fprintf(stderr,"1 2 3\n");
            sprintf(buf, "Preference order for %d modified successed, new preference order is AZ > BNT > Moderna.\n",id);
        }
        else{
            sprintf(buf, "Preference order for %d modified successed, new preference order is AZ > Moderna > BNT.\n",id);
        }
        break;
    case 2:
        if (reg->BNT < reg-> Moderna){
            fprintf(stderr, "2 1 3\n");
            sprintf(buf, "Preference order for %d modified successed, new preference order is BNT > AZ > Moderna.\n",id);
        }
        else{
            sprintf(buf, "Preference order for %d modified successed, new preference order is Moderna > AZ > BNT.\n",id);
        }
        break;
    case 3:
        if (reg->BNT < reg-> Moderna){
            fprintf(stderr,"3 1 2\n");
            sprintf(buf, "Preference order for %d modified successed, new preference order is Moderna > BNT > AZ.\n",id);
        }
        else{
            sprintf(buf, "Preference order for %d modified successed, new preference order is BNT > Moderna > AZ.\n",id);
        }
        break;
    
    default:
        break;
    }
}

void handle_client_input_r(request* reqP, int id){//open fd -> lock -> read -> close
    //id is 902001~ 
    
    // check id is vaild or not 
    if (id <902001 || id > 902020 || id == 0){
        fprintf(stderr, "user input invaild id\n");
        write(reqP->conn_fd,fail_msg,strlen(fail_msg));
    }
    else{
        // open file as fd
        int fd = open(read_path, O_RDONLY);
        if (fd < 0 ){
            fprintf(stderr, "open file error at reqP %d\n", reqP->conn_fd);
            write(reqP->conn_fd,fail_msg,strlen(fail_msg));
            perror("can't open fd");
            exit(EXIT_FAILURE);
        }
        else{
            //lock
            int num_of_item = id - 902001;
            struct flock lock_read;
            lock_read.l_whence = SEEK_SET;
            lock_read.l_start = num_of_item * sizeof(registerRecord);
            lock_read.l_len = sizeof(registerRecord);
            lock_read.l_pid = getpid();
            lock_read.l_type = F_RDLCK;

            registerRecord reg;
            
            if(fcntl( fd , F_SETLK, &lock_read) < 0){
                //lock fail 
                fprintf(stderr, "lock file fail at fd %d\n", reqP->conn_fd);
                fprintf(stderr, "%s\n", strerror(errno));
                write(reqP->conn_fd,lock_msg,sizeof(lock_msg));//locked.
                close(fd);
            }
            else{
                fprintf(stderr, "lock succes for fd = %d\n", reqP->conn_fd);
                lseek( fd , num_of_item * sizeof(reg),SEEK_SET);
                if (read( fd , &reg , sizeof(reg)) < 0 ){
                    //read fail
                    fprintf(stderr, "read file error\n");
                    write(reqP->conn_fd,fail_msg,sizeof(fail_msg));
                    close(fd);
                }
                else{
                    fprintf(stderr, "read succese for fd %d\n", reqP->conn_fd);
                    char buf[512];
                    print_order(buf,&reg);
                    //sprintf(buf, "bnt = %d , az = %d , moderna = %d\n", reg.BNT,reg.AZ,reg.Moderna);
                    write(reqP->conn_fd, buf , strlen(buf));
                }

            }
            
        }
        //unlock
        close(fd);

    }
}

void handle_client_input_w(request* reqP, int id){//open fd -> lock -> read -> close
    //id is 902001~ 
    int fd;
    // check id is vaild or not 
    if (id <902001 || id > 902020 || id == 0){
        fprintf(stderr, "user input invaild id\n");
        write(reqP->conn_fd,fail_msg,strlen(fail_msg));
    }
    else{
        // open file as fd

        int pos = id - 902001;// accesse file pos 
        if (online[pos] == false){

            fd = open(read_path, O_RDWR);
            if (fd < 0 ){
                fprintf(stderr, "open file error at reqP %d\n", reqP->conn_fd);
                write(reqP->conn_fd,fail_msg,strlen(fail_msg));
                perror("can't open fd");
                exit(EXIT_FAILURE);
            }
            else{
                //lock
                int num_of_item = id - 902001;
                struct flock lock_write;
                lock_write.l_whence = SEEK_SET;
                lock_write.l_start = num_of_item*sizeof(registerRecord);
                lock_write.l_len = sizeof(registerRecord);
                lock_write.l_pid = getpid();
                lock_write.l_type = F_WRLCK;

                registerRecord reg;
                
                if(fcntl( fd , F_SETLK, &lock_write) < 0){
                    //lock fail 
                    fprintf(stderr, "lock file fail at fd %d\n", reqP->conn_fd);
                    fprintf(stderr, "%s\n", strerror(errno));
                    write(reqP->conn_fd,lock_msg,sizeof(lock_msg));//locked.
                    close(fd);
                }
                else{

                    lseek( fd , num_of_item * sizeof(reg),SEEK_SET);
                    if (read( fd , &reg , sizeof(reg)) < 0 ){
                        //read fail
                        fprintf(stderr, "read file error\n");
                        write(reqP->conn_fd,fail_msg,sizeof(fail_msg));
                        close(fd);
                    }
                    else{

                        fprintf(stderr, "read succese for fd %d\n", reqP->conn_fd);
                        char buf[512];
                        print_order(buf,&reg);
                        //buf => az > bnt > ...
                        write(reqP->conn_fd, buf , strlen(buf));
                        fprintf(stderr,"asking client %d to write \n",reqP->conn_fd);
                        write(reqP->conn_fd,ask_client_to_write,strlen(ask_client_to_write));// ask to change preferences

                        
                        online[pos] = true;
                    }

                }
                
            }
        }
        else{
            fprintf(stderr, "reqp locked at reqP -> %d\n", reqP->conn_fd);  
            write(reqP->conn_fd, lock_msg , strlen(lock_msg));
            return;
        }
        
        
        //select init for write to file 
        fd_set write_set;
        FD_ZERO(&write_set);
        FD_SET(reqP->conn_fd, &write_set);

        struct timeval timeout2; 
        timeout2.tv_sec = 5;
        timeout2.tv_usec = 0; 

        int sc = select(reqP->conn_fd + 1, &write_set, NULL,NULL,&timeout2);
        if (sc < 0 ){
            fprintf(stderr, "select err\n");
        }

        if (FD_ISSET(reqP->conn_fd, &write_set)){
            printf("ok\n");
            int r = handle_read(reqP);
            
            if ( r < 0 ){
                fprintf(stderr,"read form client error in sec input\n");
                return;
            }
            else if (r > 0 ){
                fprintf(stderr,"resicved client sec input\n");
                write(reqP->conn_fd, modify_msg, strlen(modify_msg));


            }
            else{
                fprintf(stderr,"disconnected");
            }

            //unlock
            close(reqP->conn_fd);
            free_request(reqP);
            online[pos] = false;
            


            
        }



    }
}

int check_input_to_write(char *buf, registerRecord* reg){//handle sec input
    const char* case1 = "1 2 3";
    const char* case2 = "1 3 2";
    const char* case3 = "2 1 3";
    const char* case4 = "2 3 1";
    const char* case5 = "3 1 2";
    const char* case6 = "3 2 1";

    if(strcmp(buf, case1) == 0){
        fprintf(stderr,"cmp = 1\n");
        reg->AZ = 1 ; reg->BNT = 2; reg->Moderna = 3;
        return 1;
    }
    else if(strcmp(buf, case2) == 0){
        reg->AZ = 1 ; reg->BNT = 3; reg->Moderna = 2;
        return 1;
    }
    else if(strcmp(buf, case3) == 0){
        reg->AZ = 2 ; reg->BNT = 1; reg->Moderna = 3;
        return 1;
    }
    else if(strcmp(buf, case4) == 0){
        reg->AZ = 2 ; reg->BNT = 3; reg->Moderna = 1;
        return 1;
    }
    else if(strcmp(buf, case5) == 0){
        reg->AZ = 3 ; reg->BNT = 1; reg->Moderna = 2;
        return 1;
    }
    else if(strcmp(buf, case6) == 0){
        reg->AZ = 3 ; reg->BNT = 2; reg->Moderna = 1;
        return 1;
    }
    else{
        return -1;
    }
    
}

int main(int argc, char** argv) {

    // Parse args.
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(1);
    }

    struct sockaddr_in cliaddr;  // used by accept()
    int clilen;

    int conn_fd;  // fd for a new connection with client
    int file_fd;  // fd for file that we open for reading
    char buf[512];
    int buf_len;

    //my init
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    for (int i = 0 ; i < 20; i++ ){
        online[i] = false ;
    }

    //Enter massgae ------------
    const char* enter_msg = "Please enter your id (to check your preference order):\n";
    fd_set master_set, working_set;

    // Initialize server
    init_server((unsigned short) atoi(argv[1]));

    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);

    while (1) {
        // TODO: Add IO multiplexing
        
        // Check new connection
        clilen = sizeof(cliaddr);

        FD_ZERO(&master_set);
        FD_SET(svr.listen_fd, &master_set);

        for (int i = 0; i < maxfd; i++) {
			if (requestP[i].conn_fd != -1) {
				FD_SET(i, &master_set);
			}
		}
        memcpy(&working_set,&master_set ,sizeof(master_set));

        int sc = select(maxfd, &working_set, NULL, NULL, &timeout);
        
        if(FD_ISSET(svr.listen_fd, &working_set)){
            conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
            if (conn_fd < 0) {
                if (errno == EINTR || errno == EAGAIN) continue;  // try again
                if (errno == ENFILE) {
                    (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                    continue;
                }
                ERR_EXIT("accept");
            }
            requestP[conn_fd].conn_fd = conn_fd;
            strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
            write(requestP[conn_fd].conn_fd, enter_msg, strlen(enter_msg));
            fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);    
        }
       
        if (sc < 0){
            perror("select_err");
            exit(EXIT_FAILURE);
        }
        if (sc == 0 ){
            //printf("time out ...\n");
        }
        if (sc > 0 ){
            for(int i = 0  ; i < maxfd ; i++){
        
    // TODO: handle requests from clients

#ifdef READ_SERVER  
                if (requestP[i].conn_fd != svr.listen_fd && FD_ISSET(requestP[i].conn_fd, &working_set) ){
                    int ret = handle_read(&requestP[i]);

                    if (ret < 0 ){
                        fprintf(stderr, "bad request from %s\n", requestP[i].host);
            		    continue;
                    }
                    else if (ret > 0 ){
                        //test input
                        //todo : flock , open -> read -> write
                        fprintf(stderr, "client %d input : %s\n",i, requestP[i].buf);
                        //sprintf(buf,"%s : %s\n",accept_read_header,requestP[i].buf);
                        //write(requestP[conn_fd].conn_fd, buf, strlen(buf));
                        handle_client_input_r(&requestP[i],atoi(requestP[i].buf));
                    }
                    else{
                        fprintf(stderr, "Host disconnected, fd: %d from %s\n", i, requestP[i].host);
                    }
                    close(requestP[i].conn_fd);
                    free_request(&requestP[i]);
                }
    
#elif defined WRITE_SERVER           

                if (requestP[i].conn_fd != svr.listen_fd && FD_ISSET(requestP[i].conn_fd, &working_set) ){
                    if (requestP[i].fd == -1 ){
                        int ret = handle_read(&requestP[i]);

                        if (ret < 0 ){
                            fprintf(stderr, "bad request from %s\n", requestP[i].host);
                            continue;
                        }
                        else if (ret > 0 ){
                            //todo : flock , open -> read -> write
                            fprintf(stderr, "client %d input : %s\n",i, requestP[i].buf);
                            
                            int id = atoi(requestP[i].buf);
                            if (id == 0 || id > 902020 || id < 902001){
                                fprintf(stderr,"invaild id for client %d\n",requestP[i].conn_fd);
                                write(requestP[i].conn_fd, fail_msg, strlen(fail_msg));
                            }
                            else{
                                int pos = id - 902001;

                                if(online[pos] == false){
                                    int fd = open(read_path, O_RDWR);

                                    if (fd < 0 ){
                                        //read err
                                        fprintf(stderr,"read err\n");
                                        write(requestP[i].conn_fd,fail_msg,strlen(fail_msg));
                                        ERR_EXIT("fd<0 read err");
                                    }
                                    else{
                                        //fcntl init 
                                        struct flock w_lock;
                                        w_lock.l_whence = SEEK_SET;
                                        w_lock.l_start = pos * sizeof(registerRecord);
                                        w_lock.l_len = sizeof(registerRecord);
                                        w_lock.l_pid = getpid();
                                        w_lock.l_type = F_WRLCK;
                                        
                                        if(fcntl( fd , F_SETLK , &w_lock ) < 0 ){
                                            fprintf(stderr, "file id lcoked");
                                            write(requestP[i].conn_fd , lock_msg , strlen(lock_msg));
                                            close(fd);
                                        }
                                        else{
                                            fprintf(stderr,"lock success for fd  = %d ", requestP[i].conn_fd);
                                            registerRecord reg;
                                            lseek( fd , pos * sizeof(reg), SEEK_SET);

                                            if (read( fd , &reg , sizeof(reg)) < 0){
                                                //read fd err
                                                fprintf(stderr,"cant read file");
                                                write(requestP[i].conn_fd, fail_msg,strlen(fail_msg));
                                                close(fd);
                                            }
                                            else{
                                                fprintf(stderr, "read success c lient %d\n", requestP[i].conn_fd);
                                                char buf[512];
                                                print_order(buf,&reg);
                                                write(requestP[i].conn_fd, buf, strlen(buf));
                                                fprintf(stderr , "ask client %d to modify\n", requestP[i].conn_fd);
                                                write(requestP[i].conn_fd, ask_client_to_write, strlen(ask_client_to_write));

                                                online[pos] = true;
                                                requestP[i].id = pos;
                                                requestP[i].fd = fd;
                                            }
                                        
                                        }
                                        
                                    }
                                }
                                else{
                                    // file is locked -> print locked and close connection
                                    fprintf(stderr,"file is locked client is %d\n",requestP[i].conn_fd);
                                    write(requestP[i].conn_fd,lock_msg,strlen(lock_msg));
                                    //close connection
                                    close(requestP[i].conn_fd);
                                    free_request(&requestP[i]);
                                }
                            }
                        }
                        else{
                            fprintf(stderr, "Host disconnected, fd: %d from %s\n", i, requestP[i].host);
                        }
                    }
                    else{
                        fd_set write_set;
                        FD_ZERO(&write_set);
                        FD_SET(requestP[i].conn_fd , &write_set);

                        select(requestP[i].conn_fd + 1 , &write_set , NULL,NULL,&timeout);

                        if (FD_ISSET(requestP[i].conn_fd , &write_set)){
                            //printf("ok\n");

                            int r = handle_read(&requestP[i]);
                            int pos = requestP[i].id;

                            if (r < 0){
                                fprintf(stderr,"read err form %d\n",requestP[i].conn_fd);
                                continue;
                            }
                            else if(r > 0 ) {
                                //chenck input and modifly the file 
                                registerRecord reg;
                                fprintf(stderr,"modifing client %d\n", requestP[i].conn_fd);
                                if (check_input_to_write(requestP[i].buf,&reg) < 0 ){
                                    fprintf(stderr, "client enter invaile input\n");
                                    write(requestP[i].conn_fd, fail_msg, strlen(fail_msg));
                                }
                                else{
                                    // reg write to file 
                                    lseek(requestP[i].fd, pos * sizeof(reg), SEEK_SET);
                                    write(requestP[i].fd,&reg, sizeof(reg));
                                    // msg to client 
                                    
                                    char buf[512];
                                    print_order_with_id(buf, &reg, requestP[i].id + 902001);//pos + 902001 -> id
                                    write(requestP[i].conn_fd, buf , strlen(buf)) ;//modifly msg 

                                }

                            }
                            else{
                                fprintf(stderr, "dicconnented\n");
                            }
                            online[pos] = false;
                            close(requestP[i].fd);
                            close(requestP[i].conn_fd);
                            free_request(&requestP[i]);

                        }
    
                    }
                }
                
            
#endif

            }   
        }
    // close(requestP[conn_fd].conn_fd);
    // free_request(&requestP[conn_fd]);
        
    }
    free(requestP);
    return 0;
}

// ======================================================================================================
// You don't need to know how the following codes are working
#include <fcntl.h>

static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->id = 0;
    reqP->fd = -1;
}

static void free_request(request* reqP) {
    /*if (reqP->filename != NULL) {
        free(reqP->filename);
        reqP->filename = NULL;
    }*/
    init_request(reqP);
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }

    // Get file descripter table size and initialize request table
    maxfd = getdtablesize();
    requestP = (request*) malloc(sizeof(request) * maxfd);
    if (requestP == NULL) {
        ERR_EXIT("out of memory allocating all requests");
    }
    for (int i = 0; i < maxfd; i++) {
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    return;
}
