function [ s_vector ] = MonteCarloSimulation(s0)

global rf
global delta
global delta_time
global periods
%% Set initial parameters
s = s0;
s_vector = zeros(periods+1,1); % Create a container to record the intermediate value
s_vector(1) = s; % the first value in the container is the initial value
%% Start Monte Carlo simulations
for i = 1:periods
    delta_s = s*(rf*delta_time+delta*randn(1)*sqrt(delta_time));
    s = s + delta_s; % calcuate the asset value in the next period
    s_vector(i+1) = s; % record the asset value
end

end

