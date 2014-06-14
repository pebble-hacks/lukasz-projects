var fetchtime = 5;
var feeds = [];
var lastheadline;
var lastheadlniepubdate;

var allfeeds = [
 ["Top", "http://sports.yahoo.com/top/rss.xml", "#checkbox-top"],
 ["MLB", "http://sports.yahoo.com/mlb/rss.xml", "#checkbox-mlb"],
 ["NFL", "http://sports.yahoo.com/nfl/rss.xml", "#checkbox-nfl"],
 ["NBA", "http://sports.yahoo.com/nba/rss.xml", "#checkbox-nba"],
 ["NHL", "http://sports.yahoo.com/nhl/rss.xml", "#checkbox-nhl"],
 ["NASCAR", "http://sports.yahoo.com/nascar/rss.xml", "#checkbox-nascar"],
 ["Golf", "http://sports.yahoo.com/golf/rss.xml", "#checkbox-golf"],
 ["MMA", "http://sports.yahoo.com/mma/rss.xml", "#checkbox-mma"],
 ["Boxing", "http://sports.yahoo.com/box/rss.xml", "#checkbox-box"],
 ["NCAAB", "http://sports.yahoo.com/ncaab/rss.xml", "#checkbox-ncaab"],
 ["NCAAW", "http://sports.yahoo.com/ncaaw/rss.xml", "#checkbox-ncaaw"],
 ["Tennis", "http://sports.yahoo.com/ten/rss.xml", "#checkbox-tennis"],
 ["NCAABB", "http://sports.yahoo.com/ncaabb/rss.xml", "#checkbox-ncaabb"],
 ["WNBA", "http://sports.yahoo.com/wnba/rss.xml", "#checkbox-wnba"],
 ["NCAAF", "http://sports.yahoo.com/ncaaf/rss.xml", "#checkbox-ncaaf"],
 ["Olympics", "http://sports.yahoo.com/oly/rss.xml", "#checkbox-olympics"],
 ["IRL", "http://sports.yahoo.com/irl/rss.xml", "#checkbox-irl"],
 ["Soccer", "http://sports.yahoo.com/sow/rss.xml", "#checkbox-soccer"],
 ["MLS", "http://sports.yahoo.com/mls/rss.xml", "#checkbox-mls"],
 ["Skiing", "http://sports.yahoo.com/ski/rss.xml", "#checkbox-ski"],
 ["Cycling", "http://sports.yahoo.com/sc/rss.xml", "#checkbox-cycling"],
 ["Horse Racing", "http://sports.yahoo.com/rah/rss.xml", "#checkbox-hr"]
 ];

function headlineDownloaded(headline, pubdate) {

  if(lastheadlniepubdate>pubdate){
    return;
  }

  if(headline.length>125){
    headline = headline.substring(0,125);
  }
  
  if(headline===lastheadline){
    return;
  }
  
  console.log('Got new headline: ' + headline);
  sendHeadline(headline, pubdate, 3);
}

function sendHeadline(headline, pubdate, num){

  if(lastheadlniepubdate>pubdate){
    //second check in case we've just sent another headline in meantime
    console.log("Newer headline sent, retreating");
    return;
  }
  
  Pebble.sendAppMessage({
    "headline":headline
  },
  function(e) {
    console.log("Successfully delivered headline");
    lastheadline=headline;
    lastheadlniepubdate = pubdate;
  },
  function(e) {
    if(num>0){
      console.log("Unable to deliver headline, retrying");
      sendHeadline(headline, pubdate, num-1);
    }else{
      console.log("Unable to deliver headline, out of tries");
    }
  });
}

function headlineError() {
  console.warn('Headline error');
  /*
  Pebble.sendAppMessage({
    "headline":"Feed error"
  });
  */
}

function downloadHeadlines(){
  console.log("Downloading headlines...");
  for(var i = 0; i < feeds.length; i++){
    downloadHeadline(feeds[i]);
  }
}

function downloadHeadline(source){
  console.log('Downloading RSS from ' + source);
  var response,
      req = new XMLHttpRequest();
  
  req.open('GET', "http://ajax.googleapis.com/ajax/services/feed/load?v=2.0&q=" + source, true);
  req.onload = function(e) {
    if (req.readyState === 4) {
      if(req.status === 200) {
        response = JSON.parse(req.responseText);
        
        var headline, category, pubdate = 0;
        for(var i=0; i<response.responseData.feed.entries.length; i++){
          try{
            var newpubdate = Date.parse(response.responseData.feed.entries[i].publishedDate);
            var newheadline = response.responseData.feed.entries[i].title;
            var newcategory = response.responseData.feed.entries[i].categories[0];
          
            if(!newpubdate||newpubdate==="undefined"||!newheadline||newheadline==="undefined"){
              //we skip this
            }else if(pubdate<newpubdate){
              headline = newheadline;
              category = newcategory;
              pubdate = newpubdate;
            }
          }catch(A){}
        }
        
        if(!category||category=="undefined"){
          for(var i=0; i<allfeeds.length; i++){
            if(allfeeds[i][1]===source){
              category = allfeeds[i][0];
              break;
            }
          }
        }
        if(!newpubdate||newpubdate==""||newpubdate=="undefined"){
          //we skip this
          //console.log("Pebble JS: Bad headline for category = " + category + "  headline = " + headline + " pubdate = " + pubdate);
        }else{
          //console.log("Pebble JS: category = " + category + "  headline = " + headline + " pubdate = " + pubdate);
          headlineDownloaded("[" + category + "] " + headline, pubdate);
        }
      } else {
        headlineError();
      }
    } else {
      headlineError();
    }
  }
  req.onerror = function(e) {
    console.log("Error downloading RSS: " + req.status);
    headlineError();
  }
  req.send(null);
}

function saveFeed(name, active){
  localStorage.setItem(name, active);
}

function loadFeeds(){
  console.log("reading feeds...");
  feeds = [];
  for(var i=0; i<allfeeds.length; i++){
    if(localStorage.getItem(allfeeds[i][0])==="true"){
      feeds[feeds.length] = allfeeds[i][1];
    }
  }
}

function getSavedFeeds(){
  savedFeeds = [];
  for(var i=0; i<allfeeds.length; i++){
    if(localStorage.getItem(allfeeds[i][0])==="true"){
      savedFeeds[savedFeeds.length] = allfeeds[i][0];
    }
  }
  
  if(savedFeeds.length==0)savedFeeds[0]=allfeeds[0][0];
  return savedFeeds;
}

Pebble.addEventListener("ready",
                        function(e) {
                          loadFeeds();
                          fetchtime = localStorage.getItem("sportsfetchtime");
                          
                          if (!feeds||feeds==""||feeds.length==0) {
                            feeds.push(allfeeds[0][1]);
                          }
                          if (!fetchtime||fetchtime=="undefined"||fetchtime<=0||fetchtime>180) {
                            fetchtime = 5; //minutes
                          }

                          console.log("Fetch time: " + fetchtime);
                          
                          downloadHeadlines();	

                          window.setInterval(function(){
                            downloadHeadlines();	
                          }, 60000*fetchtime);
                          
                          console.log("ready and up.");
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
                          console.log("got message!");
                        });

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration with feeds: " + getSavedFeeds());
  Pebble.openURL(encodeURI("http://rawgit.com/pebble-hacks/lukasz-projects/master/sports/resources/website/configuration.html?&settings=" + getSavedFeeds() + "&fetchtime=" + fetchtime));
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("Pebble JS: configuration closed");
  var configuration = JSON.parse(decodeURIComponent(e.response));

  feeds = [];
  for(var i=0; i<allfeeds.length; i++){
    if(configuration[allfeeds[i][0]]===true){
      feeds[feeds.length] = allfeeds[i][1];
      saveFeed(allfeeds[i][0], true);
    }else{
      saveFeed(allfeeds[i][0], false);
    }
  }
  
  if(feeds.length==0)feeds.push(allfeeds[0][1]);

  fetchtime = configuration["fetchtime"];
  localStorage.setItem("sportsfetchtime", fetchtime);
  console.log("new configuration is: " + getSavedFeeds() + " every " + fetchtime + " minutes");
  
  downloadHeadlines();
  lastheadlniepubdate = 0;
});
