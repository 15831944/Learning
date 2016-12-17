close all;
clear;clc;
%% Load parameters
LoadGlobalParams;
%% Create results container
assetValueRecords = zeros(simutimes,periods+1);
c_container = zeros(simutimes,1);
c0_container = zeros(simutimes,1);
%% Start Monte Carlo Simulations
for i = 1:simutimes
    assetValueRecords(i,:) = MonteCarloSimulation(initAssetValue);
    c_container(i) = max(assetValueRecords(i,end)-MAPrice,0);
    c0_container(i) = c_container(i)*exp(-rf*time);
end
%% Calculate the results
c_mean = mean(c_container);
c0_mean = mean(c0_container);
figure
hist(c0_container(c0_container>0),100);
figure
plot(1:periods+1,assetValueRecords);