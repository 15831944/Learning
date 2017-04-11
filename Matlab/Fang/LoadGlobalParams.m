global rf
global delta
global time
global periods
global delta_time


rf = 0.03514; % risk free rate
delta = 0.45; % volatility
time = 1; % Unit : year
periods = 10;
delta_time = time/periods; % period
initAssetValue = 6.89;
MAPrice = 6.8;
simutimes = 100;