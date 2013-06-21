<!doctype html public "-//W3C//DTD HTML 4.0 //EN">

<html>
<head>
	<title>pcf</title>
	<script type="text/javascript" src="js.js"></script>
	<style type="text/css">
		@import "style.css";
	</style>
</head>

<body onload="loadCookies1()">
	
<?php 
$selected="active";
include "header.php";
?>

<div id="content">
<div id="aktual">

<?php

error_reporting(E_ERROR | E_WARNING | E_PARSE);

function debugOutput($message)
{
	echo "<div style=\"color: red; background-color: yellow; \">$message</div>";
}

function saveComputer($type){
	$database = getDatabaseName($type);
	
	if (!file_exists($database)) {
		$xmlDoc = new DOMDocument();
		$root = $xmlDoc->appendChild($xmlDoc->createElement("computers"));
		$xmlDoc->formatOutput = true;
		$xmlDoc->save($database);
	}
	$computers = simplexml_load_file($database);
	$computer = $computers->addChild("computer");
	$computer->addAttribute("skew", $_POST["skew_$type"]);

	$computer->addChild("name", $_POST["name"]);

	$computer->addChild("address", $_POST["address_$type"]);

	$computer->addChild("frequency", $_POST["frequency_$type"]);

	$computer->addChild("date", $_POST["date_$type"]);

	$computers->asXML($database);
}

if (isset($_POST["name"])) {
	saveComputer("tcp");
	saveComputer("javascript");
	saveComputer("icmp");
}

if (file_exists("data/active.xml")) {
	$computersTcp = simplexml_load_file("data/active.xml");
}
if (file_exists("data/javascript/active.xml")){
	$computersJavascript = simplexml_load_file("data/javascript/active.xml");
}
if (file_exists("data/icmp/active.xml")){
	$computersIcmp = simplexml_load_file("data/icmp/active.xml");
}

function printComputers($computers){
	echo "<hr>";
	foreach ($computers->computer as $computer){
		echo "address: " . $computer->address . "<br/>";
	}
}

/*
echo printComputers($computersTcp);
echo printComputers($computersIcmp);
echo printComputers($computersJavascript);
*/

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

// echo serialize($mergedComputers);


echo "<span style='position: absolute; top: 120px; right: 10px'><a href='javascript:fold(100)'>[+/-]</a></span>";

$i = 100;
/*
foreach ($computers->computer as $computer) {
	if (isset($computer->identity)) {
		echo "<b><font color='green'><a href=\"javascript:aktual('", $i, "')\">", $computer->address, "</a></font></b><br />";
        	echo "<div id='", $i, "'>";
	        echo "<form method='post' action='", $_SERVER['PHP_SELF'], "'>";
	        echo "<table>";
	        echo "<tr><td width='40%'>Skew:</td><td>", $computer["skew"], "</td></tr>";
		echo "<tr><td>Packets:</td><td>", $computer->packets, "</td></tr>";
	        foreach ($computer->identity as $identity) {
	               echo "<tr><td>Similar skew:</td><td>", $identity->name, "</td></tr>";
	               echo "<tr><td>Diff:</td><td>", $identity->diff, "</td></tr>";
	        }
	        echo "<tr><td>Date:</td><td name='date'>", $computer->date, "</td></tr>";
	        echo "<tr><td>Name:</td><td><input type='text' name='name' size='31' /></td><td><input type='submit' value='Save computer'></td></tr>";

	        echo "<input type='hidden' name='skew' value='", $computer["skew"], "' />";
	        echo "<input type='hidden' name='address' value='", $computer->address, "' />";
	        echo "<input type='hidden' name='frequency' value='", $computer->frequency, "' />";
	        echo "<input type='hidden' name='date' value='", $computer->date, "' />";

        #echo "<script>aktual('", $i, "')</script>";
	        echo "</table>";
	        echo "</form>";

        	$i = $i + 1;
		echo "<font color='#0000b2'><a href=\"javascript:aktual('", $i, "')\">Show graph</a></font><br /><br />";
	        echo "<center><img id='", $i, "' src='graph/", $computer->address, ".svg' /></center>";

	        echo "</div>";

	        $i = $i + 1;
	}
}
*/

echo "<br />";


foreach ($mergedComputers as $computer) {
	if (!isset($computer->identity)) {
		echo "<b><font color='#0000b2'><a href=\"javascript:aktual('", $i, "')\">", $computer->address, "</a></font></b><br />";
		echo "<div id='", $i, "'>";
		
		printComputer($computer, true);
		echo "</br>";
	
		echo "<form method='post' action='", $_SERVER['PHP_SELF'], "'>";
		echo "<span style='margin-right: 20px;'>Name:</span><input type='text' name='name' size='45' /><input type='submit' value='Save computer'>";
	
		echo "<input type='hidden' name='skew_tcp' value='", $computer->skews["tcp"], "' />";
		echo "<input type='hidden' name='address_tcp' value='", $computer->address, "' />";
		echo "<input type='hidden' name='frequency_tcp' value='", $computer->frequencies["tcp"], "' />";
		echo "<input type='hidden' name='date_tcp' value='", $computer->dates["tcp"], "' />";
		
		echo "<input type='hidden' name='skew_javascript' value='", $computer->skews["javascript"], "' />";
		echo "<input type='hidden' name='address_javascript' value='", $computer->address, "' />";
		echo "<input type='hidden' name='frequency_javascript' value='", $computer->frequencies["javascript"], "' />";
		echo "<input type='hidden' name='date_javascript' value='", $computer->dates["javascript"], "' />";
		
		echo "<input type='hidden' name='skew_icmp' value='", $computer->skews["icmp"], "' />";
		echo "<input type='hidden' name='address_icmp' value='", $computer->address, "' />";
		echo "<input type='hidden' name='frequency_icmp' value='", $computer->frequencies["icmp"], "' />";
		echo "<input type='hidden' name='date_icmp' value='", $computer->dates["icmp"], "' />";

		#echo "<script>aktual('", $i, "')</script>";
		
		echo "</form>";
		echo "</br>";

		$i = $i + 1;
		echo "<font color='#0000b2'><a href=\"javascript:aktual('", $i, "')\">Show graph</a></font><br /><br />";
		echo "<center>";
		echo "<img id='", $i, "' src='graph/", $computer->address, ".svg' />";
		echo "<img id='", $i, "' src='graph/javascript/", $computer->address, ".svg' />";
		echo "<img id='", $i, "' src='graph/icmp/", $computer->address, ".svg' />";
		echo "</center>";

		echo "</div>";

		$i = $i + 1;
	}
}

?>

</div>
</div>

</body>
</html>
