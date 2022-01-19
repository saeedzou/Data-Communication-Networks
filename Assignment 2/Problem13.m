% Saeedreza Zouashkiani
% 400206262
clear all; clc; close all;
N = 10; % Number of frames
codingRate=[2/5 3/5 4/5];fignum=1;
L = 64800; % Length of the frame
snr_min = -5; snr_max = 20;step_size=1;
snr = (snr_min:step_size:snr_max)';
noiseVar = 10.^(-snr/10); % noise variance for unity power signals
M = [64]; % Modulation order
bitsPerSym = log2(M);
numOfError = zeros(length(snr_min:step_size:snr_max), length(M), length(codingRate));
numOfCorrect = zeros(length(snr_min:step_size:snr_max), length(M), length(codingRate));
for m=1:length(M)
    for c=1:length(codingRate)
        x = randi([0 1], L*codingRate(c), N);
        enc = comm.LDPCEncoder(dvbs2ldpc(codingRate(c)));
        dec = comm.LDPCDecoder(dvbs2ldpc(codingRate(c)));
        for i=1:length(snr)
            numOfBits = 0;
            for j=1:N
                x_enc = enc(x(:, j));
                x_mod = qammod(x_enc, M(m), 'InputType','bit','UnitAveragePower',true);
                y = awgn(x_mod, snr(i));
                y_soft_demod = qamdemod(y, M(m), 'bin', 'OutputType', ...
                    'approxllr','UnitAveragePower',true,'NoiseVariance',noiseVar(i));
                y_soft_dec = dec(y_soft_demod);
                numOfError(i, m, c) = numOfError(i, m, c) + sum(double(y_soft_dec~=x(:, j)));
            end
        end
        numOfCorrect(:, m, c) = N*L*codingRate(c) - numOfError(:, m, c);
        numOfError(:, m, c) = numOfError(:, m, c)/(N*L*codingRate(c));
        figure(fignum);plot(snr, 10*log10(numOfError(:, m, c)), 'r-*');title('bit error rate (BER)');ylabel('P_e (dB)');xlabel('SNR (dB)');grid on;xlim([-5 20]);
        fignum=fignum+1;
        figure(fignum);plot(snr, 10*log10(numOfCorrect(:, m, c)/10), 'r-*');title('Channel throughput');ylabel('Throughput (dB)');xlabel('SNR (dB)');grid on;xlim([-5 20]);
        fignum=fignum+1;
    end
end
