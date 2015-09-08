function [t traj] = trajektory(basename,numbermax,pnummer)
%UNTITLED4 Summary of this function goes here
%   Detailed explanation goes here
  for number=1:numbermax 
      filename=sprintf('%s%04d',basename,number);
      data=load(filename,'-ascii');
      traj(number,:)=data(pnummer,:);
      t(number)=number;
end

