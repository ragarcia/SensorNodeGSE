clc;close all;clear all
%C:\Users\ragarci2\workspace_v5_4\SensorNodeGSE\UART_to_file\Debug\UART_to_file.exe
cd 'C:\Users\ragarci2\workspace_v5_4\SensorNodeGSE\MHZ_868';

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

%       [current energy (0:1) duty cycle (2:3) cap_min (4:5) cap_max (6:7) cap_avg (8:9) 
%       current_val (10:11) current_min (12:13) current_max(14:15) wake_stat (16) pkt_cnt (17)
%       (blank bytes) RTC (27:30) chksum (31)]

        newval = [hex2dec(cvals(1:find(cvals == ' ',1)))];
        cvals = cvals(find(cvals == ' ',1)+1:end);
        for i = 1:33
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
                    newval(11)*2^8+newval(12) ...    % current_val
                    newval(13)*2^8+newval(14) ...    % current_min
                    newval(15)*2^8+newval(16) ...    % current_max
                    newval(17) ...    % wake_state
                    newval(18) ...    % pkt_cnt
                    newval(28)*2^24+newval(29)*2^16+newval(30)*2^8+newval(31) ...   %RTC
                    newval(32) ...    % chksum
                    newval(end)]; % RSSI
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
            plot([1:size(vals,1)], [vals(:,1:10) vals(:,12:end)]);
            %plot([1:size(vals,1)], [vals(:,:)]);
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
            plot([1:size(vals,1)], [vals(:,1:10) vals(:,12:end)]);
            %plot([1:size(vals,1)], [vals(:,:)]);
            %set(H1,'LineWidth',2);
            %set(H2,'LineWidth',2);
            legend('current energy', 'duty cycle', 'cap min','cap max', 'cap avg', 'CRC')
        end
        %keyboard
        %pause(0.01)
    end
end