// can message data structure
typedef struct
{
    unsigned long id; 			// identifier (11 or 29 bit)
    struct {
       unsigned char rtr : 1;		// remote transmit request
       unsigned char extended : 1;	// extended identifier
    } flags;

    unsigned char dlc;                  // data length code
    unsigned char data[8];		// payload data
    unsigned short timestamp;           // timestamp
} canmsg_t;
