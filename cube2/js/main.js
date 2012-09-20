var SearchArgs = {};
(function() {
  var searchArgs = window.location.search.split("&");
  for (var i = 0; i < searchArgs.length; ++i) {
    var eqsplit = searchArgs[i].split("=");
    if (eqsplit.length == 1) {
      SearchArgs[eqsplit[0]] = true;
    } else {
      SearchArgs[eqsplit[0]] = eqsplit[1];
    }
  }
})();

(function(){
  
  var SLIDE_DURATION = 5000;

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

      return {
        enter: function(skipSlideIn){
          counterNode.className = 'slide-counter-node on';
        },
        exit: function(){
          counterNode.className = 'slide-counter-node';
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
      slides[nextIndex].enter();
      slideIndex = nextIndex;
      container.style.left = -nextIndex * slideWidth + 'px';
    }

    function nextGallerySlide(){
      goToSlide((slideIndex + 1) % slides.length);
    }

    slides[0].enter();

    external = {
      start: function(){
        interval = setInterval(nextGallerySlide, SLIDE_DURATION);
      },
      stop: function(){
        clearInterval(interval);
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

  // Load additional gallery images after page finished initial loading
  var imageIds = ['26', '07', '05', '09', '10', '11', '06', '12', '03', '13', '14', '15', '16'];
  var imageCounter = 0;
  window.onload = function() {
    function loadOne() {
      document.getElementById('late-image-' + imageCounter).src = 'assets/screenshots/' + imageIds[imageCounter] + '.jpg';
      imageCounter++;
      if (imageCounter < imageIds.length) {
        setTimeout(loadOne, 200);
      }
    }
    setTimeout(loadOne, 200);
  };
}());

