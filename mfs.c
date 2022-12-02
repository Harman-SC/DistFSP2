#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include "mfs.h"
#include "udp.h"

int sd, rc;
struct sockaddr_in addrSnd, addrRcv;

int MFS_Init(char *hostname, int port) {
    for (int i = 0; i < 10000; i++) {
        sd = UDP_Open(20000 + i);
        if (sd >= 0) {
            break;
        }
    }
    if (sd < 0) {
        return -1;
    }

    rc = UDP_FillSockAddr(&addrSnd, hostname, port);
    printf("In MFS_Init after UDP_FillSockAddr rc %d \n", rc);
    return 0;
}

int MFS_Lookup(int pinum, char *name) {
    if (sd < 0) {
        return -1;
    }
    
    // Create request param
    MFS_Lookup_Request request;
    request.method = Lookup;
    request.pinum = pinum;
    strcpy(request.name, name);

    int ready = 0;
    fd_set rfds;
    struct timeval tv;

    int status = -1;

    do {
        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        // Send request
        #if 0
        UDP_Write(sd, &addrSnd, (char *) &request, BUFFER_SIZE);
        ready = select(sd+1, &rfds, NULL, NULL, &tv);
        #endif
        UDP_Write(sd, &addrSnd, (char *) &request, sizeof(MFS_Lookup_Request));
        ready = select(sd+1, &rfds, NULL, NULL, &tv);
        //ready = select(1, &rfds, NULL, NULL, &tv);
        printf("In MFS_Lookup after UDP_Write rc %d \n", rc);

        if (ready == -1) {
            return -1;
        } else if (ready) {
            // Wait response
            rc = UDP_Read(sd, &addrRcv, (char *) &status, sizeof(int));
        }
    } while(!ready);

    return status;
}

int MFS_Stat(int inum, MFS_Stat_t *m) {
    if (sd < 0) {
        return -1;
    }
    
    // Create request param
    MFS_Stat_Request request;
    request.method = Stat;
    request.inum = inum;

    int ready = 0;
    fd_set rfds;
    struct timeval tv;

    MFS_Stat_Response response;

    do {
        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        // Send request
        #if 0
        UDP_Write(sd, &addrSnd, (char *) &request, BUFFER_SIZE);
        ready = select(sd+1, &rfds, NULL, NULL, &tv);
        #endif
        rc = UDP_Write(sd, &addrSnd, (char *) &request, sizeof(MFS_Stat_Response));
        printf("In MFS_Stat after UDP_Write rc %d \n", rc);

        ready = select(sd+1, &rfds, NULL, NULL, &tv);
        //ready = select(1, &rfds, NULL, NULL, &tv);
        if (ready == -1) {
            return -1;
        } else if (ready) {
            // Wait response
            rc = UDP_Read(sd, &addrRcv, (char *) &response, sizeof(MFS_Stat_Response));
        }
    } while(!ready);

    m->type = response.stat.type;
    m->size = response.stat.size;

    return response.status;
}

int MFS_Write(int inum, char *buffer, int block) {
    if (sd < 0) {
        return -1;
    }
    
    // Create request param
    MFS_Write_Request request;
    request.method = Write;
    request.inum = inum;
    for(int i = 0; i < MFS_BLOCK_SIZE; i++) {
        request.buffer[i]=buffer[i];
    }
    request.block = block;

    int ready = 0;
    fd_set rfds;
    struct timeval tv;

    int status = -1;

    do {
        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        // Send request
#if 0
        UDP_Write(sd, &addrSnd, (char *) &request, BUFFER_SIZE);
        ready = select(sd+1, &rfds, NULL, NULL, &tv);
#endif
        rc = UDP_Write(sd, &addrSnd, (char *) &request, sizeof(MFS_Write_Request));
        ready = select(sd+1, &rfds, NULL, NULL, &tv);
        //ready = select(1, &rfds, NULL, NULL, &tv);
        printf("In MFS_Write after UDP_Write rc %d \n", rc);
        if (ready == -1) {
            return -1;
        } else if (ready) {
            // Wait response
            rc = UDP_Read(sd, &addrRcv, (char *) &status, sizeof(int));
        }
    } while(!ready);

    return status;
}

int MFS_Read(int inum, char *buffer, int block) {
    if (sd < 0) {
        return -1;
    }
    
    // Create request param
    MFS_Read_Request request;
    request.method = Read;
    request.inum = inum;
    request.block = block;
    
    int ready = 0;
    fd_set rfds;
    struct timeval tv;

    MFS_Read_Response response;

    do {
        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        // Send request
#if 0
        UDP_Write(sd, &addrSnd, (char *) &request, BUFFER_SIZE);
        ready = select(sd+1, &rfds, NULL, NULL, &tv);
#endif
        rc = UDP_Write(sd, &addrSnd, (char *) &request, sizeof(MFS_Read_Request));
        ready = select(sd+1, &rfds, NULL, NULL, &tv);
        //ready = select(1, &rfds, NULL, NULL, &tv);
        printf(" In MFS_Read After UDP_Write rc %d \n", rc);
        if (ready == -1) {
            return -1;
        } else if (ready) {
            // Wait response
            rc = UDP_Read(sd, &addrRcv, (char*) &response, sizeof(MFS_Read_Response));
        }
    } while(!ready);

    if (rc > -1) {
        for (int i = 0; i < MFS_BLOCK_SIZE; i++) {
            buffer[i] = response.buffer[i];
        }
    }

    return response.status;
}

int MFS_Creat(int pinum, int type, char *name) {
    if (sd < 0) {
        return -1;
    }
    
    // Create request param
    MFS_Creat_Request request;
    request.method = Creat;
    request.pinum = pinum;
    request.type = type;
    printf("MFS_Creat In pt A0 \n");
    strcpy(request.name, name);

    printf("MFS_Creat In pt A2 \n");
    int ready = 0;
    fd_set rfds;
    struct timeval tv;

    int status = -1;

    do {
        printf("MFS_Creat In pt A3 \n");
        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);
        printf("MFS_Creat In pt A4 \n");
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        printf("MFS_Creat In pt A5 sd %d\n", sd);
        // Send request

        //Bug fix
        #if 0
        int rc = UDP_Write(sd, &addrSnd, (char *) &request, BUFFER_SIZE);
        #endif
        int rc = UDP_Write(sd, &addrSnd, (char *) &request, sizeof( MFS_Creat_Request));
        printf("MFS_Creat In pt A6 after UDP_Write rc %d \n", rc);

        //Bug fix
       // #if 0
        ready = select(sd+1, &rfds, NULL, NULL, &tv);
        //#endif
        #if 0
        ready = select(1, &rfds, NULL, NULL, &tv);
        #endif

        printf("MFS_Creat In pt A7 \n");
        if (ready == -1) {
          printf("MFS_Creat In pt A8 \n");
            return -1;
        } else if (ready) {
            printf("MFS_Creat In pt A9 \n");
            // Wait response
            rc = UDP_Read(sd, &addrRcv, (char *) &status, sizeof(int));
            printf("MFS_Creat In pt A10 \n");
        }
        printf("MFS_Creat In pt A11  ready %d \n", ready);
    } while(!ready);
    printf("MFS_Creat In pt A12 \n");

    return status;
}

int MFS_Unlink(int pinum, char *name) {
    if (sd < 0) {
        return -1;
    }
    
    // Create request param
    MFS_Unlink_Request request;
    request.method = Unlink;
    request.pinum = pinum;
    strcpy(request.name, name);

    int ready = 0;
    fd_set rfds;
    struct timeval tv;

    int status = -1;

    do {
        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        // Send request
        //Bug Fix
        #if 0
        UDP_Write(sd, &addrSnd, (char *) &request, BUFFER_SIZE);
        ready = select(sd+1, &rfds, NULL, NULL, &tv);
        #endif
        rc = UDP_Write(sd, &addrSnd, (char *) &request, sizeof(MFS_Unlink_Request));
        printf("In MFS_Unlink after UDP_Write rc %d \n ", rc);

        ready = select(sd+1, &rfds, NULL, NULL, &tv);
        //ready = select(1, &rfds, NULL, NULL, &tv);
        if (ready == -1) {
            return -1;
        } else if (ready) {
            // Wait response
            rc = UDP_Read(sd, &addrRcv, (char *) &status, sizeof(int));
        }
    } while(!ready);

    return status;
}

int MFS_Shutdown() {
    if (sd < 0) {
        return -1;
    }
    
    // Create request param
    MFS_Shutdown_Request request;
    request.method = ShutDown;

    int ready = 0;
    fd_set rfds;
    struct timeval tv;

    int status = -1;

    do {
        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        // Send request
        UDP_Write(sd, &addrSnd, (char *) &request, BUFFER_SIZE);
        ready = select(sd+1, &rfds, NULL, NULL, &tv);
        if (ready == -1) {
            return -1;
        } else if (ready) {
            // Wait response
            rc = UDP_Read(sd, &addrRcv, (char *) &status, sizeof(int));
        }
    } while(!ready);

    return status;
}
