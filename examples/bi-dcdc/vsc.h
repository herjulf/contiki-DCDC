#define MTDC_MAX 1     /* Max MTDC terminals */

/* 
   VSC_MAX to VSC_MIN defines the operational voltage interval 
   for load sharing according to droop control algorithm.
*/

#define VSC_MAX 12         /* Max Grid Voltage droop */
#define VSC_INTERVAL 0.87  /* Grid voltage interval */
#define VSC_MIN VSC_MAX * VSC_INTERVAL  /* Min Grid Voltage droop */

#define V_DIS   VSC_MAX * 0.7 /* Disconnent voltage */
#define V_HYST  VSC_MAX*0.1   /* Reconnect voltage */

