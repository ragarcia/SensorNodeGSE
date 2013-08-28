clc;close all;clear all
cd 'C:\Users\ragarci2\workspace_v5_4\Fixed_LT_FIFO_plus\MHZ_868';

vals = [];
FID = fopen('loggy.txt','r');
tline = fgetl(FID);

% while  (fgetl(FID) ~= -1)
%     % scan to end of file
% end

while(1)
    while  (fgetl(FID) ~= -1)
    % scan to end of file
    end
    pause(0.1 );

    tline = fgetl(FID);
    
    if (tline == -1)
        pause(0.1)
    else
        cvals = tline(18:end);
        disp(cvals);

%       [current energy (2 bytes) duty cycle (2 bytes) cap_min (2 bytes) cap_max (2 bytes) cap_avg (2 bytes) RTC (4 bytes)]

        newval = [hex2dec(cvals(1:find(cvals == ' ',1)))];
        cvals = cvals(find(cvals == ' ',1)+1:end);
        for i = 1:15
        %for i = 1:2
            newval = [newval hex2dec(cvals(1:find(cvals == ' ',1)))];
            cvals = cvals(find(cvals == ' ',1)+1:end);
        end
%         csum = bitxor(newval(1:end-1));
        newval = [newval(1)*2^8+newval(2) ...       % current energy
                    newval(3)*2^8+newval(4) ...     % duty cycle
                    newval(5)*2^8+newval(6) ...     % cap_min
                    newval(7)*2^8+newval(8) ...     % cap_max
                    newval(9)*2^8+newval(10) ...    % cap_avg
                    ...%newval(11)*2^24+newval(12)*2^16+newval(13)*2^8+newval(14) ...   %RTC
                    newval(end)];
        %newval = [newval(1)*2^8+newval(2)];
%         if (csum == newval(end))
%                 vals = [vals; newval(1:end-1)];
%         end
        if (length(vals) < 100)
            vals = [vals; newval];
        else
            vals = [vals(2:end,:);newval];
        end
        set(figure(1),'color','w');

        if (length(vals) < 100)
            %subplot(2,1,2)
            %[AX,H1,H2] = plotyy([1:size(vals,1)], [vals(:,1:5) vals(:,7)], [1:size(vals,1)], vals(:,6));
            plot([1:size(vals,1)], [vals(:,1:5) vals(:,6)]);
            %ylim([0 2^30]);
            %set(AX(1),'xlim',[1 100]);
            %set(AX(2),'xlim',[1 100]);
            %set(AX,'xlim',[1 100]);
            %set(H1,'LineWidth',2);
            %set(H2,'LineWidth',2);
            legend('current energy', 'duty cycle', 'cap min','cap max', 'cap avg', 'CRC')
        else
            %subplot(2,1,1)
            %plot(vals,'LineWidth',3);ylim([0 2^10])
            %subplot(2,1,2)
            %plot(vals(end-99:end,:),'LineWidth',3);ylim([0 2^30]);
            %[AX,H1,H2] = plotyy([1:size(vals,1)], [vals(:,1:5) vals(:,7)], [1:size(vals,1)], vals(:,6));
            plot([1:size(vals,1)], [vals(:,1:5) vals(:,6)])
            %set(H1,'LineWidth',2);
            %set(H2,'LineWidth',2);
            legend('current energy', 'duty cycle', 'cap min','cap max', 'cap avg', 'CRC')
        end
        %keyboard
        %pause(0.01)
    end
end