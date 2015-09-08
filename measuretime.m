N=[10 500 1000 2000 4000 8000 12000 ...
   14000 20000 40000 50000 ...
   60000 100000 200000 300000 600000 1000000]
anzahl=length(N);
tCLGPU3=zeros(1,anzahl);
LX=2010;LY=2010;
for i=1:anzahl
    startstring=sprintf('./minimalDemCL %i %f %f',N(i),LX,LY);
    tic;
    system(startstring);
    tCLGPU3(i)=toc;
end
benchdata=[N' tCLGPU3'];