audioFile = 'xgrzyb00.wav';

% ----------------- 1. ------------------ %
audioinfo(audioFile);

% ----------------- 2. ------------------ %
[samples, Fs] = audioread(audioFile); %Samples - array of samples
samplesNumber = length(samples); %samplesNumber - number of samples
vectorAid = (0:(samplesNumber/2 - 1)/samplesNumber * Fs); %Vector from 0 to 7999
fftSamples = abs(fft(samples)); %Fourier transform of samples in abs value
fftSpan = 10 * log10(1/samplesNumber * fftSamples.^2); 
fftSpan = fftSpan(1:samplesNumber/2); 
plot(vectorAid, fftSpan);
xlabel('Frekvencia [Hz]');
ylabel('PSD [dB]');
grid;

% ----------------- 3. ------------------ %
[maxFreq,indexFreq] = max(fftSpan); %indexFreq - at what frequency the max is located
indexFreq = indexFreq - 1; %signal starts at 0, max's index at 1
% ----------------- 4. ------------------ %
b = [0.2324 -0.4112 0.2324]; %numerator coefficients
a = [1 0.2289 0.4662]; %denominator coefficients
zplane(b,a); %defined in in terms of z^(-1)
grid;
stability = roots(a); %computes roots of a polynom
if (isempty(stability) | abs(stability) < 1)
	disp('The filter is stable');
else
	disp('The filter is not stable');
end;

% ----------------- 5. ------------------ %
CFR = freqz(b,a,Fs/2); % b and a from the 4th
plot(vectorAid, abs(CFR));
xlabel('Frekvencia [Hz]'); 
ylabel ('|H(f)|');
grid;

% ----------------- 6. ------------------ %
filteredSamples = filter(b, a, samples); % b and a from the 4th
filteredFftSamples = abs(fft(filteredSamples));
filteredFftSpan = 10 * log10(1/samplesNumber * filteredFftSamples.^2); 
filteredFftSpan = filteredFftSpan(1:samplesNumber/2);
plot(vectorAid, filteredFftSpan);
xlabel('Frekvencia [Hz]');
ylabel('PSD [dB]');
grid;

% ----------------- 7. ------------------ %
[filteredMaxFreq,filteredIndexFreq] = max(filteredFftSpan);
filteredIndexFreq = filteredIndexFreq - 1; %signal starts at 0, max's index at 1
% ----------------- 8. ------------------ %

	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 
% ----------------- 9. ------------------ %
estimate = xcorr(samples,'biased');
estimateShorten = estimate((samplesNumber - 50) : (samplesNumber + 50));
k = -50:50;
plot(k, estimateShorten);
xlabel('k');
ylabel('R[k]');
grid;

% ----------------- 10. ----------------- %
r10 = estimate(samplesNumber+10);

% ----------------- 11. ----------------- %
%courtesy of 5_random.pdf
samplesMin = min(samples);
samplesMax = max(samples);
span = 50; %could have been anything else, 5_random pdf suggests 50. At 10 the hist resembles lego bricks, though.
vectorAid = linspace(samplesMin, samplesMax, span);

%courtesy of hist2opt
vectorAid = vectorAid(:);
histogram = zeros(span, span);
vectorAidMatrix = repmat(vectorAid, 1, samplesNumber);
samplesMatrix = repmat(samples, 1, span)';
[dummy, index1] = min(abs(samplesMatrix - vectorAidMatrix));

index2 = index1(11 : end); %shifting by 10

for ii = 1 : samplesNumber - 10,
	d1 = index1(ii);
	d2 = index2(ii);
	histogram(d1, d2) = histogram(d1, d2) + 1;
end

surface = ((vectorAid(2) - vectorAid(1)) ^ 2);
probEstimate = histogram / samplesNumber / surface;

bar3(probEstimate);

% ----------------- 12. ----------------- %
check = sum(sum(probEstimate)) * surface;
%disp(['2D integral should be 1 and is ' num2str(check)]);

% ----------------- 13. ----------------- %
X1 = repmat(vectorAid, 1, span);
vectorAid = vectorAid';
X2 = repmat(vectorAid, span, 1);
coeff = sum(sum(X1 .* X2 .* probEstimate)) * surface;
