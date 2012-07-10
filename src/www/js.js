function fold(num) {
  var list = document.getElementById("aktual");
  var forms = list.getElementsByTagName("form");
  var imgs = list.getElementsByTagName("img");
  var element = document.getElementById(num);

  if (element.style.display == "none") {
    for (var i = 0; i < (forms.length + imgs.length); i++) {
      element = document.getElementById(i+num);
      element.style.display = "block";
      saveCookies(i+num, "block");
    }
  }
  else {
    for (var i = 0; i < (forms.length + imgs.length); i++) {
      element = document.getElementById(i+num);
      element.style.display = "none";
      saveCookies(i+num, "none");
    }
  }
}

function aktual(elem) {
  var element = document.getElementById(elem);

  if (element.style.display == "none") {
    element.style.display = "block";
    saveCookies(elem, "block");
  }
  else {
    element.style.display = "none";
    saveCookies(elem, "none");
  }
}

function saveCookies(n, v) {
  time = new Date();
  time.setTime(time.getTime() + 1000 * 3600 * 24);
  document.cookie = n+"="+v+"; EXPIRES="+time.toGMTString();
}

function loadCookies1() {
  var cookieList = document.cookie.split("; ");
  var cookieArray = new Array();

  for (var i = 0; i < cookieList.length; i++) {
    var n = cookieList[i].split("=");
    cookieArray[unescape(n[0])] = unescape(n[1]);
  }

  for (var i = 0; i < cookieList.length; i++) {
    document.getElementById(i+100).style.display = cookieArray[i+100];
  }
}

function loadCookies2() {
  var cookieList = document.cookie.split("; ");
  var cookieArray = new Array();

  for (var i = 0; i < cookieList.length; i++) {
    var n = cookieList[i].split("=");
    cookieArray[unescape(n[0])] = unescape(n[1]);
  }

  for (var i = 0; i < cookieList.length; i++) {
    document.getElementById(i+200).style.display = cookieArray[i+200];
  }
}
