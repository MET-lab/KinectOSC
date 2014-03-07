clear all; close all; clc;

noteMap_(1) = 52;
noteMap_(2) = 55;
noteMap_(3) = 57;
noteMap_(4) = 59;
noteMap_(5) = 62;
noteMap_(6) = 64;
noteMap_(7) = 67;
noteMap_(8) = 69;
noteMap_(9) = 71;
noteMap_(10) = 74;
noteMap_(11) = 76;
noteMap_(12) = 79;

key = 'E';
octave = 4;
scale = 'Chromatic';
scale = 'Diatonic';
scale = 'Pentatonic';
tonality = 'Major';
tonality = 'Minor';

% % Major W W H W W W H W W H W W
% major = [2 2 1 2 2 2 1 2 2 1 2];
% % Minor W H W W H W W W H W W H
% minor = [2 1 2 2 1 2 2 2 1 2 2];
% % MajPt x 
% maPen = [2 2 3 2 3 2 2 3 2 3 2];
% miPen = [3 2 2 3 2 3 2 2 3 2 3];

major = [2 4 5  7  9 11 12 14 16 17 19];
minor = [2 3 5  7  8 10 12 14 15 17 19];
maPen = [2 4 7  9 12 14 16 19 21 24 26];
miPen = [3 5 7 10 12 15 17 19 22 24 27];

if strcmpi(key, 'C');
        base = 0;
    elseif strcmpi(key, 'C#');
        base = 1;
    elseif strcmpi(key, 'D');
        base = 2;
    elseif strcmpi(key, 'D#');
        base = 3;
    elseif strcmpi(key, 'E');
        base = 4;
    elseif strcmpi(key, 'F');
        base = 5;
    elseif strcmpi(key, 'F#');
        base = 6;
    elseif strcmpi(key, 'G');
        base = 7;
    elseif strcmpi(key, 'G#');
        base = 8;
    elseif strcmpi(key, 'A');
        base = 9;
    elseif strcmpi(key, 'A#');
        base = 10;
    elseif strcmpi(key, 'B');
        base = 11;
end

base = base + octave*12;
    
map = base*ones(12, 1);
for i = 1:11
   
    % If scale = pentatonic and tonality = minor
    map(i+1) = base + miPen(i);   
end






