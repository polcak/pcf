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

<div id="head">
<font size="1"><br /></font>
<font color=0000b2 size="7" face="Verdana"><b><center><a href="index.php">pcf</a></center></b></font>
</div>

<div id="menu">
<ul>
<li><a class="selected" href="index.php">Active computers</a></li>
<li><a href="all_graphs.php">All graphs</a></li>
<li><a href="saved_computers.php">Saved computers</a></li>
</ul>
</div>

<div id="content">
<div id="aktual">

<?php

if (isset($_POST["name"])) {

	$database = "data/database.xml";

	if (!file_exists($database)) {
		$xmlDoc = new DOMDocument();
		$root = $xmlDoc->appendChild($xmlDoc->createElement("computers"));
		$xmlDoc->formatOutput = true;
		$xmlDoc->save($database);
	}
        
	$computers = simplexml_load_file($database);

	$computer = $computers->addChild("computer");
	$computer->addAttribute("skew", $_POST["skew"]);

	$computer->addChild("name", $_POST["name"]);

	$computer->addChild("address", $_POST["address"]);

	$computer->addChild("frequency", $_POST["frequency"]);

	$computer->addChild("date", $_POST["date"]);

	$computers->asXML($database);
}


$active = "data/active.xml";

if (file_exists($active)) {
	$computers = simplexml_load_file($active);
}
else {
	#exit("Failed to open $filename");
	exit();
}

echo "<span style='position: absolute; top: 120px; right: 10px'><a href='javascript:fold(100)'>[+/-]</a></span>";

$i = 100;
foreach ($computers->computer as $computer) {
	if ($computer->name != "") {
		echo "<b><font color='green'><a href=\"javascript:aktual('", $i, "')\">", $computer->address, " (", $computer->name, ")</a></font></b><br />";
        	echo "<div id='", $i, "'>";
	        echo "<form method='post' action='", $SERVER['PHP_SELF'], "'>";
	        echo "<table>";
	        if ($computer->name != "") {
        	        echo "<tr><td>Diff:</td><td>", $computer->diff, "</td></tr>";
	        }
	        echo "<tr><td width='40%'>Skew:</td><td>", $computer["skew"], "</td></tr>";
		echo "<tr><td>Packets:</td><td>", $computer->packets, "</td></tr>";
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
	        echo "<center><img id='", $i, "' src='graph/", $computer->address, ".jpg' /></center>";

	        echo "</div>";

	        $i = $i + 1;
	}
}

echo "<br />";

foreach ($computers->computer as $computer) {
	if ($computer->name == "") {
		echo "<b><font color='#0000b2'><a href=\"javascript:aktual('", $i, "')\">", $computer->address, "</a></font></b><br />";
		echo "<div id='", $i, "'>";
		echo "<form method='post' action='", $SERVER['PHP_SELF'], "'>";
		echo "<table>";
		if ($computer->name != "") {
			echo "<tr><td>Diff:</td><td>", $computer->diff, "</td></tr>";
		}
		echo "<tr><td width='40%'>Skew:</td><td>", $computer["skew"], "</td></tr>";
		echo "<tr><td>Packets:</td><td>", $computer->packets, "</td></tr>";
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
		echo "<center><img id='", $i, "' src='graph/", $computer->address, ".jpg' /></center>";

		echo "</div>";

		$i = $i + 1;
	}
}

?>

</div>
</div>

</body>
</html>
