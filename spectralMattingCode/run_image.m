
load my_images
load scribbles
%%
% if this variable is set to 0, only the final alpha matting is saved.
% Otherwise, partial results (such as the matting components) are saved.
save_partial_results = 0;

% try to group the matting components for forground/background matting
do_grouping = 1;

% for all examples here we use 50 eigen vectors...
eigs_num=50;
%%

if (1)
    nclust =10;
    [alpha_comps,alpha] = SpectralMatting(image, [], 'image_', eigs_num, nclust, ...
        [], save_partial_results);
end