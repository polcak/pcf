<!doctype html public "-//W3C//DTD HTML 4.0 //EN">

<html>
<head>
    <meta http-equiv="content-type" content="text/html; charset=UTF-8">
	<title>pcf</title>
	<style type="text/css">
		@import "style.css";
	</style>
</head>

<body>

<div id="head">
<font size="1"><br /></font>
<font color=0000b2 size="7" face="Verdana"><b><center><a href="index.php">pcf</a></center></b></font>
</div>

<div id="menu">
<ul>
<li><a href="index.php">Active computers</a></li>
<li><a class="selected" href="all_graphs.php">All graphs</a></li>
<li><a href="saved_computers.php">Saved computers</a></li>
</ul>
</div>

<div id="content">

<center>
<?php
	$directory = "graph/";
	$graphs = array_diff(scandir($directory), array('..', '.','.gitignore'));
	foreach ($graphs as $graph) {
		echo "<img src='graph/$graph' alt='$graph'></img>";
	}
?>
</center>

</body>
</html>
