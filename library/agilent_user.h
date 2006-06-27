#include "../../vxi11/vxi11_user.h"

int	agilent_init(CLINK *clink);
int	agilent_report_status(CLINK *clink, unsigned long timeout);
int	agilent_get_setup(CLINK *clink, char *buf, unsigned long buf_len);
int	agilent_send_setup(CLINK *clink, char *buf, unsigned long buf_len);
long	agilent_get_screen_data(CLINK *clink, char chan, char *buf, unsigned long buf_len, unsigned long timeout, double s_rate, long npoints);
int	agilent_set_for_capture(CLINK *clink, double s_rate, long npoints, unsigned long timeout);
void	agilent_set_for_auto(CLINK *clink);
long	agilent_get_data(CLINK *clink, char chan, char *buf, unsigned long buf_len,unsigned long timeout);
long	agilent_get_data(CLINK *clink, char chan, int digitise, char *buf, unsigned long buf_len,unsigned long timeout);
int	agilent_get_preamble(CLINK *clink, char *buf, unsigned long buf_len);
long	agilent_write_wfi_file(CLINK *clink, char *wfiname, char chan, char *captured_by, int no_of_traces, unsigned long timeout);
long	agilent_write_wfi_file(CLINK *clink, char *wfiname, long no_of_bytes, char *captured_by, int no_of_traces, unsigned long timeout);
long	agilent_calculate_no_of_bytes(CLINK *clink, char chan, unsigned long timeout);
void	agilent_scope_channel_str(char chan, char *source);
int	agilent_set_averages(CLINK *clink, int no_averages);


