fs = 10000;
fc = 100;
Q  = 5;

omega0 = 2*pi*fc/fs;
alpha  = sin(omega0)/(2*Q);

b0 = 1;
b1 = -2*cos(omega0);
b2 = 1;
a0 = 1 + alpha;
a1 = -2*cos(omega0);
a2 = 1 - alpha;

b = [b0 b1 b2]/a0;
a = [1 a1/a0 a2/a0];

freqz(b,a,1024,fs)
[b a]

ans =

    0.9938   -1.9836    0.9938    1.0000   -1.9836    0.9875

simulink
경고: The model name 'untitled' is shadowing another name in the MATLAB workspace or path. Type "which -all
untitled" at the command line to find the other uses of this name. You should change the name of the model to avoid
problems. 
> sltemplate.internal.request.createSimulinkModel
connector.internal.fevalMatlab에서
connector.internal.fevalJSON에서 
input_sig  = out.input_sig;
output_sig = out.output_sig;
t          = out.tout;

fs = 1/(t(2)-t(1));
N  = length(t); Nhalf = floor(N/2);

IN  = fft(input_sig); OUT = fft(output_sig);
IN  = IN(1:Nhalf); OUT = OUT(1:Nhalf);
f = linspace(0, fs/2, Nhalf);

mag   = abs(OUT)./abs(IN);
phase = unwrap(angle(OUT)-angle(IN));

figure;
subplot(2,1,1);
semilogx(f,20*log10(mag),'LineWidth',1.5);
xlabel('Frequency (Hz)'); ylabel('Magnitude (dB)');
title('BSF Frequency Response (AC Sweep)');
grid on; xlim([10 2000]); ylim([-60 5]);

subplot(2,1,2);
semilogx(f,phase*180/pi,'LineWidth',1.5);
xlabel('Frequency (Hz)'); ylabel('Phase (deg)');
title('BSF Phase Response');
grid on; xlim([10 2000]);



