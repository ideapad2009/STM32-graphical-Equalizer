close all 
clc
clear
Fs  = 48000;
N   = 6;
G   = 4;
Q   = 1.5;
Wo1 = 300/(Fs/2);
Wo2 = 1000/(Fs/2);
Wo3 = 2000/(Fs/2);
Wo4 = 3000/(Fs/2);
Wo5 = 4000/(Fs/2);
% Obtain the bandwidth of the equalizers from the center frequencies and
% Q-factors.
BW1 = Wo1/Q;
BW2 = Wo2/Q;
BW3 = Wo3/Q;
BW4 = Wo4/Q;
BW5 = Wo5/Q;
a={};
b={};
c={};
d={};
e={};
SOS_B1=[];
SOS_B2=[];
SOS_B3=[];
SOS_B4=[];
SOS_B5=[];
% Design the equalizers and obtain their SOS and SV values.
for x = -100:100
    [SOS1,SV1] = iirparameq(N,x,Wo1,0.01);  
    [SOS2,SV2] = iirparameq(N,x,Wo2,0.01);
    [SOS3,SV3] = iirparameq(N,x,Wo3,0.01);
    [SOS4,SV4] = iirparameq(N,x,Wo4,0.01);
    [SOS5,SV5] = iirparameq(N,x,Wo5,0.01);
    SOS1_CMSIS=SOS1;
    SOS2_CMSIS=SOS2;
    SOS3_CMSIS=SOS3;
    SOS4_CMSIS=SOS4;
    SOS5_CMSIS=SOS5;
    SOS1_CMSIS(:,5) = -1 *SOS1(:,5);
    SOS1_CMSIS(:,6) = -1 *SOS1(:,6);
    SOS2_CMSIS(:,5) = -1 *SOS2(:,5);
    SOS2_CMSIS(:,6) = -1 *SOS2(:,6);
    SOS3_CMSIS(:,5) = -1 *SOS3(:,5);
    SOS3_CMSIS(:,6) = -1 *SOS3(:,6);
    SOS4_CMSIS(:,5) = -1 *SOS4(:,5);
    SOS4_CMSIS(:,6) = -1 *SOS4(:,6);
    SOS5_CMSIS(:,5) = -1 *SOS5(:,5);
    SOS5_CMSIS(:,6) = -1 *SOS5(:,6);
    SOS_B1 =[SOS_B1;SOS1_CMSIS(:,[1 2 3 5 6])];
    SOS_B2 =[SOS_B2;SOS2_CMSIS(:,[1 2 3 5 6])];
    SOS_B3 =[SOS_B3;SOS3_CMSIS(:,[1 2 3 5 6])];
    SOS_B4 =[SOS_B4;SOS4_CMSIS(:,[1 2 3 5 6])];
    SOS_B5 =[SOS_B5;SOS5_CMSIS(:,[1 2 3 5 6])];
    BQ1 = dsp.BiquadFilter('SOSMatrix',SOS1,'ScaleValues',SV1);
    BQ2 = dsp.BiquadFilter('SOSMatrix',SOS2,'ScaleValues',SV2);
    BQ3 = dsp.BiquadFilter('SOSMatrix',SOS3,'ScaleValues',SV3);
    BQ4 = dsp.BiquadFilter('SOSMatrix',SOS4,'ScaleValues',SV4);
    BQ5 = dsp.BiquadFilter('SOSMatrix',SOS5,'ScaleValues',SV5);
    a{end+1} =BQ1;
    b{end+1} =BQ2;
    c{end+1} =BQ3;
    d{end+1} =BQ4;
    e{end+1} =BQ5;
end
string_B1=strings;
string_B2=strings;
string_B3=strings;
string_B4=strings;
string_B5=strings;
[row,col] = size(SOS_B1);
 for N = 1:col
     for M = 1:row
         string_B1(M,N)=strcat(num2str(SOS_B1(M,N),'%.18f'),'f');
         string_B2(M,N)=strcat(num2str(SOS_B2(M,N),'%.18f'),'f');
         string_B3(M,N)=strcat(num2str(SOS_B3(M,N),'%.18f'),'f');
         string_B4(M,N)=strcat(num2str(SOS_B4(M,N),'%.18f'),'f');
         string_B5(M,N)=strcat(num2str(SOS_B5(M,N),'%.18f'),'f');
     end
 end

file_B1 = fopen('Core/Inc/biquad_band1.h','w');
file_B2 = fopen('Core/Inc/biquad_band2.h','w');
file_B3 = fopen('Core/Inc/biquad_band3.h','w');
file_B4 = fopen('Core/Inc/biquad_band4.h','w');
file_B5 = fopen('Core/Inc/biquad_band5.h','w');
fprintf(file_B1,'%s\n','float band1_coeff[]={');
fprintf(file_B2,'%s\n','float band2_coeff[]={');
fprintf(file_B3,'%s\n','float band3_coeff[]={');
fprintf(file_B4,'%s\n','float band4_coeff[]={');
fprintf(file_B5,'%s\n','float band5_coeff[]={');
for i=1:row
    fprintf(file_B1,'%s,%s,%s,%s,%s,\n',string_B1(i,:));
    fprintf(file_B2,'%s,%s,%s,%s,%s,\n',string_B2(i,:));
    fprintf(file_B3,'%s,%s,%s,%s,%s,\n',string_B3(i,:));
    fprintf(file_B4,'%s,%s,%s,%s,%s,\n',string_B4(i,:));
    fprintf(file_B5,'%s,%s,%s,%s,%s,\n',string_B5(i,:));
end
fprintf(file_B1,'%s\n','};');
fprintf(file_B2,'%s\n','};');
fprintf(file_B3,'%s\n','};');
fprintf(file_B4,'%s\n','};');
fprintf(file_B5,'%s\n','};');
fclose(file_B1);
fclose(file_B2);
fclose(file_B3);
fclose(file_B4);
fclose(file_B5);

%h=fvtool(a{:},b{:},c{:},d{:},e{:},'Fs',Fs);

%{
fvtool(BQ1,BQ2,'Fs',Fs,'FrequencyScale','Log');
%legend('Equalizer centered at 100 Hz','Equalizer centered at 1000 Hz');
%}