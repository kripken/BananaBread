(function(){
  
  var SLIDE_DURATION_AFTER_START = 5000;
  var YOUTUBE_WIDTH = '711';
  var YOUTUBE_HEIGHT = '400';

  var __slideDuration = 8000;

  var __youtubeListeners = [];

  window.onYouTubeIframeAPIReady = function(){
    while(__youtubeListeners.length){
      __youtubeListeners.pop()();
    }
  };

  function onYoutubeReady(listener){
    if(window.YT && window.YT.player){
      listener();
    }
    else{
      __youtubeListeners.push(listener);
    }
  }

  function makeAnchor(element){
    element.addEventListener('click', function(){
      // set to null first to make browser jump here regardless of whether or not hash is already set
      window.location.hash = null;
      window.location.hash = element.getAttribute('data-anchor');
    }, false);
  }

  function setupGallery(){
    var interval = -1;
    var external;
    var slides = [];

    function Slide(element, left, counterNode){
      element.style.top = '0px';
      element.style.left = left + 'px';

      var _this = this;

      var video = element.querySelector('video');
      var youtubeDiv = element.querySelector('div[data-youtube]');
      var youtubePlayer;

      function onVideoPlay(){
        external.stop();
      }

      function onVideoPause(){
      }

      function onVideoEnded(){
        external.start();
      }

      function onYoutubePlayerStateChange(e){
        if(external.currentSlide === _this){
          if([YT.PlayerState.PLAYING, YT.PlayerState.BUFFERING].indexOf(e.data) > -1){
            external.stop();
          }
          else{
            external.start();
          }
        }
      }

      if(youtubeDiv){
        youtubeUrl = youtubeDiv.getAttribute('data-youtube');
        onYoutubeReady(function(){
          youtubeDiv.id = youtubeDiv.id || "slide-" + Date.now() + Math.random();
          youtubePlayer = new YT.Player(youtubeDiv.id, {
            videoId: youtubeUrl,
            width: YOUTUBE_WIDTH,
            height: YOUTUBE_HEIGHT,
            playerVars: {
              controls: 1,
              wmode: 'opaque'
            },
            events: {
              'onStateChange': onYoutubePlayerStateChange,
            }
          });
        });
      }

      _this.enter = function(skipSlideIn){
        counterNode.className = 'slide-counter-node on';
        if(video){
          if(video.readyState > 0){
            video.currentTime = 0;
          }
          if(youtubePlayer){
            youtubePlayer.seekTo(0, false);
          }
          video.setAttribute('disabled', true);
          video.addEventListener('play', onVideoPlay, false);
          video.addEventListener('ended', onVideoEnded, false);
          video.addEventListener('pause', onVideoPause, false);
        }
      };

      _this.exit = function(){
        counterNode.className = 'slide-counter-node';
        if(youtubePlayer){
          youtubePlayer.pauseVideo();
        }
        if(video){
          video.pause();
          video.setAttribute('disabled', true);
          video.removeEventListener('play', onVideoPlay, false);
          video.removeEventListener('ended', onVideoEnded, false);
          video.removeEventListener('pause', onVideoPause, false);
        }
      };
    }

    var gallery = document.getElementById('gallery');
    var slidesElements = gallery.querySelectorAll('.slide');
    var container = document.getElementById('slide-container');
    var counter = document.getElementById('slide-counter');
    var slideWidth = slidesElements[0].getBoundingClientRect().width;
    var slideIndex = 0;

    container.style.width = slideWidth * (slides.length + 1) + 'px';
    slidesElements = Array.prototype.slice.apply(slidesElements);
    slidesElements.forEach(function(slide, index){
      var counterNode = document.createElement('div');
      counterNode.className = 'slide-counter-node';
      slides.push(new Slide(slide, index * slideWidth, counterNode, generateCounterNodeClickFunction(index, counterNode)));
      counter.insertBefore(counterNode, counter.firstChild);
    });

    function generateCounterNodeClickFunction(index, counterNode){
      counterNode.addEventListener('click', function(e){
        external.stop();
        goToSlide(index);
        external.start();
      }, false);
    }

    function goToSlide(nextIndex){
      var oldSlideIndex = slideIndex;
      slides[oldSlideIndex].exit();
      external.currentSlide = slides[nextIndex];
      slides[nextIndex].enter();
      slideIndex = nextIndex;
      container.style.left = -nextIndex * slideWidth + 'px';
    }

    function nextGallerySlide(){
      __slideDuration = SLIDE_DURATION_AFTER_START;
      goToSlide((slideIndex + 1) % slides.length);
    }

    slides[0].enter();

    external = {
      currentSlide: slides[0],
      start: function(){
        if(interval === -1){
          interval = setInterval(nextGallerySlide, __slideDuration);
        }
      },
      stop: function(){
        if(interval > -1){
          clearInterval(interval);
          interval = -1;
        }
      }
    };

    return external;
  }

  document.addEventListener('DOMContentLoaded', function(e){
    var anchors = document.querySelectorAll('[data-anchor]');
    for(var i=0, l=anchors.length; i<l; ++i){
      makeAnchor(anchors[i]);
    }

    var gallery = setupGallery();
    gallery.start();
  }, false);

}());