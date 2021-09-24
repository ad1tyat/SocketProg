/*
• Request_Type is either 0 for item or 1 for close.
• UPC-code is a 3-digit unique product code; this field is meaningful only if
the Request_Type is 0. • Number is the number of items being purchased; this
field is meaningful only if the Request_Type is 0.
*/
typedef struct RequestMessage {
    int Request_type;
    int UPC_CODE;
    int number;
} RequestMessage;

typedef struct ResponseMessage {
    int Response_type;
    int item;
    int total_amount;
    int price;
    char error[50];
    char name[50];
} ResponseMessage;

char err1[] = "Protocol Error";
char err2[] = "UPC is not found in database";

// TODO: A max message size constant instead of using 256 as magic everywhere (buffer)

void encode_response(ResponseMessage message, char msg[]) {
    int res_type = message.Response_type;
    if (res_type == 1) {
        /*
        If error, then <Response> is as follows: a null terminated string
        containing the error; the only possible errors are "Protocol Error" or
        "UPC is not found in database".
        */
        sprintf(msg, "%d$%s#", res_type, message.error);
    } else {
        int item = message.item;
        if (item == 1) {
            // item 
            int price = message.price;
            char *name = message.name;
            sprintf(msg, "%d$%d$%s#", res_type, price, name);
        } else {
            // close
            int total_amount = message.total_amount;
            sprintf(msg, "%d$%d#", res_type, total_amount);
        }
    }
    // printf("Encoded : %s\n", msg);
}

ResponseMessage decode_response(char response[], int type) {
    int res_type = response[0] - '0';
    ResponseMessage ret;
    ret.Response_type = res_type;
    if (res_type == 1) {
        // error
        int idx = 2;
        char err[50];
        int eidx = 0;
        while (response[idx] != '#') {
            err[eidx] = response[idx];
            idx++;
            eidx++;
        }
        err[eidx] = '\0';
        strcpy(ret.error, err);
    } else {
        if (type == 0) {
            // item
            int idx = 2;
            int price = 0;
            while (response[idx] != '#' && response[idx] <= '9' &&
                   response[idx] >= '0') {
                price *= 10;
                price += response[idx] - '0';
                idx++;
            }
            idx++;  //'$' delimeter
            char name[50];
            int nidx = 0;
            while (response[idx] != '#') {
                name[nidx] = response[idx];
                idx++;
                nidx++;
            }
            ret.price = price;
            strcpy(ret.name, name);
        } else {
            // close
            int idx = 2;
            int amount = 0;
            while (response[idx] != '#') {
                amount *= 10;
                amount += response[idx] - '0';
                idx++;
            }
            ret.total_amount = amount;
        }
    }
    return ret;
}

void encode_request(RequestMessage message, char msg[]) {
    int req_type = message.Request_type;
    int upc_code = message.UPC_CODE;
    int number = message.number;
    sprintf(msg, "%d$%d$%d#", req_type, upc_code, number);
}
RequestMessage decode_request(char request[]) {
    int req_type = request[0] - '0';
    int upc_code = 0;
    int number = 0;
    int idx = 2;
    while (request[idx] != '#' && request[idx] >= '0' && request[idx] <= '9') {
        upc_code *= 10;
        upc_code += request[idx] - '0';
        idx++;
    }
    idx++;
    while (request[idx] != '#' && request[idx] >= '0' && request[idx] <= '9') {
        number *= 10;
        number += request[idx] - '0';
        idx++;
    }
    RequestMessage ret;
    ret.UPC_CODE = upc_code;
    ret.number = number;
    ret.Request_type = req_type;
    return ret;
}