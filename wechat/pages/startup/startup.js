//index.js
//获取应用实例
var app = getApp()
Page({
    data: {
        motto: 'Hello Startup',
        defaultSize : 'default'
    },
    click: function() {
        console.log('123456')
    },
    onLoad : function() {
        console.log('I am loading')
    },
    onReady : function() {
        console.log('I am ready')
    },
    onShow : function() {
        console.log('I am showing')
    }
})
