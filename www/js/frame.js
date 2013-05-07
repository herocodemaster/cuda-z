
var currentId = "undefined";
var menuPrefix = "menu-";
var blockPrefix = "block-";

function blockShow(id) {

	if(id == currentId)
		return;

	if(currentId != "undefined") 
		oldMenu = menuPrefix + currentId;
	newMenu = menuPrefix + id;

	if(currentId != "undefined") 
		document.getElementById(oldMenu).className = menuPrefix + "inactive";
	document.getElementById(newMenu).className = menuPrefix + "active";

	newBlock = blockPrefix + id;
	if(currentId != "undefined") 
		oldBlock = blockPrefix + currentId;

/*	if(currentId != "undefined") 
		document.getElementById(oldBlock).className = blockPrefix + "inactive";
	document.getElementById(newBlock).className = blockPrefix + "active";*/

	$("#block").scrollTo($('#' + newBlock), 600);

	currentId = id;
}

function blockDetectOsAndShow() {

	var id = "windows";

/*	alert("Platform is \"" + navigator.platform + "\"");*/

	if(navigator.platform.indexOf("Win") != -1) {
		id = "windows";
	} else if(navigator.platform.indexOf("Linux") != -1) {
		id = "linux";
	} else if(navigator.platform.indexOf("Mac") != -1) {
		id = "macosx";
	} else if(navigator.platform.indexOf("iPhone") != -1) {
		id = "macosx";
	} else if(navigator.platform.indexOf("iPod") != -1) {
		id = "macosx";
	} else if(navigator.platform.indexOf("iPad") != -1) {
		id = "macosx";
	}

	blockShow(id);
}

function blockOnLoad() {

	var id = "about";
	var url = parent.document.URL;

	if(url.indexOf('#') != -1) {

		var endUrl = url.length;
		if(url.indexOf('?') != -1) {
			endUrl = url.indexOf('?');
		}

		var urlAnchor = url.substring(url.indexOf('#'), endUrl);
		blockFullPrefix = '#' + blockPrefix;
		if(urlAnchor.indexOf(blockFullPrefix) == 0) {
			id = urlAnchor.substring(blockFullPrefix.length, urlAnchor.length);
		}
	}

	blockShow(id);
}
