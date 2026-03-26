input_sig  = out.input_sig;   % or workspace variable directly
output_sig = out.output_sig;
t          = out.tout;

fs = 1 / (t(2) - t(1));     % 샘플링 주파수
N  = length(t);             % 샘플 개수

% FFT 계산
IN  = fft(input_sig);
OUT = fft(output_sig);

% 양수 주파수 절반만 사용
IN  = IN(1:N/2);
OUT = OUT(1:N/2);

f = linspace(0, fs/2, N/2); % 주파수 벡터
H = abs(OUT) ./ abs(IN);    % 주파수 응답 (이득)
경고: 인덱스로 사용 시, 콜론 연산자에는 정수형 피연산자가 필요합니다. 
 
경고: 인덱스로 사용 시, 콜론 연산자에는 정수형 피연산자가 필요합니다. 
 

figure;
semilogx(f, 20*log10(H), 'LineWidth', 1.5);
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('AC Sweep (FFT-based Frequency Response)');
grid on;
xlim([10 1000]);  % Chirp 대역에 맞게 조정
ylim([-60 10]);


%% 


input_sig  = out.input_sig;   % or workspace variable directly
output_sig = out.output_sig;
t          = out.tout;

fs = 1 / (t(2) - t(1));     % 샘플링 주파수
N  = length(t);             % 샘플 개수

% FFT 계산
IN  = fft(input_sig);
OUT = fft(output_sig);

% 양수 주파수 절반만 사용
IN  = IN(1:N/2);
OUT = OUT(1:N/2);

f = linspace(0, fs/2, N/2); % 주파수 벡터
H = abs(OUT) ./ abs(IN);    % 주파수 응답 (이득)


figure;
semilogx(f, 20*log10(H), 'LineWidth', 1.5);
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('AC Sweep (LPF)');
grid on;
xlim([10 1000]);  % Chirp 대역에 맞게 조정
ylim([-30 10]);





