% design the filter as poles, zeroes and gain
%[z,p,k] = butter(2,[445 561]*2/48000); % bandpass filter 
[z,p,k] = butter(4,1000/(48000/2));%low pass filter 

% turn into biquads
sos = zp2sos(z,p,k);

% eliminate a0
%coeffs = sos(:,[1 2 3 5 6]);
% make a linear array
%coeffs = [coeffs(1,:) coeffs(2,:)];

h=fvtool(sos,'Analysis','freq');
h.Fs = 48000;
%h.FrequencyRange='[-Fs/2, Fs/2)';
