var tempFilePath,savedFilePath;

Page({
  onReady: function (e) {
    // 使用 wx.createAudioContext 获取 audio 上下文 context
    this.audioCtx = wx.createAudioContext('myAudio')
  },
  data: {  },
  audioPlay: function () {
    wx.playBackgroundAudio({
        dataUrl: 'http://ws.stream.qqmusic.qq.com/M500001VfvsJ21xFqb.mp3?guid=ffffffff82def4af4b12b3cd9337d5e7&uin=346897220&vkey=6292F51E1E384E06DCBDC9AB7C49FD713D632D313AC4858BACB8DDD29067D3C601481D36E62053BF8DFEAF74C0A5CCFADD6471160CAF3E6A&fromtag=46'
    });
  },
  audioPause: function () {
    wx.pauseBackgroundAudio();
  },

  startRecord: function() {
    wx.startRecord({
      success: function(res) {
        tempFilePath = res.tempFilePath;
        wx.saveFile({
          tempFilePath: tempFilePath,
          success: function(res) {
            savedFilePath = res.savedFilePath
          }
        })
      }
    })
  },
  stopRecord: function() {
    wx.stopRecord();
  },

  voicePlay: function() {
    wx.playVoice({ filePath: tempFilePath })
  },
  voicePause: function() {
    wx.pauseVoice();
  },
  voiceStop: function() {
    wx.stopVoice();
  }
})