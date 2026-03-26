%% === 데이터 불러오기 ===
% Simulink의 To Workspace 블록 이름에 맞게 수정하세요.
input_sig  = out.input_sig;     % 입력 신호
output_sig = out.output_sig;    % 출력 신호
t          = out.tout;          % 시간 벡터

%% === 기본 파라미터 ===
fs = 1 / (t(2) - t(1));         % 샘플링 주파수
N  = length(t);                 
Nhalf = floor(N/2);

%% === FFT 계산 ===
IN  = fft(input_sig);
OUT = fft(output_sig);

IN  = IN(1:Nhalf);
OUT = OUT(1:Nhalf);

f = linspace(0, fs/2, Nhalf);   % 주파수 벡터

%% === Magnitude / Phase 계산 ===
mag   = abs(OUT) ./ abs(IN);           % 진폭비
phase = angle(OUT) - angle(IN);        % 위상 차이

%% === dB 스케일 Magnitude 및 Phase 플롯 ===
figure;

subplot(2,1,1);
semilogx(f, 20*log10(mag), 'LineWidth', 1.5);
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('All-Pass Filter Magnitude Response');
ylim([-1 1]); grid on;

subplot(2,1,2);
semilogx(f, phase*180/pi, 'LineWidth', 1.5);
xlabel('Frequency (Hz)');
ylabel('Phase (deg)');
title('All-Pass Filter Phase Response');
grid on;
% 기존 코드 이후에 추가
phase_unwrapped = unwrap(phase);                % 위상 연속화
phase_deg = phase_unwrapped * 180/pi;           % 도 단위 변환
phase_smooth = movmean(phase_deg, 50);          % 이동 평균으로 스무딩

figure;
semilogx(f, phase_smooth, 'LineWidth', 1.5);
xlabel('Frequency (Hz)');
ylabel('Phase (deg)');
title('All-Pass Filter Phase (Unwrapped & Smoothed)');
grid on;
