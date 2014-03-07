function c = pointInPolygon(nVert, xVert, yVert, xTest, yTest) 

c = 0;
j = nVert - 1;

for i = 1:nVert
    
    if ((yVert(i) > yTest) ~= (yVert(j) > yTest)) && ...
            (xTest < (xVert(j)-xVert(i)) * (yTest-yVert(i)) / (yVert(j)-yVert(i)) + xVert(i))
        c = ~c;
    end
    
    j = i+1;
    i = i+1;
end