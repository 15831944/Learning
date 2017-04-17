var col = [
  {name:'czz',old:1},
  {name:'cbb',old:2},
  {name:'rtt',old:3}];
// pages/index/index.js
Page({
  data:{
    obj: [1,2,3,4,5]
  },
  test: function() {
    var query = this.Retrieve('cbb');
    query.old = 10;
    console.log(col);
  },

  Retrieve: function(name) {
    for (var index = 0; index < col.length; index++) {
      if (col[index].name == name) {
        return col[index];
      }
    }
    return null; 
  },

  onLoad:function(options){
    // 页面初始化 options为页面跳转所带来的参数
  },
  onReady:function(){
    // 页面渲染完成
  },
  onShow:function(){
    // 页面显示
  },
  onHide:function(){
    // 页面隐藏
  },
  onUnload:function(){
    // 页面关闭
  }
})