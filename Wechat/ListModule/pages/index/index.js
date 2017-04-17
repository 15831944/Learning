//index.js
//获取应用实例
var app = getApp()
Page({
  data: {
    items: []
  },
  
  onShow: function() {
    var objs = [];
    for (var i = 0; i < 20; i++)
    {
      objs.push({
        index: i+1
      });
    }
    this.setData({items: objs});
  }


})
