NCLCPU=[10 100 1000 10000 100000 1000000];
anzahl=length(NCLCPU);
tCLCPU=zeros(1,anzahl);
LX=2010;LY=2010;
for i=1:anzahl
    startstring=sprintf('./minimalDemCL %i %f %f',NCLCPU(i),LX,LY);
    tic;
    system(startstring);
    tCLCPU(i)=toc;
end