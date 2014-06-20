#ifndef SSL_UTILS_H
#define SSL_UTILS_H
#include <stdlib.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "utils.h"

typedef struct{
    SSL_CTX* handle;
    SSL* socket;
} SSL_Connection;


SSL_CTX* SSL_SERVER_HANDLE;


/*
    Load libraries for SSL and init globals.
    @param certFile the RSA signed CA file
    @param privKeyFile the private key for CA file
*/
void SSL_Init(char *certFile, char* privKeyFile){
    SSL_library_init();
    OpenSSL_add_all_algorithms();   
    SSL_load_error_strings();     
    SSL_SERVER_HANDLE = SSL_CTX_new( SSLv23_server_method() );
    
    if (SSL_SERVER_HANDLE == NULL)
        ERR_print_errors_fp(stderr);

    SSL_CTX_use_certificate_file(SSL_SERVER_HANDLE, certFile,
                                SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(SSL_SERVER_HANDLE, privKeyFile,
                                SSL_FILETYPE_PEM);
      
    if ( !SSL_CTX_check_private_key(SSL_SERVER_HANDLE)){
        printf("Private key and CA don\'t match up\n");
        exit(2);
    }
}

/*
    Takes a already connected TCP file descriptor
    and wraps it in SSL. 
    @param sockfd the file descriptor
*/
#define SSL_CLIENT (1 << 0)
#define IS_SSL_CLIENT(x) ((x) & SSL_CLIENT)
#define SSL_SERVER (1 << 1)
#define IS_SSL_SERVER(x) ((x) & SSL_SERVER)
SSL_Connection* SSL_Connect(int sockfd, int flags){
    SSL_Connection* sslcon = malloc(sizeof(SSL_Connection));
    if (sslcon == (SSL_Connection*) 0)
        die("SSL_Connection: malloc returned NULL");
    if (IS_SSL_SERVER(flags)){
        sslcon->handle = SSL_SERVER_HANDLE;
    }else{
        sslcon->handle = SSL_CTX_new(SSLv23_client_method());
        if (sslcon->handle == (SSL_CTX*) 0)
            ERR_print_errors_fp (stderr);
    }
    sslcon->socket = SSL_new(sslcon->handle);
    if (sslcon->socket == (SSL*) 0)
        ERR_print_errors_fp (stderr);
    
    if ( SSL_set_fd(sslcon->socket, sockfd) != 1 )
        ERR_print_errors_fp (stderr);
    if (IS_SSL_SERVER(flags)){
        if (SSL_accept(sslcon->socket) != 1)
            ERR_print_errors_fp (stderr);
    }else{
        if (SSL_connect(sslcon->socket) != 1)
            ERR_print_errors_fp (stderr);
    }
    return sslcon;
}

/*
    Free a SSL_Connection structure
*/
void SSL_Close(SSL_Connection* sslcon){
    if (sslcon != (SSL_Connection*) 0){
        SSL_shutdown(sslcon->socket);
        SSL_free(sslcon->socket);
        SSL_CTX_free(sslcon->handle);
        free(sslcon);
    }
}




#endif









