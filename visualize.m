function [film] = visualize(basename,numbermax,LX,LY)
%UNTITLED3 Summary of this function goes here
%   Detailed explanation goes here
    xx=-1:0.1:1;
    y1x=sqrt(1-xx.^2);
    y2x=-sqrt(1-xx.^2);
    figure(1);clf;hold all;
    xlim([0,LX]);ylim([0,LY]);hold all;
    axis equal;
for number=0:numbermax  
    clf;
    xlim([0,LX]);ylim([0,LY])
    set(gca, 'XTick', []);
    set(gca, 'YTick', []);
    filename=sprintf('%s%04d',basename,number);
    data=load(filename,'-ascii');
    [m n]=size(data);
    radius=1;
    hold on;
    %positions=[data(:,1)-radius,data(:,2)-...
    %    radius,ones(m,1)*2*radius,ones(m,1)*2*radius];
    for i=1:m
        plot(data(i,1)+xx,data(i,2)+y1x);
        plot(data(i,1)+xx,data(i,2)+y2x);
    end
    drawnow;
    film(number+1)=getframe();
end

