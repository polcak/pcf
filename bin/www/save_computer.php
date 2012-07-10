<?php

$filename = "data/database.xml";

if (file_exists($filename)) {
        $computers = simplexml_load_file($filename);
}
else {
        #exit("Failed to open $filename");
        exit();
}

$sxe = new SimpleXMLElement($xml);
$sxe->addAttribute("skew", "2222");

$computer = $sxe->addChild("computer");

$computer->addChild("name", $_GET["name"]);

$computer->addChild("address");

$computer->addChild("frequency");

$computer->addChild("date");

?>
