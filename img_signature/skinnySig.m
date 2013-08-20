function outMap = skinnySig( img, param )

img = imresize(img, param.mapWidth/size(img, 2));
cSalMap = zeros(size(img));


for i = 1:3
	cSalMap(:,:,i) = idct2(sign(dct2(img(:,:,i)))).^2;
end

outMap = sum(cSalMap, 3);
outMap = mynorm(outMap, param);

end

