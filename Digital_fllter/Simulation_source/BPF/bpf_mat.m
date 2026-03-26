fs = 10000;               % Sampling frequency
fc = 100; Q = 5;
W0 = fc/(fs/2);           % normalized center frequency
BW = W0/Q;                % bandwidth
[b, a] = iirpeak(W0, BW); % 2nd-order band-pass (Q=5)

freqz(b,a,1024,fs)
[b a]
'iirpeak'에는 DSP System Toolbox이(가) 필요합니다.
 
clear
fs = 10000;
fc = 100;
Q = 5;

omega0 = 2*pi*fc/fs;
alpha = sin(omega0)/(2*Q);

b0 =  alpha;
b1 =  0;
b2 = -alpha;
a0 =  1 + alpha;
a1 = -2*cos(omega0);
a2 =  1 - alpha;

% 정규화
b = [b0 b1 b2] / a0;
a = [1 a1/a0 a2/a0];

freqz(b, a, 1024, fs);
[b a]

ans =

    0.0062         0   -0.0062    1.0000   -1.9836    0.9875

simulink
경고: The model name 'untitled' is shadowing another name
in the MATLAB workspace or path. Type "which -all
untitled" at the command line to find the other uses of
this name. You should change the name of the model to
avoid problems. 
> sltemplate.internal.request.createSimulinkModel
connector.internal.fevalMatlab에서
connector.internal.fevalJSON에서 
%% === Load Data ===
input_sig  = out.input_sig;    % Chirp 입력
output_sig = out.output_sig;   % 필터 출력
t          = out.tout;         % 시간 벡터

%% === Sampling Info ===
fs = 1 / (t(2) - t(1));        % 샘플링 주파수
N  = length(t);                % 샘플 개수
Nhalf = floor(N/2);

%% === FFT 계산 ===
IN  = fft(input_sig);
OUT = fft(output_sig);
IN  = IN(1:Nhalf);
OUT = OUT(1:Nhalf);

f = linspace(0, fs/2, Nhalf);  % 주파수 벡터

%% === Magnitude & Phase 계산 ===
mag   = abs(OUT) ./ abs(IN);
phase = unwrap(angle(OUT) - angle(IN));

%% === Plot ===
figure;
subplot(2,1,1);
semilogx(f, 20*log10(mag), 'LineWidth', 1.5);
xlabel('Frequency (Hz)'); ylabel('Magnitude (dB)');
title('BPF Frequency Response (AC Sweep)');
grid on; xlim([10 2000]); ylim([-60 5]);

subplot(2,1,2);
semilogx(f, phase*180/pi, 'LineWidth', 1.5);
xlabel('Frequency (Hz)'); ylabel('Phase (deg)');
grid on; xlim([10 2000]);
title('BPF Phase Response');
