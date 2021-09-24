#include <stdio.h>
#include <string.h>
#include "message.h"
int main(int argc, char *argv[])
{
    RequestMessage dummy;
    dummy.Request_type = 1;
    dummy.number = 69;
    dummy.UPC_CODE = 420;
    char localBuffer[1000];
    encode_request(dummy, localBuffer);
    printf("%s\n", localBuffer);
    RequestMessage dummy2 = decode_request(localBuffer);
    printf("%d %d %d\n", dummy2.Request_type, dummy2.UPC_CODE, dummy2.number);
    

    ResponseMessage err_dummy;
    err_dummy.Response_type = 1;
    strcpy(err_dummy.error, err2);

    char localBuffer2[1000];
    encode_response(err_dummy, localBuffer2);
    ResponseMessage err_dummy2 = decode_response(localBuffer2, -1);

    printf("%d %s--", err_dummy2.Response_type, err_dummy2.error);
    
    

    ResponseMessage resp_dummy;
    resp_dummy.Response_type = 0;
    resp_dummy.price = 5;
    resp_dummy.item = 1;
    strcpy(resp_dummy.name, "bananas");

    char localBuffer3[1000];
    encode_response(resp_dummy, localBuffer3);
    ResponseMessage resp_dummy2 = decode_response(localBuffer3, 0);

    printf("%d %d %s\n", resp_dummy2.Response_type, resp_dummy2.price, resp_dummy2.name);
    
    ResponseMessage close_dummy;
    close_dummy.Response_type = 0;
    close_dummy.total_amount = 68793;
    close_dummy.item = 0;

    char localBuffer4[1000];
    encode_response(close_dummy, localBuffer4);
    ResponseMessage close_dummy2 = decode_response(localBuffer4, 1);

    printf("%d %d\n", resp_dummy2.Response_type, close_dummy2.total_amount);
    
    
    return 0;
}
