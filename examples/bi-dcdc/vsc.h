#define MTDC_MAX 1     /* Max MTDC terminals */

/* 
   VSC_MAX to VSC_MIN defines the operational voltage interval 
   for load sharing according to droop control algorithm.
*/

#define VSC_MAX 18          /* Max Grid Voltage droop */
#define VSC_INTERVAL 0.87  /* Grid voltage interval */
#define VSC_MIN VSC_MAX * VSC_INTERVAL  /* Min Grid Voltage droop */

#define V_DIS   VSC_MAX * 0.7 /* Disconnent voltage */
#define V_HYST  VSC_MAX*0.1   /* Reconnect voltage */

/*
   Voltage Droop Method (VDM) 
   u = slope*p + Vdmax
   u            output voltage
   slope        linear coefficient for the slope of the voltage droop
   p            injected active power
   Vdmax        linear coefficient for max output voltage (when power = 0)
*/
typedef enum
{
        SLOPE,
        PMAX,
}vsc_droop_t;

int set_vsc_droop(vsc_droop_t var, float value);
float get_vsc_droop(vsc_droop_t var);

void VSC_Calc(double i);
