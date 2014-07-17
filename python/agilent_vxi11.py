# agilent_vxi11.c
# Copyright (C) 2006 Steve D. Sharples
#
# User library of useful functions for talking to Agilent Infiniium
# oscilloscopes using the VXI11 protocol, for Linux. You will also need the
# vxi11_X.XX.tar.gz source, currently available from:
# http://optics.eee.nottingham.ac.uk/vxi11/
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# The author's email address is steve.sharples@nottingham.ac.uk
#
# Python conversion by Roger Light

import vxi11

class AgilentVxi11(vxi11.Vxi11):
    def __init__(self, address, device="inst0"):
        super().__init__(address, device)

        # Set up some fundamental settings for data transfer. It's possible
        # (although not certain) that some or all of these would be reset after
        # a system reset. It's a very tiny overhead right at the beginning.
        self.send(":SYSTEM:HEADER 0")
        self.send(":ACQUIRE:COMPLETE 100")
        self.send(":WAVEFORM:BYTEORDER LSBFIRST")
        self.send(":WAVEFORM:FORMAT BINARY")

    def __del__(self):
        self.close()


    def report_status(self, timeout)
        """ Debugging function whilst developing the library. Might as well
        leave it here"""
        buf = self.send_and_receive(":WAV:SOURCE?", timeout)
        print("Source:        %s" % buf)

        dval = self.obtain_double_value(":TIM:RANGE?", timeout)
        print("Time range:    %g (%g us)" % (dval, (dval * 1e6)))

        dval = self.obtain_double_value(":ACQ:SRAT?", timeout)
        lval = self.obtain_long_value(":ACQ:SRAT:AUTO?", timeout)
        if lval == 0:
            print("Sample rate:   %g (%g GS/s), set to MANUAL" % (dval, (dval / 1e9)))
        else:
            print("Sample rate:   %g (%g GS/s), set to AUTO" % (dval, (dval / 1e9)))

        lval = self.obtain_long_value(":ACQ:POINTS?", timeout)
        s = "No of points:  %ld, set to ", lval
        print(s,)
        lval = self.obtain_long_value(":ACQ:POINTS:AUTO?", timeout)
        if lval == 0:
            print("MANUAL")
        else:
            print("AUTO")

        lval = self.obtain_long_value(":ACQ:INT?", timeout)
        if lval == 0:
            print("(sin x)/x:     OFF")
        else:
            print("(sin x)/x:     ON")

        dval = self.obtain_double_value(":WAV:XINC?", timeout)
        print("xinc:          %g (%g us, %g ps)" % (dval, (dval * 1e6), (dval * 1e12)))

    def get_setup(self):
        """Gets the system setup for the scope, dumps it in an array. Can be
        saved for later."""
        ret = self.send(":SYSTEM:SETUP?")
        if ret < 0:
            print("error, could not ask for system setup, quitting...")
            return None

        return self.receive_data_block()

    def send_setup(self, buf):
        """Sends a previously saved system setup back to the scope."""

        ret = self.send_data_block(":SYSTEM:SETUP ", buf)
        if ret < 0:
            printf("error, could not send system setup, quitting...")
            return ret
        return 0

    def calculate_no_of_bytes(self, chan, timeout=vxi11.READ_TIMEOUT):
        """Fairly important function. Calculates the number of bytes that
        digitised data will take up, based on timebase settings, whether
        interpolation is turned on etc. Can be used before you actually acquire
        any data, to find out how big you need your data buffer to be."""

        # First we need to digitize, to get the correct values for the
        # waveform data. This is a pain in the arse.
        source = agilent_scope_channel_str(chan)
        self.send(":WAV:SOURCE %s" % source)
        self.send(":DIG")

        # Now find the info we need to calculate the number of points
        hinterval = self.obtain_double_value(":WAV:XINC?", timeout)
        time_range = self.obtain_double_value(":TIM:RANGE?", timeout)

        # Are we in equivalent time (ETIM) mode? If so, the value of ACQ:SRAT will
        # be meaningless, and there's a different formula
        etim_result = self.send_and_receive(":ACQ:MODE?", 256, timeout)
        # Equivalent time (ETIM) mode:
        if etim_result[0:3] == "ETIM":
            no_of_bytes = (2 * ((time_range / hinterval) + 0.5))
        else:
            srat = vxi11_obtain_double_value(clink, ":ACQ:SRAT?")

            no_of_bytes = (2 * (((time_range - (1 / srat)) / hinterval) + 1) + 0.5)
            # 2x because 2 bytes per data point
            # +0.5 to round up when casting as a long
            # -(1/srat) and +1 so that both raw data, and interpolated (sinx/x) data works

        return no_of_bytes
}

    def write_wfi_file(self, wfiname, captured_by, no_of_traces, no_of_bytes, timeout):
        """This function, agilent_write_wfi_file(), saves useful (to us!)
        information about the waveforms. This is NOT the full set of "preamble"
        data - things like the data and other bits of info you may find useful
        are not included, and there are extra elements like the number of
        waveforms acquired. 
 
        It is up to the user to ensure that the scope is in the same condition
        as when the data was taken before running this function, otherwise the
        values returned may not reflect those during your data capture; ie if
        you run agilent_set_for_capture() before you grab the data, then either
        call that function again with the same values, or, run this function
        straight after you have acquired your data.
 
        Pass no_of_bytes == -1 to instruct the function to do a :DIG and find
        out how many bytes of data will be collected.  This may be useful if
        you are only capturing one trace; you might as well call this function
        at the start, find out how many bytes you need for your data array, and
        write the wfi file at the same time.
 
        But maybe you'll not know how many traces you're taking until the end,
        in which case you will want to write the wfi file at the end. In that
        case, you should already know how many bytes per trace there are, and
        have also probably recently done a :DIG to capture your data; in that
        case pass no_of_bytes that you have already found."""

        if no_of_bytes == 1:
            # First we'll calculate the number of bytes. This involves doing a
            # :DIG command, which (depending on how your scope's set up) may
            # take a fraction of a second, or a very long time.
            no_of_bytes = self.calculate_no_of_bytes(chan, timeout)

        # Now find the other info we need to write the wfi-file
        # (those paying attention will notice some repetition here, it's just a
        # bit more logical this way)
        hinterval = self.obtain_double_value_timeout(":WAV:XINC?" % timeout)
        hoffset = self.obtain_double_value(":WAV:XORIGIN?")
        vgain = self.obtain_double_value(":WAV:YINC?")
        voffset = self.obtain_double_value(":WAV:YORIGIN?")

        try:
            wfi = open(wfiname, "w")
        except IOError:
            print("error: agilent_write_wfi_file: could not open %s for writing" % wfiname)
            return -1
            
        wfi.write("%% %s" % wfiname)
        wfi.write("%% Waveform captured using %s\n" % captured_by)
        wfi.write("%% Number of bytes:\n%d\n" % no_of_bytes)
        wfi.write("%% Vertical gain:\n%g\n" % vgain)
        wfi.write("%% Vertical offset:\n%g\n" % -voffset)
        wfi.write("%% Horizontal interval:\n%g\n" % hinterval)
        wfi.write("%% Horizontal offset:\n%g\n" % hoffset)
        wfi.write("%% Number of traces:\n%d\n" % no_of_traces)
        wfi.write("%% Number of bytes per data-point:\n%d\n" % 2);    # always 2 on Agilent scopes
        wfi.write("%% Keep all datapoints (0 or missing knocks off 1 point, legacy lecroy):\n%d\n" % 1)
        wfi.close()
        return no_of_bytes

    def get_screen_data(self, s_rate, npoints, timeout):
        """This function is more a demonstration of the sequence of capturing
        data, than a useful function in itself (though it can be used as-is).
        Agilent scopes, when either sample rate or no of acquisition points (or
        both) are set to "auto", sets the time period for acquisition to be
        greater than what is displayed on the screen. We find this way of
        working confusing, and so we set the sample rate and no of acquisition
        points manually, such that the acquisition period matches the time
        range (i.e. the period of time displayed on the screen).

        We may also want to control either the sample rate or the no of
        acquisition points (we can't control both, otherwise that would change
        the time range!) or, we may want the scope to select sensible values
        for us. So we have a function agilent_set_for_capture() which sets
        everthing up for us. Once this is done, we have a very basic (and hence
        fast) function that actually digitises the data in binary form,
        agilent_get_data().
        
        Finally, we like to return the scope back to automatic mode, whereby it
        chooses sensible values for the sample rate and no of acquisition
        points, and also restarts the acquisition (which gets frozen after
        digitisation).

        If you would like to grab the preamble information - see the function
        agilent_get_preamble() - or write a wfi file - agilent_write_wfi_file()
        - make sure that you do this BEFORE you return the scope to automatic
        mode."""

        self.set_for_capture(s_rate, npoints, timeout)
        (rc, buf) = self.get_data(chan, timeout=timeout)
        self.set_for_auto()
        return (rc, buf)


    def set_for_capture(self, s_rate, npoints, timeout):
        """See comments for get_screen_data() and comments within this
        function, for a description of what it does."""

        # First we need to find out if we're in "ETIM" (equivalent time) mode,
        # because things are done differently. You can't set the sample rate, and
        # if you query it, you get a meaningless answer. You must work out what the
        # effective sample rate is from the waveform xincrement. A pain in the
        # arse, quite frankly.
        etim_result = self.send_and_receive(":ACQ:MODE?", 256, timeout)

        # Equivalent time (ETIM) mode:
        if etim_result[0:3] == "ETIM":
            # Find out the time range displayed on the screen
            time_range = self.obtain_double_value(":TIM:RANGE?")

            # Find the xincrement, whilst we're still in auto (points) mode
            auto_npoints = self.obtain_long_value(":ACQ:POINTS?")

            # Set the no of acquisition points to manual
            self.send(":ACQ:POINTS:AUTO 0")

            if npoints <= 0: # if we've not been passed a value for npoints
                npoints = auto_npoints

            # Remember we want at LEAST the number of points specified.
            # To some extent, the xinc value is determined by the number of points.
            # So to get the best xinc value we ask for double what we actually
            # want.
            self.send(":ACQ:POINTS %ld" % (2 * npoints) - 1)

            # Unfortunately we have to do a :dig, to make sure our changes have
            # been registered
            self.send(":DIG")

            # Find the xincrement is now
            xinc = self.obtain_double_value(":WAV:XINC?", timeout)

            # Work out the number of points there _should_ be to cover the time
            # range
            actual_npoints = ((time_range / xinc) + 0.5)

            # Set the number of points accordingly. Hopefully the xincrement won't
            # have changed!
            self.send(":ACQ:POINTS %ld" % actual_npoints)

            # This is a bit anal... we can work out very easily what the equivalent
            # sampling rate is (1 / xinc); the scope seems to store this value
            # somewhere, even though it doesn't use it. We may as well write it to
            # the scope, in case some user program asks for it while in equivalent
            # time mode. Should not be depended upon, though!

            self.send(":ACQ:SRAT %G" % (1 / xinc))
            return 0

        # Real time (RTIM, NORM or PDET) mode:
        else:
            # First find out what the sample rate is set to. Each time you
            # switch from auto to manual for either of these, the scope
            # remembers the values from last time you set these manually.  This
            # is not very useful to us. We want to be able to set either the
            # sample rate (and derive npoints from that and the timebase), or
            # the minimum number of points (and derive the sample rate) or let
            # the scope choose sensible values for both of these. We only want
            # to capture the data for the time period displayed on the scope
            # screen, which is equal to the time range. If you leave the scope
            # to do everything auto, then it always acquires a bit more than
            # what's on the screen.  
            auto_srat = self.obtain_double_value(":ACQ:SRAT?")

            # Set the sample rate (SRAT) and no of acquisition points to manual
            self.send(":ACQ:SRAT:AUTO 0;:ACQ:POINTS:AUTO 0")

            # Find out the time range displayed on the screen
            time_range = self.obtain_double_value(":TIM:RANGE?")

            # If we've not been passed a sample rate (ie s_rate <= 0) then...
            if s_rate <= 0:
                # ... if we've not been passed npoints, let scope set rate
                if npoints <= 0:
                    s_rate = auto_srat

            else:
                # ... otherwise set the sample rate based on no of points.
                s_rate = npoints / time_range

            # We make a note here of what we're expecting the sample rate to be.
            # If it has to change for any reason (dodgy value, or not enough
            # memory) we will know about it.
            expected_s_rate = s_rate

            # Now we set the number of points to acquire. Of course, the scope
            # may not have enough memory to acquire all the points, so we just
            # sit in a loop, reducing the sample rate each time, until it's happy.
            not_enough_memory = 1
            while not_enough_memory == 1:
                # Send scope our desired sample rate.
                self.send(":ACQ:SRAT %G" % s_rate)
                # Scope will choose next highest allowed rate.
                # Find out what this is
                actual_s_rate = self.obtain_double_value(":ACQ:SRAT?")

                # Calculate the number of points on display (and round up for
                # rounding errors)
                npoints = math.ceil((time_range * actual_s_rate) + 0.5)

                # Set the number of points accordingly
                # Note this won't necessarily be the no of points you receive,
                # eg if you have sin(x)/x interpolation turned on, you will
                # probably get more.
                self.send(":ACQ:POINTS %ld" % npoints)

                # We should do a check, see if there's enough memory
                actual_npoints = self.obtain_long_value(":ACQ:POINTS?")

                if actual_npoints < npoints:
                    not_enough_memory = 1
                    ret_val = -1;    # We should report this fact to the calling function
                    s_rate = s_rate * 0.75 * (actual_npoints / npoints)
                else:
                    not_enough_memory = 0

            #  Will possibly remove the explicit printf's here, maybe leave it
            #  up to the calling function to spot potential problems (the user
            #  may not care!)
            if actual_s_rate != expected_s_rate:
                #print("Warning: the sampling rate has been adjusted,")
                #print("from %g to %g, because " % (expected_s_rate, actual_s_rate))
                if ret_val == -1:
                    #print("there was not enough memory.")
                else:
                    #print("because %g Sa/s is not a valid sample rate." % expected_s_rate)
                    ret_val = -2

            return ret_val

    def set_for_auto(self):
        """Return the scope to its auto condition"""
        self.send(":ACQ:SRAT:AUTO 1;:ACQ:POINTS:AUTO 1;:RUN")

    def scope_channel_str(self, chan):
        if chan.upper() == 'A':
            return "FUNC1"
        elif chan.upper() == 'B':
            return "FUNC2";
        elif chan.upper() == 'C':
            return "FUNC3";
        elif chan.upper() == 'D':
            return "FUNC4";
        elif chan == "1":
            return "CHAN1"
        elif chan == "2":
            return "CHAN2"
        elif chan == "3":
            return "CHAN3"
        elif chan == "4":
            return "CHAN4"
        else:
            print("error: unknown channel '%s', using channel 1" % chan)
            return "CHAN1"

    def get_data(self, chan, digitise=True, timeout=vxi11.DEFAULT_TIMEOUT):
        source = agilent_scope_channel_str(chan)
        ret = self.send(":WAV:SOURCE %s" % source)
        if ret < 0:
            print("Error, could not send :WAV:SOURCE %s cmd, quitting..." % source)
            return (ret, None)

        if digitise:
            self.send(":DIG")

        rc = -vxi11.NULL_READ_RESP
        while rc == -vxi11.NULL_READ_RESP:
            self.send(":WAV:DATA?")
            (rc, buf) = self.receive_data_block(timeout)
        return (rc, buf)

    def get_preamble(self, buf):
        ret = self.send(":WAV:PRE?")
        if ret < 0:
            print("error, could not send :WAV:PRE? cmd, quitting...")
            return ret

        return = self.receive()

    def set_averages(self, no_averages):
        # Sets the number of averages. If no_averages <= 0 then the averaging is turned
        # off; otherwise the no_averages is set and averaging is turned on. No checking
        # is done for limits. If you enter a number greater than the scope it able to 
        # cope with, it (from experience so far) sets the number of averages to its
        # maximum capability.

        if no_averages <= 0:
            return self.send(":ACQ:AVER 0")
        else:
            self.send_str(":ACQ:COUNT %d" %no_averages)
            return self.send(":ACQ:AVER 1")

    def get_averages(self):
        """Gets the number of averages. If acq:aver==0 then returns 0,
        otherwise returns the actual no of averages."""

        result = self.obtain_long_value(":ACQ:AVER?")
        if result == 0:
            return 0
        return self.obtain_long_value(":ACQ:COUNT?")

    def get_sample_rate(self):
        """Get sample rate. Trivial, but used quite often, so worth a little wrapper fn"""
        return self.obtain_double_value(":ACQ:SRAT?")

    def get_n_points(self):
        """ Get no of points (may not necessarily relate to no of bytes directly!)"""
        return self.obtain_long_value(":ACQ:POINTS?")

    def display_channel(self, chan, status):
        """Turns a channel on or off (pass "1/True" for on, "0/False" for off)"""
        if status:
            on_or_off = 1
        else:
            on_or_off = 0

        source = agilent_scope_channel_str(chan)
        return self.send(:%s:DISP %d" % (source, on_or_off))

