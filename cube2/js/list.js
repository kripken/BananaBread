(function(){

var levels = [
  {
    internalName: 'six',
    externalName: 'Colony',
    type: 'multiplayer',
    url: 'game.html?setup=six&preload=six&debug&serve'
  },
  {
    internalName: 'seven',
    externalName: 'Bunker',
    type: 'multiplayer',
    url: 'game.html?setup=seven&preload=seven&debug&serve'
  },
  {
    internalName: 'eight',
    externalName: 'Hanger',
    type: 'multiplayer',
    url: 'game.html?setup=eight&preload=eight&debug&serve'
  },
  {
    internalName: 'nine',
    externalName: 'Ship',
    type: 'multiplayer',
    url: 'game.html?setup=nine&preload=nine&debug&serve'
  },
  {
    internalName: 'ten',
    externalName: 'Ruins',
    type: 'multiplayer',
    url: 'game.html?setup=ten&preload=ten&debug&serve'
  },
  {
    internalName: 'one',
    externalName: 'Arena',
    type: 'singleplayer',
    url: 'game.html?setup=low&preload=low&debug'
  },
  {
    internalName: 'two',
    externalName: 'Two Towers',
    type: 'singleplayer',
    url: 'game.html?setup=medium&preload=medium&debug'
  },
  {
    internalName: 'three',
    externalName: 'Lava Chamber',
    type: 'singleplayer',
    url: 'game.html?setup=high&preload=high&debug'
  },
  {
    internalName: 'four',
    externalName: 'Future',
    type: 'singleplayer',
    url: 'game.html?setup=four&preload=four&debug'
  },
  {
    internalName: 'five',
    externalName: 'Lava Rooms',
    type: 'singleplayer',
    url: 'game.html?setup=five&preload=five&debug'
  }
];

var brokerUrl = 'http://wrtcb.jit.su:80';

if (window.location.search) {
  var params = window.location.search.substring(1).split('&');
  for (var i = 0; i < params.length; ++ i) {
    if (params[i].match('^webrtc-broker')) {
      brokerUrl = params[i].split('=')[1];
    }
  }
}

document.querySelector('#broker-url').innerHTML = 'Broker: ' + brokerUrl;

var hosts = {};
var filter = {
  'metadata': {
    'name': '^BananaBread$'
  }
};

var listElement = document.querySelector('#join-list');
var listItemElement = listElement.querySelector('li');
listElement.removeChild(listItemElement);

var socket = io.connect(brokerUrl + '/list');
socket.on('connect', function() {
  socket.emit('list', filter);
});
socket.on('error', function(error) {
  console.error(error);
});
socket.on('truncate', function(list) {
  clear();
  append(list);
});
socket.on('append', function(host) {
  append([host]);
});
socket.on('update', function(host) {
  update([host]);
});
socket.on('remove', function(route) {
  remove([route]);
});

function setQuery(url, item) {
  var urlParts = url.split('?');
  if(urlParts.length < 2) {
    urlParts[1] = item;
  } else {
    var query = urlParts[1].split('&');
    query.push(item);
    urlParts[1] = query.join('&');
  }
  return urlParts.join('?');
};

function clear() {
  hosts = {};
  while(listElement.firstChild){
    listElement.removeChild(listElement.firstChild);
  }
}

var createGameList = document.querySelector('#create-list');
var createGameTemplate = createGameList.querySelector('li');

createGameList.removeChild(createGameTemplate);

levels.forEach(function(level){
  var li = createGameTemplate.cloneNode(true);
  li.querySelector('.game-name').innerHTML = level.externalName;
  li.querySelector('.game-type').innerHTML = level.type.substr(0, 1).toUpperCase() + level.type.substr(1);
  li.querySelector('.game-image').classList.add(level.internalName);
  li.querySelector('a').href = level.url;
  createGameList.appendChild(li);
});

function createListItemFromHost(host, element){
  var newListItem = element || listItemElement.cloneNode(true);
  var date = new Date(host.ctime);

  newListItem.querySelector('a').href = setQuery(host['url'], 'webrtc-session=' + host['route']);
  newListItem.querySelector('.game-time').innerHTML = date.toLocaleString();
  newListItem.querySelector('.game-users').innerHTML = host['metadata']['connected']
    + ' Player' + (host['metadata']['connected'] !== 1 ? 's' : '');
  newListItem.querySelector('.game-name').innerHTML = host['metadata']['name'] || '';
  
  var level = host.url.match(/setup=(\w+)[&$]/);

  if (level) {
    newListItem.querySelector('.game-image').classList.add(level[1]);
  }

  return newListItem;
}

function append(list) {
  list = list || [];
  list.forEach(function(host) {
    hosts[host['route']] = host;
    var newListItem = createListItemFromHost(host);
    host.element = newListItem;
    listElement.appendChild(newListItem);
  });
}

function update(list) {
  list = list || [];
  list.forEach(function(host) {
    if(hosts[host['route']]) {
      createListItemFromHost(host, hosts[host['route']].element);
    }
  });
};

function remove(list) {
  list = list || [];
  list.forEach(function(route) {
    var host = hosts[route];
    var element = host.element;
    element.parentNode.removeChild(element);
    delete hosts[route];
  });
};

}());