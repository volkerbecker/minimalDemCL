function export_fig(fig,name);
    set(fig,'position',[0 0, 800 550]);  
    set(fig,'PaperPositionMode','Auto');
    epsname=[name '.eps'];
    saveas(fig,name,'epsc');
    befehl=['epstopdf ' epsname]; 
    system(befehl);
end