clc;close all;clear all
%C:\Users\ragarci2\workspace_v5_4\SensorNodeGSE\UART_to_file\Debug\UART_to_file.exe
cd 'C:\Users\ragarci2\workspace_v5_4\SensorNodeGSE\MHZ_868';

vals = [];
FID = fopen('loggy.txt','r');
tline = fgetl(FID);

%       [current energy (0:1) duty cycle (2:3) cap_min (4:5) cap_max (6:7) cap_avg (8:9) 
%       current_val (10:11) current_min (12:13) current_max(14:15) wake_stat (16) pkt_cnt (17)
%       (blank bytes) RTC (27:30) chksum (31) RSSI (32)]

legend_arr = {'current energy' 'duty cycle' 'cap\_min'  'cap\_max' 'cap\_avg' ... 
                'current\_val' 'current\_min' 'current\_max' 'wake\_stat' 'pkt\_cnt' ...
                'RTC' 'chksum' 'RSSI' 'Data Rate'};
%my_hist_data = [];
my_hist_data = zeros(1024,1024);

set(figure(1),'color','w');
subplot(2,2,1)
legend(legend_arr{1:10}, legend_arr{12:end});
xlabel('Sample Number');
ylabel('16 bit value');
title('Telemetry');
subplot(2,2,3);
legend(legend_arr{6},legend_arr{1});
title('Current Value and Current Energy');
xlabel('Sample Number');
ylabel('16 bit value');
myimg = imagesc(my_hist_data);
axis xy;   %sets the coordinate system so that(0,0) is in the lower left

plot_ok = 0;
cnt_ok = 0;
pkt_cnt = 0;
drate = 0;
pkts_dropped = 0;
last_pk_cnt = 0;
tic;

while  (fgetl(FID) ~= -1)
    % scan to end of file
end

while(1)
%     while  (fgetl(FID) ~= -1)
%     % scan to end of file
%     end
%     pause(0.1 );

    tline = fgetl(FID);
    
    if (length(tline) == 1)
        if (plot_ok == 1)
            
            pkt_time = toc;
            tic;
            drate = round(pkt_cnt*32 / pkt_time);
            
            subplot(2,2,1);
            plot([1:size(vals,1)], vals(:,3:5), 'LineWidth', 2);
            legend(legend_arr{3:5});

            subplot(2,2,2);
            plot([1:size(vals,1)], vals(:,6:8), 'LineWidth', 2);
            legend(legend_arr{6:8});

            subplot(2,2,3);
            %plot(vals(:,6), vals(:,1), '.', 'LineWidth', 2);
            %hist(my_hist_data);
             %p = imagesc(my_hist_data);
            set(myimg,'CData',my_hist_data);
            [x y] = ind2sub(size(my_hist_data),find(my_hist_data));
            axes(myimg); %theoretically, I should be able to pass the handle into the axis function on the next line... but doesn't seem to be working for me
            axis([min(min(x)-10,1) max(max(x)+10,1024) min(min(y)-10,0) max(max(y)+10,1024));   %scale the image axis appropriately
            clear x y;
            %legend(legend_arr{6},legend_arr{1});
            %title('Current vs. Cap Voltage');
            colorbar();

            subplot(2,2,4);
            plot([1:size(vals,1)], [vals(:,2) vals(:,9:10) vals(:,12:14)], 'LineWidth', 2);
            legend(legend_arr{2}, legend_arr{9:10}, legend_arr{12:14});
            
            annotation('textbox',[0.9 .5 .011 .02], 'string', ['packets dropped ' num2str(pkts_dropped)], 'FontSize', 9, 'EdgeColor', [0 0 0], 'LineStyle','none');
            
        else
            plot_ok = 1;
        end
        pause(0.1);
        pkt_cnt = 0;
    elseif (length(tline) == 116)
        pkt_cnt = pkt_cnt + 1;
        cvals = tline(18:end);
        disp(cvals);

        newval = [hex2dec(cvals(1:find(cvals == ' ',1)))];
        cvals = cvals(find(cvals == ' ',1)+1:end);
        for i = 1:33
        %for i = 1:2
            newval = [newval hex2dec(cvals(1:find(cvals == ' ',1)))];
            cvals = cvals(find(cvals == ' ',1)+1:end);
        end
        formatted_values = [newval(1)*2^8+newval(2) ...       % current energy
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
        
        %my_hist_data = [my_hist_data; formatted_values(1) formatted_values(6)];
        if (formatted_values(1) < 1024 && formatted_values(6) < 1024 && formatted_values(1) ~= 0 && formatted_values(6) ~= 0)
            my_hist_data(formatted_values(1), formatted_values(6)) = my_hist_data(formatted_values(1), formatted_values(6)) + 1;
        end
        if (length(vals) < 100)
            vals = [vals; formatted_values drate];
        else
            vals = [vals(2:end,:);formatted_values drate];
        end
        
        pkt_wrap = formatted_values(10) - last_pk_cnt;
        if (last_pk_cnt > formatted_values(10))
            pkt_wrap = formatted_values(10) + 256 - last_pk_cnt;
        end
        if (cnt_ok == 1)
            pkts_dropped = pkts_dropped + pkt_wrap - 1;
        else
            cnt_ok = 1;
        end
        last_pk_cnt = formatted_values(10);
        
    end
end