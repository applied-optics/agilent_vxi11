#include "../../vxi11/vxi11_user.h"

int	agilent_init(CLIENT *client, VXI11_LINK *link);
int	agilent_report_status(CLIENT *client, VXI11_LINK *link, unsigned long timeout);
int	agilent_get_setup(CLIENT *client, VXI11_LINK *link, char *buf, unsigned long buf_len);
int	agilent_send_setup(CLIENT *client, VXI11_LINK *link, char *buf, unsigned long buf_len);
long	agilent_get_screen_data(CLIENT *client, VXI11_LINK *link, char chan, char *buf, unsigned long buf_len, unsigned long timeout, double s_rate, long npoints);
int	agilent_set_for_capture(CLIENT *client, VXI11_LINK *link, double s_rate, long npoints, unsigned long timeout);
void	agilent_set_for_auto(CLIENT *client, VXI11_LINK *link);
long	agilent_get_data(CLIENT *client, VXI11_LINK *link, char chan, char *buf, unsigned long buf_len,unsigned long timeout);
long	agilent_get_data(CLIENT *client, VXI11_LINK *link, char chan, int digitise, char *buf, unsigned long buf_len,unsigned long timeout);
int	agilent_get_preamble(CLIENT *client, VXI11_LINK *link, char *buf, unsigned long buf_len);
long	agilent_write_wfi_file(CLIENT *client, VXI11_LINK *link, char *wfiname, char chan, char *captured_by, int no_of_traces, unsigned long timeout);
long	agilent_write_wfi_file(CLIENT *client, VXI11_LINK *link, char *wfiname, long no_of_bytes, char *captured_by, int no_of_traces, unsigned long timeout);
long	agilent_calculate_no_of_bytes(CLIENT *client, VXI11_LINK *link, char chan, unsigned long timeout);
void	agilent_scope_channel_str(char chan, char *source);


