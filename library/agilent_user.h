/* agilent_user.h
 * Copyright (C) 2006 Steve D. Sharples
 *
 * User library of useful functions for talking to Agilent Infiniium
 * oscilloscopes using the VXI11 protocol, for Linux. You will also need the
 * vxi11_X.XX.tar.gz source, currently available from:
 * http://optics.eee.nottingham.ac.uk/vxi11/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The author's email address is steve.sharples@nottingham.ac.uk
 */

#include "../../vxi11/vxi11_user.h"

int	agilent_open(char *ip, CLINK *clink);
int	agilent_close(char *ip, CLINK *clink);
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
long	agilent_get_averages(CLINK *clink);
double	agilent_get_sample_rate(CLINK *clink);
long	agilent_get_n_points(CLINK *clink);
int	agilent_display_channel(CLINK *clink, char chan, int on_or_off);
