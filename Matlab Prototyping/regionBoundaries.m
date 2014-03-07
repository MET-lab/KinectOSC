clear all; close all; clc;

aspectRatio = 16/9;
viewWidth = 1024;
viewHeight = viewWidth / aspectRatio;

xx = 0:viewWidth;   % x-axis

% Tops of note regions
upperBound = viewHeight/2 * ones(size(xx));

figure(), hold on;
plot(xx, upperBound);
xlim([0 viewWidth]);
ylim([0 viewHeight]);

% Note regions
nRegions = 12;
regionWidth = viewWidth / nRegions;

% Angles of region boundary lines
theta = 90 - linspace(pi/4, 3*pi/4, nRegions+1);

for i = 1:length(theta) / 2
    
    % Coordinates of upper point on region boundary line
    x(i) = upperBound(1) * cos(theta(i));
    y(i) = upperBound(1) * sin(theta(i));
    m(i) = y / x;  % Boundary line slope
    
    x0 = (i-1)*regionWidth;
    regionBoundX = x0:x0 + x(i);
    
    regionBoundY = m(i) * regionBoundX;     % y = mx + b
    
    plot(regionBoundX, regionBoundY);
%     pause(1);
end

hold off;







