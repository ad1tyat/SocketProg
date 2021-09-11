/*
• Request_Type is either 0 for item or 1 for close.
• UPC-code is a 3-digit unique product code; this field is meaningful only if the Request_Type is 0.
• Number is the number of items being purchased; this field is meaningful only if the Request_Type is 0.
*/
struct message
{
    int Request_type;
    int UPC_CODE;
    int quantity;
};
