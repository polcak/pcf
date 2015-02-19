<?php

function getDatabaseName($type)
{
	$directories = array(
		"tcp" => "tcp/",
		"javascript" => "javascript/",
		"icmp" => "icmp/",
	);
	
	return "data/" . $directories[$type] . "database.xml";
}

class MergedComputerInfo{
	function __construct($address){
		$this->address = $address;
	}
	public $address;
	public $name;
	public $skews = array();
	public $dates = array();
	public $packets = array();
	public $frequencies = array();
}

function addComputer($computer, $type){
	global $mergedComputers;
	
	$addressString = (string) $computer->ip;
	if(!array_key_exists($addressString, $mergedComputers)){
		$mergedComputers[$addressString] = new MergedComputerInfo($addressString);
		$mergedComputers[$addressString]->address = $addressString;
		$mergedComputers[$addressString]->name = (string)$computer->name;
	}
	
	$mergedComputers[$addressString]->skews[$type] = (string)$computer["skew"];
	$mergedComputers[$addressString]->dates[$type] = (string)$computer->date;
	$mergedComputers[$addressString]->packets[$type] = (string)$computer->packets;
	$mergedComputers[$addressString]->frequencies[$type] = (string)$computer->frequency;
}

function printComputer($computer, $showPackets)
{
	echo "<table border='1'>";
	echo "<tr><th>&nbsp;</th><th>TCP</th><th>Javascript</th><th>ICMP</th></tr>";
	echo "<tr><td width='150px'>Skew:</td><td width='250px'>" . $computer->skews["tcp"] . "</td><td width='250px'>" . $computer->skews["javascript"] . "</td><td width='250px'>" . $computer->skews["icmp"] . "</td></tr>";
	if($showPackets)
	{
		echo "<tr><td>Packets:</td><td>" . $computer->packets["tcp"] . "</td><td>" . $computer->packets["javascript"] . "</td><td>" . $computer->packets["icmp"] . "</td></tr>";
	}
	echo "<tr><td>Date:</td><td>" . $computer->dates["tcp"] . "</td><td>" . $computer->dates["javascript"] . "</td><td>" . $computer->dates["icmp"] . "</td></tr>";
	echo "</table>";
}

?>

<div id="head">
<font size="1"><br /></font>
<font color=0000b2 size="7" face="Verdana"><b><center><a href="index.php">pcf</a></center></b></font>
</div>

<div id="menu">
<ul>
<li><a <?php if($selected=="active") echo "class=\"selected\"";?> href="index.php">Active computers</a></li>
<li><a <?php if($selected=="graphs") echo "class=\"selected\"";?> href="all_graphs.php">All graphs</a>
<ul class="submenu">
<li><a class="submenu <?php if($selected=="graphs" && $type=="tcp") echo "selected";?>" href="all_graphs.php?type=tcp">TCP</a></li>
<li><a class="submenu <?php if($selected=="graphs"&& $type=="icmp") echo "selected";?>" href="all_graphs.php?type=icmp">ICMP</a></li>
<li><a class="submenu <?php if($selected=="graphs"&& $type=="javascript") echo "selected";?>" href="all_graphs.php?type=javascript">Javascript</a></li>
</ul>
</li>
<li><a <?php if($selected=="saved") echo "class=\"selected\"";?> href="saved_computers.php">Saved computers</a></li>
</ul>
</div>
