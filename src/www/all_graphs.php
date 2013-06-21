<!doctype html public "-//W3C//DTD HTML 4.0 //EN">
<?php 
$type = $_GET["type"];
if($type == ""){
  $type = "tcp";
}
?>

<html>
<head>
	<title>pcf</title>
	<style type="text/css">
		@import "style.css";
	</style>
</head>

<body>

<?php 
$selected="graphs";
include "header.php"; 
?>

<div id="content">

<center>
<?php
	if($type == "tcp"){
		$inputDirectory = "";
	}
	else{
		$inputDirectory = $type . "/";
	}
	$directory = "graph/" . $inputDirectory;
	$files = array_diff(scandir($directory), array('..', '.','.gitignore', 'javascript', 'icmp'));
	foreach ($files as $file) {
		echo "<img src='graph/" . $inputDirectory . $file . "' alt='$graph'></img>";
	}
?>
</center>

</body>
</html>
