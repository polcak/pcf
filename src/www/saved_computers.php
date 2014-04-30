<!doctype html public "-//W3C//DTD HTML 4.0 //EN">

<html>
<head>
    <meta http-equiv="content-type" content="text/html; charset=UTF-8">
	<title>pcf</title>
	<script type="text/javascript" src="js.js"></script>
	<style type="text/css">
		@import "style.css";
	</style>
</head>

<body onload="loadCookies2()">

	
<?php 
$selected="saved";
include "header.php"; 
?>

<div id="content">
<div id="aktual">

<?php

function removeComputer($type)
{
	$database = getDatabaseName($type);
	
	$xmlDoc = new DOMDocument();
	$xmlDoc->load($database);
	$xpath = new DOMXpath($xmlDoc);
	foreach($xpath->query("//computer[@skew=\"" . $_POST["skew_$type"] . "\"]") as $computer) {
		$dom = dom_import_simplexml($computer);
		$s = simplexml_import_dom($dom);
		if ($s->name == $_POST["name"]) {
			$dom->parentNode->removeChild($dom);
		}
	}

	$xmlDoc->save($database);
}

if ($_POST["name"]) {
	removeComputer("tcp");
	removeComputer("javascript");
	removeComputer("icmp");
}

if (file_exists("data/database.xml")) {
	$computersTcp = simplexml_load_file("data/tcp/database.xml");
}
if (file_exists("data/javascript/database.xml")){
	$computersJavascript = simplexml_load_file("data/javascript/database.xml");
}
if (file_exists("data/icmp/database.xml")){
	$computersIcmp = simplexml_load_file("data/icmp/database.xml");
}


$mergedComputers = array();

foreach ($computersTcp->computer as $computer){
	addComputer($computer, "tcp");
}

foreach ($computersJavascript->computer as $computer){
	addComputer($computer, "javascript");
}

foreach ($computersIcmp->computer as $computer){
	addComputer($computer, "icmp");
}

echo "<span style='position: absolute; top: 120px; right: 10px'><a href='javascript:fold(200)'>[+/-]</a></span>";

$i = 200;
foreach ($mergedComputers as $computer) {
	echo "<b><font color='#0000b2'><a href=\"javascript:aktual('", $i, "')\">", $computer->name, "</font></a></b><br />";
	
	printComputer($computer, false);
	echo "</br>";
	
	echo "<form method='post' action='", $_SERVER['PHP_SELF'], "'>";
	echo "<input type='submit' value='Delete' />";
	echo "<input type='hidden' name='skew_tcp' value='", $computer->skews["tcp"], "' />";
	echo "<input type='hidden' name='skew_javascript' value='", $computer->skews["javascript"], "' />";
	echo "<input type='hidden' name='skew_icmp' value='", $computer->skews["icmp"], "' />";
	echo "<input type='hidden' name='name' value='", $computer->name, "' />";

	#echo "<script>aktual('", $i, "')</script>";
	$i = $i + 1;
	
	echo "</form>";
	echo "</br>";
}

?>

</div>
</div>

</body>
</html>
