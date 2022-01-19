% Saeedreza Zouashkiani
% 400206262
clear all; clc; close all;
N = 10; % Number of frames
L = 1000; % Length of the frame
snr_min = -5; snr_max = 20;
snr = (snr_min:0.2:snr_max)';
M = 4; % Modulation order
bitsPerSym = log2(M);
numOfError = zeros(length(snr_min:0.2:snr_max), 1);
numOfCorrect = zeros(length(snr_min:0.2:snr_max), 1);
for n=1:10 % for smoothing the result
x = randi([0 1], L, N);
x_mod = qammod(x,M,'InputType','bit','UnitAveragePower',true);
for i=1:length(snr)
    y = awgn(x_mod, snr(i));
    y_demod = qamdemod(y, M, 'bin', 'OutputType', 'bit');
    numOfError(i) = numOfError(i) + sum(double(y_demod~=x), 'all');
    numOfCorrect(i) =numOfCorrect(i) + sum(double(y_demod==x), 'all');
end
end
numOfError = numOfError/(N*L*10);
figure(1);plot(snr, 10*log10(numOfError), 'r-*');title('bit error rate (BER)');ylabel('P_e (dB)');xlabel('SNR (dB)');xlim([-5 20]);grid on;
figure(2);plot(snr, 10*log10(numOfCorrect/100), 'r-*');title('Channel throughput');ylabel('Throughput (dB)');xlabel('SNR (dB)');xlim([-5 20]);grid on;