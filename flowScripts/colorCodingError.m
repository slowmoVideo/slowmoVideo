function err = colorCodingError(orig)

mx = max(abs(orig));
mx = max(0, min(255, mx));
for i=1:length(mx)
    if mx(i) == 0
        mx(i) = 1
    end
end

b = mx/255
r = max(0, min(1, orig(1,:)./mx/2 + .5))
g = max(0, min(1, orig(2,:)./mx/2 + .5))

b32 = round(255*b)
r32 = round(255*r)
g32 = round(255*g)

recovered = zeros(size(orig));
recovered(1,:) = 2*b32 .* ((r32/255) - .5);
recovered(2,:) = 2*b32 .* ((g32/255) - .5);

recovered
orig

err = orig - recovered

errPercent =  round(100*norm(err, "columns")./norm(orig, "columns"))

end
