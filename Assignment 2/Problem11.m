% Saeedreza Zouashkiani
% 400206262
clear all; clc; close all;
N = 10; % Number of frames
codingRate=3/5;
L = 64800*codingRate; % Length of the frame
snr_min = -5; snr_max = 20;step_size=0.5;
numOfError = zeros(length(snr_min:step_size:snr_max), 1);
snr = (snr_min:step_size:snr_max)';
noiseVar = 10.^(-snr/10); % noise variance for unity power signals
M = 4; % Modulation order
bitsPerSym = log2(M);
enc = comm.LDPCEncoder(dvbs2ldpc(codingRate));
dec = comm.LDPCDecoder(dvbs2ldpc(codingRate));
x = randi([0 1], L, N);

for i=1:length(snr)
    numOfBits = 0;
    for j=1:N
        x_enc = enc(x(:, j));
        x_mod = qammod(x_enc, M, 'InputType','bit','UnitAveragePower',true);
        y = awgn(x_mod, snr(i));
%             y_hard_demod = qamdemod(y, M, 'bin', 'OutputType', 'bit');
        y_soft_demod = qamdemod(y, M, 'bin', 'OutputType', ...
            'approxllr','UnitAveragePower',true,'NoiseVariance',noiseVar(i));
%             y_hard_dec = dec(y_hard_demod);
        y_soft_dec = dec(y_soft_demod);
        numOfError(i) = numOfError(i) + sum(double(y_soft_dec~=x(:, j)));
    end
end
numOfCorrect = N*L - numOfError;
numOfError = numOfError/(N*L);
figure(1);plot(snr, 10*log10(numOfError), 'r-*');title('bit error rate (BER)');ylabel('P_e (dB)');xlabel('SNR (dB)');grid on;xlim([-5 20]);
figure(2);plot(snr, 10*log10(numOfCorrect/10), 'r-*');title('Channel throughput');ylabel('Throughput (dB)');xlabel('SNR (dB)');grid on;xlim([-5 20]);