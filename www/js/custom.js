
$(document).ready(function() {
	$("a[rel=group_win32]").fancybox({
		'hideOnContentClick'	: false
	});

	$("a[rel=group_linux]").fancybox({
		'hideOnContentClick'	: false
	});

	$("a[rel=group_macosx]").fancybox({
		'hideOnContentClick'	: false
	});

	blockOnLoad();
}); 
