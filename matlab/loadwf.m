%[waveform,timebase,delayed_timebase]=loadwf(filename)
% 
%Loads scope waveforms saved using 'getwf' by Steve.
% 
%Loads the waveform contained in binary data in filename.wf
%Uses the assaciated waveform info file filename.wfi to find the number
%of points used and to scale the data.
%
%Updated 6/11/00 to allow multiple waveforms per file
% 
%The waveform is returned in the first array (size [no_of_traces,no_of_points])
%The timebase, starting at zero seconds is returned in the second array
%The delayed timebase is returned in the third array.
% 
%Also knocks off the extra random two points....

function [w,t,t_d]=loadwf(name)

f=findstr(name,'.wf');
if isempty(f)==1 fname=name;
else fname=name(1:(f(size(f,2)))-1);end;
wfifile=strcat(fname,'.wfi');

c=load(wfifile);

if size(c,1)<7
	resolution=1; %old 8-bit
else
	resolution=c(7);
end

knock_off_points=2*resolution;

if length(c)>7
	if c(8)==1 knock_off_points=0;end
end

no_points=(c(1)-(knock_off_points))/resolution;
no_bytes=c(1);
vgain=c(2);
voffset=c(3);
hinterval=c(4);
hoffset=c(5);
if size(c,1)==5
	no_of_traces=1;
else
	no_of_traces=c(6);
end

wffile=strcat(fname,'.wf');

fi=fopen(wffile,'r');
if resolution==1
	d=fread(fi,[no_bytes no_of_traces],'int8');
else
	d=fread(fi,[(no_bytes)/2 no_of_traces],'int16');
end
fclose(fi);


w=d(1:no_points,:).*vgain-voffset;
t=(0:no_points-1).*hinterval;
t_d=(0:no_points-1).*hinterval+hoffset;

