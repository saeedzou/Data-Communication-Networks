% Saeedreza Zouashkiani
% 400206262
clear all; clc; close all;
N = 10; % Number of frames
L = 1000; % Length of the frame
snr_min = 8.4; snr_max = 12.4;
snr = (snr_min:0.2:snr_max)';
allBits=zeros(length(snr), 1);
M = 4; % Modulation order
bitsPerSym = log2(M);
x = randi([0 1], L, N);
% x_mod = qammod(x,M,'InputType','bit','UnitAveragePower',true);
for n=1:10 % for smoothing the result
    for i=1:length(snr)
        numOfBits = 0;
        for j=1:N
            err=1;
            while err~=0
                x_enc = lteCRCEncode(x(:, j), '16');
                x_mod = qammod(x_enc, M, 'InputType','bit','UnitAveragePower',true);
                y = awgn(x_mod, snr(i));
                y_demod = qamdemod(y, M, 'bin', 'OutputType', 'bit');
                [~, err] = lteCRCDecode(y_demod, '16');
                numOfBits = numOfBits + 1016;
                if err==0
                    break
                end
            end
        end
        allBits(i)=numOfBits+allBits(i);
    end
end
allBits = 10*log10(allBits/10);
plot(snr,allBits, 'r-*');grid on;title('Total Number of Bits sent');ylabel('Transmitted bits (dB)');xlabel('snr(dB)');
